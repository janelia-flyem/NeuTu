#include "taskbodycleave.h"

#include "dvid/zdvidreader.h"
#include "dvid/zdvidtarget.h"
#include "dvid/zdvidwriter.h"
#include "flyem/zflyembody3ddoc.h"
#include "flyem/zflyemproofmvc.h"
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
#include <QMenu>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QPixmap>
#include <QPushButton>
#include <QSlider>
#include <QUndoCommand>
#include <QUrl>
#include <QVBoxLayout>

namespace {

  static const QString KEY_TASKTYPE = "task type";
  static const QString VALUE_TASKTYPE = "body cleave";
  static const QString KEY_BODYID = "body ID";
  static const QString KEY_MAXLEVEL = "maximum level";

  static const QString CLEAVING_STATUS_DONE = "Cleaving status: done";
  static const QString CLEAVING_STATUS_IN_PROGRESS = "Cleaving status: in progress...";
  static const QString CLEAVING_STATUS_FAILED = "Cleaving status: failed";

  // https://sashat.me/2017/01/11/list-of-20-simple-distinct-colors/
  static const std::vector<glm::vec4> INDEX_COLORS({
    glm::vec4(255, 255, 255, 255) / 255.0f, // white (no body)
    glm::vec4(230,  25,  75, 255) / 255.0f, // red
    glm::vec4(255, 225,  25, 255) / 255.0f, // yellow
    glm::vec4(  0, 130, 200, 255) / 255.0f, // blue
    glm::vec4(245, 130,  48, 255) / 255.0f, // orange
    glm::vec4(145,  30, 180, 255) / 255.0f, // purple
    glm::vec4( 70, 240, 240, 255) / 255.0f, // cyan
    glm::vec4( 60, 180,  75, 255) / 255.0f, // green
    glm::vec4(240,  50, 230, 255) / 255.0f, // magenta
    glm::vec4(210, 245,  60, 255) / 255.0f, // lime
    glm::vec4(250, 190, 190, 255) / 255.0f, // pink
    glm::vec4(  0, 128, 128, 255) / 255.0f, // teal
    glm::vec4(230, 190, 255, 255) / 255.0f, // lavender
    glm::vec4(170, 110,  40, 255) / 255.0f, // brown
    glm::vec4(255, 250, 200, 255) / 255.0f, // beige
    glm::vec4(128,   0,   0, 255) / 255.0f, // maroon
    glm::vec4(170, 255, 195, 255) / 255.0f, // mint
    glm::vec4(128, 128,   0, 255) / 255.0f, // olive
    glm::vec4(255, 215, 180, 255) / 255.0f, // coral
    glm::vec4(  0,   0, 128, 255) / 255.0f, // navy
    glm::vec4(128, 128, 128, 255) / 255.0f, // gray
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
  static bool splitTaskLoadingEnabled;
  static bool showingTodo;
  static bool showingSynapse;
  static bool preservingSourceColorEnabled;
  static bool showingSourceColors;
  static bool showingAnnotations;

  void applyOverallSettings(ZFlyEmBody3dDoc* bodyDoc)
  {
    if (applyOverallSettingsNeeded) {
      applyOverallSettingsNeeded = false;

      zoomToLoadedBodyEnabled = Neu3Window::zoomToLoadedBodyEnabled();

      garbageLifetimeLimitEnabled = bodyDoc->garbageLifetimeLimitEnabled();
      bodyDoc->enableGarbageLifetimeLimit(false);

      splitTaskLoadingEnabled = bodyDoc->splitTaskLoadingEnabled();
      bodyDoc->enableSplitTaskLoading(false);

      showingTodo = bodyDoc->showingTodo();
//      bodyDoc->showTodo(false);

      showingSynapse = bodyDoc->showingSynapse();
      bodyDoc->showSynapse(false);

      if (Z3DMeshFilter *filter = getMeshFilter(bodyDoc)) {
        preservingSourceColorEnabled = filter->preservingSourceColorsEnabled();
        filter->enablePreservingSourceColors(true);

        // For a large set of super voxels, merely showing all the source colors in the
        // filter's configuration UI can introduce a significant delay, so turn off
        // that feature (which is not really useful in this context anyway).

        showingSourceColors = filter->showingSourceColors();
        filter->showSourceColors(false);
      }

      showingAnnotations = ZFlyEmProofMvc::showingAnnotations();
      ZFlyEmProofMvc::showAnnotations(false);
    }
  }

  void restoreOverallSettings(ZFlyEmBody3dDoc* bodyDoc)
  {
    if (!applyOverallSettingsNeeded) {
      applyOverallSettingsNeeded = true;

      Neu3Window::enableZoomToLoadedBody(zoomToLoadedBodyEnabled);

      bodyDoc->enableGarbageLifetimeLimit(garbageLifetimeLimitEnabled);
      bodyDoc->enableSplitTaskLoading(splitTaskLoadingEnabled);
      bodyDoc->showTodo(showingTodo);
      bodyDoc->showSynapse(showingSynapse);

      if (Z3DMeshFilter *filter = getMeshFilter(bodyDoc)) {
        filter->enablePreservingSourceColors(preservingSourceColorEnabled);
        filter->showSourceColors(showingSourceColors);
      }

      ZFlyEmProofMvc::showAnnotations(showingAnnotations);
    }
  }

}

//

class TaskBodyCleave::SetCleaveIndicesCommand : public QUndoCommand
{
public:
  SetCleaveIndicesCommand(TaskBodyCleave *task,
                          std::map<uint64_t, std::size_t> meshIdToCleaveIndex,
                          int comboBoxIndex,
                          const QString &comboBoxText) :
    m_task(task),
    m_meshIdToCleaveIndexBefore(task->m_meshIdToCleaveIndex),
    m_meshIdToCleaveIndexAfter(meshIdToCleaveIndex),
    m_comboBoxIndex(comboBoxIndex),
    m_comboBoxTextBefore(task->m_cleaveIndexComboBox->itemText(m_comboBoxIndex)),
    m_comboBoxTextAfter(comboBoxText)
  {
    setText("seeding for next cleave");
  }

  virtual void undo() override
  {
    m_task->m_meshIdToCleaveIndex = m_meshIdToCleaveIndexBefore;
    m_task->updateColors();
    m_task->m_cleaveIndexComboBox->setItemText(m_comboBoxIndex, m_comboBoxTextBefore);
  }

  virtual void redo() override
  {
    m_task->m_meshIdToCleaveIndex = m_meshIdToCleaveIndexAfter;
    m_task->updateColors();
    m_task->m_cleaveIndexComboBox->setItemText(m_comboBoxIndex, m_comboBoxTextAfter);
  }

private:
  TaskBodyCleave *m_task;
  std::map<uint64_t, std::size_t> m_meshIdToCleaveIndexBefore;
  std::map<uint64_t, std::size_t> m_meshIdToCleaveIndexAfter;
  int m_comboBoxIndex;
  QString m_comboBoxTextBefore;
  QString m_comboBoxTextAfter;
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
  return "Body cleaving:";
}

QString TaskBodyCleave::targetString()
{
  return QString::number(m_bodyId);
}

void TaskBodyCleave::beforeNext()
{
  applyPerTaskSettings();

  // Clear the mesh cache when changing tasks so it does not grow without bound
  // during an assignment, which causes a performance degradation.  The assumption
  // is that improving performance as a user progresses through an assignment is
  // more important than eliminating the need to reload meshses if the user goes
  // back to a previous task.

  m_bodyDoc->clearGarbage(true);
}

void TaskBodyCleave::beforePrev()
{
  applyPerTaskSettings();

  // See the comment in beforeNext().

  m_bodyDoc->clearGarbage(true);
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

QMenu *TaskBodyCleave::getTaskMenu()
{
  return m_menu;
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

void TaskBodyCleave::onShowCleavingChanged(int state)
{
  bool show = (state != Qt::Unchecked);
  if (show) {
    // Cleaving works on super voxels, which are what are displayed at level 0.
    m_levelSlider->setValue(0);
  }

  enableCleavingUI(show);
  applyColorMode(show);
}

void TaskBodyCleave::onToggleShowCleaving()
{
  m_showCleavingCheckBox->setChecked(!m_showCleavingCheckBox->isChecked());
}

void TaskBodyCleave::onShowSeedsOnlyChanged(int)
{
  updateColors();
}

void TaskBodyCleave::onToggleShowSeedsOnly()
{
  m_showSeedsOnlyCheckBox->setChecked(!m_showSeedsOnlyCheckBox->isChecked());
}

void TaskBodyCleave::onChosenCleaveIndexChanged()
{
  if (QAction* action = dynamic_cast<QAction*>(QObject::sender())) {
    int i = m_actionToComboBoxIndex[action];
    m_cleaveIndexComboBox->setCurrentIndex(i);
  }
}

void TaskBodyCleave::onSelectBody()
{
  m_bodyDoc->deselectAllMesh();

  std::set<uint64_t> toSelect;
  for (auto it : m_meshIdToCleaveIndex) {
    if (it.second == chosenCleaveIndex()) {
      toSelect.insert(it.first);
    }
  }
  if (!m_showSeedsOnlyCheckBox->isChecked()) {
    for (auto it : m_meshIdToCleaveResultIndex) {
      if (it.second == chosenCleaveIndex()) {
        toSelect.insert(it.first);
      }
    }
  }

  QList<ZMesh*> meshes = m_bodyDoc->getMeshList();
  for (auto it = meshes.cbegin(); it != meshes.cend(); it++) {
    ZMesh *mesh = *it;
    if (toSelect.find(mesh->getLabel()) != toSelect.end()) {
      m_bodyDoc->setMeshSelected(mesh, true);
    }
  }
}

void TaskBodyCleave::onToggleInChosenCleaveBody()
{
  const TStackObjectSet &selectedMeshes = m_bodyDoc->getSelected(ZStackObject::TYPE_MESH);
  std::map<uint64_t, size_t> meshIdToCleaveIndex(m_meshIdToCleaveIndex);

  // The text of the combobox item will be updated to indicate the number of seeds
  // with the current color, so count them.

  int numSeeds = 0;
  for (auto it : m_meshIdToCleaveIndex) {
    if (it.second == chosenCleaveIndex()) {
      numSeeds++;
    }
  }

  QString text = m_cleaveIndexComboBox->currentText();

  for (auto itSelected = selectedMeshes.cbegin(); itSelected != selectedMeshes.cend(); itSelected++) {
    ZMesh *mesh = static_cast<ZMesh*>(*itSelected);
    uint64_t id = mesh->getLabel();
    int change = 0;
    auto itCleave = meshIdToCleaveIndex.find(id);
    if ((itCleave == meshIdToCleaveIndex.end()) || (itCleave->second != chosenCleaveIndex())) {
      meshIdToCleaveIndex[id] = chosenCleaveIndex();
      change = 1;
    } else {
      meshIdToCleaveIndex.erase(id);
      change = -1;
    }

    // Update the text to indicate the new number of seeds.

    int i = text.indexOf(" (");
    if (i != -1) {
      text.truncate(i);
    }
    numSeeds += change;
    if (numSeeds > 0) {
      text += " (" + QString::number(numSeeds);
      text += (numSeeds == 1) ? " seed)" : " seeds)";
    }
  }

  int i = m_cleaveIndexComboBox->currentIndex();
  m_bodyDoc->pushUndoCommand(new SetCleaveIndicesCommand(this, meshIdToCleaveIndex, i, text));

  cleave();
}

void TaskBodyCleave::onNetworkReplyFinished(QNetworkReply *reply)
{
  QNetworkReply::NetworkError error = reply->error();
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

        m_cleavingStatusLabel->setText(CLEAVING_STATUS_DONE);
      }
    }

  } else {
    m_cleavingStatusLabel->setText(CLEAVING_STATUS_FAILED);

    LERROR() << "TaskBodyCleave::onNetworkReplyFinished() error: \""
             << reply->errorString().toStdString() << "\"";
  }

  reply->deleteLater();
}

QJsonObject TaskBodyCleave::addToJson(QJsonObject taskJson)
{
  taskJson[KEY_BODYID] = static_cast<double>(m_bodyId);
  taskJson[KEY_TASKTYPE] = VALUE_TASKTYPE;
  taskJson[KEY_MAXLEVEL] = m_maxLevel;

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

  QHBoxLayout *historyLayout = new QHBoxLayout;
  historyLayout->addWidget(sliderLabel);
  historyLayout->addWidget(m_levelSlider);

  m_showCleavingCheckBox = new QCheckBox("Show cleaving", m_widget);
  connect(m_showCleavingCheckBox, SIGNAL(stateChanged(int)), this, SLOT(onShowCleavingChanged(int)));

  m_cleaveIndexComboBox = new QComboBox(m_widget);
  // Let the combo box take as much width as possible, because the item text will be modified
  // to include the number of seeds set with that color, and we don't want it clipped.
  m_cleaveIndexComboBox->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Preferred);
  QSize iconSizeDefault = m_cleaveIndexComboBox->iconSize();
  QSize iconSize = iconSizeDefault * 0.8;
  for (unsigned int i = 1; i < INDEX_COLORS.size(); i++) {
    QPixmap pixmap(iconSize);
    glm::vec4 color = 255.0f * INDEX_COLORS[i];
    pixmap.fill(QColor(color.r, color.g, color.b, color.a));
    QIcon icon(pixmap);
    m_cleaveIndexComboBox->addItem(icon, "Cleaved body " + QString::number(i));
  }

  m_selectBodyButton = new QPushButton("Select", m_widget);
  connect(m_selectBodyButton, SIGNAL(clicked(bool)), this, SLOT(onSelectBody()));

  QHBoxLayout *cleaveLayout1 = new QHBoxLayout;
  cleaveLayout1->addWidget(m_showCleavingCheckBox);
  cleaveLayout1->addWidget(m_cleaveIndexComboBox);
  cleaveLayout1->addWidget(m_selectBodyButton);

  m_showSeedsOnlyCheckBox = new QCheckBox("Show seeds only", m_widget);
  m_showSeedsOnlyCheckBox->setChecked(false);
  connect(m_showSeedsOnlyCheckBox, SIGNAL(stateChanged(int)), this, SLOT(onShowSeedsOnlyChanged(int)));

  m_cleavingStatusLabel = new QLabel(CLEAVING_STATUS_DONE, m_widget);

  QHBoxLayout *cleaveLayout2 = new QHBoxLayout;
  cleaveLayout2->addWidget(m_showSeedsOnlyCheckBox);
  cleaveLayout2->addWidget(m_cleavingStatusLabel);

  QVBoxLayout *layout = new QVBoxLayout;
  layout->addLayout(historyLayout);
  layout->addLayout(cleaveLayout1);
  layout->addLayout(cleaveLayout2);

  m_widget->setLayout(layout);

  m_menu = new QMenu("Body Cleaving", m_widget);

  QAction *showCleavingAction = new QAction("Show Cleaving", m_widget);
  showCleavingAction->setShortcut(Qt::Key_C);
  m_menu->addAction(showCleavingAction);
  connect(showCleavingAction, SIGNAL(triggered()), this, SLOT(onToggleShowCleaving()));

  m_showSeedsOnlyAction = new QAction("Show Seeds Only", m_widget);
  m_showSeedsOnlyAction->setShortcut(Qt::Key_S);
  m_menu->addAction(m_showSeedsOnlyAction);
  connect(m_showSeedsOnlyAction, SIGNAL(triggered()), this, SLOT(onToggleShowSeedsOnly()));

  m_toggleInBodyAction = new QAction("Toggle Selection in Body", m_widget);
  m_toggleInBodyAction->setShortcut(Qt::Key_Space);
  m_menu->addAction(m_toggleInBodyAction);
  connect(m_toggleInBodyAction, SIGNAL(triggered()), this, SLOT(onToggleInChosenCleaveBody()));

  QMenu *setChosenCleaveIndexMenu = new QMenu("Set Cleaved Body To");
  m_menu->addMenu(setChosenCleaveIndexMenu);

  const int NUM_DISTINCT_KEYS = 10;
  int n = std::min(m_cleaveIndexComboBox->count(), 2 * NUM_DISTINCT_KEYS);
  for (int i = 0; i < n; i++) {
    // Treat "0" as coming after 9 instead of before 1.
    int j = i + 1;

    QString key = QString::number(j % NUM_DISTINCT_KEYS);
    if (j > NUM_DISTINCT_KEYS) {
      key.prepend("Shift+");
    }

    QIcon icon = m_cleaveIndexComboBox->itemIcon(i);
    QAction *action = new QAction(icon, "Cleaved Body " + QString::number(j), m_widget);
    action->setShortcut(key);
    setChosenCleaveIndexMenu->addAction(action);
    connect(action, SIGNAL(triggered()), this, SLOT(onChosenCleaveIndexChanged()));

    // To avoid having to algorithmically invert the mapping of keys to combobox indices
    // when the shortcut is triggered, just store it.
    m_actionToComboBoxIndex[action] = i;
  }

  m_levelSlider->setValue(m_maxLevel);
}

void TaskBodyCleave::updateColors()
{
  if (Z3DMeshFilter *filter = getMeshFilter(m_bodyDoc)) {
    std::map<uint64_t, std::size_t> meshIdToCleaveIndex;
    if (!m_showSeedsOnlyCheckBox->isChecked()) {
      meshIdToCleaveIndex = m_meshIdToCleaveResultIndex;
    }

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
  m_selectBodyButton->setEnabled(showingCleaving);
  m_showSeedsOnlyCheckBox->setEnabled(showingCleaving);
  m_cleavingStatusLabel->setEnabled(showingCleaving);
  m_showSeedsOnlyAction->setEnabled(showingCleaving);
  m_toggleInBodyAction->setEnabled(showingCleaving);
  for (auto it : m_actionToComboBoxIndex) {
    it.first->setEnabled(showingCleaving);
  }
}

void TaskBodyCleave::cleave()
{
  m_cleavingStatusLabel->setText(CLEAVING_STATUS_IN_PROGRESS);

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
  if (const char* user = std::getenv("USER")) {
    requestJson["user"] = user;
  }

  // TODO: Teporary cleaving sevrver URL.
  QString server = "http://bergs-ws1.int.janelia.org:5556/compute-cleave";
  if (const char* serverOverride = std::getenv("NEU3_CLEAVE_SERVER")) {
    server = serverOverride;
  }

  QUrl url(server);
  QNetworkRequest request(url);
  request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");

  QJsonDocument requestJsonDoc(requestJson);
  QByteArray requestData(requestJsonDoc.toJson());

  m_networkManager->post(request, requestData);
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

