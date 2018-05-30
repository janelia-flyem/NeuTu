#include "taskbodymerge.h"

#include "dvid/zdvidtarget.h"
#include "flyem/zflyembody3ddoc.h"
#include "flyem/zflyemproofdoc.h"
#include "flyem/zflyemproofmvc.h"
#include "neu3window.h"
#include "z3dmeshfilter.h"
#include "z3dwindow.h"
#include "zdvidutil.h"
#include "zstackdocproxy.h"

#include <QHBoxLayout>
#include <QMessageBox>
#include <QRadioButton>

namespace {

  static const QString KEY_TASKTYPE = "task type";
  static const QString VALUE_TASKTYPE = "body merge";
  static const QString KEY_SUPERVOXEL_ID1 = "supervoxel ID 1";
  static const QString KEY_SUPERVOXEL_ID2 = "supervoxel ID 2";
  static const QString KEY_SUPERVOXEL_POINT1 = "supervoxel point 1";
  static const QString KEY_SUPERVOXEL_POINT2 = "supervoxel point 2";
  static const QString KEY_ASSIGNED_USER = "assigned user";

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
      if (Z3DMeshFilter *filter =
          dynamic_cast<Z3DMeshFilter*>(window->getMeshFilter())) {
          return filter;
       }
    }
    return nullptr;
  }

  std::string getOutputInstanceName(const ZDvidTarget &dvidTarget)
  {
    return dvidTarget.getBodyLabelName() + "_merged";
  }

  // All the TaskBodyMerge instances loaded from one JSON file need certain changes
  // to some settings until all of them are done.  This code manages making those
  // changes and restore the changed values when the tasks are done.

  static bool applyOverallSettingsNeeded = true;
  static bool zoomToLoadedBodyEnabled;
  static bool garbageLifetimeLimitEnabled;
  static bool splitTaskLoadingEnabled;
  static bool showingSynapse;
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

      if (Z3DMeshFilter *filter = getMeshFilter(bodyDoc)) {
        filter->enablePreservingSourceColors(preservingSourceColorEnabled);
        filter->showSourceColors(showingSourceColors);
      }

      ZFlyEmProofMvc::showAnnotations(showingAnnotations);

      ZFlyEmProofDoc::enableBodySelectionMessage(bodySelectionMessageEnabled);
    }
  }

}

TaskBodyMerge::TaskBodyMerge(QJsonObject json, ZFlyEmBody3dDoc *bodyDoc)
{
  m_bodyDoc = bodyDoc;

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

void TaskBodyMerge::beforeNext()
{
  // Clear the mesh cache when changing tasks so it does not grow without bound
  // during an assignment, which causes a performance degradation.  The assumption
  // is that improving performance as a user progresses through an assignment is
  // more important than eliminating the need to reload meshses if the user goes
  // back to a previous task.

  m_bodyDoc->clearGarbage(true);
}

void TaskBodyMerge::beforePrev()
{
  // See the comment in beforeNext().

  m_bodyDoc->clearGarbage(true);
}

void TaskBodyMerge::beforeDone()
{
  restoreOverallSettings(m_bodyDoc);
  applyColorMode(false);
}

QWidget *TaskBodyMerge::getTaskWidget()
{
  applyColorMode(true);
  return m_widget;
}

QMenu *TaskBodyMerge::getTaskMenu()
{
  return m_menu;
}

void TaskBodyMerge::onCycleAnswer()
{
  if (m_dontMergeButton->isChecked()) {
    m_mergeButton->setChecked(true);
  } else if (m_mergeButton->isChecked()) {
    m_dontKnowButton->setChecked(true);
  } else {
    m_dontMergeButton->setChecked(true);
  }
}

void TaskBodyMerge::onButtonToggled()
{
  updateColors();
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

  int level = 0;
  m_visibleBodies.insert(ZFlyEmBody3dDoc::encode(m_bodyId1, level));
  m_visibleBodies.insert(ZFlyEmBody3dDoc::encode(m_bodyId2, level));

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

  m_dontKnowButton = new QRadioButton("Don't Know", m_widget);
  connect(m_dontKnowButton, SIGNAL(toggled(bool)), this, SLOT(onButtonToggled()));

  QHBoxLayout *radioLayout = new QHBoxLayout(m_widget);
  radioLayout->addWidget(m_dontMergeButton);
  radioLayout->addWidget(m_mergeButton);
  radioLayout->addWidget(m_dontKnowButton);

  m_widget->setLayout(radioLayout);

  m_menu = new QMenu("Body Merging", m_widget);

  QAction *cycleAnswerAction = new QAction("Cycle Through Answers", m_widget);
  cycleAnswerAction->setShortcut('`');
  m_menu->addAction(cycleAnswerAction);
  connect(cycleAnswerAction, SIGNAL(triggered()), this, SLOT(onCycleAnswer()));
}

void TaskBodyMerge::onLoaded()
{
  applyColorMode(true);

  ZPoint point = mergePosition();

  std::size_t index1 = 1;
  std::size_t index2 = m_mergeButton->isChecked() ? index1 : 2;
  QHash<uint64_t, QColor> idToColor;
  glm::vec4 color1 = INDEX_COLORS[index1] * 255.0f;
  idToColor[m_bodyId1] = QColor(color1.x, color1.y, color1.z);
  glm::vec4 color2 = INDEX_COLORS[index2] * 255.0f;
  idToColor[m_bodyId2] = QColor(color2.x, color2.y, color2.z);

  emit browseGrayscale(point.x(), point.y(), point.z(), idToColor);

  zoomToMergePosition();
}

void TaskBodyMerge::onCompleted()
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
    LERROR() << "TaskBodyCleave::onCompleted() could not create DVID instance \"" << instance << "\"";
    return;
  }

  QString result = m_mergeButton->isChecked() ? "y" : m_dontMergeButton->isChecked() ? "n" : "?";
  QJsonArray json;
  json.append(result);
  QJsonDocument jsonDoc(json);
  std::string jsonStr(jsonDoc.toJson(QJsonDocument::Compact).toStdString());

  std::string key = std::to_string(m_supervoxelId1) + "+" + std::to_string(m_supervoxelId2);
  writer.writeJsonString(instance, key, jsonStr);
}

void TaskBodyMerge::applyColorMode(bool merging)
{
  if (Z3DMeshFilter *filter = getMeshFilter(m_bodyDoc)) {
    if (merging) {
      filter->setColorMode("Indexed Color");
      updateColors();
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

    filter->setColorIndexing(INDEX_COLORS, [=](uint64_t id) -> std::size_t {
      uint64_t tarBodyId = m_bodyDoc->getMappedId(id);
      if (tarBodyId == m_bodyId1) {
        return index1;
      } else if (tarBodyId == m_bodyId2) {
        return index2;
      } else {
        return 0;
      }
    });

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

void TaskBodyMerge::zoomToMergePosition()
{
  if (Z3DWindow *window = m_bodyDoc->getParent3DWindow()) {
    ZPoint center = mergePosition();

    double radii[2] = { 0, 0 };
    QList<ZMesh*> meshes = ZStackDocProxy::GetGeneralMeshList(m_bodyDoc);
    for (auto it = meshes.cbegin(); it != meshes.cend(); it++) {
      ZMesh *mesh = *it;
      uint64_t tarBodyId = m_bodyDoc->getMappedId(mesh->getLabel());
      if ((tarBodyId == m_bodyId1) || (tarBodyId == m_bodyId2)) {
        int i = tarBodyId == m_bodyId1 ? 0 : 1;
        for (glm::vec3 vertex : mesh->vertices()) {
          double r = center.distanceTo(vertex.x, vertex.y, vertex.z);
          if (r > radii[i]) {
            radii[i] = r;
          }
        }
      }
    }

    // When Z3DWindow::gotoPosition() ends up calling Z3DCamera::resetCamera(),
    // the radius gets inflated, so reducing it gives a better fit.

    double radius = std::min(radii[0], radii[1]) / 3;
    window->gotoPosition(mergePosition(), radius);
  }
}
