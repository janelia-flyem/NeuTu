#include "taskfalsesplitreview.h"

#include "dvid/zdvidreader.h"
#include "dvid/zdvidtarget.h"
#include "dvid/zdvidwriter.h"
#include "flyem/zflyembody3ddoc.h"
#include "flyem/zflyemproofdoc.h"
#include "flyem/zflyemproofmvc.h"
#include "flyem/zflyemtodoitem.h"
#include "neutubeconfig.h"
#include "zdvidutil.h"
#include "zstackdocproxy.h"
#include "zwidgetmessage.h"
#include "z3dcamerautils.h"
#include "z3dmeshfilter.h"
#include "z3dwindow.h"

#include <iostream>

#include <sys/types.h>
#include <dirent.h>

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
  static const QString VALUE_TASKTYPE = "false split review";
  static const QString KEY_BODY_ID = "body ID";
  static const QString KEY_ASSIGNED_USER = "assigned user";

  // For the result JSON.
  static const QString KEY_SKIPPED = "skipped";
  static const QString KEY_USER = "user";
  static const QString KEY_TIMESTAMP = "time";
  static const QString KEY_TIME_ZONE = "time zone";
  static const QString KEY_SOURCE = "source";
  static const QString KEY_BUILD_VERSION = "build version";
  static const QString KEY_USAGE_TIME = "time to complete (ms)";
  static const QString KEY_TODO_COUNT = "merge todo count";

  static const std::vector<glm::vec4> INDEX_COLORS({
    glm::vec4(255, 255, 255, 255) / 255.0f, // white (not in the body)
    glm::vec4(  0, 130, 200, 255) / 255.0f, // blue
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
    return dvidTarget.getBodyLabelName() + "_falseSplitReview";
  }

  // By convention, our Conda builds create a "conda-meta/neu3_XXX.json" file
  // where "XXX" has information about the release version and the source version
  // (Git SHA-1 hash).  Return the base name of that file as the build version.

  std::string getBuildVersion()
  {
    std::string result;
    std::string path = NeutubeConfig::getInstance().getApplicatinDir();
#if defined(Q_OS_DARWIN)
    path += "/../../..";
#endif
    path += "/../conda-meta";
    if (DIR *dir = opendir(path.c_str())) {
      while (struct dirent *ent = readdir(dir)) {
        std::string filename(ent->d_name);
        if (filename.substr(0, 4) == "neu3") {
          std::size_t i = filename.rfind(".");
          if (i != std::string::npos) {
            filename = filename.substr(0, i);
            if (result.empty()) {
              result = filename;
            } else {
              result += "|" + filename;
            }
          }
        }
      }
      closedir(dir);
    }
    return result;
  }

  // All the TaskFalseSplitReview instances loaded from one JSON file need certain changes
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

TaskFalseSplitReview::TaskFalseSplitReview(QJsonObject json, ZFlyEmBody3dDoc* bodyDoc)
{
  m_bodyDoc = bodyDoc;

  applyOverallSettings(bodyDoc);

  loadJson(json);

  buildTaskWidget();
}

QString TaskFalseSplitReview::taskTypeStatic()
{
  return VALUE_TASKTYPE;
}

TaskFalseSplitReview* TaskFalseSplitReview::createFromJson(QJsonObject json, ZFlyEmBody3dDoc *bodyDoc)
{
  return new TaskFalseSplitReview(json, bodyDoc);
}

QString TaskFalseSplitReview::taskType() const
{
  return taskTypeStatic();
}

QString TaskFalseSplitReview::actionString()
{
  return "False split review:";
}

QString TaskFalseSplitReview::targetString()
{
  return QString::number(m_bodyId);
}

bool TaskFalseSplitReview::skip()
{
  // For now, at least, the "HEAD" command to check whether a tarsupervoxels instance has
  // a complete tar archive may be slow for large bodies.  So avoid executing it repeatedly
  // in rapid succession.

  // An environment variable can override the interval for checking, with a value of
  // -1 meaning never check.

  int interval = 1000;
  if (const char* overrideIntervalStr = std::getenv("NEU3_FALSE_SPLIT_REVIEW_SKIP_TEST_INTERVAL_MS")) {
    interval = std::atoi(overrideIntervalStr);
  }
  if (interval < 0) {
    return false;
  }

  int now = QTime::currentTime().msecsSinceStartOfDay();
  if ((m_timeOfLastSkipCheck > 0) && (now - m_timeOfLastSkipCheck < interval)) {
    return m_skip;
  }
  m_timeOfLastSkipCheck = now;

  QTime timer;
  timer.start();

  ZDvidUrl dvidUrl(m_bodyDoc->getDvidTarget());
  std::string tarUrl = dvidUrl.getTarSupervoxelsUrl(m_bodyId);
  int statusCode = 0;
  ZDvid::MakeHeadRequest(tarUrl, statusCode);
  m_skip = (statusCode != 200);

  LINFO() << "TaskFalseSplitReview::skip() HEAD took" << timer.elapsed() << "ms to decide to"
          << (m_skip ? "skip" : "not skip") << "body" << m_bodyId;

  // Record that the task was skipped.

  writeOutput();

  return m_skip;
}

void TaskFalseSplitReview::beforeNext()
{
  applyPerTaskSettings();

  // Clear the mesh cache when changing tasks so it does not grow without bound
  // during an assignment, which causes a performance degradation.  The assumption
  // is that improving performance as a user progresses through an assignment is
  // more important than eliminating the need to reload meshses if the user goes
  // back to a previous task.

  m_bodyDoc->clearGarbage(true);

  m_hiddenIds.clear();
}

void TaskFalseSplitReview::beforePrev()
{
  applyPerTaskSettings();

  // See the comment in beforeNext().

  m_bodyDoc->clearGarbage(true);

  m_hiddenIds.clear();
}

void TaskFalseSplitReview::beforeDone()
{
  restoreOverallSettings(m_bodyDoc);
}

QWidget *TaskFalseSplitReview::getTaskWidget()
{
  applyPerTaskSettings();
  return m_widget;
}

QMenu *TaskFalseSplitReview::getTaskMenu()
{
  return m_menu;
}

uint64_t TaskFalseSplitReview::getBodyId() const
{
  return m_bodyId;
}

void TaskFalseSplitReview::onShowSupervoxelsChanged(int state)
{
  bool show = (state != Qt::Unchecked);
  applyColorMode(show);
}

void TaskFalseSplitReview::onToggleShowSupervoxels()
{
  // For some reason, Qt will call this slot even when the source of the signal is disabled.
  // The source is a QAction on m_menu, and TaskProtocolWindow disables it and m_widget while
  // waiting for meshes from the previous task to e deleted and meshes for the next task to be
  // loaded.  In this case, going ahead and loading more meshes can cause problems
  // due to way meshes are deleted and loaded asynchronously.  Since this disabling does not
  // prevent this slot from being called, we must explicitly abort this slot when there is
  // disabling.

  if (!m_menu->isEnabled() || !m_widget->isEnabled()) {
    return;
  }
  m_showSupervoxelsCheckBox->setChecked(!m_showSupervoxelsCheckBox->isChecked());
}

void TaskFalseSplitReview::onHideSelected()
{
  const TStackObjectSet &selectedMeshes = m_bodyDoc->getSelected(ZStackObject::EType::MESH);
  for (auto itSelected = selectedMeshes.cbegin(); itSelected != selectedMeshes.cend(); itSelected++) {
    ZMesh *mesh = static_cast<ZMesh*>(*itSelected);
    m_hiddenIds.insert(mesh->getLabel());
  }
  updateVisibility();
}

void TaskFalseSplitReview::onClearHidden()
{
  selectBodies(m_hiddenIds);
  m_hiddenIds.clear();
  updateVisibility();
}

QJsonObject TaskFalseSplitReview::addToJson(QJsonObject taskJson)
{
  taskJson[KEY_BODY_ID] = static_cast<double>(m_bodyId);
  taskJson[KEY_TASKTYPE] = VALUE_TASKTYPE;

  return taskJson;
}

void TaskFalseSplitReview::onLoaded()
{
  LINFO() << "TaskFalseSplitReview: build version" << getBuildVersion() << ".";

  m_usageTimer.start();

  zoomToFitMeshes();
}

bool TaskFalseSplitReview::allowCompletion()
{
  bool allow = true;

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

void TaskFalseSplitReview::onCompleted()
{
  m_usageTimes.push_back(m_usageTimer.elapsed());

  // Restart the timer, to measure the time if the user reconsiders and
  // reaches a new decision for this task (without moving on and then coming
  // back to this task).

  m_usageTimer.start();

  writeOutput();
}

ProtocolTaskConfig TaskFalseSplitReview::getTaskConfig() const
{
  ProtocolTaskConfig config;
  config.setTaskType(taskType());
  config.setDefaultTodo(neutube::EToDoAction::TO_MERGE);

  return config;
}

void TaskFalseSplitReview::buildTaskWidget()
{
  m_widget = new QWidget();

  m_showSupervoxelsCheckBox = new QCheckBox("Show supervoxels", m_widget);
  connect(m_showSupervoxelsCheckBox, SIGNAL(stateChanged(int)), this, SLOT(onShowSupervoxelsChanged(int)));

  QVBoxLayout *layout = new QVBoxLayout;
  layout->addWidget(m_showSupervoxelsCheckBox);

  m_widget->setLayout(layout);

  m_menu = new QMenu("False Split Review", m_widget);

  QAction *showSupervoxelsAction = new QAction("Show Supervoxels", m_widget);
  showSupervoxelsAction->setShortcut(Qt::Key_C);
  m_menu->addAction(showSupervoxelsAction);
  connect(showSupervoxelsAction, SIGNAL(triggered()), this, SLOT(onToggleShowSupervoxels()));

  QAction *hideSelectedAction = new QAction("Hide Selected Meshes", m_widget);
  hideSelectedAction->setShortcut(Qt::Key_F1);
  m_menu->addAction(hideSelectedAction);
  connect(hideSelectedAction, SIGNAL(triggered()), this, SLOT(onHideSelected()));

  QAction *clearHiddenAction = new QAction("Clear Hidden Selected Meshes", m_widget);
  clearHiddenAction->setShortcut(Qt::Key_F2);
  m_menu->addAction(clearHiddenAction);
  connect(clearHiddenAction, SIGNAL(triggered()), this, SLOT(onClearHidden()));

  m_showSupervoxelsCheckBox->setChecked(true);
}

void TaskFalseSplitReview::updateColors()
{
  if (Z3DMeshFilter *filter = getMeshFilter(m_bodyDoc)) {
    filter->setColorIndexing(INDEX_COLORS, [=](uint64_t id) -> std::size_t {
      uint64_t tarBodyId = m_bodyDoc->getMappedId(id);
      return (tarBodyId == m_bodyId) ? 1 : 0;
    });

  }
}

void TaskFalseSplitReview::selectBodies(const std::set<uint64_t> &toSelect)
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

void TaskFalseSplitReview::applyPerTaskSettings()
{
  bool showingSupervoxels = m_showSupervoxelsCheckBox->isChecked();
  applyColorMode(showingSupervoxels);
}

void TaskFalseSplitReview::applyColorMode(bool showingSupervoxels)
{
  if (Z3DMeshFilter *filter = getMeshFilter(m_bodyDoc)) {
    if (showingSupervoxels) {
      filter->setColorMode("Mesh Source");
    } else {
      updateColors();
      filter->setColorMode("Indexed Color");
    }
  }
}

void TaskFalseSplitReview::updateVisibility()
{
  QList<ZMesh*> meshes = ZStackDocProxy::GetGeneralMeshList(m_bodyDoc);
  for (auto itMesh = meshes.cbegin(); itMesh != meshes.cend(); itMesh++) {
    ZMesh *mesh = *itMesh;
    uint64_t id = mesh->getLabel();
    bool toBeVisible = (m_hiddenIds.find(id) == m_hiddenIds.end());
    m_bodyDoc->setVisible(mesh, toBeVisible);
  }
}

void TaskFalseSplitReview::zoomToFitMeshes()
{
  Z3DMeshFilter *filter = getMeshFilter(m_bodyDoc);
  QList<ZMesh*> meshes = ZStackDocProxy::GetGeneralMeshList(m_bodyDoc);

  ZBBox<glm::dvec3> bbox;
  for (auto it = meshes.cbegin(); it != meshes.cend(); it++) {
    ZMesh *mesh = *it;
    uint64_t tarBodyId = m_bodyDoc->getMappedId(mesh->getLabel());
    if (tarBodyId == m_bodyId) {
      bbox.expand(mesh->boundBox());
    }
  }
  glm::dvec3 c = (bbox.minCorner() + bbox.maxCorner()) / 2.0;
  ZPoint center(c.x, c.y, c.z);

  double radius = 0;
  std::vector<std::vector<glm::vec3>> vertices{ std::vector<glm::vec3>() };

  const size_t stride = 3;

  for (auto it = meshes.cbegin(); it != meshes.cend(); it++) {
    ZMesh *mesh = *it;
    uint64_t tarBodyId = m_bodyDoc->getMappedId(mesh->getLabel());
    if (tarBodyId == m_bodyId) {
      for (size_t i = 0; i < mesh->vertices().size(); i++) {
        glm::vec3 vertex = mesh->vertices()[i];
        double r = center.distanceTo(vertex.x, vertex.y, vertex.z);
        radius = std::max(r, radius);

        // To improve performance, use only a fraction of the vertices
        // when determining the zoom.

        if (i % stride == 0) {
          vertices.back().push_back(vertex);
        }
      }
    }
  }

  Z3DCameraUtils::resetCamera(center, radius, filter->camera());
  Z3DCameraUtils::tightenZoom(vertices, filter->camera());

  // Adjust the near clipping plane to accomodate all the meshes, then rerender.

  filter->camera().resetCameraNearFarPlane(bbox);
  filter->invalidate();
}

void TaskFalseSplitReview::displayWarning(const QString &title, const QString &text,
                                          const QString& details, bool allowSuppression)
{
  // Display the warning at idle time, after the rendering event has been processed.

  QTimer::singleShot(0, this, [=](){
    if (details.isEmpty() && !allowSuppression) {
      ZWidgetMessage msg(title, text, neutube::EMessageType::WARNING, ZWidgetMessage::TARGET_DIALOG);
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

void TaskFalseSplitReview::writeOutput()
{
  ZDvidReader reader;
  reader.setVerbose(false);
  if (!reader.open(m_bodyDoc->getDvidTarget())) {
    LERROR() << "TaskFalseSplitReview::writeOutput() could not open DVID target for reading";
    return;
  }

  ZDvidWriter writer;
  if (!writer.open(m_bodyDoc->getDvidTarget())) {
    LERROR() << "TaskFalseSplitReview::writeOutput() could not open DVID target for writing";
    return;
  }

  std::string instance = getOutputInstanceName(m_bodyDoc->getDvidTarget());
  if (!reader.hasData(instance)) {
    writer.createKeyvalue(instance);
  }
  if (!reader.hasData(instance)) {
    LERROR() << "TaskFalseSplitReview::writeOutput() could not create DVID instance \"" << instance << "\"";
    return;
  }

  QJsonObject json;

  json[KEY_SKIPPED] = QJsonValue(m_skip);
  if (const char* user = std::getenv("USER")) {
    json[KEY_USER] = user;
  }
  json[KEY_BODY_ID] = QJsonValue(qint64(m_bodyId));
  json[KEY_TIMESTAMP] = QDateTime::currentDateTime().toString(Qt::ISODate);
  json[KEY_TIME_ZONE] = QDateTime::currentDateTime().timeZoneAbbreviation();
  json[KEY_SOURCE] = jsonSource();
  json[KEY_BUILD_VERSION] = getBuildVersion().c_str();

  QJsonArray jsonTimes;
  std::copy(m_usageTimes.begin(), m_usageTimes.end(), std::back_inserter(jsonTimes));
  json[KEY_USAGE_TIME] = jsonTimes;

  std::vector<ZFlyEmToDoItem*> todos = m_bodyDoc->getDataDocument()->getTodoItem(m_bodyId);
  int count = std::accumulate(todos.begin(), todos.end(), 0, [](int a, ZFlyEmToDoItem* b) {
    return a + (b->getAction() == neutube::EToDoAction::TO_MERGE);
  });
  json[KEY_TODO_COUNT] = QJsonValue(count);

  QJsonDocument jsonDoc(json);
  std::string jsonStr(jsonDoc.toJson(QJsonDocument::Compact).toStdString());
  std::string key = std::to_string(m_bodyId);
  writer.writeJsonString(instance, key, jsonStr);
}

bool TaskFalseSplitReview::loadSpecific(QJsonObject json)
{
  if (json.contains(KEY_BODY_ID)) {
    m_bodyId = json[KEY_BODY_ID].toDouble();
  } else {
    return false;
  }

  m_visibleBodies.insert(ZFlyEmBodyManager::encode(m_bodyId, 0));

  return true;
}

