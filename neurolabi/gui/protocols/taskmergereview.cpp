#include "taskmergereview.h"

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
#include <QRadioButton>
#include <QTimer>
#include <QUndoCommand>
#include <QUrl>
#include <QVBoxLayout>

namespace {

  static const QString KEY_TASKTYPE = "task type";
  static const QString VALUE_TASKTYPE = "merge review";
  static const QString KEY_SUPERVOXEL_IDS = "supervoxel IDs";
  static const QString KEY_ASSIGNED_USER = "assigned user";

  // For the result JSON.
  static const QString KEY_SKIPPED = "skipped";
  static const QString VALUE_SKIPPED_MAPPING = "mapping to bodies failed";
  static const QString VALUE_SKIPPED_SIZES = "zero size for supervoxel";
  static const QString VALUE_SKIPPED_MESHES = "meshes unavailable";
  static const QString KEY_RESULT = "result";
  static const QString VALUE_RESULT_MERGE = "merge";
  static const QString VALUE_RESULT_DONT_MERGE = "dontMerge";
  static const QString VALUE_RESULT_PROCESS_FURTHER = "processFurther";
  static const QString VALUE_RESULT_IRRELEVANT = "irrelevant";
  static const QString VALUE_RESULT_DONT_KNOW = "?";
  static const QString KEY_USER = "user";
  static const QString KEY_TIMESTAMP = "time";
  static const QString KEY_TIME_ZONE = "time zone";
  static const QString KEY_SOURCE = "source";
  static const QString KEY_BUILD_VERSION = "build version";
  static const QString KEY_USAGE_TIME = "time to complete (ms)";
  static const QString KEY_TODO_COUNT = "split todo count";

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
    return dvidTarget.getBodyLabelName() + "_mergeReview";
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

  // All the TaskMergeReview instances loaded from one JSON file need certain changes
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

TaskMergeReview::TaskMergeReview(QJsonObject json, ZFlyEmBody3dDoc* bodyDoc)
{
  m_bodyDoc = bodyDoc;

  applyOverallSettings(bodyDoc);

  loadJson(json);

  buildTaskWidget();
}

QString TaskMergeReview::taskTypeStatic()
{
  return VALUE_TASKTYPE;
}

TaskMergeReview* TaskMergeReview::createFromJson(QJsonObject json, ZFlyEmBody3dDoc *bodyDoc)
{
  return new TaskMergeReview(json, bodyDoc);
}

QString TaskMergeReview::taskType() const
{
  return taskTypeStatic();
}

QString TaskMergeReview::actionString()
{
  return "Merge review:";
}

QString TaskMergeReview::targetString()
{
  QString result = "none";
  if (m_superVoxelIds.size() > 0) {
    result = QString::number(*(m_superVoxelIds.begin())) + "+" +
        QString::number(m_superVoxelIds.size() - 1);
  }
  return result;
}

bool TaskMergeReview::skip()
{
  // For now, at least, the "HEAD" command to check whether a tarsupervoxels instance has
  // a complete tar archive may be slow for large bodies.  So avoid executing it repeatedly
  // in rapid succession.

  // An environment variable can override the interval for checking, with a value of
  // -1 meaning never check.

  int interval = 1000;
  if (const char* overrideIntervalStr = std::getenv("NEU3_MERGE_REVIEW_SKIP_TEST_INTERVAL_MS")) {
    interval = std::atoi(overrideIntervalStr);
  }
  if (interval < 0) {
    return false;
  }

  int now = QTime::currentTime().msecsSinceStartOfDay();
  if ((m_timeOfLastSkipCheck > 0) && (now - m_timeOfLastSkipCheck < interval)) {
    return (m_skip != Skip::NOT_SKIPPED);
  }
  m_timeOfLastSkipCheck = now;

  QTime timer;
  timer.start();

  m_skip = Skip::NOT_SKIPPED;

  SetBodiesResult setBodiesResult = setBodiesFromSuperVoxels();
  if (setBodiesResult != SetBodiesResult::SUCCEEDED) {
    m_skip = (setBodiesResult == SetBodiesResult::FAILED_MAPPING) ?
          Skip::SKIPPED_MAPPING : Skip::SKIPPED_SIZES;
  }

  if (m_skip == Skip::NOT_SKIPPED) {
    ZDvidUrl dvidUrl(m_bodyDoc->getDvidTarget());
    for (uint64_t id : m_bodyIds) {
      std::string tarUrl = dvidUrl.getTarSupervoxelsUrl(id);
      int statusCode = 0;
      ZDvid::MakeHeadRequest(tarUrl, statusCode);
      if (statusCode != 200) {
        m_skip = Skip::SKIPPED_MESHES;
        break;
      }
    }
  }

  LINFO() << "TaskMergeReview::skip() HEAD took" << timer.elapsed() << "ms to decide to"
          << ((m_skip == Skip::NOT_SKIPPED) ? "not skip" : "skip") << "bodies" << targetString();

  // Record that the task was skipped.

  if (m_skip != Skip::NOT_SKIPPED) {
    m_lastSavedButton = nullptr;
    writeOutput();
  }

  return (m_skip != Skip::NOT_SKIPPED);
}

void TaskMergeReview::beforeNext()
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

void TaskMergeReview::beforePrev()
{
  applyPerTaskSettings();

  // See the comment in beforeNext().

  m_bodyDoc->clearGarbage(true);

  m_hiddenIds.clear();
}

void TaskMergeReview::beforeDone()
{
  restoreOverallSettings(m_bodyDoc);
}

QWidget *TaskMergeReview::getTaskWidget()
{
  // It's possible that the bodies of the super voxels were merged together via
  // in another user's session.  So get the bodies from DVID again, to update the
  // result computed by skip().

  if (m_bodyIds.empty()) {
    setBodiesFromSuperVoxels();
  }

  if (m_visibleBodies.isEmpty()) {
    for (uint64_t id : m_bodyIds) {
      m_visibleBodies.insert(ZFlyEmBodyManager::encode(id, 0));
    }
  }

  if (!m_lastSavedButton) {
    QString result = readResult();
    if (!result.isEmpty()) {
      restoreResult(result);
    }
  }

  applyPerTaskSettings();

  m_bodyToSelect = m_bodyIds.cbegin();
  updateSelectCurrentBodyButton();

  return m_widget;
}

QMenu *TaskMergeReview::getTaskMenu()
{
  return m_menu;
}

const std::set<uint64_t>& TaskMergeReview::getBodyIds() const
{
  return m_bodyIds;
}

void TaskMergeReview::onCycleAnswer()
{
  if (m_dontMergeButton->isChecked()) {
    m_mergeButton->setChecked(true);
  } else if (m_mergeButton->isChecked()) {
    m_processFurtherButton->setChecked(true);
  } else if (m_processFurtherButton->isChecked()) {
    m_dontMergeButton->setChecked(true);
  }
}

void TaskMergeReview::onButtonToggled()
{
  updateColors();

  if (m_lastSavedButton) {
    if (!m_lastSavedButton->isChecked()) {
#if defined(Q_OS_DARWIN)
      m_lastSavedButton->setStyleSheet("QRadioButton { color: red }");
#else
      m_lastSavedButton->setStyleSheet("QRadioButton { color: red; border: none }");
#endif
    } else {
      m_lastSavedButton->setStyleSheet("");
    }
  }
}

void TaskMergeReview::onSelectCurrentBody()
{
  std::set<uint64_t> toSelect;
  QList<ZMesh*> meshes = ZStackDocProxy::GetGeneralMeshList(m_bodyDoc);
  for (auto it = meshes.cbegin(); it != meshes.cend(); it++) {
    ZMesh *mesh = *it;
    uint64_t tarBodyId = m_bodyDoc->getMappedId(mesh->getLabel());
    if (tarBodyId == *m_bodyToSelect) {
      toSelect.insert(mesh->getLabel());
    }
  }

  selectBodies(toSelect);
}

void TaskMergeReview::onNextBodyToSelect()
{
  m_bodyToSelect++;
  if (m_bodyToSelect == m_bodyIds.cend()) {
    m_bodyToSelect = m_bodyIds.cbegin();
  }
  updateSelectCurrentBodyButton();
  onSelectCurrentBody();
}

void TaskMergeReview::onPrevBodyToSelect()
{
  if (m_bodyToSelect == m_bodyIds.cbegin()) {
    m_bodyToSelect = m_bodyIds.cend();
  }
  m_bodyToSelect--;
  updateSelectCurrentBodyButton();
  onSelectCurrentBody();
}

void TaskMergeReview::onShowSupervoxelsChanged(int state)
{
  bool show = (state != Qt::Unchecked);
  applyColorMode(show);
}

void TaskMergeReview::onToggleShowSupervoxels()
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

void TaskMergeReview::zoomOutToShowAll()
{
  zoomToFitMeshes();
}

void TaskMergeReview::onHideSelected()
{
  const TStackObjectSet &selectedMeshes = m_bodyDoc->getSelected(ZStackObject::TYPE_MESH);
  for (auto itSelected = selectedMeshes.cbegin(); itSelected != selectedMeshes.cend(); itSelected++) {
    ZMesh *mesh = static_cast<ZMesh*>(*itSelected);
    m_hiddenIds.insert(mesh->getLabel());
  }
  updateVisibility();

  std::set<uint64_t> selectedBodies;
   for (auto itSelected = selectedMeshes.cbegin(); itSelected != selectedMeshes.cend(); itSelected++) {
     selectedBodies.insert((*itSelected)->getLabel());
   }
   selectBodies(selectedBodies, false);
}

void TaskMergeReview::onClearHidden()
{
  selectBodies(m_hiddenIds);
  m_hiddenIds.clear();
  updateVisibility();
}

void TaskMergeReview::onToggleIsolation()
{
  m_hiddenIds.clear();

  bool isolate = false;
  const TStackObjectSet &selectedMeshes = m_bodyDoc->getSelected(ZStackObject::TYPE_MESH);
  if (!selectedMeshes.isEmpty()) {
    QList<ZMesh*> meshes = ZStackDocProxy::GetGeneralMeshList(m_bodyDoc);
    for (auto it = meshes.cbegin(); it != meshes.cend(); it++) {
      ZMesh *mesh = *it;
      if (mesh->isVisible()) {
        if (!selectedMeshes.contains(mesh)) {

          // If there is a visible mesh that is not a selected mesh, then visiblilty is not
          // isolated to the selected meshes, and isolation needs to be performed.

          isolate = true;
          break;
        }
      }
    }

    if (isolate) {
      for (auto it = meshes.cbegin(); it != meshes.cend(); it++) {
        ZMesh *mesh = *it;
        if (!selectedMeshes.contains(mesh)) {
          m_hiddenIds.insert(mesh->getLabel());
        }
      }
    }
  }

  updateVisibility();
}

TaskMergeReview::SetBodiesResult TaskMergeReview::setBodiesFromSuperVoxels()
{
  std::string instance = m_bodyDoc->getDvidTarget().getBodyLabelName();
  ZDvidUrl url(m_bodyDoc->getDvidTarget());
  std::string urlMapping = url.getNodeUrl() + "/" + instance + "/mapping";

  // Use the DVID "mapping" endpoint for the "labelmap" data type to find the body IDs
  // corresponding to the supervoxel IDs.

  ZJsonArray jsonBody;
  for (uint64_t id : m_superVoxelIds) {
    jsonBody.append(id);
  }

  std::string payloadStr = jsonBody.dumpString(0);
  libdvid::BinaryDataPtr payload =
      libdvid::BinaryData::create_binary_data(payloadStr.c_str(), payloadStr.size());
  int statusCode;

  m_bodyIds.clear();

  libdvid::BinaryDataPtr response = ZDvid::MakeRequest(urlMapping, "GET", payload, libdvid::DEFAULT, statusCode);
  if (statusCode == 200) {
    QJsonDocument responseDoc = QJsonDocument::fromJson(response->get_data().c_str());
    if (responseDoc.isArray())  {
      QJsonArray responseArray = responseDoc.array();
      for (QJsonValue responseElem : responseArray) {
        if (!responseElem.isUndefined()) {
          uint64_t id = responseElem.toDouble();
          if (id == 0) {
            return SetBodiesResult::FAILED_MAPPING;
          }
          m_bodyIds.insert(id);
        }
      }
    }
  } else {
    LWARN() << "TaskMergeReview::setBodiesFromSuperVoxels() failed for" << targetString()
            << ": supervoxel ID mapping failed (DVID status" << statusCode << ")";
    return SetBodiesResult::FAILED_MAPPING;
  }

  // As of November, 2018, there are cases of old supervoxels that may no longer exist
  // (due to merging) yet "mapping" may not return 0 (due to some problems with older data).
  // A work-around is to use the "sizes" endpoint to see if any supervoxels have no size.
  // At some point, removing this work-around would be a good idea, to improve performance.
  // See: https://github.com/janelia-flyem/dvid/issues/289

  QTime timer;
  timer.start();

  statusCode = 0;
  std::string urlSizes = url.getNodeUrl() + "/" + instance + "/sizes?supervoxels=true";
  response = ZDvid::MakeRequest(urlSizes, "GET", payload, libdvid::DEFAULT, statusCode);
  if (statusCode == 200) {
    QJsonDocument responseDoc = QJsonDocument::fromJson(response->get_data().c_str());
    if (responseDoc.isArray())  {
      QJsonArray responseArray = responseDoc.array();
      for (QJsonValue responseElem : responseArray) {
        if (!responseElem.isUndefined()) {
          if (responseElem.toDouble() == 0) {
            return SetBodiesResult::FAILED_SIZES;
          }
        }
      }
    }
  } else {
    LWARN() << "TaskMergeReview::setBodiesFromSuperVoxels() failed for" << targetString()
            << ": supervoxel sizes could not be verified (DVID status" << statusCode << ")";
    return SetBodiesResult::FAILED_SIZES;
  }

  LINFO() << "TaskBodyMerge: checking sizes for supervoxels" << targetString()
          << "took" << timer.elapsed() << "ms.";

  m_bodyToSelect = m_bodyIds.cbegin();

  return SetBodiesResult::SUCCEEDED;
}

void TaskMergeReview::buildTaskWidget()
{
  m_widget = new QWidget();

  m_dontMergeButton = new QRadioButton("Don't Merge", m_widget);
  m_dontMergeButton->setChecked(true);
  connect(m_dontMergeButton, SIGNAL(toggled(bool)), this, SLOT(onButtonToggled()));

  m_mergeButton = new QRadioButton("Merge", m_widget);
  connect(m_mergeButton, SIGNAL(toggled(bool)), this, SLOT(onButtonToggled()));

  m_processFurtherButton = new QRadioButton("Process Further", m_widget);
  connect(m_processFurtherButton, SIGNAL(toggled(bool)), this, SLOT(onButtonToggled()));

  m_irrelevantButton = new QRadioButton("Irrelevant", m_widget);
  connect(m_irrelevantButton, SIGNAL(toggled(bool)), this, SLOT(onButtonToggled()));

  m_dontKnowButton = new QRadioButton("Don't Know", m_widget);
  connect(m_dontKnowButton, SIGNAL(toggled(bool)), this, SLOT(onButtonToggled()));

  // Points to the button corresonding to the result most recently saved to DVID,
  // to support feedback of the need to save a changed result.

  m_lastSavedButton = nullptr;

  QHBoxLayout *radioTopLayout = new QHBoxLayout;
  radioTopLayout->addWidget(m_dontMergeButton);
  radioTopLayout->addWidget(m_mergeButton);
  radioTopLayout->addWidget(m_processFurtherButton);
  QHBoxLayout *radioBottomLayout = new QHBoxLayout;
  radioBottomLayout->addWidget(m_irrelevantButton);
  radioBottomLayout->addWidget(m_dontKnowButton);

  m_selectCurrentBodyButton = new QPushButton("Select", m_widget);
  connect(m_selectCurrentBodyButton, SIGNAL(clicked(bool)), this, SLOT(onSelectCurrentBody()));

  m_nextBodyToSelectButton = new QPushButton("Select Next", m_widget);
  connect(m_nextBodyToSelectButton, SIGNAL(clicked(bool)), this, SLOT(onNextBodyToSelect()));

  m_prevBodyToSelectButton = new QPushButton("Select Previous", m_widget);
  connect(m_prevBodyToSelectButton, SIGNAL(clicked(bool)), this, SLOT(onPrevBodyToSelect()));

  QHBoxLayout *selectionLayout = new QHBoxLayout;
  selectionLayout->addStretch();
  selectionLayout->addWidget(m_prevBodyToSelectButton);
  selectionLayout->addWidget(m_selectCurrentBodyButton);
  selectionLayout->addWidget(m_nextBodyToSelectButton);
  selectionLayout->addStretch();

  m_showSupervoxelsCheckBox = new QCheckBox("Show supervoxels", m_widget);
  connect(m_showSupervoxelsCheckBox, SIGNAL(stateChanged(int)), this, SLOT(onShowSupervoxelsChanged(int)));

  QPushButton *zoomOutButton = new QPushButton("Zoom Out to Show All", m_widget);
  connect(zoomOutButton, SIGNAL(clicked(bool)), this, SLOT(zoomOutToShowAll()));

  QHBoxLayout *bottomLayout = new QHBoxLayout;
  bottomLayout->addWidget(m_showSupervoxelsCheckBox);
  bottomLayout->addWidget(zoomOutButton);

  QVBoxLayout *layout = new QVBoxLayout;
  layout->addLayout(radioTopLayout);
  layout->addLayout(radioBottomLayout);
  layout->addLayout(selectionLayout);
  layout->addLayout(bottomLayout);

  m_widget->setLayout(layout);

  m_menu = new QMenu("Merge Review", m_widget);

  QAction *cycleAnswerAction = new QAction("Cycle Through Answers", m_widget);
  cycleAnswerAction->setShortcut(Qt::Key_V);
  m_menu->addAction(cycleAnswerAction);
  connect(cycleAnswerAction, SIGNAL(triggered()), this, SLOT(onCycleAnswer()));

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

  QAction *isolateAction = new QAction("Toggle Isolation of Selected Meshes", m_widget);
  isolateAction->setShortcut(Qt::Key_F3);
  m_menu->addAction(isolateAction);
  connect(isolateAction, SIGNAL(triggered()), this, SLOT(onToggleIsolation()));
}

void TaskMergeReview::updateColors()
{
  if (Z3DMeshFilter *filter = getMeshFilter(m_bodyDoc)) {
    m_bodyIdToColorIndex.clear();
    std::size_t index = 1;
    for (uint64_t id : m_bodyIds) {
      m_bodyIdToColorIndex[id] = index;
      index++;
      if (index == INDEX_COLORS.size()) {
        index = 1;
      }
    }

    filter->setColorIndexing(INDEX_COLORS, [=](uint64_t id) -> std::size_t {
      uint64_t tarBodyId = m_bodyDoc->getMappedId(id);
      auto it = m_bodyIds.find(tarBodyId);
      std::size_t result = 0;
      if (it != m_bodyIds.end()) {
        result = m_mergeButton->isChecked() ? 1 : m_bodyIdToColorIndex.at(*it);
      }
      return result;
    });

  }
}

void TaskMergeReview::updateSelectCurrentBodyButton()
{
  uint64_t id = *m_bodyToSelect;

  QSize iconSizeDefault = m_selectCurrentBodyButton->iconSize();
  QSize iconSize = iconSizeDefault * 0.8;
  QPixmap pixmap(iconSize);
  glm::vec4 color = 255.0f * INDEX_COLORS[m_bodyIdToColorIndex[id]];
  pixmap.fill(QColor(color.r, color.g, color.b, color.a));
  QIcon icon(pixmap);
  m_selectCurrentBodyButton->setIcon(icon);

  m_selectCurrentBodyButton->setText("Select " + QString::number(id));
}

void TaskMergeReview::selectBodies(const std::set<uint64_t> &bodies, bool select)
{
  if (select) {
    m_bodyDoc->deselectAllMesh();
  }

  QList<ZMesh*> meshes = ZStackDocProxy::GetGeneralMeshList(m_bodyDoc);
  for (auto it = meshes.cbegin(); it != meshes.cend(); it++) {
    ZMesh *mesh = *it;
    if (bodies.find(mesh->getLabel()) != bodies.end()) {
      m_bodyDoc->setMeshSelected(mesh, select);
    }
  }
}

void TaskMergeReview::applyPerTaskSettings()
{
  bool showingSupervoxels = m_showSupervoxelsCheckBox->isChecked();
  applyColorMode(showingSupervoxels);
}

void TaskMergeReview::applyColorMode(bool showingSupervoxels)
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

void TaskMergeReview::updateVisibility()
{
  QList<ZMesh*> meshes = ZStackDocProxy::GetGeneralMeshList(m_bodyDoc);
  for (auto itMesh = meshes.cbegin(); itMesh != meshes.cend(); itMesh++) {
    ZMesh *mesh = *itMesh;
    uint64_t id = mesh->getLabel();
    bool toBeVisible = (m_hiddenIds.find(id) == m_hiddenIds.end());

    // Set the visibility of the mesh in a way that will be processed once, with the final
    // processObjectModified() call.  This approach is important to maintain good performance
    // for a large number of meshes.

    mesh->setVisible(toBeVisible);
    m_bodyDoc->bufferObjectModified(mesh, ZStackObjectInfo::STATE_VISIBITLITY_CHANGED);
  }

  m_bodyDoc->processObjectModified();
}

void TaskMergeReview::zoomToFitMeshes()
{
  Z3DMeshFilter *filter = getMeshFilter(m_bodyDoc);
  QList<ZMesh*> meshes = ZStackDocProxy::GetGeneralMeshList(m_bodyDoc);

  ZBBox<glm::dvec3> bbox;
  for (auto it = meshes.cbegin(); it != meshes.cend(); it++) {
    ZMesh *mesh = *it;
    uint64_t tarBodyId = m_bodyDoc->getMappedId(mesh->getLabel());
    if (m_bodyIds.find(tarBodyId) != m_bodyIds.end()) {
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
    if (m_bodyIds.find(tarBodyId) != m_bodyIds.end()) {
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

void TaskMergeReview::displayWarning(const QString &title, const QString &text,
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

std::string TaskMergeReview::outputKey() const
{
  std::string result;
  uint64_t sum = 0;
  for (uint64_t id : m_superVoxelIds) {
    if (result.empty()) {
      result = std::to_string(id);
    } else {
      sum += id;
    }
  }
  result += "." + std::to_string(sum);
  return result;
}

void TaskMergeReview::writeOutput()
{
  ZDvidReader reader;
  reader.setVerbose(false);
  if (!reader.open(m_bodyDoc->getDvidTarget())) {
    LERROR() << "TaskMergeReview::writeOutput() could not open DVID target for reading";
    return;
  }

  ZDvidWriter writer;
  if (!writer.open(m_bodyDoc->getDvidTarget())) {
    LERROR() << "TaskMergeReview::writeOutput() could not open DVID target for writing";
    return;
  }

  std::string instance = getOutputInstanceName(m_bodyDoc->getDvidTarget());
  if (!reader.hasData(instance)) {
    writer.createKeyvalue(instance);
  }
  if (!reader.hasData(instance)) {
    LERROR() << "TaskMergeReview::writeOutput() could not create DVID instance \"" << instance << "\"";
    return;
  }

  QJsonObject json;

  if (m_lastSavedButton) {
    m_lastSavedButton->setStyleSheet("");
  }

  switch (m_skip) {
    case Skip::SKIPPED_MAPPING:
      json[KEY_SKIPPED] = QJsonValue(VALUE_SKIPPED_MAPPING);
      break;
    case Skip::SKIPPED_SIZES:
      json[KEY_SKIPPED] = QJsonValue(VALUE_SKIPPED_SIZES);
      break;
    case Skip::SKIPPED_MESHES:
      json[KEY_SKIPPED] = QJsonValue(VALUE_SKIPPED_MESHES);
      break;
    default:
      if (m_mergeButton->isChecked()) {
        json[KEY_RESULT] = QJsonValue(VALUE_RESULT_MERGE);
        m_lastSavedButton = m_mergeButton;
      } else if (m_dontMergeButton->isChecked()) {
        json[KEY_RESULT] = QJsonValue(VALUE_RESULT_DONT_MERGE);
        m_lastSavedButton = m_dontMergeButton;
      } else if (m_processFurtherButton->isChecked()) {
        json[KEY_RESULT] = QJsonValue(VALUE_RESULT_PROCESS_FURTHER);
        m_lastSavedButton = m_processFurtherButton;
      } else if (m_irrelevantButton->isChecked()) {
        json[KEY_RESULT] = QJsonValue(VALUE_RESULT_IRRELEVANT);
        m_lastSavedButton = m_irrelevantButton;
      } else {
        json[KEY_RESULT] = QJsonValue(VALUE_RESULT_DONT_KNOW);
        m_lastSavedButton = m_dontKnowButton;
      }
  }

  if (const char* user = std::getenv("USER")) {
    json[KEY_USER] = user;
  }
  QJsonArray array;
  for (uint64_t id : m_superVoxelIds) {
    array.append(static_cast<double>(id));
  }
  json[KEY_SUPERVOXEL_IDS] = array;
  json[KEY_TIMESTAMP] = QDateTime::currentDateTime().toString(Qt::ISODate);
  json[KEY_TIME_ZONE] = QDateTime::currentDateTime().timeZoneAbbreviation();
  json[KEY_SOURCE] = jsonSource();
  json[KEY_BUILD_VERSION] = getBuildVersion().c_str();

  QJsonArray jsonTimes;
  std::copy(m_usageTimes.begin(), m_usageTimes.end(), std::back_inserter(jsonTimes));
  json[KEY_USAGE_TIME] = jsonTimes;

  int allTodoCount = 0;
  for (uint64_t id : m_bodyIds) {
    std::vector<ZFlyEmToDoItem*> todos = m_bodyDoc->getDataDocument()->getTodoItem(id);
    allTodoCount += std::accumulate(todos.begin(), todos.end(), 0, [](int a, ZFlyEmToDoItem* b) {
      return a + (b->getAction() == neutube::EToDoAction::TO_SPLIT);
    });
  }
  json[KEY_TODO_COUNT] = QJsonValue(allTodoCount);

  QJsonDocument jsonDoc(json);
  std::string jsonStr(jsonDoc.toJson(QJsonDocument::Compact).toStdString());
  std::string key = outputKey();
  writer.writeJsonString(instance, key, jsonStr);
}

QString TaskMergeReview::readResult() const
{
  ZDvidReader reader;
  reader.setVerbose(false);
  if (reader.open(m_bodyDoc->getDvidTarget())) {
    std::string instance = getOutputInstanceName(m_bodyDoc->getDvidTarget());
    if (reader.hasData(instance)) {
      std::string key = outputKey();
      ZJsonObject valueObj = reader.readJsonObjectFromKey(instance.c_str(), key.c_str());
      if (valueObj.isObject()) {
        const char *skippedKey = KEY_SKIPPED.toLatin1().data();
        if (valueObj.hasKey(skippedKey)) {
          return QString();
        }
        const char *resultKey = KEY_RESULT.toLatin1().data();
        if (valueObj.hasKey(resultKey)) {
          ZJsonValue resultValue = valueObj.value(resultKey);
          if (resultValue.isString()) {
            return QString(resultValue.toString().c_str());
          }
        }
      }
    }
  }
  return QString();
}

void TaskMergeReview::restoreResult(QString result)
{
  if (result == VALUE_RESULT_MERGE) {
    m_mergeButton->setChecked(true);
    m_lastSavedButton = m_mergeButton;
  } else if (result == VALUE_RESULT_DONT_MERGE) {
    m_dontMergeButton->setChecked(true);
    m_lastSavedButton = m_dontMergeButton;
  } else if (result == VALUE_RESULT_PROCESS_FURTHER) {
    m_processFurtherButton->setChecked(true);
    m_lastSavedButton = m_processFurtherButton;
  } else if (result == VALUE_RESULT_IRRELEVANT) {
    m_irrelevantButton->setChecked(true);
    m_lastSavedButton = m_irrelevantButton;
  } else if (result == VALUE_RESULT_DONT_KNOW) {
    m_dontKnowButton->setChecked(true);
    m_lastSavedButton = m_dontKnowButton;
  } else {

    // If a task saved as being skipped, and then stops being skipped, present it
    // the way it would be the first time a user would see any task.

    m_dontMergeButton->setChecked(true);
    m_lastSavedButton = m_dontMergeButton;
  }
}

bool TaskMergeReview::loadSpecific(QJsonObject json)
{
  if (json.contains(KEY_SUPERVOXEL_IDS)) {
    QJsonValue value = json[KEY_SUPERVOXEL_IDS];
    if (value.isArray()) {
      QJsonArray array = value.toArray();
      for (QJsonValue idValue : array) {
        if (idValue.isDouble()) {
          m_superVoxelIds.insert(idValue.toDouble());
        }
      }
    }
  } else {
    return false;
  }

  return true;
}

QJsonObject TaskMergeReview::addToJson(QJsonObject taskJson)
{
  QJsonArray array;
  for (uint64_t id : m_superVoxelIds) {
    array.append(static_cast<double>(id));
  }
  taskJson[KEY_SUPERVOXEL_IDS] = array;
  taskJson[KEY_TASKTYPE] = VALUE_TASKTYPE;

  return taskJson;
}

void TaskMergeReview::onLoaded()
{
  LINFO() << "TaskMergeReview: build version" << getBuildVersion() << ".";

  m_usageTimer.start();

  zoomToFitMeshes();
}

bool TaskMergeReview::allowCompletion()
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

void TaskMergeReview::onCompleted()
{
  m_usageTimes.push_back(m_usageTimer.elapsed());

  // Restart the timer, to measure the time if the user reconsiders and
  // reaches a new decision for this task (without moving on and then coming
  // back to this task).

  m_usageTimer.start();

  writeOutput();
}

ProtocolTaskConfig TaskMergeReview::getTaskConfig() const
{
  ProtocolTaskConfig config;
  config.setTaskType(taskType());
  config.setDefaultTodo(neutube::EToDoAction::TO_SPLIT);

  return config;
}
