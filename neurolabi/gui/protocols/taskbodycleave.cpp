#include "taskbodycleave.h"

#include "dvid/zdvidreader.h"
#include "dvid/zdvidtarget.h"
#include "dvid/zdvidwriter.h"
#include "flyem/zflyembody3ddoc.h"
#include "flyem/zflyemproofmvc.h"
#include "zstackdocproxy.h"
#include "zwidgetmessage.h"
#include "z3dmeshfilter.h"
#include "z3dwindow.h"

#include <iostream>

#include <QCheckBox>
#include <QComboBox>
#include <QHBoxLayout>
#include <QInputDialog>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QLabel>
#include <QMenu>
#include <QMessageBox>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QPixmap>
#include <QPushButton>
#include <QSlider>
#include <QTimer>
#include <QUndoCommand>
#include <QUrl>
#include <QVBoxLayout>

namespace {

  static const QString KEY_TASKTYPE = "task type";
  static const QString VALUE_TASKTYPE = "body cleave";
  static const QString KEY_BODYID = "body ID";
  static const QString KEY_MAXLEVEL = "maximum level";
  static const QString KEY_ASSIGNED_USER = "assigned user";

  static const QString CLEAVING_STATUS_DONE = "Cleaving status: done";
  static const QString CLEAVING_STATUS_IN_PROGRESS = "Cleaving status: in progress...";
  static const QString CLEAVING_STATUS_FAILED = "Cleaving status: failed";
  static const QString CLEAVING_STATUS_SERVER_WARNINGS = "Cleaving status: server warnings";
  static const QString CLEAVING_STATUS_SERVER_INCOMPLETE = "Cleaving status: omitted meshes";

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

  static bool garbageLifetimeLimitEnabled;
  static bool splitTaskLoadingEnabled;
  static bool showingSynapse;
  static bool preservingSourceColorEnabled;
  static bool showingSourceColors;
  static bool showingAnnotations;

  void applyOverallSettings(ZFlyEmBody3dDoc* bodyDoc)
  {
    if (applyOverallSettingsNeeded) {
      applyOverallSettingsNeeded = false;

      garbageLifetimeLimitEnabled = bodyDoc->garbageLifetimeLimitEnabled();
      bodyDoc->enableGarbageLifetimeLimit(false);

      splitTaskLoadingEnabled = bodyDoc->splitTaskLoadingEnabled();
      bodyDoc->enableSplitTaskLoading(false);

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

      bodyDoc->enableGarbageLifetimeLimit(garbageLifetimeLimitEnabled);
      bodyDoc->enableSplitTaskLoading(splitTaskLoadingEnabled);
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
                          const std::vector< QString>& comboBoxItemsText) :
    m_task(task),
    m_meshIdToCleaveIndexBefore(task->m_meshIdToCleaveIndex),
    m_meshIdToCleaveIndexAfter(meshIdToCleaveIndex),
    m_comboBoxItemsTextBefore(getComboBoxItemsText(task->m_cleaveIndexComboBox)),
    m_comboBoxItemsTextAfter(comboBoxItemsText)
  {
    setText("seeding for next cleave");
  }

  virtual void undo() override
  {
    m_task->m_meshIdToCleaveIndex = m_meshIdToCleaveIndexBefore;
    m_task->updateColors();
    m_task->updateVisibility();
    setComboBoxItemsText(m_task->m_cleaveIndexComboBox, m_comboBoxItemsTextBefore);
  }

  virtual void redo() override
  {
    m_task->m_meshIdToCleaveIndex = m_meshIdToCleaveIndexAfter;
    m_task->updateColors();
    m_task->updateVisibility();
    setComboBoxItemsText(m_task->m_cleaveIndexComboBox, m_comboBoxItemsTextAfter);
  }

private:
  static std::vector<QString> getComboBoxItemsText(QComboBox* comboBox)
  {
    std::vector<QString> result;
    for (int i = 0; i < comboBox->count(); i++) {
      result.push_back(comboBox->itemText(i));
    }
    return result;
  }

  static void setComboBoxItemsText(QComboBox* comboBox, const std::vector<QString> &itemsText)
  {
    for (int i = 0; i < comboBox->count(); i++) {
      comboBox->setItemText(i, itemsText[i]);
    }
  }

  TaskBodyCleave *m_task;
  std::map<uint64_t, std::size_t> m_meshIdToCleaveIndexBefore;
  std::map<uint64_t, std::size_t> m_meshIdToCleaveIndexAfter;
  std::vector<QString> m_comboBoxItemsTextBefore;
  std::vector<QString> m_comboBoxItemsTextAfter;
};

class TaskBodyCleave::CleaveCommand : public QUndoCommand
{
public:
  CleaveCommand(TaskBodyCleave *task, std::map<uint64_t, std::size_t> meshIdToCleaveIndex,
                const QJsonObject &cleaveReply) :
    m_task(task),
    m_meshIdToCleaveResultIndexBefore(task->m_meshIdToCleaveResultIndex),
    m_meshIdToCleaveResultIndexAfter(meshIdToCleaveIndex),
    m_cleaveReplyBefore(task->m_cleaveReply),
    m_cleaveReplyAfter(cleaveReply)
  {
    setText("cleave");
  }

  virtual void undo() override
  {
    m_task->m_meshIdToCleaveResultIndex = m_meshIdToCleaveResultIndexBefore;
    m_task->m_cleaveReply = m_cleaveReplyBefore;
    m_task->updateColors();
    m_task->updateVisibility();
  }

  virtual void redo() override
  {
    m_task->m_meshIdToCleaveResultIndex = m_meshIdToCleaveResultIndexAfter;
    m_task->m_cleaveReply = m_cleaveReplyAfter;
    m_task->updateColors();
    m_task->updateVisibility();
  }

private:
  TaskBodyCleave *m_task;
  std::map<uint64_t, std::size_t> m_meshIdToCleaveResultIndexBefore;
  std::map<uint64_t, std::size_t> m_meshIdToCleaveResultIndexAfter;
  QJsonObject m_cleaveReplyBefore;
  QJsonObject m_cleaveReplyAfter;
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

  // If the user returns to this task, there is no good way (at least for now) to
  // reapply the visiblity settings to this task's meshes after they have been
  // reloaded.  So clear the visiblity settings.

  m_hiddenCleaveIndices.clear();
  m_showBodyCheckBox->setChecked(true);

  m_hiddenIds.clear();
}

void TaskBodyCleave::beforePrev()
{
  applyPerTaskSettings();

  // See the comment in beforeNext().

  m_bodyDoc->clearGarbage(true);

  // See the comment in beforeNext().

  m_hiddenCleaveIndices.clear();
  m_showBodyCheckBox->setChecked(true);

  m_hiddenIds.clear();
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

uint64_t TaskBodyCleave::getBodyId() const
{
  return m_bodyId;
}

void TaskBodyCleave::onShowCleavingChanged(int state)
{
  bool show = (state != Qt::Unchecked);
  enableCleavingUI(show);
  applyColorMode(show);
}

void TaskBodyCleave::onToggleShowCleaving()
{
  // For some reason, Qt will call this slot even when the source of the signal is disabled.
  // The source is a QAction on m_menu, and TaskProtocolWindow disables it and m_widget while
  // waiting for meshes from the previous task to e deleted and meshes for the next task to be
  // loaded.  In this case, going ahead and loading more meshes for cleaving can cause problems
  // due to way meshes are deleted and loaded asynchronously.  Since this disabling does not
  // prevent this slot from being called, we must explicitly abort this slot when there is
  // disabling.

  if (!m_menu->isEnabled() || !m_widget->isEnabled()) {
    return;
  }
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

void TaskBodyCleave::onCleaveIndexShortcut()
{
  if (QAction* action = dynamic_cast<QAction*>(QObject::sender())) {
    int i = m_actionToComboBoxIndex[action];
    m_cleaveIndexComboBox->setCurrentIndex(i);
  }
}

void TaskBodyCleave::onCleaveIndexChanged(int)
{
  bool visible = (m_hiddenCleaveIndices.find(chosenCleaveIndex()) ==
                  m_hiddenCleaveIndices.end());
  m_showBodyCheckBox->setChecked(visible);
}

void TaskBodyCleave::onSelectBody()
{
  std::set<uint64_t> toSelect;
  bodiesForCleaveIndex(toSelect, chosenCleaveIndex());
  selectBodies(toSelect);
}

void TaskBodyCleave::onShowBodyChanged(int state)
{
  if (state) {
    m_hiddenCleaveIndices.erase(chosenCleaveIndex());
  } else {
    m_hiddenCleaveIndices.insert(chosenCleaveIndex());
  }

  std::set<uint64_t> bodiesForIndex;
  bodiesForCleaveIndex(bodiesForIndex, chosenCleaveIndex(), true);

  QList<ZMesh*> meshes = ZStackDocProxy::GetGeneralMeshList(m_bodyDoc);
  for (auto it = meshes.cbegin(); it != meshes.cend(); it++) {
    ZMesh *mesh = *it;
    if (bodiesForIndex.find(mesh->getLabel()) != bodiesForIndex.end()) {
      if (m_hiddenIds.find(mesh->getLabel()) != m_hiddenIds.end()) {
        continue;
      }

      m_bodyDoc->setVisible(mesh, state);
    }
  }
}

void TaskBodyCleave::onToggleInChosenCleaveBody()
{
  const TStackObjectSet &selectedMeshes = m_bodyDoc->getSelected(ZStackObject::TYPE_MESH);
  std::map<uint64_t, std::size_t> meshIdToCleaveIndex(m_meshIdToCleaveIndex);

  // The text for each combobox item indicates the count the of seeds set for that item's color,
  // and we need to compute how those counts change.

  std::vector<int> countChanges;
  for (unsigned int i = 0; i < INDEX_COLORS.size(); i++) {
    countChanges.push_back(0);
  }

  for (auto itSelected = selectedMeshes.cbegin(); itSelected != selectedMeshes.cend(); itSelected++) {
    ZMesh *mesh = static_cast<ZMesh*>(*itSelected);
    uint64_t id = mesh->getLabel();
    auto itCleave = meshIdToCleaveIndex.find(id);
    if (itCleave == meshIdToCleaveIndex.end()) {

      // The selected mesh has not been assigned to a color before, so the count for the current color
      // increases by one.

      countChanges[chosenCleaveIndex()] += 1;
      meshIdToCleaveIndex[id] = chosenCleaveIndex();
    } else if (itCleave->second != chosenCleaveIndex()) {

      // The selected mesh has been assigned to a different color, so it is being reassigned from that
      // color.  So that color's count decreases by one, and the current color's count increases by one.

      countChanges[chosenCleaveIndex()] += 1;
      countChanges[itCleave->second] -= 1;
      meshIdToCleaveIndex[id] = chosenCleaveIndex();
    } else {

      // The selected mesh has been assigned to the current color, so it is being assigned no color,
      // and the current color's count decreases by one.

      countChanges[chosenCleaveIndex()] -= 1;
      meshIdToCleaveIndex.erase(id);
    }
  }

  // The command to change the colors, and update the combobox items' text, takes an array of text
  // for all the items.  Only a few will change, but since more than one could change, the undo/redo
  // managment is simpler with the complete array.

  std::vector<QString> itemTexts;
  for (std::size_t i = 1; i < countChanges.size(); i++) {
    int change = countChanges[i];
    QString text = m_cleaveIndexComboBox->itemText(i-1);

    if (change != 0) {
      int numSeeds = 0;
      int j = text.indexOf(" (");
      if (j != -1) {
        int k = text.indexOf(" ", j + 1);
        QString numSeedsStr = text.mid(j + 2, k - j - 2);
        numSeeds = numSeedsStr.toInt();
        text.truncate(j);
      }
      numSeeds += change;
      if (numSeeds > 0) {
        text += " (" + QString::number(numSeeds);
        text += (numSeeds == 1) ? " seed)" : " seeds)";
      }
    }

    itemTexts.push_back(text);
  }

  m_bodyDoc->pushUndoCommand(new SetCleaveIndicesCommand(this, meshIdToCleaveIndex, itemTexts));

  cleave();
}

void TaskBodyCleave::onToggleShowChosenCleaveBody()
{
  m_showBodyCheckBox->setChecked(!m_showBodyCheckBox->isChecked());
}

void TaskBodyCleave::onNetworkReplyFinished(QNetworkReply *reply)
{
  m_cleaveReplyPending = false;
  QNetworkReply::NetworkError error = reply->error();

  // In addition to checking for QNetworkReply::NetworkError, also check for
  // a status code other than OK (200), to be extra certain about catching errors.

  int statusCode = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
  const int STATUS_OK = 200;

  QString status = CLEAVING_STATUS_DONE;

  QByteArray replyBytes = reply->readAll();
  QJsonDocument replyJsonDoc = QJsonDocument::fromJson(replyBytes);
  QJsonObject replyJson = replyJsonDoc.object();

  if ((error == QNetworkReply::NoError) && (statusCode == STATUS_OK) && replyJsonDoc.isObject()) {
    if (showCleaveReplyWarnings(replyJson)) {
      status = CLEAVING_STATUS_SERVER_WARNINGS;
    }

    QJsonValue replyJsonAssnVal = replyJson["assignments"];
    if (replyJsonAssnVal.isObject()) {
      std::map<uint64_t, std::size_t> meshIdToCleaveIndex;

      QJsonObject replyJsonAssn = replyJsonAssnVal.toObject();
      for (QString key : replyJsonAssn.keys()) {
        uint64_t cleaveIndex = key.toInt();
        QJsonValue value = replyJsonAssn[key];
        if (value.isArray()) {
          for (QJsonValue idVal : value.toArray()) {
            uint64_t id = idVal.toDouble();
            meshIdToCleaveIndex[id] = cleaveIndex;
          }
        }
      }

      std::set<std::size_t> hiddenChangedIndices = hiddenChanges(meshIdToCleaveIndex);

      m_bodyDoc->pushUndoCommand(new CleaveCommand(this, meshIdToCleaveIndex, replyJson));

      if (showCleaveReplyOmittedMeshes(meshIdToCleaveIndex)) {
        status = CLEAVING_STATUS_SERVER_INCOMPLETE;
      }

      showHiddenChangeWarning(hiddenChangedIndices);
    }
  } else {
    // On OS X, the title is not displayed, so include it in the text, too.
    QString title = "Cleave Server Error";
    QString text = "Cleave server error: ";
    if (statusCode != STATUS_OK) {
      text += reply->errorString() + "\n";
    } else if (!replyJsonDoc.isObject()) {
      text += "Reply is not a JSON object";
    }
    if (replyJsonDoc.isObject()) {
      auto errorsIt = replyJson.constFind("errors");
      if (errorsIt != replyJson.end()) {
        QJsonValue errorsVal = errorsIt.value();
        if (errorsVal.isArray()) {
          QJsonArray errorsArray = errorsVal.toArray();
          for (QJsonValue error : errorsArray) {
            text += error.toString() + "\n";
          }
        }
      }
    }
    QString details = QString(replyBytes);
    displayWarning(title, text, details);

    status = CLEAVING_STATUS_FAILED;

    LERROR() << "TaskBodyCleave::onNetworkReplyFinished(): " << text << "\n";
  }

  m_cleavingStatusLabel->setText(status);
  reply->deleteLater();
}

void TaskBodyCleave::onHideSelected()
{
  const TStackObjectSet &selectedMeshes = m_bodyDoc->getSelected(ZStackObject::TYPE_MESH);
  for (auto itSelected = selectedMeshes.cbegin(); itSelected != selectedMeshes.cend(); itSelected++) {
    ZMesh *mesh = static_cast<ZMesh*>(*itSelected);
    m_hiddenIds.insert(mesh->getLabel());
  }
  updateVisibility();
}

void TaskBodyCleave::onClearHidden()
{
  selectBodies(m_hiddenIds);
  m_hiddenIds.clear();
  updateVisibility();
}

void TaskBodyCleave::onChooseCleaveMethod()
{
  if (Z3DWindow *window = m_bodyDoc->getParent3DWindow()) {
    bool ok = true;
    QString text = QInputDialog::getText(window, "Set Cleaving Method", "Cleaving method:",
                                         QLineEdit::Normal,  m_cleaveMethod, &ok);
    if (ok) {
      m_cleaveMethod = text;
    }
  }
}

QJsonObject TaskBodyCleave::addToJson(QJsonObject taskJson)
{
  taskJson[KEY_BODYID] = static_cast<double>(m_bodyId);
  taskJson[KEY_TASKTYPE] = VALUE_TASKTYPE;
  taskJson[KEY_MAXLEVEL] = m_maxLevel;

  return taskJson;
}

bool TaskBodyCleave::allowCompletion()
{
  bool allow = true;

  if (m_cleaveReplyPending) {
    if (Z3DWindow *window = m_bodyDoc->getParent3DWindow()) {
      QString title = "Warning";
      QString text = "A reply from the cleave server is pending. "
                     "Really save now without the cleaving in that reply?";
      QMessageBox::StandardButton chosen =
          QMessageBox::warning(window, title, text, QMessageBox::Save | QMessageBox::Cancel,
                               QMessageBox::Cancel);
      allow = (chosen == QMessageBox::Save);
    }
  }

  if (!m_hiddenCleaveIndices.empty()) {
    if (Z3DWindow *window = m_bodyDoc->getParent3DWindow()) {
      QString title = "Warning";
      QString text = "The following ";
      text += (m_hiddenCleaveIndices.size() == 1) ? "body (color) is" : "bodies (colors) are";
      text += " hidden:";
      for (std::size_t index : m_hiddenCleaveIndices) {
        text += " " + QString::number(index);
      }
      text += ". Really save now without checking what is hidden?";

      QMessageBox msgBox(QMessageBox::Warning, title, text, QMessageBox::Save,
                         window);
      QPushButton *cancelAndShow = msgBox.addButton("Cancel and Show All", QMessageBox::RejectRole);
      msgBox.setDefaultButton(cancelAndShow);

      msgBox.exec();

      if (msgBox.clickedButton() == cancelAndShow) {
        m_hiddenCleaveIndices.clear();
        m_showBodyCheckBox->setChecked(true);
        m_showSeedsOnlyCheckBox->setChecked(false);
        updateVisibility();

        allow = false;
      }
    }
  }

  if (!m_hiddenIds.empty()) {
    if (Z3DWindow *window = m_bodyDoc->getParent3DWindow()) {
      QString title = "Warning";
      QString text = (m_hiddenIds.size() == 1) ? "A mesh is hidden. " : "Some meshes are hidden. ";
      text += "Really save now without checking what is hidden?";

      QMessageBox msgBox(QMessageBox::Warning, title, text, QMessageBox::Save,
                         window);
      QPushButton *cancelAndShow = msgBox.addButton("Cancel and Show All", QMessageBox::RejectRole);
      msgBox.setDefaultButton(cancelAndShow);

      msgBox.exec();

      if (msgBox.clickedButton() == cancelAndShow) {
        onClearHidden();

        allow = false;
      }
    }
  }

  return allow;
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

  std::map<std::size_t, std::vector<uint64_t>> cleaveIndexToMeshIds;
  for (auto itMesh : m_meshIdToCleaveResultIndex) {
    std::size_t cleaveIndex = itMesh.second;
    auto itCleave = cleaveIndexToMeshIds.find(cleaveIndex);
    if (itCleave == cleaveIndexToMeshIds.end()) {
      cleaveIndexToMeshIds[cleaveIndex] = std::vector<uint64_t>();
    }
    uint64_t id = itMesh.first;
    cleaveIndexToMeshIds[cleaveIndex].push_back(id);
  }

  // The output is JSON, an array of arrays, where each inner array is the super voxels in a cleaved body.

  if (cleaveIndexToMeshIds.size() < 2) {
    // Fewer than two cleave indices means no actual cleaving, so omit the output.
    return;
  }

  QJsonArray json;
  for (auto itCleave : cleaveIndexToMeshIds) {
    QJsonArray jsonForCleaveIndex;

    // Sort the mesh IDs, since one of the main uses of this output is inspection by a human.

    std::vector<uint64_t> ids(itCleave.second);
    std::sort(ids.begin(), ids.end());

    for (auto itMesh : ids) {
      jsonForCleaveIndex.append(QJsonValue(qint64(itMesh)));
    }
    json.append(jsonForCleaveIndex);
  }

  // For debugging, append verbatin the cleave server response that produced the arrays of super voxels.
  // It can be distinguished as the only item in the output array that is a JSON object and not an array.

  json.append(m_cleaveReply);

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
  connect(m_cleaveIndexComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(onCleaveIndexChanged(int)));

  m_selectBodyButton = new QPushButton("Select", m_widget);
  connect(m_selectBodyButton, SIGNAL(clicked(bool)), this, SLOT(onSelectBody()));

  m_showBodyCheckBox = new QCheckBox("Show", m_widget);
  m_showBodyCheckBox->setChecked(true);
  connect(m_showBodyCheckBox, SIGNAL(stateChanged(int)), this, SLOT(onShowBodyChanged(int)));

  QVBoxLayout *cleaveIndexActionsLayout = new QVBoxLayout;
  cleaveIndexActionsLayout->addWidget(m_selectBodyButton);
  cleaveIndexActionsLayout->addWidget(m_showBodyCheckBox);

  QHBoxLayout *cleaveLayout1 = new QHBoxLayout;
  cleaveLayout1->addWidget(m_showCleavingCheckBox);
  cleaveLayout1->addWidget(m_cleaveIndexComboBox);
  cleaveLayout1->addLayout(cleaveIndexActionsLayout);

  m_showSeedsOnlyCheckBox = new QCheckBox("Show seeds only", m_widget);
  m_showSeedsOnlyCheckBox->setChecked(false);
  connect(m_showSeedsOnlyCheckBox, SIGNAL(stateChanged(int)), this, SLOT(onShowSeedsOnlyChanged(int)));

  m_cleavingStatusLabel = new QLabel(CLEAVING_STATUS_DONE, m_widget);

  QHBoxLayout *cleaveLayout2 = new QHBoxLayout;
  cleaveLayout2->addWidget(m_showSeedsOnlyCheckBox);
  cleaveLayout2->addWidget(m_cleavingStatusLabel);

  QVBoxLayout *layout = new QVBoxLayout;
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

  m_toggleInBodyAction = new QAction("Toggle Selection in to/out of Current Body", m_widget);
  m_toggleInBodyAction->setShortcut(Qt::Key_Space);
  m_menu->addAction(m_toggleInBodyAction);
  connect(m_toggleInBodyAction, SIGNAL(triggered()), this, SLOT(onToggleInChosenCleaveBody()));

  m_toggleShowChosenCleaveBodyAction = new QAction("Toggle Visibilty of Current Body", m_widget);
  m_toggleShowChosenCleaveBodyAction->setShortcut(Qt::Key_H);
  m_menu->addAction(m_toggleShowChosenCleaveBodyAction);
  connect(m_toggleShowChosenCleaveBodyAction, SIGNAL(triggered()),
          this, SLOT(onToggleShowChosenCleaveBody()));

  QMenu *setChosenCleaveIndexMenu = new QMenu("Set Current  Body To");
  m_menu->addMenu(setChosenCleaveIndexMenu);

  QAction *hideSelectedAction = new QAction("Hide Selected Meshes", m_widget);
  hideSelectedAction->setShortcut(Qt::Key_F1);
  m_menu->addAction(hideSelectedAction);
  connect(hideSelectedAction, SIGNAL(triggered()), this, SLOT(onHideSelected()));

  QAction *clearHiddenAction = new QAction("Clear Hidden Selected Meshes", m_widget);
  clearHiddenAction->setShortcut(Qt::Key_F2);
  m_menu->addAction(clearHiddenAction);
  connect(clearHiddenAction, SIGNAL(triggered()), this, SLOT(onClearHidden()));

  QMenu *advancedMenu = new QMenu("Advanced");
  m_menu->addMenu(advancedMenu);

  QAction *methodAction = new QAction("Cleaving Method", m_widget);
  advancedMenu->addAction(methodAction);
  connect(methodAction, SIGNAL(triggered()), this, SLOT(onChooseCleaveMethod()));

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
    connect(action, SIGNAL(triggered()), this, SLOT(onCleaveIndexShortcut()));

    // To avoid having to algorithmically invert the mapping of keys to combobox indices
    // when the shortcut is triggered, just store it.
    m_actionToComboBoxIndex[action] = i;
  }

  m_showCleavingCheckBox->setChecked(true);
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

void TaskBodyCleave::bodiesForCleaveIndex(std::set<uint64_t> &result,
                                          std::size_t cleaveIndex,
                                          bool ignoreSeedsOnly)
{
  for (auto it : m_meshIdToCleaveIndex) {
    if (it.second == cleaveIndex) {
      result.insert(it.first);
    }
  }
  if (ignoreSeedsOnly || !m_showSeedsOnlyCheckBox->isChecked()) {
    for (auto it : m_meshIdToCleaveResultIndex) {
      if (it.second == cleaveIndex) {
        result.insert(it.first);
      }
    }
  }
}

void TaskBodyCleave::selectBodies(const std::set<uint64_t> &toSelect)
{
  m_bodyDoc->deselectAllMesh();

  QList<ZMesh*> meshes = ZStackDocProxy::GetGeneralMeshList(m_bodyDoc);
  for (auto it = meshes.cbegin(); it != meshes.cend(); it++) {
    ZMesh *mesh = *it;
    if (toSelect.find(mesh->getLabel()) != toSelect.end()) {
      m_bodyDoc->setMeshSelected(mesh, true);
    }
  }
}

void TaskBodyCleave::applyPerTaskSettings()
{
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
  m_showBodyCheckBox->setEnabled(showingCleaving);
  m_showSeedsOnlyCheckBox->setEnabled(showingCleaving);
  m_cleavingStatusLabel->setEnabled(showingCleaving);
  m_showSeedsOnlyAction->setEnabled(showingCleaving);
  m_toggleInBodyAction->setEnabled(showingCleaving);
  m_toggleShowChosenCleaveBodyAction->setEnabled(showingCleaving);
  for (auto it : m_actionToComboBoxIndex) {
    it.first->setEnabled(showingCleaving);
  }
}

void TaskBodyCleave::cleave()
{
  m_cleavingStatusLabel->setText(CLEAVING_STATUS_IN_PROGRESS);

  QJsonObject requestJson;
  requestJson["body-id"] = qint64(m_bodyId);

  std::map<std::size_t, std::vector<uint64_t>> cleaveIndexToMeshIds;
  for (auto it1 : m_meshIdToCleaveIndex) {
    std::size_t cleaveIndex = it1.second;
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

  if (!m_cleaveMethod.isEmpty()) {
    requestJson["method"] = m_cleaveMethod;
  }
  requestJson["seeds"] = requestJsonSeeds;
  if (const char* user = std::getenv("USER")) {
    requestJson["user"] = user;
  }
  requestJson["server"] = m_bodyDoc->getDvidTarget().getAddress().c_str();
  requestJson["port"] = m_bodyDoc->getDvidTarget().getPort();
  requestJson["uuid"] = m_bodyDoc->getDvidTarget().getUuid().c_str();
  requestJson["segmentation-instance"] = m_bodyDoc->getDvidTarget().getBodyLabelName().c_str();
  requestJson["mesh-instance"] =
      ZDvidData::GetName(ZDvidData::ROLE_MESHES_TARS,
                         ZDvidData::ROLE_BODY_LABEL,
                         m_bodyDoc->getDvidTarget().getBodyLabelName()).c_str();

  // TODO: Teporary cleaving sevrver URL.
  QString server = "http://emdata3.int.janelia.org:5552/compute-cleave";
  if (const char* serverOverride = std::getenv("NEU3_CLEAVE_SERVER")) {
    server = serverOverride;
  }

  QUrl url(server);
  QNetworkRequest request(url);
  request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");

  QJsonDocument requestJsonDoc(requestJson);
  QByteArray requestData(requestJsonDoc.toJson());

  m_cleaveReplyPending = true;
  m_networkManager->post(request, requestData);
}

void TaskBodyCleave::updateVisibility()
{
  QList<ZMesh*> meshes = ZStackDocProxy::GetGeneralMeshList(m_bodyDoc);
  for (auto itMesh = meshes.cbegin(); itMesh != meshes.cend(); itMesh++) {
    ZMesh *mesh = *itMesh;
    uint64_t id = mesh->getLabel();
    bool toBeVisible = true;

    // First check whether the mesh is a seed, to give the correct update when
    // undoing the setting of a seed.

    auto itCleave = m_meshIdToCleaveIndex.find(id);
    if (itCleave != m_meshIdToCleaveIndex.end()) {
      std::size_t index = itCleave->second;
      toBeVisible = (m_hiddenCleaveIndices.find(index) == m_hiddenCleaveIndices.end());
    } else {

      // If it is not a seed check for it in the cleaving results.

      auto itCleaveResult = m_meshIdToCleaveResultIndex.find(id);
      if (itCleaveResult != m_meshIdToCleaveResultIndex.end()) {
        std::size_t index = itCleaveResult->second;
        toBeVisible = (m_hiddenCleaveIndices.find(index) == m_hiddenCleaveIndices.end());
      }
    }

    if (toBeVisible) {
      toBeVisible = (m_hiddenIds.find(id) == m_hiddenIds.end());
    }

    m_bodyDoc->setVisible(mesh, toBeVisible);
  }
}

std::set<std::size_t> TaskBodyCleave::hiddenChanges(const std::map<uint64_t, std::size_t>&
                                                    newMeshIdToCleaveIndex) const
{
  std::set<std::size_t> changedHiddenIndices;

  QList<ZMesh*> meshes = ZStackDocProxy::GetGeneralMeshList(m_bodyDoc);
  for (auto itMesh = meshes.cbegin(); itMesh != meshes.cend(); itMesh++) {
    ZMesh *mesh = *itMesh;
    if (!mesh->isVisible()) {
      uint64_t id = mesh->getLabel();

      auto itCleave = m_meshIdToCleaveResultIndex.find(id);
      if (itCleave != m_meshIdToCleaveResultIndex.end()) {
        size_t index = itCleave->second;

        auto itNewCleave = newMeshIdToCleaveIndex.find(id);
        if (itNewCleave != newMeshIdToCleaveIndex.end()) {
          size_t indexNew = itNewCleave->second;
          if (index != indexNew) {
            bool newHidden = (m_hiddenCleaveIndices.find(indexNew) !=
                              m_hiddenCleaveIndices.end());
            size_t indexHidden = newHidden ? indexNew : index;
            changedHiddenIndices.insert(indexHidden);
          }
        }
      }
    }
  }

  return changedHiddenIndices;
}

void TaskBodyCleave::showHiddenChangeWarning(const std::set<std::size_t> &
                                             hiddenChangedIndices)
{
  if (!hiddenChangedIndices.empty()) {
    QString title = "Hidden Changes";
    QString text = "Changes occurred in hidden ";
    text += (hiddenChangedIndices.size() == 1) ? "body (color): " : "bodies (colors): ";
    for (std::size_t index : hiddenChangedIndices) {
      text += QString::number(index) + " ";
    }
    displayWarning(title, text);
  }
}

bool TaskBodyCleave::showCleaveReplyWarnings(const QJsonObject &replyJson)
{
  auto warningsIt = replyJson.constFind("warnings");
  if (warningsIt != replyJson.end()) {
    QJsonValue warningsVal = warningsIt.value();
    if (warningsVal.isArray()) {
      QJsonArray warningsArray = warningsVal.toArray();
      if (!warningsArray.isEmpty()) {
        QString title = "Cleave Server Warning";
        title += (warningsArray.size() > 1) ? "s" : "";
        QString text;
        for (QJsonValue warning : warningsArray) {
          text += warning.toString() + "\n";
        }

        if (m_warningTextToSuppress.find(text) == m_warningTextToSuppress.end()) {
          displayWarning(title, text, "", true);
        }

        return true;
      }
    }
  }

  return false;
}

bool TaskBodyCleave::showCleaveReplyOmittedMeshes(std::map<uint64_t, std::size_t> meshIdToCleaveIndex)
{
  std::vector<ZMesh*> missing;
  QList<ZMesh*> meshes = ZStackDocProxy::GetGeneralMeshList(m_bodyDoc);
  for (auto it = meshes.cbegin(); it != meshes.cend(); it++) {
    ZMesh *mesh = *it;
    if (meshIdToCleaveIndex.find(mesh->getLabel()) == meshIdToCleaveIndex.end()) {
      missing.push_back(mesh);
    }
  }

  if (!missing.empty()) {
    std::set<uint64_t> toSelect;
    for (ZMesh* mesh : missing) {
      toSelect.insert(mesh->getLabel());
    }
    selectBodies(toSelect);

    QString title = "Cleave Server Error";
    // On OS X, the title is not displayed, so include it in the text, too.
    QString text = "Cleave server error: " + QString::number(missing.size());
    text += (missing.size() > 1) ? " super voxels were " : " super voxel was ";
    text += "missing in server response";

    std::vector<uint64_t> missingIds;
    for (ZMesh* mesh : missing) {
      missingIds.push_back(mesh->getLabel());
    }
    std::sort(missingIds.begin(), missingIds.end());
    QString details("Missing: ");
    for (size_t i = 0; i < missingIds.size(); i++) {
      if (i == 200) {
        details += "etc.";
        break;
      }
      details += QString::number(missingIds[i]) + " ";
    }

    displayWarning(title, text, details);

    return true;
  } else {
    return false;
  }
}

void TaskBodyCleave::displayWarning(const QString &title, const QString &text,
                                    const QString& details, bool allowSuppression)
{
  // Display the warning at idle time, after the rendering event has been processed,
  // so the results of cleaving will be visible when the warning is presented.

  QTimer::singleShot(0, this, [=](){
    if (details.isEmpty() && !allowSuppression) {
      ZWidgetMessage msg(title, text, neutube::MSG_WARNING, ZWidgetMessage::TARGET_DIALOG);
      m_bodyDoc->notify(msg);
    } else {
      QMessageBox msgBox(QMessageBox::Warning, title, text, QMessageBox::NoButton, m_bodyDoc->getParent3DWindow());
      QPushButton *okAndSuppress = nullptr;
      if (allowSuppression) {
        okAndSuppress = msgBox.addButton("OK, Don't Ask Again", QMessageBox::AcceptRole);
      }
      QPushButton *ok = msgBox.addButton("OK", QMessageBox::AcceptRole);
      msgBox.setDefaultButton(ok);
      if (!details.isEmpty()) {
        msgBox.setDetailedText(details);
      }

      msgBox.exec();

      if (msgBox.clickedButton() == okAndSuppress) {
          m_warningTextToSuppress.insert(text);
      }
    }
  });
}

bool TaskBodyCleave::loadSpecific(QJsonObject json)
{
  if (!json.contains(KEY_BODYID)) {
    return false;
  }

  m_bodyId = json[KEY_BODYID].toDouble();
  m_maxLevel = json[KEY_MAXLEVEL].toDouble();

  m_visibleBodies.insert(ZFlyEmBody3dDoc::encode(m_bodyId, 0));

  QString assignedUser = json[KEY_ASSIGNED_USER].toString();
  if (!assignedUser.isEmpty()) {
    if (const char* user = std::getenv("USER")) {
      if (user != assignedUser) {
        QString title = "Warning";
        QString text = "Task for ID " + QString::number(m_bodyId) +
            " was assigned to user \"" + assignedUser + "\"";
        displayWarning(title, text);
      }
    }
  }

  return true;
}

