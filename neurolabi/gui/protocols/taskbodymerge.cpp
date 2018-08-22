#include "taskbodymerge.h"

#include "dvid/zdvidtarget.h"
#include "flyem/zflyembody3ddoc.h"
#include "flyem/zflyembodyconfig.h"
#include "flyem/zflyemproofdoc.h"
#include "flyem/zflyemproofmvc.h"
#include "misc/miscutility.h"
#include "neutubeconfig.h"
#include "neu3window.h"
#include "z3dcamera.h"
#include "z3dcanvas.h"
#include "z3dmeshfilter.h"
#include "z3dview.h"
#include "z3dwindow.h"
#include "zdvidutil.h"
#include "zintcuboid.h"
#include "zstackdocproxy.h"
#include "zintpoint.h"

#include <limits>
#include <random>

#include <sys/types.h>
#include <dirent.h>

#include <QCheckBox>
#include <QDateTime>
#include <QDockWidget>
#include <QHBoxLayout>
#include <QMessageBox>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QPointer>
#include <QPushButton>
#include <QRadioButton>
#include <QSet>
#include <QUrl>
#include <QVBoxLayout>

namespace {

  static const QString KEY_TASKTYPE = "task type";
  static const QString VALUE_TASKTYPE = "body merge";
  static const QString KEY_SUPERVOXEL_ID1 = "supervoxel ID 1";
  static const QString KEY_SUPERVOXEL_ID2 = "supervoxel ID 2";
  static const QString KEY_SUPERVOXEL_POINT1 = "supervoxel point 1";
  static const QString KEY_SUPERVOXEL_POINT2 = "supervoxel point 2";

  // For the result JSON.
  static const QString KEY_RESULT = "result";
  static const QString KEY_USER = "user";
  static const QString KEY_BODY_ID1 = "body ID 1";
  static const QString KEY_BODY_ID2 = "body ID 2";
  static const QString KEY_TIMESTAMP = "time";
  static const QString KEY_TIME_ZONE = "time zone";
  static const QString KEY_SOURCE = "source";
  static const QString KEY_BUILD_VERSION = "build version";
  static const QString KEY_USAGE_TIME = "time to complete (ms)";
  static const QString KEY_RESULT_HISTORY = "result history";
  static const QString KEY_INITIAL_ANGLE_METHOD = "initial 3D angle method";

  // TODO: Duplicated in TaskBodyCleave, so factor out.
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

  static const std::vector<QString> INITIAL_ANGLE_METHOD({
    "method 0 (unchanged from previous task)",
    "method 1 (normal to acquisition axis Z)",
    "method 2 (normal to SV pts cross up when loaded)"
  });

  size_t initialAngleMethod()
  {
#if 1
    // FOR DEBUGGING: Force the prefered initial angle method.
    return 2;
#endif

    if (const char* method = std::getenv("NEU3_INITIAL_ANGLE_METHOD")) {
      try {
        size_t i = std::stoul(method);
        if (i <  INITIAL_ANGLE_METHOD.size()) {
          return i;
        }
      } catch(...) {
      }
    }

    static std::random_device r;
    static std::default_random_engine e(r());
    static std::uniform_int_distribution<int> uniform(1, INITIAL_ANGLE_METHOD.size() - 1);
    return uniform(e);
  }

  bool pointFromJSON(const QJsonValue &value, ZPoint &result)
  {
    if (value.isArray()) {
      QJsonArray array = value.toArray();
      if (array.size() == 3) {
        result = ZPoint(array[0].toDouble(), array[1].toDouble(), array[2].toDouble());
        return true;
      }
    }
    return false;
  }

  Z3DMeshFilter *getMeshFilter(ZStackDoc *doc)
  {
    if (Z3DWindow *window = doc->getParent3DWindow()) {
      return window->getMeshFilter();
    }
    return nullptr;
  }

  std::string getOutputInstanceName(const ZDvidTarget &dvidTarget)
  {
    return dvidTarget.getBodyLabelName() + "_merged";
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

  // All the TaskBodyMerge instances loaded from one JSON file need certain changes
  // to some settings until all of them are done.  This code manages making those
  // changes and restore the changed values when the tasks are done.

  static bool applyOverallSettingsNeeded = true;
  static bool zoomToLoadedBodyEnabled;
  static bool garbageLifetimeLimitEnabled;
  static bool splitTaskLoadingEnabled;
  static bool showingSynapse;
  static int minResLevel;
  static bool preservingSourceColorEnabled;
  static bool showingSourceColors;
  static bool showingAnnotations;
  static bool bodySelectionMessageEnabled;

  void applyOverallSettings(ZFlyEmBody3dDoc* bodyDoc)
  {
    if (applyOverallSettingsNeeded) {
      applyOverallSettingsNeeded = false;

      zoomToLoadedBodyEnabled = Neu3Window::zoomToLoadedBodyEnabled();
      Neu3Window::enableZoomToLoadedBody(false);

      garbageLifetimeLimitEnabled = bodyDoc->garbageLifetimeLimitEnabled();
      bodyDoc->enableGarbageLifetimeLimit(false);

      splitTaskLoadingEnabled = bodyDoc->splitTaskLoadingEnabled();
      bodyDoc->enableSplitTaskLoading(false);

      showingSynapse = bodyDoc->showingSynapse();
      bodyDoc->showSynapse(false);

      minResLevel = bodyDoc->getMinDsLevel();
      bodyDoc->setMinDsLevel(bodyDoc->getMaxDsLevel());

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

      bodySelectionMessageEnabled = ZFlyEmProofDoc::bodySelectionMessageEnabled();
      ZFlyEmProofDoc::enableBodySelectionMessage(false);
    }
  }

  void restoreOverallSettings(ZFlyEmBody3dDoc* bodyDoc)
  {
    if (!applyOverallSettingsNeeded) {
      applyOverallSettingsNeeded = true;

      bodyDoc->enableGarbageLifetimeLimit(garbageLifetimeLimitEnabled);
      bodyDoc->enableSplitTaskLoading(splitTaskLoadingEnabled);
      bodyDoc->showSynapse(showingSynapse);
      bodyDoc->setMinDsLevel(minResLevel);

      if (Z3DMeshFilter *filter = getMeshFilter(bodyDoc)) {
        filter->enablePreservingSourceColors(preservingSourceColorEnabled);
        filter->showSourceColors(showingSourceColors);
      }

      ZFlyEmProofMvc::showAnnotations(showingAnnotations);

      ZFlyEmProofDoc::enableBodySelectionMessage(bodySelectionMessageEnabled);
    }
  }

  QPointer<QNetworkAccessManager> s_networkManager;

  // A separate 3D view that provides a "bird's eye view" zoomed out to show both bodies,
  // so the user can quickly get a more global context.  The Z3DView take time to initialize,
  // so there is one such view shared by all the tasks in the assignment.

  QPointer<QDockWidget> s_birdsEyeDockWidget;
  QPointer<Z3DView> s_birdsEyeView;

}

TaskBodyMerge::TaskBodyMerge(QJsonObject json, ZFlyEmBody3dDoc *bodyDoc)
{
  m_bodyDoc = bodyDoc;

  m_initialAngleMethod = initialAngleMethod();

  applyOverallSettings(m_bodyDoc);

  loadJson(json);
  buildTaskWidget();
}

QString TaskBodyMerge::tasktype()
{
  return VALUE_TASKTYPE;
}

QString TaskBodyMerge::actionString()
{
  return "Body merging:";
}

QString TaskBodyMerge::targetString()
{
  return "SV " + QString::number(m_supervoxelId1) + " +<br>SV " + QString::number(m_supervoxelId2);
}

bool TaskBodyMerge::skip()
{
  if ((m_bodyId1 == 0) || (m_bodyId2 == 0)) {

    // Users do not explicitly choose to skip tasks where one or other super voxel does not
    // map to a body (e.g., because the super voxel was split), but we still want a record
    // of these tasks in the results, so write that record here.

    writeResult("autoSkippedNoBody");
    m_lastSavedButton = nullptr;
    return true;
  } else if (m_bodyId1 == m_bodyId2) {

    // Likewise for redundant tasks.

    writeResult("autoSkippedSameBody");
    m_lastSavedButton = nullptr;
    return true;
  }
  return false;
}

void TaskBodyMerge::beforeNext()
{
  suggestWriting();

  // Clear the mesh cache when changing tasks so it does not grow without bound
  // during an assignment, which causes a performance degradation.  The assumption
  // is that improving performance as a user progresses through an assignment is
  // more important than eliminating the need to reload meshses if the user goes
  // back to a previous task.

  m_bodyDoc->clearGarbage(true);
}

void TaskBodyMerge::beforePrev()
{
  suggestWriting();

  // See the comment in beforeNext().

  m_bodyDoc->clearGarbage(true);
}

void TaskBodyMerge::beforeDone()
{
  restoreOverallSettings(m_bodyDoc);
  applyColorMode(false);

  showBirdsEyeView(false);
}

QWidget *TaskBodyMerge::getTaskWidget()
{
  // It's possible that the bodies of the super voxels were merge together via
  // another pair of super voxels in another user's session.  So get the bodies from
  // DVID again, to update the result computed by skip().

  setBodiesFromSuperVoxels();

  // Now set the visible bodies.  For fastest task loading, start with the original bodies
  // at low resolution.

  if (m_visibleBodies.isEmpty()) {
    m_visibleBodies.insert(m_bodyId1);
    m_visibleBodies.insert(m_bodyId2);
  }

  if (!m_lastSavedButton) {
    QString result = readResult();
    if (!result.isEmpty()) {
      restoreResult(result);
    }
  }

  configureShowHiRes();
  applyColorMode(true);

  return m_widget;
}

QMenu *TaskBodyMerge::getTaskMenu()
{
  return m_menu;
}

bool TaskBodyMerge::usePrefetching()
{
  // Prefetching is not necessary for this protocol, which loads only the lowest resolution
  // meshes until the user explicitly ask for higher resolution.  In fact, prefetcing interferes
  // in that it will bring in higher resolution meshes for later tasks.

  return false;
}

void TaskBodyMerge::onCycleAnswer()
{
  if (m_dontMergeButton->isChecked()) {
    m_mergeButton->setChecked(true);
  } else if (m_mergeButton->isChecked()) {
    m_dontMergeButton->setChecked(true);
  }
}

void TaskBodyMerge::onTriggerShowHiRes()
{
  m_showHiResCheckBox->setChecked(true);
}

void TaskBodyMerge::onButtonToggled()
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

void TaskBodyMerge::onShowHiResStateChanged(int state)
{
  QSet<uint64_t> visible;
  if (state) {
    int level = 0;
    visible.insert(ZFlyEmBodyManager::encode(m_bodyId1, level));
    visible.insert(ZFlyEmBodyManager::encode(m_bodyId2, level));

    // Going back to low resolution is not working for some reason, so disable it for now.

    m_showHiResCheckBox->setEnabled(false);
  } else {
    visible.insert(m_bodyId1);
    visible.insert(m_bodyId2);
  }
  updateBodies(visible, QSet<uint64_t>());
}

bool TaskBodyMerge::loadSpecific(QJsonObject json)
{
  if (!json.contains(KEY_SUPERVOXEL_ID1) || !json.contains(KEY_SUPERVOXEL_ID2)) {
    return false;
  }

  m_supervoxelId1 = json[KEY_SUPERVOXEL_ID1].toDouble();
  m_supervoxelId2 = json[KEY_SUPERVOXEL_ID2].toDouble();
  setBodiesFromSuperVoxels();

  if (!pointFromJSON(json[KEY_SUPERVOXEL_POINT1], m_supervoxelPoint1)) {
    if (Z3DWindow *window = m_bodyDoc->getParent3DWindow()) {
      QMessageBox::warning(window, "Task Loading Failed",
                           "For merger " + QString::number(m_supervoxelId1) + " + " +
                           QString::number(m_supervoxelId2) +
                           ", the key \"" + KEY_SUPERVOXEL_POINT1 + "\" cannot be parsed as a 3D point.");
    }
  }

  if (!pointFromJSON(json[KEY_SUPERVOXEL_POINT2], m_supervoxelPoint2)) {
    if (Z3DWindow *window = m_bodyDoc->getParent3DWindow()) {
      QMessageBox::warning(window, "Task Loading Failed",
                           "For merger " + QString::number(m_supervoxelId1) + " + " +
                           QString::number(m_supervoxelId2) +
                           ", the key \"" + KEY_SUPERVOXEL_POINT2 + "\" cannot be parsed as a 3D point.");
    }
  }

  return true;
}

QJsonObject TaskBodyMerge::addToJson(QJsonObject taskJson)
{
  taskJson[KEY_TASKTYPE] = VALUE_TASKTYPE;

  taskJson[KEY_SUPERVOXEL_ID1] = static_cast<double>(m_supervoxelId1);
  taskJson[KEY_SUPERVOXEL_ID2] = static_cast<double>(m_supervoxelId2);

  QJsonArray array1;
  array1.append(m_supervoxelPoint1.x());
  array1.append(m_supervoxelPoint1.y());
  array1.append(m_supervoxelPoint1.z());
  taskJson[KEY_SUPERVOXEL_POINT1] = array1;
  QJsonArray array2;
  array2.append(m_supervoxelPoint2.x());
  array2.append(m_supervoxelPoint2.y());
  array2.append(m_supervoxelPoint2.z());
  taskJson[KEY_SUPERVOXEL_POINT2] = array2;

  return taskJson;
}

void TaskBodyMerge::setBodiesFromSuperVoxels()
{
  std::string instance = m_bodyDoc->getDvidTarget().getBodyLabelName();
  ZDvidUrl url(m_bodyDoc->getDvidTarget());
  std::string urlMapping = url.getNodeUrl() + "/" + instance + "/mapping";

  ZJsonArray jsonBody;
  jsonBody.append(m_supervoxelId1);
  jsonBody.append(m_supervoxelId2);

  std::string payloadStr = jsonBody.dumpString(0);
  libdvid::BinaryDataPtr payload =
      libdvid::BinaryData::create_binary_data(payloadStr.c_str(), payloadStr.size());
  int statusCode;
  libdvid::BinaryDataPtr response = ZDvid::MakeRequest(urlMapping, "GET", payload, libdvid::DEFAULT, statusCode);
  if (statusCode == 200) {
    QJsonDocument responseDoc = QJsonDocument::fromJson(response->get_data().c_str());
    if (responseDoc.isArray())  {
      QJsonArray responseArray = responseDoc.array();
      if (responseArray.size() == 2) {
        QJsonValue responseElem = responseArray[0];
        if (!responseElem.isUndefined()) {
          m_bodyId1 = responseElem.toDouble();
        }
        responseElem = responseArray[1];
        if (!responseElem.isUndefined()) {
          m_bodyId2 = responseElem.toDouble();
        }
      }
    }
  } else {
    if (Z3DWindow *window = m_bodyDoc->getParent3DWindow()) {
      QMessageBox::warning(window, "Task Loading Failed",
                           "For merger " + QString::number(m_supervoxelId1) + " + " +
                           QString::number(m_supervoxelId2) +
                           ", supervoxel IDs could not be mapped to body IDs (DVID status " +
                           QString::number(statusCode));
    }
  }
}

void TaskBodyMerge::buildTaskWidget()
{
  m_widget = new QWidget();

  m_dontMergeButton = new QRadioButton("Don't Merge", m_widget);
  m_dontMergeButton->setChecked(true);
  connect(m_dontMergeButton, SIGNAL(toggled(bool)), this, SLOT(onButtonToggled()));

  m_mergeButton = new QRadioButton("Merge", m_widget);
  connect(m_mergeButton, SIGNAL(toggled(bool)), this, SLOT(onButtonToggled()));

  m_mergeLaterButton = new QRadioButton("Merge Later", m_widget);
  connect(m_mergeLaterButton, SIGNAL(toggled(bool)), this, SLOT(onButtonToggled()));

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
  QHBoxLayout *radioBottomLayout = new QHBoxLayout;
  radioBottomLayout->addWidget(m_mergeLaterButton);
  radioBottomLayout->addWidget(m_irrelevantButton);
  radioBottomLayout->addWidget(m_dontKnowButton);

  m_showHiResCheckBox = new QCheckBox("Show High Resolution", m_widget);
  connect(m_showHiResCheckBox, SIGNAL(stateChanged(int)), this, SLOT(onShowHiResStateChanged(int)));

  QPushButton *zoomToInitialButton = new QPushButton("Zoom to Initial Position", m_widget);
  connect(zoomToInitialButton, SIGNAL(clicked(bool)), this, SLOT(zoomToMergePosition()));

  QPushButton *zoomOutButton = new QPushButton("Zoom Out to Show All", m_widget);
  connect(zoomOutButton, SIGNAL(clicked(bool)), this, SLOT(zoomOutToShowAll()));

  QHBoxLayout *bottomLayout = new QHBoxLayout;
  bottomLayout->addWidget(m_showHiResCheckBox);
  QVBoxLayout *buttonsLayout = new QVBoxLayout;
  buttonsLayout->addWidget(zoomToInitialButton);
  buttonsLayout->addWidget(zoomOutButton);
  bottomLayout->addLayout(buttonsLayout);

  QVBoxLayout *layout = new QVBoxLayout;
  layout->addLayout(radioTopLayout);
  layout->addLayout(radioBottomLayout);
  layout->addLayout(bottomLayout);

  m_widget->setLayout(layout);

  m_menu = new QMenu("Body Merging", m_widget);

  QAction *cycleAnswerAction = new QAction("Cycle Through Answers", m_widget);
  cycleAnswerAction->setShortcut(Qt::Key_V);
  m_menu->addAction(cycleAnswerAction);
  connect(cycleAnswerAction, SIGNAL(triggered()), this, SLOT(onCycleAnswer()));

  m_showHiResAction = new QAction("Show High Resolution", m_widget);
  m_showHiResAction->setShortcut(Qt::Key_C);
  m_menu->addAction(m_showHiResAction);
  connect(m_showHiResAction, SIGNAL(triggered()), this, SLOT(onTriggerShowHiRes()));
}

void TaskBodyMerge::onLoaded()
{
  LINFO() << "TaskBodyMerge: build version" << getBuildVersion() << ".";

  m_usageTimer.start();

  showBirdsEyeView(true);

  applyColorMode(true);
  zoomToMergePosition(true);

  if (const char* showHybrid = std::getenv("NEU3_SHOW_HYBRID_MESHES")) {
    if (std::string(showHybrid) == "yes") {
      //Do not show hybrid meshes if either of the body is shown as agglomeration
      //of supervoxels.
      if (!m_bodyDoc->isAgglo(m_bodyId1) && !m_bodyDoc->isAgglo(m_bodyId2)) {
        showHybridMeshes();
      }
    }
  }
}

void TaskBodyMerge::onCompleted()
{
  m_usageTimes.push_back(m_usageTimer.elapsed());

  // Restart the timer, to measure the time if the user reconsiders and
  // reaches a new decision for this task (without moving on and then coming
  // back to this task).

  m_usageTimer.start();

  writeResult();
}

void TaskBodyMerge::applyColorMode(bool merging)
{
  if (Z3DMeshFilter *filter = getMeshFilter(m_bodyDoc)) {
    if (merging) {
      updateColors();
      filter->setColorMode("Indexed Color");
      if (s_birdsEyeView) {
        s_birdsEyeView->getMeshFilter()->setColorMode("Indexed Color");
      }
    } else {
      filter->setColorMode("Mesh Source");
    }
  }
}

void TaskBodyMerge::updateColors()
{
  if (Z3DMeshFilter *filter = getMeshFilter(m_bodyDoc)) {
    std::size_t index1 = 1;
    std::size_t index2 = m_mergeButton->isChecked() ? index1 : 2;

    std::vector<Z3DMeshFilter*> filters({ filter });
    if (s_birdsEyeView) {
      filters.push_back(s_birdsEyeView->getMeshFilter());
    }
    for (Z3DMeshFilter *filt : filters) {
      filt->setColorIndexing(INDEX_COLORS, [=](uint64_t id) -> std::size_t {
        uint64_t tarBodyId = m_bodyDoc->getMappedId(id);
        if (tarBodyId == m_bodyId1) {
          return index1;
        } else if (tarBodyId == m_bodyId2) {
          return index2;
        } else {
          return 0;
        }
      });
    }

    QHash<uint64_t, QColor> idToColor;
    glm::vec4 color1 = INDEX_COLORS[index1] * 255.0f;
    idToColor[m_bodyId1] = QColor(color1.x, color1.y, color1.z);
    glm::vec4 color2 = INDEX_COLORS[index2] * 255.0f;
    idToColor[m_bodyId2] = QColor(color2.x, color2.y, color2.z);

    emit updateGrayscaleColor(idToColor);
  }
}

ZPoint TaskBodyMerge::mergePosition() const
{
  return (m_supervoxelPoint1 + m_supervoxelPoint2) / 2.0;
}

void TaskBodyMerge::initAngleForMergePosition(bool justLoaded)
{
  if (Z3DWindow *window = m_bodyDoc->getParent3DWindow()) {
    if (Z3DMeshFilter *filter =
        dynamic_cast<Z3DMeshFilter*>(window->getMeshFilter())) {

      glm::vec3 eye;
      glm::vec3 up;

      switch (m_initialAngleMethod) {
        case 1: {
          up = glm::vec3(0, 1, 0);
          eye = filter->camera().center() + glm::vec3(0, 0, 1);
          break;
        }

        case 2: {
          glm::vec3 p1(m_supervoxelPoint1.x(), m_supervoxelPoint1.y(), m_supervoxelPoint1.z());
          glm::vec3 p2(m_supervoxelPoint2.x(), m_supervoxelPoint2.y(), m_supervoxelPoint2.z());
          glm::vec3 p1ToP2 = glm::normalize(p2 - p1);

          // TODO: Choose an up vector that gives the best view, in some sense.
          if (justLoaded) {
            m_initialUp = window->getMeshFilter()->camera().upVector();
          }

          up = m_initialUp;
          glm::vec3 toEye = glm::cross(p1ToP2, up);
          float toEyeLength = glm::length(toEye);
          if (toEyeLength > 1e-5) {
            toEye /= toEyeLength;
          } else {

            // The vector between the two supervoxel points is parellel to the up vector.
            // So the camera is already giving a good view of the supervoxel points.

            toEye = glm::normalize(filter->camera().eye() - filter->camera().center());
          }
          eye = filter->camera().center() + toEye;
          break;
        }

        default: {
          eye = filter->camera().eye();
          up = filter->camera().upVector();
          break;
        }
      }

      // Update the camera for the 3D view.

      filter->camera().setEye(eye);
      filter->camera().setUpVector(up);

      if (s_birdsEyeView) {
        Z3DMeshFilter *birdsEyeMeshFilter = s_birdsEyeView->getMeshFilter();
        birdsEyeMeshFilter->camera().setEye(eye);
        birdsEyeMeshFilter->camera().setUpVector(up);
      }

      // Update the orientaton of the grayscale slice.

      window->getCamera()->setEye(eye);
    }
  }
}

namespace {

// Set the frustum to just enclose the bounding sphere.  The result is a tighter fit than
// available with Z3DCamera::resetCamera(const ZBBox<glm::dvec3>& bound, ResetOption options),
// whose input would be a box around the sphere, and which then would put asphere around the box.

void resetCamera(ZPoint pos, double radius, Z3DCamera &camera)
{
  glm::vec3 center = glm::vec3(pos.x(), pos.y(), pos.z());

  double angle = camera.fieldOfView();
  if (camera.aspectRatio() < 1.0) {
    // use horizontal angle to calculate
    angle = 2.0 * std::atan(std::tan(angle * 0.5) * camera.aspectRatio());
  }

  float centerDist = radius / std::sin(angle * 0.5);
  glm::vec3 eye = center - centerDist * glm::normalize(camera.center() - camera.eye());
  camera.setCamera(eye, center, camera.upVector());

  double xMin = pos.x() - radius, xMax = pos.x() + radius;
  double yMin = pos.y() - radius, yMax = pos.y() + radius;
  double zMin = pos.z() - radius, zMax = pos.z() + radius;
  camera.resetCameraNearFarPlane(xMin, xMax, yMin, yMax, zMin, zMax);
}

void projectToViewPlane3D(const std::vector<glm::vec3> &vertices,
                          const Z3DCamera &camera,
                          std::vector<glm::vec3> &result)
{
  glm::vec3 view = camera.center() - camera.eye();
  float viewDist = glm::length(view);
  view /= viewDist;

  // Using a perspective projection for the current camera parameters, project each
  // vertex onto the plane passing through camera.center() that is normal to the
  // view vector (from camera.eye() to camera.center()).

  for (const glm::vec3 &vert : vertices) {
    glm::vec3 eyeToVert = vert - camera.eye();
    glm::vec3 eyeToVertOnView = glm::dot(eyeToVert, view) * view;
    float eyeToVertDist = glm::length(eyeToVert);
    eyeToVert /= eyeToVertDist;

    // By similar triangles, viewDist / glm::length(eyeToVertOnView) = eyeToVertOnPlaneDist / eyeToVertDist

    float eyeToVertOnPlaneDist = viewDist / glm::length(eyeToVertOnView) * eyeToVertDist;
    glm::vec3 eyeToVertOnPlane = camera.eye() + eyeToVert * eyeToVertOnPlaneDist;
    result.push_back(eyeToVertOnPlane);
  }
}

void viewPlane3Dto2D(const std::vector<glm::vec3> &vertices,
                     const Z3DCamera &camera,
                     std::vector<glm::vec2> &result)
{
  // Extend w, the normal to the plane containing the vertices, to a
  // 3D coordinate frame, with u and v lying on the plane.

  glm::vec3 w = glm::normalize(camera.eye() - camera.center());
  glm::vec3 u = glm::normalize(glm::cross(camera.upVector(), w));
  glm::vec3 v = glm::normalize(glm::cross(w, u));

  // Use that frame to generate 2D points on the plane, with camera.center()
  // as the origin.

  for (const glm::vec3 &vert : vertices) {
    glm::vec3 centerToVert = vert - camera.center();
    glm::vec2 p = glm::vec2(glm::dot(centerToVert, u), glm::dot(centerToVert, v));
    result.push_back(p);
  }
}

bool resetCameraForViewPlane2D(const ZBBox<glm::vec2> &bbox,
                               glm::vec3 &eyeValid,
                               Z3DCamera& camera)
{
  if ((camera.right() - std::abs(camera.left()) > 1e-6) ||
      (camera.top() - std::abs(camera.bottom()) > 1e-6)) {

    // The code is not designed to handle a skewed view frustum (but it is unlikely anyway).

    return false;
  }

  // Move eye, the tip of the frustum, so the frustum most tightly fits bbox, the 2D bounding box
  // of the vertices projected onto the plane through camera.center().  First consider the vertical
  // dimension of bbox.  Use similar triangles to determine the new distance from camera.center()
  // to eye.

  float height = std::max(bbox.maxCorner().y, std::abs(bbox.minCorner().y));
  float viewDistY = camera.nearDist() * height / camera.top();

  // Repeat for the horizontal dimension of bbox.

  float width = std::max(bbox.maxCorner().x, std::abs(bbox.minCorner().x));
  float viewDistX = camera.nearDist() * width / camera.right();

  // Use the larger of the two distances.

  float viewDist = std::max(viewDistX, viewDistY);

  glm::vec3 viewOld = camera.center() - camera.eye();
  float viewDistOld = glm::length(viewOld);
  viewOld /= viewDistOld;
  glm::vec3 eye;

  if (std::abs(viewDist - viewDistOld) < 1) {

    // Stop the iteration (repeatedly calling this routine) if the new eye has barely moved.

    return false;
  } else if (viewDist < viewDistOld) {

    // If eye has moved closer to camera.center() then we want to use it as camera.eye() for
    // the next iteration.  And the current camera.eye(), which was used for the projection and
    // computation of the new eye also is valid, and should be saved in case the next iteration goes
    // too far and has to be backed out.

    eye = camera.center() - viewDist * viewOld;
    eyeValid = camera.eye();
  } else {

    // If eye has moved further from camera.center() then camera.eye() was too close and is not valid.
    // Back out halfway to the previous valid eye for the next iteration.

    glm::vec3 eyeToEyeValid = eyeValid - camera.eye();
    float eyeToEyeValidDist = glm::length(eyeToEyeValid);
    eyeToEyeValid /= eyeToEyeValidDist;
    eye = camera.eye() + eyeToEyeValidDist / 2 * eyeToEyeValid;
  }

  camera.setCamera(eye, camera.center(), camera.upVector());

  return true;
}

void tightenZoom(const std::vector<std::vector<glm::vec3>> &vertices,
                 Z3DCamera &camera)
{
  float closestDist = std::numeric_limits<float>::max();
  size_t iClosestMesh;
  size_t iClosestVertex;
  bool found = false;

  // The algorithm iterqtively moves the eye point, and at each iteration it must
  // make sure that no vertex is in front of the near clipping plane.  Precompute
  // which vertex is closest to the eye, as it won't change during the iteration.

  glm::vec3 view = glm::normalize(camera.center() - camera.eye());
  for (size_t i = 0; i < vertices.size(); i++) {
    for (size_t j = 0; j < vertices[i].size(); j++) {
      glm::vec3 eyeToVert = vertices[i][j] - camera.eye();
      float dist = glm::dot(eyeToVert, view);
      if (dist < closestDist) {
        closestDist = dist;
        iClosestMesh = i;
        iClosestVertex = j;
        found = true;
      }
    }
  }

  if (!found) {
    return;
  }

  // Iteratively adjust the eye point.

  glm::vec3 eyeValid = camera.eye();
  for (size_t iter = 0; iter < 10; iter++) {
    ZBBox<glm::vec2> bbox;

    for (size_t i = 0; i < vertices.size(); i++) {
      std::vector<glm::vec3> onViewPlane3D;
      projectToViewPlane3D(vertices[i], camera, onViewPlane3D);

      std::vector<glm::vec2> onViewPlane2D;
      viewPlane3Dto2D(onViewPlane3D, camera, onViewPlane2D);

      for (glm::vec2 point : onViewPlane2D) {
        bbox.expand(point);
      }
    }

    if (!resetCameraForViewPlane2D(bbox, eyeValid, camera)) {
      break;
    }

    // Check the closest vertex against the near clipping plane for the moved eye point.

    glm::vec3 view = glm::normalize(camera.center() - camera.eye());
    glm::vec3 eyeToVert = vertices[iClosestMesh][iClosestVertex] - camera.eye();
    float near = glm::dot(eyeToVert, view);

    if (near < camera.nearDist()) {

      // If adjusting the near clipping plane would make it too small relative to the
      // far clipping plane, then stop the iteration.  The definition of "too small"
      // matches that in Z3DCamera.

      float nearClippingPlaneTolerance = 0.001;
      float minNear = nearClippingPlaneTolerance * camera.farDist();
      if (near < minNear) {
        glm::vec3 eye = camera.eye() + near * view;
        eye -= minNear * view;
        camera.setEye(eye);
        camera.setNearDist(minNear);
        break;
      }
      camera.setNearDist(near);
    }
  }
}

}

void TaskBodyMerge::zoomToMergePosition(bool justLoaded)
{
  if (m_bodyDoc->getParent3DWindow()) {
    ZPoint pos = mergePosition();

    std::size_t index1 = 1;
    std::size_t index2 = m_mergeButton->isChecked() ? index1 : 2;
    QHash<uint64_t, QColor> idToColor;
    glm::vec4 color1 = INDEX_COLORS[index1] * 255.0f;
    idToColor[m_bodyId1] = QColor(color1.x, color1.y, color1.z);
    glm::vec4 color2 = INDEX_COLORS[index2] * 255.0f;
    idToColor[m_bodyId2] = QColor(color2.x, color2.y, color2.z);

    QTimer::singleShot(0, this, [=](){
      emit browseGrayscale(pos.x(), pos.y(), pos.z(), idToColor);
    });

    initAngleForMergePosition(justLoaded);
    zoomToMeshes(true);
  }
}

void TaskBodyMerge::zoomOutToShowAll()
{
  zoomToMeshes(false);
}

void TaskBodyMerge::zoomToMeshes(bool onlySmaller)
{
  ZPoint pos = mergePosition();
  Z3DMeshFilter *filter = getMeshFilter(m_bodyDoc);

  std::vector<double> radii{ 0, 0 };
  std::vector<std::vector<glm::vec3>> vertices{ std::vector<glm::vec3>(), std::vector<glm::vec3>() };

  ZBBox<glm::dvec3> bbox;

  QList<ZMesh*> meshes = ZStackDocProxy::GetGeneralMeshList(m_bodyDoc);
  for (auto it = meshes.cbegin(); it != meshes.cend(); it++) {
    ZMesh *mesh = *it;
    uint64_t tarBodyId = m_bodyDoc->getMappedId(mesh->getLabel());
    if ((tarBodyId == m_bodyId1) || (tarBodyId == m_bodyId2)) {
      int i = tarBodyId == m_bodyId1 ? 0 : 1;
      for (glm::vec3 vertex : mesh->vertices()) {
        double r = pos.distanceTo(vertex.x, vertex.y, vertex.z);
        if (r > radii[i]) {
          radii[i] = r;
        }
        vertices[i].push_back(vertex);
      }

      if (onlySmaller) {

        // The code below will tighten the zoom to the body or bodies of interest.  If it is
        // only the smaller body, then the near clipping plane might be wrong for two bodies together.
        // So compute their bounding box for use in a final adjustment.

        bbox.expand(mesh->boundBox());
      }
    }
  }

  if (s_birdsEyeView) {
    Z3DMeshFilter *birdsEyeMeshFilter = s_birdsEyeView->getMeshFilter();

    double radius = std::max(radii[0], radii[1]);
    resetCamera(mergePosition(), radius, birdsEyeMeshFilter->camera());
    tightenZoom(vertices, birdsEyeMeshFilter->camera());

    birdsEyeMeshFilter->invalidate();
  }

  double radius;
  if (onlySmaller) {
    size_t iSmaller = (radii[0] < radii[1]) ? 0 : 1;
    radius = radii[iSmaller];
    size_t iOther = (iSmaller == 0) ? 1 : 0;
    vertices.erase(vertices.begin() + iOther);
  } else {
    radius = std::max(radii[0], radii[1]);
  }

  resetCamera(mergePosition(), radius, filter->camera());
  tightenZoom(vertices, filter->camera());

  if (onlySmaller) {

    // Adjust the near clipping plane to accomodate both bodies, as mentioned above.

    filter->camera().resetCameraNearFarPlane(bbox);
  }

  filter->invalidate();
}

//Replace lambda with function for easier debuggging
void TaskBodyMerge::updateHighResWidget(QNetworkReply *reply)
{
  if (reply->error() == QNetworkReply::NoError) {
    QByteArray replyBytes = reply->readAll();
    qDebug() << "Reply:" << replyBytes;
    QJsonDocument replyJsonDoc = QJsonDocument::fromJson(replyBytes);
    if (replyJsonDoc.isArray()) {
      QJsonArray replyJsonArray = replyJsonDoc.array();
      if (!replyJsonArray.isEmpty()) {

        // If both tar archives exist, then re-enable the controls.

        m_hiResCount++;
        if (m_hiResCount == 2) {
          m_showHiResCheckBox->setEnabled(true);
          m_showHiResAction->setEnabled(true);
        }
      }
    }
  } else {
    qDebug() << "Reading error:" << reply->errorString();
  }
  reply->deleteLater();
}

void TaskBodyMerge::configureShowHiRes()
{
  if (m_showHiResCheckBox->isChecked()) {

    // Going back to low resolution is not working for some reason, so disable it for now.

    return;
  }

  ZDvidUrl dvidUrl(m_bodyDoc->getDvidTarget());

  // Disable the controls for switching to high resolution until we verify that the
  // high-resolution tar archives exist.

  m_showHiResCheckBox->setEnabled(false);
  m_showHiResAction->setEnabled(false);
  m_hiResCount = 0;

  // TODO: Until this issue is fixed, always disable high resolution.
  // https://github.com/janelia-flyem/NeuTu/issues/185
//  return;

  if (!s_networkManager) {
    s_networkManager = new QNetworkAccessManager(m_bodyDoc->getParent3DWindow());
  }

  // If the DVID query, issued below, returns a JSON object containing the key
  // then the tar archive exists.

  disconnect(s_networkManager, 0, 0, 0);
  connect(s_networkManager, &QNetworkAccessManager::finished,
          this, &TaskBodyMerge::updateHighResWidget);

  // Issue the DVID queries, each of which is a "range" query for
  // the range including just the key for one body's tar archive.
  // As of now, that is the fastest way to check whether a key exists.

  std::vector<uint64_t> ids({ m_bodyId1, m_bodyId2 });
  for (uint64_t id : ids) {
    int level = 0;
    id = ZFlyEmBodyManager::encode(id, level);
    std::string url = dvidUrl.getMeshesTarsKeyRangeUrl(id, id);
    qDebug() << "Mesh tar:" << url;
    QUrl requestUrl(url.c_str());
    QNetworkRequest request(requestUrl);
    s_networkManager->get(request); //No waiting?
  }
}

void TaskBodyMerge::showBirdsEyeView(bool show)
{
  if (show) {
    if (Z3DWindow *window = m_bodyDoc->getParent3DWindow()) {
      if (!s_birdsEyeDockWidget) {
        s_birdsEyeDockWidget = new QDockWidget("Bird's Eye View", window);
        window->addDockWidget(Qt::NoDockWidgetArea, s_birdsEyeDockWidget);

        s_birdsEyeView = new Z3DView(m_bodyDoc, Z3DView::INIT_NORMAL, false, s_birdsEyeDockWidget);
        s_birdsEyeView->canvas().setMinimumWidth(512);
        s_birdsEyeView->canvas().setMinimumHeight(512);
        s_birdsEyeDockWidget->setWidget(&s_birdsEyeView->canvas());

        s_birdsEyeDockWidget->setFeatures(
              QDockWidget::DockWidgetMovable | QDockWidget::DockWidgetFloatable);
        s_birdsEyeDockWidget->setFloating(true);
        s_birdsEyeDockWidget->setAllowedAreas(Qt::NoDockWidgetArea);
        s_birdsEyeDockWidget->setAttribute(Qt::WA_DeleteOnClose);
      }

      s_birdsEyeDockWidget->show();

      Z3DCamera camera = window->getMeshFilter()->camera();
      Z3DMeshFilter *birdsEyeMeshFilter = s_birdsEyeView->getMeshFilter();
      birdsEyeMeshFilter->camera().setCamera(camera.eye(), camera.center(), camera.upVector());
    }
  } else {
    if (s_birdsEyeDockWidget) {
      QAction *closer = s_birdsEyeDockWidget->toggleViewAction();
      closer->trigger();
    }
  }
}

void TaskBodyMerge::showHybridMeshes()
{
  // The high-res region of the hybrid meshes will go all the way through both bodies in the dimension
  // closest to the viewing direction.  So first compute overall boudning box of the two bodies.

  ZBBox<glm::dvec3> bbox;
  QList<ZMesh*> meshes = ZStackDocProxy::GetBodyMeshList(m_bodyDoc);
  for (auto it = meshes.cbegin(); it != meshes.cend(); it++) {
    ZMesh *mesh = *it;
    uint64_t tarBodyId = m_bodyDoc->getMappedId(mesh->getLabel());
    if ((tarBodyId == m_bodyId1) || (tarBodyId == m_bodyId2)) {
      bbox.expand(mesh->boundBox());
    }
  }

  // In the other two dimensions, the high-res region will have a fixed, smallish size.

  int halfWidth = 256 / 2;

  glm::vec3 view(0, 0, -1);
  if (Z3DMeshFilter *filter = getMeshFilter(m_bodyDoc)) {
    view = filter->camera().center() - filter->camera().eye();
  }

  // Compute the two corners of the high-res region.

  ZIntPoint p1, p2;
  glm::vec3 viewAbs(std::abs(view.x), std::abs(view.y), std::abs(view.z));
  size_t iMax = (viewAbs[0] > viewAbs[1]) ?
        (viewAbs[0] > viewAbs[2] ? 0 : 2) :
    (viewAbs[1] > viewAbs[2] ? 1 : 2);
  ZPoint p = mergePosition();
  int s = zgeom::GetZoomScale(m_bodyDoc->getMaxDsLevel());
  for (size_t i = 0; i < 3; i++) {
    if (i == iMax) {
      p1[i] = bbox.minCorner()[i];
      p2[i] = bbox.maxCorner()[i];
    } else {
      p1[i] = p[i] - halfWidth;
      p2[i] = p[i] + halfWidth;
    }

    // Snap the region to low-res voxel boundaries.

    p1[i] -= p1[i] % s;
    p2[i] += 64 - p2[i] % s;
  }

  // Trigger asynchronous generation of hybrid meshes for the region.

  ZIntCuboid range(p1, p2);
  m_bodyDoc->showMoreDetail(m_bodyId1, range);
  m_bodyDoc->showMoreDetail(m_bodyId2, range);
}

void TaskBodyMerge::writeResult()
{
  if (m_lastSavedButton) {
    m_lastSavedButton->setStyleSheet("");
  }

  QString result;
  if (m_mergeButton->isChecked()) {
    result = "merge";
    m_lastSavedButton = m_mergeButton;
  } else if (m_dontMergeButton->isChecked()) {
    result = "dontMerge";
    m_lastSavedButton = m_dontMergeButton;
  } else if (m_mergeLaterButton->isChecked()) {
    result = "mergeLater";
    m_lastSavedButton = m_mergeLaterButton;
  } else if (m_irrelevantButton->isChecked()) {
    result = "irrelevant";
    m_lastSavedButton = m_irrelevantButton;
  } else {
    result = "?";
    m_lastSavedButton = m_dontKnowButton;
  }

  writeResult(result);
}

void TaskBodyMerge::writeResult(const QString &result)
{
  ZDvidReader reader;
  reader.setVerbose(false);
  if (!reader.open(m_bodyDoc->getDvidTarget())) {
    LERROR() << "TaskBodyMerge::onCompleted() could not open DVID target for reading";
    return;
  }

  ZDvidWriter writer;
  if (!writer.open(m_bodyDoc->getDvidTarget())) {
    LERROR() << "TaskBodyMerge::onCompleted() could not open DVID target for writing";
    return;
  }

  std::string instance = getOutputInstanceName(m_bodyDoc->getDvidTarget());

  if (!reader.hasData(instance)) {
    writer.createKeyvalue(instance);
  }
  if (!reader.hasData(instance)) {
    LERROR() << "TaskBodyCleave::writeResult() could not create DVID instance \"" << instance << "\"";
    return;
  }

  QJsonObject json;
  json[KEY_RESULT] = result;
  if (const char* user = std::getenv("USER")) {
    json[KEY_USER] = user;
  }
  json[KEY_SUPERVOXEL_ID1] = QJsonValue(qint64(m_supervoxelId1));
  json[KEY_SUPERVOXEL_ID2] = QJsonValue(qint64(m_supervoxelId2));
  json[KEY_BODY_ID1] = QJsonValue(qint64(m_bodyId1));
  json[KEY_BODY_ID2] = QJsonValue(qint64(m_bodyId2));
  json[KEY_SUPERVOXEL_POINT1] =
      QJsonArray({ m_supervoxelPoint1.x(), m_supervoxelPoint1.y(), m_supervoxelPoint1.z() });
  json[KEY_SUPERVOXEL_POINT2] =
      QJsonArray({ m_supervoxelPoint2.x(), m_supervoxelPoint2.y(), m_supervoxelPoint2.z() });
  json[KEY_TIMESTAMP] = QDateTime::currentDateTime().toString(Qt::ISODate);
  json[KEY_TIME_ZONE] = QDateTime::currentDateTime().timeZoneAbbreviation();
  json[KEY_SOURCE] = jsonSource();
  json[KEY_BUILD_VERSION] = getBuildVersion().c_str();

  QJsonArray jsonTimes;
  std::copy(m_usageTimes.begin(), m_usageTimes.end(), std::back_inserter(jsonTimes));
  json[KEY_USAGE_TIME] = jsonTimes;

  m_resultHistory.push_back(result);
  QJsonArray jsonResultHistory;
  std::copy(m_resultHistory.begin(), m_resultHistory.end(), std::back_inserter(jsonResultHistory));
  json[KEY_RESULT_HISTORY] = jsonResultHistory;

  size_t i = (m_initialAngleMethod < INITIAL_ANGLE_METHOD.size()) ? m_initialAngleMethod : 0;
  json[KEY_INITIAL_ANGLE_METHOD] = INITIAL_ANGLE_METHOD[i];

  QJsonDocument jsonDoc(json);
  std::string jsonStr(jsonDoc.toJson(QJsonDocument::Compact).toStdString());
  std::string key = std::to_string(m_supervoxelId1) + "+" + std::to_string(m_supervoxelId2);
  writer.writeJsonString(instance, key, jsonStr);
}

QString TaskBodyMerge::readResult()
{
  ZDvidReader reader;
  reader.setVerbose(false);
  if (reader.open(m_bodyDoc->getDvidTarget())) {
    std::string instance = getOutputInstanceName(m_bodyDoc->getDvidTarget());
    if (reader.hasData(instance)) {
      std::string key = std::to_string(m_supervoxelId1) + "+" + std::to_string(m_supervoxelId2);
      ZJsonObject valueObj = reader.readJsonObjectFromKey(instance.c_str(), key.c_str());
      if (valueObj.isObject()){
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

void TaskBodyMerge::restoreResult(const QString &result)
{
  if (result == "merge") {
    m_mergeButton->setChecked(true);
    m_lastSavedButton = m_mergeButton;
  } else if (result == "dontMerge") {
    m_dontMergeButton->setChecked(true);
    m_lastSavedButton = m_dontMergeButton;
  } else if (result == "mergeLater") {
    m_mergeLaterButton->setChecked(true);
    m_lastSavedButton = m_mergeLaterButton;
  } else if (result == "irrelevant") {
    m_irrelevantButton->setChecked(true);
    m_lastSavedButton = m_irrelevantButton;
  } else if (result == "?") {
    m_dontKnowButton->setChecked(true);
    m_lastSavedButton = m_dontKnowButton;
  } else {

    // If a task saved as being skipped, and then stops being skipped, present it
    // the way it would be the first time a user would see any task.

    m_dontMergeButton->setChecked(true);
    m_lastSavedButton = m_dontMergeButton;
  }
}

void TaskBodyMerge::suggestWriting()
{
  if (completed() && m_lastSavedButton && !m_lastSavedButton->isChecked()) {
    if (Z3DWindow *window = m_bodyDoc->getParent3DWindow()) {
      QString title = "Warning";
      QString text = "The chosen result differs from the saved result (indicated in red). "
                     "Resave?";
      QMessageBox::StandardButton chosen =
          QMessageBox::warning(window, title, text, QMessageBox::Save | QMessageBox::No,
                               QMessageBox::Save);
      if (chosen == QMessageBox::Save) {
        writeResult();
      }
    }
  }
}
