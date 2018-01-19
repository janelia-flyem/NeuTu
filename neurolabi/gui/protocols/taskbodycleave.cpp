#include "taskbodycleave.h"

#include "dvid/zdvidreader.h"
#include "dvid/zdvidtarget.h"
#include "dvid/zdvidwriter.h"
#include "flyem/zflyembody3ddoc.h"
#include "neu3window.h"
#include "z3dmeshfilter.h"
#include "z3dwindow.h"

#include <iostream>

#include <QCheckBox>
#include <QComboBox>
#include <QHBoxLayout>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QLabel>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QPixmap>
#include <QPushButton>
#include <QShortcut>
#include <QSlider>
#include <QUndoCommand>
#include <QUrl>
#include <QVBoxLayout>

namespace {

  static const QString KEY_TASKTYPE = "task type";
  static const QString VALUE_TASKTYPE = "body cleave";
  static const QString KEY_BODYID = "body ID";
  static const QString KEY_MAXLEVEL = "maximum level";

  static const std::vector<glm::vec4> INDEX_COLORS({
    glm::vec4(255, 255, 255, 255) / 255.0f,
    glm::vec4( 88, 121, 163, 255) / 255.0f,
    glm::vec4(227, 146,  68, 255) / 255.0f,
    glm::vec4(207,  96,  94, 255) / 255.0f,
    glm::vec4(134, 181, 177, 255) / 255.0f,
    glm::vec4(108, 158,  88, 255) / 255.0f,
    glm::vec4(231, 200,  96, 255) / 255.0f,
    glm::vec4(166, 125, 159, 255) / 255.0f,
    glm::vec4(240, 162, 168, 255) / 255.0f,
    glm::vec4(150, 118,  99, 255) / 255.0f
  });

  Z3DMeshFilter *getMeshFilter(ZStackDoc *doc)
  {
    if (Z3DWindow *window = doc->getParent3DWindow()) {
      if (Z3DMeshFilter *filter =
          dynamic_cast<Z3DMeshFilter*>(window->getMeshFilter())) {
          return filter;
       }
    }
    return nullptr;
  }

  std::string getOutputInstanceName(const ZDvidTarget &dvidTarget)
  {
    return dvidTarget.getBodyLabelName() + "_cleaved";
  }

  // All the TaskBodyCleave instances loaded from one JSON file need certain changes
  // to some settings until all of them are done.  This code manages making those
  // changes and restore the changed values when the tasks are done.

  static bool applyOverallSettingsNeeded = true;
  static bool zoomToLoadedBodyEnabled;
  static bool garbageLifetimeLimitEnabled;
  static bool preservingSourceColorEnabled;

  void applyOverallSettings(ZFlyEmBody3dDoc* bodyDoc)
  {
    if (applyOverallSettingsNeeded) {
      applyOverallSettingsNeeded = false;

      zoomToLoadedBodyEnabled = Neu3Window::zoomToLoadedBodyEnabled();

      garbageLifetimeLimitEnabled = bodyDoc->garbageLifetimeLimitEnabled();
      bodyDoc->enableGarbageLifetimeLimit(false);

      if (Z3DMeshFilter *filter = getMeshFilter(bodyDoc)) {
        preservingSourceColorEnabled = filter->preservingSourceColorsEnabled();
        filter->enablePreservingSourceColors(true);
      }
    }
  }

  void restoreOverallSettings(ZFlyEmBody3dDoc* bodyDoc)
  {
    if (!applyOverallSettingsNeeded) {
      applyOverallSettingsNeeded = true;

      Neu3Window::enableZoomToLoadedBody(zoomToLoadedBodyEnabled);

      bodyDoc->enableGarbageLifetimeLimit(garbageLifetimeLimitEnabled);

      if (Z3DMeshFilter *filter = getMeshFilter(bodyDoc)) {
        filter->enablePreservingSourceColors(preservingSourceColorEnabled);
      }
    }
  }

}

//

class TaskBodyCleave::SetCleaveIndicesCommand : public QUndoCommand
{
public:
  SetCleaveIndicesCommand(TaskBodyCleave *task, std::map<uint64_t, std::size_t> meshIdToCleaveIndex) :
    m_task(task),
    m_meshIdToCleaveIndexBefore(task->m_meshIdToCleaveIndex),
    m_meshIdToCleaveIndexAfter(meshIdToCleaveIndex)
  {
    setText("seeding for next cleave");
  }

  virtual void undo() override
  {
    m_task->m_meshIdToCleaveIndex = m_meshIdToCleaveIndexBefore;
    m_task->updateColors();
  }

  virtual void redo() override
  {
    m_task->m_meshIdToCleaveIndex = m_meshIdToCleaveIndexAfter;
    m_task->updateColors();
  }

private:
  TaskBodyCleave *m_task;
  std::map<uint64_t, std::size_t> m_meshIdToCleaveIndexBefore;
  std::map<uint64_t, std::size_t> m_meshIdToCleaveIndexAfter;

};

class TaskBodyCleave::CleaveCommand : public QUndoCommand
{
public:
  CleaveCommand(TaskBodyCleave *task, std::map<uint64_t, std::size_t> meshIdToCleaveIndex) :
    m_task(task),
    m_meshIdToCleaveResultIndexBefore(task->m_meshIdToCleaveResultIndex),
    m_meshIdToCleaveResultIndexAfter(meshIdToCleaveIndex)
  {
    setText("cleave");
  }

  virtual void undo() override
  {
    m_task->m_meshIdToCleaveResultIndex = m_meshIdToCleaveResultIndexBefore;
    m_task->updateColors();
  }

  virtual void redo() override
  {
    m_task->m_meshIdToCleaveResultIndex = m_meshIdToCleaveResultIndexAfter;
    m_task->updateColors();
  }

private:
  TaskBodyCleave *m_task;
  std::map<uint64_t, std::size_t> m_meshIdToCleaveResultIndexBefore;
  std::map<uint64_t, std::size_t> m_meshIdToCleaveResultIndexAfter;

};

//

TaskBodyCleave::TaskBodyCleave(QJsonObject json, ZFlyEmBody3dDoc* bodyDoc)
{
  m_bodyDoc = bodyDoc;

  applyOverallSettings(bodyDoc);

  loadJson(json);
  buildTaskWidget();

  m_networkManager = new QNetworkAccessManager(m_widget);
  connect(m_networkManager, SIGNAL(finished(QNetworkReply*)),
          this, SLOT(onNetworkReplyFinished(QNetworkReply*)));
}

QString TaskBodyCleave::tasktype()
{
  return VALUE_TASKTYPE;
}

QString TaskBodyCleave::actionString()
{
  return "Body history:";
}

QString TaskBodyCleave::targetString()
{
  return QString::number(m_bodyId);
}

void TaskBodyCleave::beforeNext()
{
  applyPerTaskSettings();
}

void TaskBodyCleave::beforePrev()
{
  applyPerTaskSettings();
}

void TaskBodyCleave::beforeDone()
{
  restoreOverallSettings(m_bodyDoc);
}

QWidget *TaskBodyCleave::getTaskWidget()
{
  applyPerTaskSettings();
  return m_widget;
}

void TaskBodyCleave::updateLevel(int level)
{
  bool showingCleaving = m_showCleavingCheckBox->isChecked();
  enableCleavingUI(showingCleaving && (level == 0));

  // See the comment in applyPerTaskSettings().

  Neu3Window::enableZoomToLoadedBody(false);

  QSet<uint64_t> visible({ ZFlyEmBody3dDoc::encode(m_bodyId, level) });
  updateBodies(visible, QSet<uint64_t>());
}

void TaskBodyCleave::onShowCleavingChanged(bool show)
{
  if (show) {
    // Cleaving works on super voxels, which are what are displayed at level 0.
    m_levelSlider->setValue(0);
  }

  enableCleavingUI(show);
  applyColorMode(show);
}

void TaskBodyCleave::onChosenCleaveIndexChanged()
{
  if (QShortcut* shortcut = dynamic_cast<QShortcut*>(QObject::sender())) {
    int i = shortcut->key().toString().toInt();
    m_cleaveIndexComboBox->setCurrentIndex(i - 1);
  }
}

void TaskBodyCleave::onAddToChosenCleaveBody()
{
  const TStackObjectSet &selectedMeshes = m_bodyDoc->getSelected(ZStackObject::TYPE_MESH);
  std::map<uint64_t, size_t> meshIdToCleaveIndex(m_meshIdToCleaveIndex);

  for (auto it = selectedMeshes.cbegin(); it != selectedMeshes.cend(); it++) {
    ZMesh *mesh = static_cast<ZMesh*>(*it);
    meshIdToCleaveIndex[mesh->getLabel()] = chosenCleaveIndex();
  }

  m_bodyDoc->pushUndoCommand(new SetCleaveIndicesCommand(this, meshIdToCleaveIndex));
}

void TaskBodyCleave::onRemoveFromChosenCleaveBody()
{
  const TStackObjectSet &selectedMeshes = m_bodyDoc->getSelected(ZStackObject::TYPE_MESH);
  std::map<uint64_t, size_t> meshIdToCleaveIndex(m_meshIdToCleaveIndex);

  for (auto it = selectedMeshes.cbegin(); it != selectedMeshes.cend(); it++) {
    ZMesh *mesh = static_cast<ZMesh*>(*it);
    meshIdToCleaveIndex.erase(mesh->getLabel());
  }

  m_bodyDoc->pushUndoCommand(new SetCleaveIndicesCommand(this, meshIdToCleaveIndex));
}

void TaskBodyCleave::onToggleInChosenCleaveBody()
{
  const TStackObjectSet &selectedMeshes = m_bodyDoc->getSelected(ZStackObject::TYPE_MESH);
  std::map<uint64_t, size_t> meshIdToCleaveIndex(m_meshIdToCleaveIndex);

  for (auto itSelected = selectedMeshes.cbegin(); itSelected != selectedMeshes.cend(); itSelected++) {
    ZMesh *mesh = static_cast<ZMesh*>(*itSelected);
    uint64_t id = mesh->getLabel();
    auto itCleave = meshIdToCleaveIndex.find(id);
    if ((itCleave == meshIdToCleaveIndex.end()) || (itCleave->second != chosenCleaveIndex())) {
      meshIdToCleaveIndex[id] = chosenCleaveIndex();
    } else {
      meshIdToCleaveIndex.erase(id);
    }
  }

  m_bodyDoc->pushUndoCommand(new SetCleaveIndicesCommand(this, meshIdToCleaveIndex));
}

void TaskBodyCleave::onCleave()
{
  QJsonObject requestJson;
  requestJson["body-id"] = qint64(m_bodyId);

  std::map<unsigned int, std::vector<uint64_t>> cleaveIndexToMeshIds;
  for (auto it1 : m_meshIdToCleaveIndex) {
    unsigned int cleaveIndex = it1.second;
    auto it2 = cleaveIndexToMeshIds.find(cleaveIndex);
    if (it2 == cleaveIndexToMeshIds.end()) {
      cleaveIndexToMeshIds[cleaveIndex] = std::vector<uint64_t>();
    }
    uint64_t id = it1.first;
    cleaveIndexToMeshIds[cleaveIndex].push_back(id);
  }

  QJsonObject requestJsonSeeds;

  for (auto it : cleaveIndexToMeshIds) {
    QJsonArray requestJsonSeedsForCleaveIndex;
    for (uint64_t id : it.second) {
      requestJsonSeedsForCleaveIndex.append(QJsonValue(qint64(id)));
    }
    int cleaveIndex = it.first;
    requestJsonSeeds[QString::number(cleaveIndex)] = requestJsonSeedsForCleaveIndex;
  }

  requestJson["seeds"] = requestJsonSeeds;

  // TODO: Switch to a better server host.
  QUrl url("http://bergs-ws1.int.janelia.org:5555/compute-cleave");

  QNetworkRequest request(url);
  request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");

  QJsonDocument requestJsonDoc(requestJson);
  QByteArray requestData(requestJsonDoc.toJson());

  m_networkManager->post(request, requestData);
}

void TaskBodyCleave::onNetworkReplyFinished(QNetworkReply *reply)
{
  QNetworkReply::NetworkError error = reply->error();
  std::cerr << "** received cleave reply **\n";
  if (error == QNetworkReply::NoError) {
    QByteArray replyBytes = reply->readAll();

    QJsonDocument replyJsonDoc = QJsonDocument::fromJson(replyBytes);
    if (replyJsonDoc.isObject()) {
      QJsonObject replyJson = replyJsonDoc.object();
      QJsonValue replyJsonAssnVal = replyJson["assignments"];
      if (replyJsonAssnVal.isObject()) {
        std::map<uint64_t, std::size_t> meshIdToCleaveIndex;

        QJsonObject replyJsonAssn = replyJsonAssnVal.toObject();
        for (QString key : replyJsonAssn.keys()) {
          uint64_t cleaveIndex = key.toInt();
          QJsonValue value = replyJsonAssn[key];
          if (value.isArray()) {
            for (QJsonValue idVal : value.toArray()) {
              uint64_t id = idVal.toInt();
              meshIdToCleaveIndex[id] = cleaveIndex;
            }
          }
        }

        m_bodyDoc->pushUndoCommand(new CleaveCommand(this, meshIdToCleaveIndex));
      }
    }

  } else {
    LERROR() << "TaskBodyCleave::onNetworkReplyFinished() error: \""
             << reply->errorString().toStdString() << "\"";
  }

  reply->deleteLater();
}

QJsonObject TaskBodyCleave::addToJson(QJsonObject taskJson)
{
  return taskJson;
}

void TaskBodyCleave::onCompleted()
{
  ZDvidReader reader;
  reader.setVerbose(false);
  if (!reader.open(m_bodyDoc->getDvidTarget())) {
    LERROR() << "TaskBodyCleave::onCompleted() could not open DVID target for reading";
    return;
  }

  ZDvidWriter writer;
  if (!writer.open(m_bodyDoc->getDvidTarget())) {
    LERROR() << "TaskBodyCleave::onCompleted() could not open DVID target for writing";
    return;
  }

  std::string instance = getOutputInstanceName(m_bodyDoc->getDvidTarget());
  if (!reader.hasData(instance)) {
    writer.createKeyvalue(instance);
  }
  if (!reader.hasData(instance)) {
    LERROR() << "TaskBodyCleave::onCompleted() could not create DVID instance \"" << instance << "\"";
    return;
  }

  std::map<unsigned int, std::vector<uint64_t>> cleaveIndexToMeshIds;
  std::map<uint64_t, std::size_t> meshIdToCleaveIndex(m_meshIdToCleaveResultIndex);
  for (auto it : m_meshIdToCleaveIndex) {
    meshIdToCleaveIndex[it.first] = it.second;
  }
  for (auto itMesh : meshIdToCleaveIndex) {
    unsigned int cleaveIndex = itMesh.second;
    auto itCleave = cleaveIndexToMeshIds.find(cleaveIndex);
    if (itCleave == cleaveIndexToMeshIds.end()) {
      cleaveIndexToMeshIds[cleaveIndex] = std::vector<uint64_t>();
    }
    uint64_t id = itMesh.first;
    cleaveIndexToMeshIds[cleaveIndex].push_back(id);
  }

  // The output is JSON, an array of arrays, where each inner array is the super voxels in a cleaved body.

  QJsonArray json;
  for (auto itCleave : cleaveIndexToMeshIds) {
    QJsonArray jsonForCleaveIndex;

    // Sort the super voxel IDs, since one of the main uses of this output is inspection by a human.

    std::vector<uint64_t> ids(itCleave.second);
    std::sort(ids.begin(), ids.end());

    for (auto itMesh : ids) {
      jsonForCleaveIndex.append(QJsonValue(qint64(itMesh)));
    }
    json.append(jsonForCleaveIndex);
  }

  QJsonDocument jsonDoc(json);
  std::string jsonStr(jsonDoc.toJson(QJsonDocument::Compact).toStdString());
  std::string key(std::to_string(m_bodyId));
  writer.writeJsonString(instance, key, jsonStr);
}

std::size_t TaskBodyCleave::chosenCleaveIndex() const
{
  return m_cleaveIndexComboBox->currentIndex() + 1;
}


void TaskBodyCleave::buildTaskWidget()
{
  m_widget = new QWidget();

  QLabel *sliderLabel = new QLabel("History level", m_widget);
  m_levelSlider = new QSlider(Qt::Horizontal, m_widget);
  m_levelSlider->setMaximum(m_maxLevel);
  m_levelSlider->setTickInterval(1);
  m_levelSlider->setTickPosition(QSlider::TicksBothSides);

  connect(m_levelSlider, SIGNAL(valueChanged(int)), this, SLOT(updateLevel(int)));

  QHBoxLayout *sliderLayout = new QHBoxLayout;
  sliderLayout->addWidget(sliderLabel);
  sliderLayout->addWidget(m_levelSlider);

  m_showCleavingCheckBox = new QCheckBox("Show cleaving", m_widget);
  connect(m_showCleavingCheckBox, SIGNAL(clicked(bool)), this, SLOT(onShowCleavingChanged(bool)));

  m_cleaveIndexComboBox = new QComboBox(m_widget);
  QSize iconSizeDefault = m_cleaveIndexComboBox->iconSize();
  QSize iconSize = iconSizeDefault * 0.8;
  for (unsigned int i = 1; i < INDEX_COLORS.size(); i++) {
    QPixmap pixmap(iconSize);
    glm::vec4 color = 255.0f * INDEX_COLORS[i];
    pixmap.fill(QColor(color.r, color.g, color.b, color.a));
    QIcon icon(pixmap);
    m_cleaveIndexComboBox->addItem(icon, "Cleaved body " + QString::number(i));
  }

  m_buttonAdd = new QPushButton("Add selection", m_widget);
  connect(m_buttonAdd, SIGNAL(clicked(bool)), this, SLOT(onAddToChosenCleaveBody()));

  m_buttonRemove = new QPushButton("Remove selection", m_widget);
  connect(m_buttonRemove, SIGNAL(clicked(bool)), this, SLOT(onRemoveFromChosenCleaveBody()));

  m_buttonCleave = new QPushButton("Cleave", m_widget);
  connect(m_buttonCleave, SIGNAL(clicked(bool)), this, SLOT(onCleave()));

  QHBoxLayout *familiesLayout = new QHBoxLayout;
  familiesLayout->addWidget(m_showCleavingCheckBox);
  familiesLayout->addWidget(m_cleaveIndexComboBox);
  familiesLayout->addWidget(m_buttonAdd);
  familiesLayout->addWidget(m_buttonRemove);

  QVBoxLayout *layout = new QVBoxLayout;
  layout->addLayout(sliderLayout);
  layout->addLayout(familiesLayout);
  layout->addWidget(m_buttonCleave);

  m_widget->setLayout(layout);

  // These explicit shortcuts seem to work better than shorcuts set on the buttons.

  m_shortcutToggle = new QShortcut(Qt::Key_Space, m_widget);
  connect(m_shortcutToggle, SIGNAL(activated()), this, SLOT(onToggleInChosenCleaveBody()));

  for (int i = 0; i < m_cleaveIndexComboBox->count(); i++) {
    QShortcut *shortcut = new QShortcut(QString::number(i + 1), m_widget);
    connect(shortcut, SIGNAL(activated()), this, SLOT(onChosenCleaveIndexChanged()));
  }

  m_levelSlider->setValue(m_maxLevel);
}

void TaskBodyCleave::updateColors()
{
  if (Z3DMeshFilter *filter = getMeshFilter(m_bodyDoc)) {
    std::map<uint64_t, std::size_t> meshIdToCleaveIndex(m_meshIdToCleaveResultIndex);

    for (auto it : m_meshIdToCleaveIndex) {
      meshIdToCleaveIndex[it.first] = it.second;
    }

    filter->setColorIndexing(INDEX_COLORS, meshIdToCleaveIndex);
  }
}

void TaskBodyCleave::applyPerTaskSettings()
{
  // When the overall body is first loaded, the user probably wants the view to zoom to it.
  // But when the user is going back and forth between history levels for the body, the
  // user may have set the view to an area of interest, and it would be annoying to have
  // zooming destroy that view.  So try a heuristic solution.  When the a task starts (or
  // resumes) at the history level of the overall body, enable zooming.  And when the user
  // changes the level, disable zooming (in the updateLevel() function).

  bool doZoom = (m_levelSlider->value() == m_maxLevel);
  Neu3Window::enableZoomToLoadedBody(doZoom);

  // The SetCleaveIndicesCommand and CleaveCommand instances on the undo stack contain information
  // particular to the task that was current when the commands were issued.  So the undo stack will
  // not make sense when switching to another task, and the easiest solution is to clear it.

  m_bodyDoc->undoStack()->clear();

  bool showingCleaving = m_showCleavingCheckBox->isChecked();
  applyColorMode(showingCleaving);
}

void TaskBodyCleave::applyColorMode(bool showingCleaving)
{
  if (Z3DMeshFilter *filter = getMeshFilter(m_bodyDoc)) {
    if (showingCleaving) {
      filter->setColorMode("Indexed Color");
      updateColors();
    } else {
      filter->setColorMode("Mesh Source");
    }
  }
}

void TaskBodyCleave::enableCleavingUI(bool showingCleaving)
{
  m_cleaveIndexComboBox->setEnabled(showingCleaving);
  m_buttonAdd->setEnabled(showingCleaving);
  m_buttonRemove->setEnabled(showingCleaving);
  m_buttonCleave->setEnabled(showingCleaving);
  m_shortcutToggle->setEnabled(showingCleaving);
}

bool TaskBodyCleave::loadSpecific(QJsonObject json)
{
  if (!json.contains(KEY_BODYID)) {
    return false;
  }

  m_bodyId = json[KEY_BODYID].toDouble();
  m_maxLevel = json[KEY_MAXLEVEL].toDouble();

  return true;
}

