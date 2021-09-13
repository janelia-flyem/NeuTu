#include "taskbodycleave.h"

#include <iostream>
#include <exception>

#include <QCheckBox>
#include <QComboBox>
#include <QFileDialog>
#include <QHBoxLayout>
#include <QInputDialog>
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
#include <QSlider>
#include <QTimer>
#include <QUndoCommand>
#include <QUrl>
#include <QVBoxLayout>
#include <QMutexLocker>

#include "logging/zlog.h"
#include "logging/utilities.h"
#include "neutubeconfig.h"

#include "zdvidutil.h"
#include "zstackdocproxy.h"
#include "zwidgetmessage.h"
#include "z3dmeshfilter.h"
#include "z3dwindow.h"
#include "zdialogfactory.h"
#include "zglobal.h"

#include "qt/network/znetworkutils.h"

#include "dvid/zdvidwriter.h"
#include "dvid/zdvidurl.h"

#include "flyem/zflyembody3ddoc.h"
#include "flyem/zflyemproofmvc.h"
#include "flyem/zflyemsupervisor.h"
#include "flyem/logging.h"
#include "flyem/zflyembodyannotationprotocol.h"
#include "flyem/flyemdatareader.h"
#include "flyem/dialogs/flyemcleaveunassigneddialog.h"

namespace {

  static const QString KEY_TASKTYPE = "task type";
  static const QString VALUE_TASKTYPE = "body cleave";
  static const QString KEY_BODY_ID = "body ID";
  static const QString KEY_BODY_POINT = "body point";
  static const QString KEY_MAXLEVEL = "maximum level";
  static const QString KEY_ASSIGNED_USER = "assigned user";

  static const QString KEY_USER = "user";
  static const QString KEY_SKIPPED = "skipped";
  static const QString KEY_SERVER_REPLY = "latest server reply";
  static const QString KEY_BODY_IDS_CREATED = "new body IDs";
  static const QString KEY_USAGE_TIME = "time to complete (ms)";
  static const QString KEY_STARTUP_TIME = "time to start task (ms)";

  static const QString CLEAVING_STATUS_DONE = "Cleaving status: done";
  static const QString CLEAVING_STATUS_IN_PROGRESS = "Cleaving status: in progress...";
  static const QString CLEAVING_STATUS_FAILED = "Cleaving status: failed";
  static const QString CLEAVING_STATUS_SERVER_WARNINGS = "Cleaving status: server warnings";
  static const QString CLEAVING_STATUS_SERVER_INCOMPLETE = "Cleaving status: omitted meshes";

  static const QString COLOR_SPEC_KEY_AGGLO_ID_COLORS = "agglo ID colors";
  static const QString SETTINGS_KEY_AGGLO_COLOR_FILEPATH = "cleavingAggloColorFilepath";

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

  static const std::vector<glm::vec4> DEFAULT_AGGLO_INDEX_COLORS({
    glm::vec4(255, 255, 255, 255) / 255.0f, // white (no index)
    glm::vec4(  0, 128, 128, 255) / 255.0f, // teal
    glm::vec4(230, 190, 255, 255) / 255.0f, // lavender
    glm::vec4(170, 110,  40, 255) / 255.0f, // brown
    glm::vec4(128,   0,   0, 255) / 255.0f, // maroon
    glm::vec4(255, 215, 180, 255) / 255.0f, // coral
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

  // The timer for roughly measuring the time to load task N must be static,
  // because it starts when the user presses a button to end task N-1.

  static QTime s_startupTimer;

  static std::set<QString> s_warningTextToSuppress;

  static std::vector<glm::vec4> s_aggloIndexColors;
}

//

class TaskBodyCleave::CleaveCommand : public QUndoCommand
{
public:

  CleaveCommand(TaskBodyCleave *task,
                std::map<uint64_t, std::size_t> meshIdToCleaveIndex,
                const std::vector< QString>& comboBoxItemsText = {}) :
    m_task(task),
    m_meshIdToCleaveIndexBefore(task->m_meshIdToCleaveIndex),
    m_meshIdToCleaveIndexAfter(meshIdToCleaveIndex),
    m_comboBoxItemsTextBefore(getComboBoxItemsText(task->m_cleaveIndexComboBox)),
    m_comboBoxItemsTextAfter(!comboBoxItemsText.empty() ? comboBoxItemsText : m_comboBoxItemsTextBefore),
    m_requestNumber(s_requestNumber++)
  {
    setText("cleaving");

    // Each command to set seeds is followed by an asychronous request to the server to cleave
    // using all seeds set so far. Requests are numbered sequentially, and the server replies
    // can be retrieved from a cache indexed by request number, when the reply is availble.
    // To undo a command, the request number of the previous command is needed, to restore
    // that command's reply, when it is available.

    m_requestNumberBefore = 0;
    if (const CleaveCommand *last = lastCleaveCommand(m_task->m_bodyDoc->undoStack())) {
      m_requestNumberBefore = last->requestNumber();
    }
  }

  unsigned int requestNumber() const
  {
    return m_requestNumber;
  }

  // Add the server reply to a cache, referenced by the request number.

  static void addReply(unsigned int requestNumber,
                       std::map<uint64_t, std::size_t> meshIdToCleaveResultIndex,
                       const QJsonObject &cleaveReply = QJsonObject())
  {
    CleaveReply reply(meshIdToCleaveResultIndex, cleaveReply);
    if (requestNumber == s_cleaveReplyByRequestNumber.size()) {
      s_cleaveReplyByRequestNumber.push_back(reply);
    } else {
      s_cleaveReplyByRequestNumber.resize(requestNumber + 1);
      s_cleaveReplyByRequestNumber[requestNumber] = reply;
    }
  }

  // Display the reply for the given request number.  But do not do so if another request
  // has happened more recently than that request.  Since each request involves all the seeds
  // set so far, the more recent request's reply, when it arrives, will show the earlier
  // reply, too.

  static void displayReply(TaskBodyCleave *task, unsigned int requestNumber)
  {
    if (const CleaveCommand *lastCommand = lastCleaveCommand(task->m_bodyDoc->undoStack())) {
      if (lastCommand->requestNumber() == requestNumber) {
        if (requestNumber < (unsigned int) s_cleaveReplyByRequestNumber.size()) {
          const CleaveReply &reply = s_cleaveReplyByRequestNumber[requestNumber];
          task->m_meshIdToCleaveResultIndex = reply.m_meshIdToCleaveResultIndex;
          if (!reply.m_cleaveReply.isEmpty()) {
            task->m_cleaveReply = reply.m_cleaveReply;
          }
          task->updateColors();
          task->updateVisibility();
        }
      }
    }
  }

  virtual void undo() override
  {
    if (m_requestNumberBefore < (unsigned int) s_cleaveReplyByRequestNumber.size()) {

      // If the reply from the previous command's request is in the cache, then display it.

      const CleaveReply &reply = s_cleaveReplyByRequestNumber[m_requestNumberBefore];
      m_task->m_meshIdToCleaveResultIndex = reply.m_meshIdToCleaveResultIndex;
      m_task->m_cleaveReply = reply.m_cleaveReply;
    }

    m_task->m_meshIdToCleaveIndex = m_meshIdToCleaveIndexBefore;
    m_task->updateColors();
    m_task->updateVisibility();
    setComboBoxItemsText(m_task->m_cleaveIndexComboBox, m_comboBoxItemsTextBefore);
  }

  virtual void redo() override
  {
    if (m_requestNumber < (unsigned int) s_cleaveReplyByRequestNumber.size()) {

      // If the reply from this command's request is in the cache, then display it.

      const CleaveReply &reply = s_cleaveReplyByRequestNumber[m_requestNumber];
      m_task->m_meshIdToCleaveResultIndex = reply.m_meshIdToCleaveResultIndex;
      m_task->m_cleaveReply = reply.m_cleaveReply;
    }

    m_task->m_meshIdToCleaveIndex = m_meshIdToCleaveIndexAfter;
    m_task->updateColors();
    m_task->updateVisibility();
    setComboBoxItemsText(m_task->m_cleaveIndexComboBox, m_comboBoxItemsTextAfter);
  }

  // Clear the static data structures that cache replies, indexed by request numbers,
  // which are needed for undo processing.

  static void clearReplyUndo()
  {
    s_cleaveReplyByRequestNumber.clear();
    s_cleaveReplyByRequestNumber.resize(1);
    s_requestNumber = s_cleaveReplyByRequestNumber.size();
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

  static const CleaveCommand *lastCleaveCommand(const QUndoStack *undoStack)
  {
    int index = undoStack->index();
    while (index > 0) {
      index--;
      if (const CleaveCommand *cleaveCommand =
          dynamic_cast<const CleaveCommand *>(undoStack->command(index))) {
        return cleaveCommand;
      }
    }
    return nullptr;
  }

  TaskBodyCleave *m_task;
  std::map<uint64_t, std::size_t> m_meshIdToCleaveIndexBefore;
  std::map<uint64_t, std::size_t> m_meshIdToCleaveIndexAfter;
  std::vector<QString> m_comboBoxItemsTextBefore;
  std::vector<QString> m_comboBoxItemsTextAfter;

  // The request numbers for the previous command on the undo history,
  // and for this command.

  unsigned int m_requestNumberBefore;
  unsigned int m_requestNumber;

  // The next request number to use for the next commend.

  static unsigned int s_requestNumber;

  // A record in the cache of server replies.

  struct CleaveReply
  {
    CleaveReply(const std::map<uint64_t, std::size_t> &m = {},
                QJsonObject r = QJsonObject()) :
      m_meshIdToCleaveResultIndex(m), m_cleaveReply(r) {}
    std::map<uint64_t, std::size_t> m_meshIdToCleaveResultIndex;
    QJsonObject m_cleaveReply;
  };

  // The cache of server replies, indexed by request numbers.

  static std::vector<CleaveReply> s_cleaveReplyByRequestNumber;
};

unsigned int TaskBodyCleave::CleaveCommand::s_requestNumber = 0;
std::vector<TaskBodyCleave::CleaveCommand::CleaveReply>
  TaskBodyCleave::CleaveCommand::s_cleaveReplyByRequestNumber;

//

TaskBodyCleave::TaskBodyCleave(QJsonObject json, ZFlyEmBody3dDoc* bodyDoc)
{
  m_bodyDoc = bodyDoc;

  applyOverallSettings(bodyDoc);

  loadJson(json);

  // Backwards compatibility: If this task was started with an earlier version of
  // the cleaving tool, the JSON may have saved state that includes a "level 1" mesh,
  // for the overall body (as opposed go the "level 0" meshes for the super voxels).
  // This tool no longer has the "history" slider that could create this mesh, and
  // any such meshes should be filtered out to avoid problems.

  auto clean = [](QSet<uint64_t>& s) {
    QList<uint64_t> r;
    foreach (uint64_t id, s) {
      if (ZFlyEmBodyManager::EncodedLevel(id) > 0) r.append(id);
    }
    foreach (uint64_t id, r) {
      s.erase(s.find(id));
    }
  };
  clean(m_visibleBodies);
  clean(m_selectedBodies);

  buildTaskWidget();

  m_supervisor = new ZFlyEmSupervisor(m_widget);
  m_supervisor->setDvidTarget(m_bodyDoc->getDvidTarget());

  m_networkManager = new QNetworkAccessManager(m_widget);
  connect(m_networkManager, SIGNAL(finished(QNetworkReply*)),
          this, SLOT(onNetworkReplyFinished(QNetworkReply*)));

  if (s_aggloIndexColors.empty()) {
    const QSettings &settings = NeutubeConfig::getInstance().getSettings();
    QVariant value = settings.value(SETTINGS_KEY_AGGLO_COLOR_FILEPATH);
    if (!value.isNull()) {
      loadColorsAgglo(value.toString());
    } else {
      setDefaultColorsAgglo();
    }
  }
}

TaskBodyCleave::~TaskBodyCleave()
{
  if (m_checkedOut) {
    m_supervisor->checkIn(m_bodyId, neutu::EBodySplitMode::NONE);
  }
}

QString TaskBodyCleave::taskTypeStatic()
{
  return VALUE_TASKTYPE;
}

TaskBodyCleave* TaskBodyCleave::createFromJson(QJsonObject json, ZFlyEmBody3dDoc *bodyDoc)
{
  return new TaskBodyCleave(json, bodyDoc);
}

QString TaskBodyCleave::menuLabelCreateFromGuiBodyId()
{
  return "Cleave from Body ID";
}

QJsonArray TaskBodyCleave::createFromGuiBodyId(ZFlyEmBody3dDoc *bodyDoc)
{
  QJsonArray result;

  if (Z3DWindow *window = bodyDoc->getParent3DWindow()) {
    bool ok = false;
    double min = std::numeric_limits<double>::min();
    double max = std::numeric_limits<double>::max();
    double bodyId = QInputDialog::getDouble(
          window, "Create Body Cleave Task", "Body ID", 0, min, max, 0, &ok);
    if (ok && (bodyId != 0)) {
      QJsonObject json;
      json[KEY_BODY_ID] = QJsonValue(bodyId);
      json[KEY_TASKTYPE] = VALUE_TASKTYPE;
      json[KEY_MAXLEVEL] = QJsonValue(1);
      result.append(json);
    }
  }

  return result;
}

QString TaskBodyCleave::menuLabelCreateFromGui3dPoint()
{
  return "Cleave from 3D Point";
}

QJsonArray TaskBodyCleave::createFromGui3dPoint(ZFlyEmBody3dDoc *bodyDoc)
{
  QJsonArray result;

  if (Z3DWindow *window = bodyDoc->getParent3DWindow()) {
    bool ok = false;
    QString ptStr = QInputDialog::getText(window, "Create Body Cleave Task", "3D Point on Body",
                                          QLineEdit::Normal, "x, y, z", &ok,
                                          Qt::Widget, Qt::ImhPreferNumbers);
    if (ok) {
      QStringList compStrs = ptStr.split(",", QString::SkipEmptyParts);
      if (compStrs.size() == 3) {
        QJsonArray ptArray;
        for (QString compStr : compStrs) {
          int comp = compStr.toInt(&ok);
          if (ok) {
            ptArray.append(QJsonValue(comp));
          }
        }
        if (ptArray.size() == 3) {
          QJsonObject json;
          json[KEY_BODY_POINT] = ptArray;
          json[KEY_TASKTYPE] = VALUE_TASKTYPE;
          json[KEY_MAXLEVEL] = QJsonValue(1);
          result.append(json);
        }
      }
    }
  }

  return result;
}

QString TaskBodyCleave::taskType() const
{
  return taskTypeStatic();
}

QString TaskBodyCleave::actionString()
{
  return "Body cleaving:";
}

QString TaskBodyCleave::targetString()
{
  return QString::number(m_bodyId);
}

bool TaskBodyCleave::skip(QString &reason)
{
  if (m_bodyDoc->usingOldMeshesTars()) {
    return false;
  }

  // For now, at least, the "HEAD" command to check whether a tarsupervoxels instance has
  // a complete tar archive may be slow for large bodies.  So avoid executing it repeatedly
  // in rapid succession.

  // An environment variable can override the interval for checking, with a value of
  // -1 meaning never check.

  int interval = 1000;
  if (const char* overrideIntervalStr = std::getenv("NEU3_CLEAVE_SKIP_TEST_INTERVAL_MS")) {
    interval = std::atoi(overrideIntervalStr);
  }
  if (interval < 0) {
    return false;
  }

  int now = QTime::currentTime().msecsSinceStartOfDay();
  if ((m_timeOfLastSkipCheck > 0) && (now - m_timeOfLastSkipCheck < interval)) {
    if (m_skip) {
      reason = "tarsupervoxels HEAD failed";
    }
    return m_skip;
  }
  m_timeOfLastSkipCheck = now;

  QTime timer;
  timer.start();

  ZDvidUrl dvidUrl(m_bodyDoc->getDvidTarget());
  std::string tarUrl = dvidUrl.getTarSupervoxelsUrl(m_bodyId);
  int statusCode = 0;
  dvid::MakeHeadRequest(tarUrl, statusCode);
  m_skip = (statusCode != 200);

  if (m_skip) {
    reason = "tarsupervoxels HEAD failed";
  }

#ifdef _DEBUG_
  if (m_skip) {
    m_skip = false;
    reason += " - Ignore skip for debugging.";
  }
#endif

  LKINFO << QString("TaskBodyCleave::skip() HEAD took %1 ms to decide to %2 body %3").
            arg(timer.elapsed()).arg((m_skip ? "skip" : "not skip")).arg(m_bodyId);

  // Add to the auxiliary output a mention that this task was skipped.

  ZDvidReader reader;
  reader.setVerbose(false);
  if (!reader.open(m_bodyDoc->getDvidTarget())) {
    LKERROR << "TaskBodyCleave::skip() could not open DVID target for reading";
    return m_skip;
  }

  ZDvidWriter writer;
  if (!writer.open(m_bodyDoc->getDvidTarget())) {
    LKERROR << "TaskBodyCleave::skip() could not open DVID target for writing";
    return m_skip;
  }

  writeAuxiliaryOutput(reader, writer);

  return m_skip;
}

void TaskBodyCleave::checkOutCurrent()
{
  if (m_supervisor->isEmpty() || !m_supervisor->getDvidTarget().isSupervised()) {
    m_checkedOut = true;
  } else {
    m_checkedOut = m_supervisor->checkOut(m_bodyId, neutu::EBodySplitMode::NONE);
  }
}

void TaskBodyCleave::checkInCurrent()
{
  if (m_supervisor->isEmpty() || !m_supervisor->getDvidTarget().isSupervised()) {
    m_checkedOut = false;
  } else {
    if (m_checkedOut) {
      m_checkedOut = !m_supervisor->checkIn(m_bodyId, neutu::EBodySplitMode::NONE);
    }
  }
}

void TaskBodyCleave::beforeNext()
{
  checkInCurrent();

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

  s_startupTimer.start();
}

//#Review-TZ: It duplicates code from askBodyCleave::beforeNext
void TaskBodyCleave::beforePrev()
{
  /*
  if (m_checkedOut) {
    m_checkedOut = !m_supervisor->checkIn(m_bodyId, neutu::EBodySplitMode::NONE);
  }
  */
  checkInCurrent();

  applyPerTaskSettings();

  // See the comment in beforeNext().

  m_bodyDoc->clearGarbage(true);

  // See the comment in beforeNext().

  m_hiddenCleaveIndices.clear();
  m_showBodyCheckBox->setChecked(true);

  m_hiddenIds.clear();

  s_startupTimer.start();
}

void TaskBodyCleave::beforeLoading()
{
  // For the first task, beforeNext() or beforePrev() will not have been called
  // to start the timer, so starting it here is the next best choice.

  if (s_startupTimer.isNull()) {
    s_startupTimer.start();
  }

  KLog::SetOperationName("body_cleaving");

  checkOutCurrent();
  if (!m_checkedOut) {
    std::string owner = m_supervisor->getOwner(m_bodyId);

    // Bodies involved in Janelia cleaving assignments are checked out by a particular user
    // to prevent them from being modified (e.g., merged into other bodies).  When such a
    // body is being cleaved, this special check-out should be overridden.

    std::string overridableOwner = "production_cleaving";
    if (const char *s = std::getenv("NEU3_CLEAVE_OVERRIDABLE_OWNER")) {
      overridableOwner = std::string(s);
    }

    if (owner == overridableOwner) {
      LKINFO << "TaskBodyCleave overriding checkout by " + owner;
      m_supervisor->checkInAdmin(m_bodyId);
      m_checkedOut = m_supervisor->checkOut(m_bodyId, neutu::EBodySplitMode::NONE);
    }
  }

  flyem::LogBodyOperation("start cleaving", m_bodyId, neutu::EBodyLabelType::BODY);
  /*
  KLOG << ZLog::Info()
       << ZLog::Action("start cleaving")
       << ZLog::Object("body", "", std::to_string(m_bodyId));
       */
}

namespace {
  std::string getOutputServerResponseValue(const ZJsonArray &output, const char *key)
  {
    std::string result;
    ZJsonValue last = ZJsonValue(output.at(output.size() - 1), ZJsonValue::SET_INCREASE_REF_COUNT);
    if (last.isObject()) {
      ZJsonObject obj(last);
      result = obj.value(key).toString();
    }
    return result;
  }
}

void TaskBodyCleave::onLoaded()
{
  m_usageTimer.start();

  m_cleavingStatusLabel->setText(CLEAVING_STATUS_DONE);
  if (!m_checkedOut) {
    m_widget->setEnabled(false);
    m_menu->menuAction()->setEnabled(false);

    std::string owner = m_supervisor->getOwner(m_bodyId);
    std::string msg = "Cannot cleave ";
    msg += !owner.empty() ? "body<br>checked out by " + owner : " locked body";
    m_cleavingStatusLabel->setText(msg.c_str());
  } else if (m_bodyDoc->usingOldMeshesTars()) {

    // The new "tarsupervoxels" DVID data type for storing tar archives of super voxel meshes
    // gets updated automatically whenever a DVID cleaving command is issued.  But if we are
    // using the old key-value for storing the archives of meshes, then the archives may not
    // have been updated when we revisit an already cleaved body.  So issue a warning.

    ZDvidReader reader;
    reader.setVerbose(false);
    if (reader.open(m_bodyDoc->getDvidTarget())) {
      ZJsonArray output = readAuxiliaryOutput(reader);
      if (output.size() > 0) {
        QString info;
        std::string user = getOutputServerResponseValue(output, "user");
        std::string timestamp = getOutputServerResponseValue(output, "request-timestamp");
        if (!user.empty() && !timestamp.empty()) {
          info = "(by " + QString::fromStdString(user) + " at " + QString::fromStdString(timestamp) + ") ";
        }
        QString text = "Warning: Body " + QString::number(m_bodyId) + " has been cleaved " + info +
            "so its supervoxels may be out of date.";
        ZWidgetMessage msg(text);
        m_bodyDoc->notify(msg);
      }
    }
  }

  m_meshIdToAggloIndex.clear();
  updateMeshIdToAggloIndex();

  m_startupTimes.push_back(s_startupTimer.elapsed());
}

void TaskBodyCleave::beforeDone()
{
  checkInCurrent();
  restoreOverallSettings(m_bodyDoc);

  flyem::LogBodyOperation("end cleavng", m_bodyId, neutu::EBodyLabelType::BODY);
  /*
  KLOG << ZLog::Info()
       << ZLog::Action("end cleaving")
       << ZLog::Object("body", "", std::to_string(m_bodyId));
       */

  KLog::ResetOperationName();
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
  if (!uiIsEnabled()) {
    return;
  }

  bool show = (state != Qt::Unchecked);
  enableCleavingUI(show);
  bool colorBySupervoxels = m_colorSupervoxelsButton->isChecked();
  applyColorMode(show, colorBySupervoxels);

  KINFO << QString("Show cleaving: %1").arg(show);
}

void TaskBodyCleave::onToggleShowCleaving()
{
  if (!uiIsEnabled()) {
    return;
  }

  m_showCleavingCheckBox->setChecked(!m_showCleavingCheckBox->isChecked()); 
}

void TaskBodyCleave::onShowSeedsOnlyChanged(int)
{
  if (!uiIsEnabled()) {
    return;
  }

  updateColors();

  KINFO << QString("Show seeds only: %1").arg(m_showSeedsOnlyCheckBox->isChecked());
}

void TaskBodyCleave::onToggleShowSeedsOnly()
{
  if (!uiIsEnabled()) {
    return;
  }

  m_showSeedsOnlyCheckBox->setChecked(!m_showSeedsOnlyCheckBox->isChecked());
}

void TaskBodyCleave::onCleaveIndexShortcut()
{
  if (!uiIsEnabled()) {
    return;
  }

  if (QAction* action = dynamic_cast<QAction*>(QObject::sender())) {
    int i = m_actionToComboBoxIndex[action];
    m_cleaveIndexComboBox->setCurrentIndex(i);

    KINFO << QString("Set cleave index to %1").arg(i);
  }
}

void TaskBodyCleave::onCleaveIndexChanged(int)
{
  if (!uiIsEnabled()) {
    return;
  }

  bool visible = (m_hiddenCleaveIndices.find(chosenCleaveIndex()) ==
                  m_hiddenCleaveIndices.end());
  m_showBodyCheckBox->setChecked(visible);
}

void TaskBodyCleave::onSelectBody()
{
  if (!uiIsEnabled()) {
    return;
  }

  std::set<uint64_t> toSelect;
  bodiesForCleaveIndex(toSelect, chosenCleaveIndex());
  selectBodies(toSelect);

  KINFO << QString("Select %1 bodies").arg(toSelect.size());
}

void TaskBodyCleave::onShowBodyChanged(int state)
{
  if (!uiIsEnabled()) {
    return;
  }

  if (state) {
    m_hiddenCleaveIndices.erase(chosenCleaveIndex());
  } else {
    m_hiddenCleaveIndices.insert(chosenCleaveIndex());
  }

  std::set<uint64_t> bodiesForIndex;
  bodiesForCleaveIndex(bodiesForIndex, chosenCleaveIndex(), true);

  selectBodies(bodiesForIndex, false);

  updateVisibility();

  KINFO << QString("Show body: %1").arg(state);
}

void TaskBodyCleave::onToggleInChosenCleaveBody()
{
  updateChosenCleaveBody(true);
}

void TaskBodyCleave::onAddToChosenCleaveBody()
{
  updateChosenCleaveBody(false);
}

void TaskBodyCleave::updateChosenCleaveBody(bool toggle)
{
  if (!uiIsEnabled()) {
    return;
  }

  KINFO << QString("Toggle chosen cleave body: %1").arg(toggle);

  const TStackObjectSet &selectedMeshes = m_bodyDoc->getSelected(ZStackObject::EType::MESH);
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
    } else if (toggle) {

      // The selected mesh has been assigned to the current color, so if toggling, it is being assigned
      // no color, and the current color's count decreases by one.

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

  CleaveCommand *command = new CleaveCommand(this, meshIdToCleaveIndex, itemTexts);
  m_bodyDoc->pushUndoCommand(command);

  cleave(command->requestNumber());
}

void TaskBodyCleave::onToggleShowChosenCleaveBody()
{
  if (!uiIsEnabled()) {
    return;
  }

  m_showBodyCheckBox->setChecked(!m_showBodyCheckBox->isChecked());
}

void TaskBodyCleave::onNetworkReplyFinished(QNetworkReply *reply)
{
  if (reply->url().path().contains("mapping")) {
    aggloIndexReplyFinished(reply);
  } else {
    cleaveServerReplyFinished(reply);
  }
}

void TaskBodyCleave::aggloIndexReplyFinished(QNetworkReply *reply)
{
  QNetworkReply::NetworkError error = reply->error();
  if (error != QNetworkReply::NoError) {
    displayWarning("Agglo Index Server Error", "Reply error: " + reply->errorString(), "", true);
    return;
  }

  int statusCode = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
  const int STATUS_OK = 200;
  if (statusCode != STATUS_OK) {
    displayWarning("Agglo Index Server Error", "Status code: " + QString::number(statusCode), "", true);
    return;
  }

  QByteArray replyBytes = reply->readAll();
  QJsonDocument replyJsonDoc = QJsonDocument::fromJson(replyBytes);
  if (!replyJsonDoc.isArray()) {
    displayWarning("Agglo Index Server Error", "Reply is not a JSON array", "", true);
    return;
  }

  QJsonArray replyJsonArray = replyJsonDoc.array();
  QList<ZMesh*> meshes = ZStackDocProxy::GetGeneralMeshList(m_bodyDoc);
  int i = 0;
  for (auto itMesh = meshes.cbegin(); itMesh != meshes.cend(); itMesh++) {
    ZMesh *mesh = *itMesh;
    uint64_t id = mesh->getLabel();
    std::size_t aggloIndex = std::size_t(replyJsonArray[i].toInt());

    // If the query supervoxel ID is not in the mapping to agglomeration indices, the reply will
    // siply return the ID again.  In this case, treat it as agglomeration index 1.
    if (aggloIndex == id)
      aggloIndex = 1;

    m_meshIdToAggloIndex[id] = aggloIndex;
    ++i;
  }
}

void TaskBodyCleave::cleaveServerReplyFinished(QNetworkReply *reply)
{
  allowNextPrev(true);

  m_cleaveRepliesPending--;
  QNetworkReply::NetworkError error = reply->error();

  // In addition to checking for QNetworkReply::NetworkError, also check for
  // a status code other than OK (200), to be extra certain about catching errors.

  int statusCode = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
  const int STATUS_OK = 200;

  QString status = (m_cleaveRepliesPending > 0) ? CLEAVING_STATUS_IN_PROGRESS : CLEAVING_STATUS_DONE;

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
      foreach (const QString &key, replyJsonAssn.keys()) {
        uint64_t cleaveIndex = key.toInt();
        QJsonValue value = replyJsonAssn[key];
        if (value.isArray()) {
          foreach (const QJsonValue &idVal, value.toArray()) {
            uint64_t id = idVal.toDouble();
            meshIdToCleaveIndex[id] = cleaveIndex;
          }
        }
      }

      std::set<std::size_t> hiddenChangedIndices = hiddenChanges(meshIdToCleaveIndex);

      unsigned int replyRequestNumber = replyJson["request-number"].toInt();
      CleaveCommand::addReply(replyRequestNumber, meshIdToCleaveIndex, replyJson);
      CleaveCommand::displayReply(this, replyRequestNumber);

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
          foreach (const QJsonValue &error, errorsArray) {
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
  if (!uiIsEnabled()) {
    return;
  }

  KINFO << "Hide selected bodies";

  const TStackObjectSet &selectedMeshes = m_bodyDoc->getSelected(ZStackObject::EType::MESH);
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

void TaskBodyCleave::onClearHidden()
{
  if (!uiIsEnabled()) {
    return;
  }

  KINFO << "Clear hidden bodies";

  selectBodies(m_hiddenIds);
  m_hiddenIds.clear();
  updateVisibility();
}

void TaskBodyCleave::onChooseCleaveMethod()
{
  if (!uiIsEnabled()) {
    return;
  }

  if (Z3DWindow *window = m_bodyDoc->getParent3DWindow()) {
    bool ok = true;
    QString text = QInputDialog::getText(window, "Set Cleaving Method", "Cleaving method:",
                                         QLineEdit::Normal,  m_cleaveMethod, &ok);
    if (ok) {
      m_cleaveMethod = text;
    }

    KINFO << "Choose cleaving method: " + text;
  }
}

void TaskBodyCleave::onColorByChanged()
{
  bool colorBySupervoxels = m_colorSupervoxelsButton->isChecked();
  applyColorMode(false, colorBySupervoxels);
}

void TaskBodyCleave::onSpecifyAggloColor()
{
  QString filepath =
      QFileDialog::getOpenFileName(m_widget, tr("Agglo color specification file"), "", tr("JSON Files (*.json)"));
  if (!filepath.isEmpty()) {
    if (loadColorsAgglo(filepath)) {
      updateColorsAgglo();
      QSettings &settings = NeutubeConfig::getInstance().getSettings();
      settings.setValue(SETTINGS_KEY_AGGLO_COLOR_FILEPATH, filepath);
    }
  }
}

bool TaskBodyCleave::loadColorsAgglo(QString filepath)
{
  QFile file(filepath);
  if (!file.open(QIODevice::ReadOnly)) {
    displayWarning("Error opening file", "Cannot open color JSON file '" + filepath + "'");
    return false;
  }
  QByteArray data = file.readAll();
  QJsonDocument jsonDoc = QJsonDocument::fromJson(data);
  if (jsonDoc.isNull() || !jsonDoc.isObject()) {
    displayWarning("Error parsing file", "Cannot parse color JSON file '" + filepath + "'");
    return false;
  }
  QJsonObject jsonObj = jsonDoc.object();
  if (!jsonObj.contains(COLOR_SPEC_KEY_AGGLO_ID_COLORS)) {
    displayWarning("Error parsing file", "Expected key \"" + COLOR_SPEC_KEY_AGGLO_ID_COLORS + "\"");
    return false;
  }
  QJsonValue jsonColorsValue = jsonObj[COLOR_SPEC_KEY_AGGLO_ID_COLORS];
  if (!jsonColorsValue.isArray()) {
    displayWarning("Error parsing file", "Expected key \"" + COLOR_SPEC_KEY_AGGLO_ID_COLORS + "\" to have an array value");
    return false;
  }
  QJsonArray jsonColorsArr = jsonColorsValue.toArray();

  std::vector<glm::vec4> newIndexColors;
  for (int i = 0; i < jsonColorsArr.count(); ++i) {
    QJsonValue jsonColorValue = jsonColorsArr[i];
    if (!jsonColorValue.isString()) {
      displayWarning("Error parsing file", "Invalid color " + QString::number(i) + " which is not a string");
      return false;
    }
    QString colorStr = jsonColorValue.toString();
    QColor color = QColor(colorStr);
    if (!color.isValid()) {
      displayWarning("Error parsing file", "Invalid color " + QString::number(i));
      return false;
    }
    glm::vec4 colorVec = glm::vec4(color.redF(), color.greenF(), color.blueF(), 1.0f);
    newIndexColors.push_back(colorVec);
  }

  if (newIndexColors.size() >= 3) {
    s_aggloIndexColors.clear();
    for (glm::vec4 colorVec : newIndexColors) {
      s_aggloIndexColors.push_back(colorVec);
    }
  } else {
    displayWarning("Error parsing file", "At least 3 valid colors are required");
    return false;
  }

  return true;
}

void TaskBodyCleave::setDefaultColorsAgglo()
{
  s_aggloIndexColors.clear();
  for (glm::vec4 colorVec : DEFAULT_AGGLO_INDEX_COLORS) {
    s_aggloIndexColors.push_back(colorVec);
  }
}

QJsonObject TaskBodyCleave::addToJson(QJsonObject taskJson)
{
  if (m_bodyPt.isApproxOrigin()) {
    taskJson[KEY_BODY_ID] = static_cast<double>(m_bodyId);
  } else {
    QJsonArray array;
    array.append(int(m_bodyPt.x()));
    array.append(int(m_bodyPt.y()));
    array.append(int(m_bodyPt.z()));
    taskJson[KEY_BODY_POINT] = array;
  }
  taskJson[KEY_TASKTYPE] = VALUE_TASKTYPE;
  taskJson[KEY_MAXLEVEL] = m_maxLevel;

  return taskJson;
}

bool TaskBodyCleave::allowCompletion()
{
  bool allow = true;

  if (m_cleaveRepliesPending > 0) {
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

  std::vector<uint64_t> unassigned;
  if (getUnassignedMeshes(unassigned)) {
    allow = false;
    if (Z3DWindow *window = m_bodyDoc->getParent3DWindow()) {
      FlyEmCleaveUnassignedDialog *dlg =
          new FlyEmCleaveUnassignedDialog(m_bodyDoc->getParent3DWindow());
      std::size_t indexNotCleavedOff = getIndexNotCleavedOff();
      dlg->setIndexNotCleavedOff(indexNotCleavedOff);
      dlg->setUnassignedCount(unassigned.size());
      if (dlg->exec() &&
          dlg->getOption() != FlyEmCleaveUnassignedDialog::EOption::NONE) {
        auto option = dlg->getOption();
        if (option != FlyEmCleaveUnassignedDialog::EOption::NONE) {
          CleaveCommand *command = new CleaveCommand(this, m_meshIdToCleaveIndex);
          m_bodyDoc->pushUndoCommand(command);

          std::map<uint64_t, std::size_t> meshIdToCleaveResultIndex(m_meshIdToCleaveResultIndex);

          if (!unassigned.empty()) {
            std::size_t maxIndex = 0;
            for (auto it : m_meshIdToCleaveIndex) {
              if (maxIndex < it.first) {
                maxIndex = it.first;
              }
            }

            for (uint64_t id : unassigned) {
              switch (option) {
              case FlyEmCleaveUnassignedDialog::EOption::MAIN_BODY:
                meshIdToCleaveResultIndex[id] = indexNotCleavedOff;
                break;
              case FlyEmCleaveUnassignedDialog::EOption::NEW_BODY:
                meshIdToCleaveResultIndex[id] = ++maxIndex;
                break;
              default:
                throw std::runtime_error("Invalid option for cleaving unassigned supervoxels.");
                break;
              }
            }
          }
          CleaveCommand::addReply(command->requestNumber(), meshIdToCleaveResultIndex);
          CleaveCommand::displayReply(this, command->requestNumber());
          allow = true;
        }

      /*
      QString title = "Warning";
      QString text = (unassigned.size() == 1) ? "A supervoxel is unassigned. " : "Some supervoxels are unassigned. ";


      bool cleavingUnassigned = false;
      if (cleavingUnassigned) {
        text += "Saving each of then as a new body?";
      } else {
        text += "Save them with the main body (seed color " + QString::number(indexNotCleavedOff) + ")?";
      }

      QMessageBox::StandardButton chosen =
          QMessageBox::warning(window, title, text, QMessageBox::Save | QMessageBox::Cancel,
                               QMessageBox::Cancel);
      if (chosen == QMessageBox::Save) {
        CleaveCommand *command = new CleaveCommand(this, m_meshIdToCleaveIndex);
        m_bodyDoc->pushUndoCommand(command);

        std::map<uint64_t, std::size_t> meshIdToCleaveResultIndex(m_meshIdToCleaveResultIndex);

        if (!unassigned.empty()) {
          std::size_t maxIndex = 0;
          for (auto it : m_meshIdToCleaveIndex) {
            if (maxIndex < it.first) {
              maxIndex = it.first;
            }
          }

          for (uint64_t id : unassigned) {
            if (cleavingUnassigned) {
              meshIdToCleaveResultIndex[id] = ++maxIndex;
            } else {
              meshIdToCleaveResultIndex[id] = indexNotCleavedOff;
            }
          }
        }
        CleaveCommand::addReply(command->requestNumber(), meshIdToCleaveResultIndex);
        CleaveCommand::displayReply(this, command->requestNumber());
        */
      }
    }
  }

  return allow;
}

size_t TaskBodyCleave::getSupervoxelSize(uint64_t svId) const
{
  return m_bodyDoc->getSupervoxelSize(svId);
}

void TaskBodyCleave::boostSupervoxelSizeRetrieval(
    const std::map<std::size_t, std::vector<uint64_t> > &cleaveIndexToMeshIds,
    size_t indexNotCleavedOff)
const
{
  std::vector<uint64_t> supervoxelList;

  for (const auto &element : cleaveIndexToMeshIds) {
    for (const uint64_t svId : element.second){
      if (element.first != indexNotCleavedOff) {
        supervoxelList.push_back(svId);
      }
    }
  }

  m_bodyDoc->cacheSupervoxelSize(supervoxelList);
}

bool TaskBodyCleave::cleaveVerified(
    const std::map<std::size_t, std::vector<uint64_t> > &cleaveIndexToMeshIds,
    size_t indexNotCleavedOff) const
{
//  ZFlyEmBodyAnnotation annot = FlyEmDataReader::ReadBodyAnnotation(
//        m_bodyDoc->getMainDvidReader(), m_bodyId);

  ZJsonObject annot = m_bodyDoc->getBodyAnnotation(m_bodyId);

  if (!annot.isEmpty()) {
//    size_t mainSize = 0;
    size_t cleaveSize = 0;

    boostSupervoxelSizeRetrieval(cleaveIndexToMeshIds, indexNotCleavedOff);

    for (const auto &element : cleaveIndexToMeshIds) {
      for (const uint64_t svId : element.second){
        if (element.first != indexNotCleavedOff) {
          size_t svSize = getSupervoxelSize(svId);
          cleaveSize += svSize;
        }
      }
    }

    size_t bodySize = m_bodyDoc->getMainDvidReader().readBodySize(
          m_bodyId, neutu::EBodyLabelType::BODY);

    if (cleaveSize + cleaveSize > bodySize) {
      const ZFlyEmBodyAnnotationProtocol &bodyStatusProtocol =
          m_bodyDoc->getBodyStatusProtocol();
      if (!bodyStatusProtocol.isEmpty()) {
        if (bodyStatusProtocol.preservingId(
              ZFlyEmBodyAnnotation::GetStatus(annot))) {
          ZDialogFactory::Warn(
                "Cleave Forbidden",
                QString("You cannot cleave off a large portion "
                        "of this body because its ID should be preserved."),
                m_bodyDoc->getParent3DWindow());

          return false;
        }
      }

      return ZDialogFactory::WarningAskForContinue(
            "Confirm Cleaving",
            "You are about to cleave off a large portion of an annotated body.",
            m_bodyDoc->getParent3DWindow());
    }
  }

  return true;
}

void TaskBodyCleave::onCompleted()
{
  m_usageTimes.push_back(m_usageTimer.elapsed());

  // Restart the timer, to measure the time if the user reconsiders and
  // reaches a new decision for this task (without moving on and then coming
  // back to this task).

  m_usageTimer.start();

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

  if (cleaveIndexToMeshIds.empty()) {
    return;
  }

  for (auto &pair : cleaveIndexToMeshIds) {
    std::sort(pair.second.begin(), pair.second.end());
  }

  std::size_t indexNotCleavedOff = getIndexNotCleavedOff();
  std::vector<QString> responseLabels;
  std::vector<uint64_t> mutationIds;
  bool doWriteOutput = true;
  if (const char* doWriteOutputStr = std::getenv("NEU3_CLEAVE_UPDATE_SEGMENTATION")) {

    // For now, at least, allow direct updating of the DVID segmentation to be disabled
    // by an environment variable.

    doWriteOutput = (std::string(doWriteOutputStr) != "no");
  }
  bool succeeded = false;
  if (doWriteOutput) {
    if (cleaveVerified(cleaveIndexToMeshIds, indexNotCleavedOff)) {
      succeeded = writeOutput(
            writer, cleaveIndexToMeshIds, indexNotCleavedOff,
            responseLabels, mutationIds);
    }
  }
  writeAuxiliaryOutput(
        reader, writer, cleaveIndexToMeshIds, responseLabels, mutationIds);

  if (succeeded) {

    // Hide the super voxels that are being cleaved off.

    m_hiddenCleaveIndices.clear();
    for (const auto &it : cleaveIndexToMeshIds) {
      std::size_t cleaveIndex = it.first;
      if (cleaveIndex != indexNotCleavedOff) {
        m_hiddenCleaveIndices.insert(cleaveIndex);
      }
    }
    updateVisibility();
  }
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

  m_colorSupervoxelsButton = new QRadioButton("Color by supervoxels", m_widget);
  m_colorSupervoxelsButton->setChecked(true);
  connect(m_colorSupervoxelsButton, SIGNAL(toggled(bool)), this, SLOT(onColorByChanged()));

  m_colorAggloButton = new QRadioButton("Color by agglo", m_widget);
  connect(m_colorAggloButton, SIGNAL(toggled(bool)), this, SLOT(onColorByChanged()));

  m_specifyAggloColorButton = new QPushButton("Specify...", m_widget);
  connect(m_specifyAggloColorButton, SIGNAL(clicked(bool)), this, SLOT(onSpecifyAggloColor()));

  QVBoxLayout *cleaveIndexActionsLayout = new QVBoxLayout;
  cleaveIndexActionsLayout->addWidget(m_selectBodyButton);
  cleaveIndexActionsLayout->addWidget(m_showBodyCheckBox);

  QHBoxLayout *cleaveLayout1 = new QHBoxLayout;
  cleaveLayout1->addWidget(m_showCleavingCheckBox);
  cleaveLayout1->addWidget(m_cleaveIndexComboBox);
  cleaveLayout1->addLayout(cleaveIndexActionsLayout);

  QHBoxLayout *cleaveLayout3 = new QHBoxLayout;
  cleaveLayout3->addWidget(m_colorSupervoxelsButton);
  QHBoxLayout *cleaveLayout3a = new QHBoxLayout;
  cleaveLayout3a->addWidget(m_colorAggloButton);
  cleaveLayout3a->addWidget(m_specifyAggloColorButton);
  cleaveLayout3->addLayout(cleaveLayout3a);

  m_showSeedsOnlyCheckBox = new QCheckBox("Show seeds only", m_widget);
  m_showSeedsOnlyCheckBox->setChecked(false);
  connect(m_showSeedsOnlyCheckBox, SIGNAL(stateChanged(int)), this, SLOT(onShowSeedsOnlyChanged(int)));

  m_cleavingStatusLabel = new QLabel(CLEAVING_STATUS_DONE, m_widget);

  QHBoxLayout *cleaveLayout2 = new QHBoxLayout;
  cleaveLayout2->addWidget(m_showSeedsOnlyCheckBox);
  cleaveLayout2->addWidget(m_cleavingStatusLabel);

  QVBoxLayout *layout = new QVBoxLayout;
  layout->addLayout(cleaveLayout1);
  layout->addLayout(cleaveLayout3);
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

  m_addToBodyAction = new QAction("Add Selection to Current Body", m_widget);
  m_addToBodyAction->setShortcut(Qt::SHIFT + Qt::Key_Space);
  m_menu->addAction(m_addToBodyAction);
  connect(m_addToBodyAction, SIGNAL(triggered()), this, SLOT(onAddToChosenCleaveBody()));

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

bool TaskBodyCleave::uiIsEnabled() const
{
  // For some reason, Qt will call the slot connected to a menu action even when the menu
  // is disabled.  That can cause problems in this task, where all parts of the user interface
  // have keyboard shortcuts implemented with menu actions, and the user interface needs to
  // be inactive in certain cases (e.g., when the TaskProtocolWindow is waiting for the
  // meshes from the previous task to be deleted and the meshes from the current task to be
  // loaded, or when a tasks cannot check out and lock a body).  So every slot associated
  // with a menu action needs to return immediately if this function returns false.

  return (m_menu->isEnabled() && m_widget->isEnabled());
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

void TaskBodyCleave::updateMeshIdToAggloIndex()
{
  if (m_meshIdToAggloIndex.empty()) {
    QString strArray = "[";
    QList<ZMesh*> meshes = ZStackDocProxy::GetGeneralMeshList(m_bodyDoc);
    for (auto itMesh = meshes.cbegin(); itMesh != meshes.cend(); itMesh++) {
      ZMesh *mesh = *itMesh;
      uint64_t id = mesh->getLabel();
      if (strArray.at(strArray.size() - 1) != '[')
        strArray += ", ";
      strArray += QString::number(id);
    }
    strArray += "]";
    QByteArray byteArray = strArray.toUtf8();

    std::string server = "http://" + m_bodyDoc->getDvidTarget().getAddressWithPort();
    std::string uuid = m_bodyDoc->getDvidTarget().getUuid();
    std::string baseName = m_bodyDoc->getDvidTarget().getBodyLabelName();
    std::string api = server + "/api/node/" + uuid + "/" + baseName + "_agglo_levels/mapping";

    QUrl url(api.c_str());
    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");

    m_networkManager->sendCustomRequest(request, "GET", byteArray);
  }
}

void TaskBodyCleave::updateColorsAgglo()
{
  if (Z3DMeshFilter *filter = getMeshFilter(m_bodyDoc)) {
    filter->setColorIndexing(s_aggloIndexColors, m_meshIdToAggloIndex);
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

void TaskBodyCleave::selectBodies(const std::set<uint64_t> &bodies, bool select)
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

void TaskBodyCleave::applyPerTaskSettings()
{
  // The SetCleaveIndicesCommand and CleaveCommand instances on the undo stack contain information
  // particular to the task that was current when the commands were issued.  So the undo stack will
  // not make sense when switching to another task, and the easiest solution is to clear it.

  m_bodyDoc->undoStack()->clear();

  CleaveCommand::clearReplyUndo();

  bool showingCleaving = m_showCleavingCheckBox->isChecked();
  bool colorBySupervoxels = m_colorSupervoxelsButton->isChecked();
  applyColorMode(showingCleaving, colorBySupervoxels);
}

void TaskBodyCleave::applyColorMode(bool showingCleaving, bool colorBySupervoxels)
{
  if (Z3DMeshFilter *filter = getMeshFilter(m_bodyDoc)) {
    if (showingCleaving) {
      updateColors();
      filter->setColorMode("Indexed Color");
    } else if (colorBySupervoxels) {
      filter->setColorMode("Mesh Source");
    } else {
      updateColorsAgglo();
      filter->setColorMode("Indexed Color");
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
  m_colorSupervoxelsButton->setEnabled(!showingCleaving);
  m_colorAggloButton->setEnabled(!showingCleaving);
  m_specifyAggloColorButton->setEnabled(!showingCleaving);
}

QString TaskBodyCleave::getCleaveServer() const
{
  return ZGlobal::GetInstance().getCleaveServer();
}

QString TaskBodyCleave::GetCleaveServerApi(const QString &server)
{
  if (!server.isEmpty()) {
    return server + "/compute-cleave";
  }

  return "";
}

QString TaskBodyCleave::getCleaveServerApi() const
{
  return GetCleaveServerApi(getCleaveServer());
}

void TaskBodyCleave::cleave(unsigned int requestNumber)
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
  if (m_bodyDoc->usingOldMeshesTars()) {
    requestJson["mesh-instance"] =
        ZDvidData::GetName(ZDvidData::ERole::MESHES_TARS,
                           ZDvidData::ERole::SPARSEVOL,
                           m_bodyDoc->getDvidTarget().getBodyLabelName()).c_str();
  } else {
    requestJson["mesh-instance"] =
        ZDvidData::GetName(ZDvidData::ERole::TAR_SUPERVOXELS,
                           ZDvidData::ERole::SPARSEVOL,
                           m_bodyDoc->getDvidTarget().getBodyLabelName()).c_str();
  }

  requestJson["request-number"] = int(requestNumber);

  QString server = getCleaveServerApi();

  QUrl url(server);
  QNetworkRequest request(url);
  request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");

  QJsonDocument requestJsonDoc(requestJson);
  QByteArray requestData(requestJsonDoc.toJson());

  allowNextPrev(false);

  m_cleaveRepliesPending++;
  m_networkManager->post(request, requestData);

  KINFO << QString("Cleave posted: ") + QString(requestData);
}

QString TaskBodyCleave::getInfo() const
{
  QString server = getCleaveServer();
  if (server.isEmpty()) {
    return TaskProtocolTask::getInfo();
  }

  return "Cleaving server: " + server;
}

QString TaskBodyCleave::getError() const
{
  QString server = getCleaveServer();
  if (server.isEmpty()) {
    return "Cleaving CANNOT be done: cleaving server missing!";
  }

  if (!ZNetworkUtils::IsAvailable(GetCleaveServerApi(server), "HAS_OPTIONS")) {
    return "Cleaving CANNOT be done:<br>--unable to connect to " +
        server + " for cleaving.";
  }

  return "";
}

bool TaskBodyCleave::getUnassignedMeshes(std::vector<uint64_t> &result) const
{
  for (auto it : m_meshIdToCleaveResultIndex) {
    std::size_t cleaveIndex = it.second;
    if (cleaveIndex == 0) {
      result.push_back(it.first);
    }
  }
  return (!result.empty());
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

    // Set the visibility of the mesh in a way that will be processed once, with the final
    // processObjectModified() call.  This approach is important to maintain good performance
    // for a large number of meshes.

    mesh->setVisible(toBeVisible);
    m_bodyDoc->bufferObjectModified(mesh, ZStackObjectInfo::STATE_VISIBITLITY_CHANGED);
  }

  m_bodyDoc->processObjectModified();
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
        std::size_t index = itCleave->second;

        auto itNewCleave = newMeshIdToCleaveIndex.find(id);
        if (itNewCleave != newMeshIdToCleaveIndex.end()) {
          std::size_t indexNew = itNewCleave->second;
          if (index != indexNew) {
            bool newHidden = (m_hiddenCleaveIndices.find(indexNew) !=
                              m_hiddenCleaveIndices.end());
            std::size_t indexHidden = newHidden ? indexNew : index;
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
        displayWarning(title, text, "", true);
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
    for (std::size_t i = 0; i < missingIds.size(); i++) {
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
  if (allowSuppression && (s_warningTextToSuppress.find(text) != s_warningTextToSuppress.end())) {
    return;
  }

  // Display the warning at idle time, after the rendering event has been processed,
  // so the results of cleaving will be visible when the warning is presented.

  QTimer::singleShot(0, this, [=](){
    if (details.isEmpty() && !allowSuppression) {
      ZWidgetMessage msg(title, text, neutu::EMessageType::WARNING,
                         ZWidgetMessage::TARGET_DIALOG);
      m_bodyDoc->notify(msg);
    } else {
      QMessageBox msgBox(QMessageBox::Warning, title, text,
                         QMessageBox::NoButton, m_bodyDoc->getParent3DWindow());
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
        s_warningTextToSuppress.insert(text);
      }
    }
  });
}

std::size_t TaskBodyCleave::getIndexNotCleavedOff() const
{
  // Do not cleave off of the original body the group of supervoxels assigned to the first index;
  // by convention, those supervoxels define the new version of the original body.  But skip past
  // supervoxels with index 0, which are the supervoxels that could not be assigned by the
  // cleaving server (because they are not reachable from the seeds).

  std::size_t result = std::numeric_limits<std::size_t>::max();
  for (auto it : m_meshIdToCleaveIndex) {
    std::size_t index = it.second;
    if (index != 0) {
      if (index == 1) {

        // By convention, users usually choose 1 as the first index.  If it is ever found,
        // we can return it immediately, as they can be no lower non-zero index.

        result = 1;
        break;
      }
      result = std::min(result, index);
    }
  }
  return result;
}

bool TaskBodyCleave::writeOutput(ZDvidWriter &writer,
                                 const std::map<std::size_t, std::vector<uint64_t> > &cleaveIndexToMeshIds,
                                 const std::size_t &indexNotCleavedOff,
                                 std::vector<QString> &responseLabels,
                                 std::vector<uint64_t> &mutationIds)
{
  std::string instance = writer.getDvidTarget().getBodyLabelName();
  ZDvidUrl url(writer.getDvidTarget());
  std::string urlCleave = url.getNodeUrl() + "/" + instance + "/cleave/" + std::to_string(m_bodyId);

  std::size_t i = 0;
  for (const auto &pair : cleaveIndexToMeshIds) {
    std::size_t cleaveIndex = pair.first;
    const std::vector<uint64_t> &ids = pair.second;

    if (cleaveIndex != indexNotCleavedOff) {
      ZJsonArray jsonBody;
      for (uint64_t id : ids) {
        jsonBody.append(id);
      }

      // For error reporting.

      QString frac = QString::number(i) + " of " + QString::number(cleaveIndexToMeshIds.size());

      std::string response = writer.post(
            ZDvidUrl::AppendSourceQuery(urlCleave), jsonBody);
      QString labelStr;
      if (writer.isStatusOk()) {
        QJsonDocument responseDoc = QJsonDocument::fromJson(response.c_str());
        if (responseDoc.isObject())  {
          QJsonObject responseObj = responseDoc.object();

          uint64_t mutationId = 0;
          QJsonValue mutationIdVal = responseObj.value("MutationID");
          if (!mutationIdVal.isUndefined()) {
            mutationId = mutationIdVal.toDouble();
          }

          QJsonValue labelVal = responseObj.value("CleavedLabel");
          if (!labelVal.isUndefined()) {
            uint64_t label = labelVal.toDouble();
            labelStr = QString::number(label);
            responseLabels.push_back(labelStr);
            mutationIds.push_back(mutationId);
          }
        }
        if (labelStr.isEmpty()) {
          QString title = "Warning";
          QString text = "DVID did not respond with a new body ID when writing cleaving results " + frac;
          displayWarning(title, text);
        }
      } else {
        QString title = "Writing of cleaving results " + frac + " failed";
        QString text = "Writing of cleaving results " + frac + " failed, code " + QString::number(writer.getStatusCode()) + ":\n" +
            writer.getStatusErrorMessage();
        displayWarning(title, text);
        return false;
      }
    }
    i++;
  }

  // It can be useful to know the new bodies created by cleaving, so display that information.

  QString responsesText = "Cleaving body " + QString::number(m_bodyId) + " created new ";
  responsesText += (responseLabels.size() > 1) ? "bodies " : "body ";
  for (std::size_t i = 0; i < responseLabels.size(); i++) {
    responsesText += responseLabels[i];
    if (i < responseLabels.size() - 1) {
      responsesText += ", ";
    }
  }
  ZWidgetMessage msg(responsesText);
  notify(msg);

  // And for now, at least, print to the shell the HTTPie command that would undo the cleaving.

  std::string undoText = "echo '[" + QString::number(m_bodyId).toStdString() + ", ";
  for (std::size_t i = 0; i < responseLabels.size(); i++) {
    undoText += responseLabels[i].toStdString();
    if (i < responseLabels.size() - 1) {
      undoText += ", ";
    }
  }
  undoText += "]' | http POST " + url.getNodeUrl() + "/" + instance + "/merge";
  std::string undoMsg = "\nUndo the previous cleave with the following shell command (using HTTPie):\n" + undoText + "\n\n";
  std::cout << undoMsg;

  return true;
}

void TaskBodyCleave::writeAuxiliaryOutput(const ZDvidReader &reader, ZDvidWriter &writer,
                                          const std::map<std::size_t, std::vector<uint64_t> > &cleaveIndexToMeshIds,
                                          const std::vector<QString> &newBodyIds,
                                          const std::vector<uint64_t> &mutationIds)
{
  std::string instance = getOutputInstanceName(m_bodyDoc->getDvidTarget());
  if (!reader.hasData(instance)) {
    writer.createKeyvalue(instance);
  }
  if (!reader.hasData(instance)) {
    LERROR() << "TaskBodyCleave could not create DVID instance \"" << instance << "\"";
    return;
  }

  // The output is JSON, an array of arrays, where each inner array is the super voxels in a cleaved body.

  QJsonArray json;

  if (!m_skip) {
    for (const auto &pair : cleaveIndexToMeshIds) {
      QJsonArray jsonForCleaveIndex;
      for (uint64_t id : pair.second) {
        jsonForCleaveIndex.append(QJsonValue(qint64(id)));
      }
      json.append(jsonForCleaveIndex);
    }
  }

  // It is useful to include a collection of arbitrary extra infomration at the last element of the array.
  // It can be distinguished as the only item in the output array that is a JSON object and not an array.

  QJsonObject jsonExtra;

  if (const char* user = std::getenv("USER")) {
    jsonExtra[KEY_USER] = user;
  }
  jsonExtra[KEY_SKIPPED] = QJsonValue(m_skip);

  if (!m_skip) {
    // For debugging, append verbatim the cleave server response that produced the arrays of super voxels.

    jsonExtra[KEY_SERVER_REPLY] = m_cleaveReply;

    // Include the IDs (labels) for the new bodies DVID created when cleaving (to allow undo later).

    QJsonArray jsonNewBodyIds;
    for (const QString &id : newBodyIds) {
      jsonNewBodyIds.append(id);
    }
    jsonExtra[KEY_BODY_IDS_CREATED] = jsonNewBodyIds;

    QJsonArray jsonTimes;
    std::copy(m_usageTimes.begin(), m_usageTimes.end(), std::back_inserter(jsonTimes));
    jsonExtra[KEY_USAGE_TIME] = jsonTimes;

    QJsonArray jsonStartupTimes;
    std::copy(m_startupTimes.begin(), m_startupTimes.end(), std::back_inserter(jsonStartupTimes));
    jsonExtra[KEY_STARTUP_TIME] = jsonStartupTimes;
  }

  json.append(jsonExtra);

  QJsonDocument jsonDoc(json);
  std::string jsonStr(jsonDoc.toJson(QJsonDocument::Compact).toStdString());

  // The key is the body ID plus the first valid mutation ID returned by DVID when
  // cleaving off of the body.  The mutation ID makes the entry unique if the user
  // goes back and does further cleaving on the same body.

  std::string key(std::to_string(m_bodyId));
  for (uint64_t mutationId : mutationIds) {
    if (mutationId != 0) {
      key += "." + std::to_string(mutationId);
      break;
    }
  }

  writer.writeJsonString(instance, key, jsonStr);
}

ZJsonArray TaskBodyCleave::readAuxiliaryOutput(const ZDvidReader &reader) const
{
  ZJsonArray result;

  std::string instance = getOutputInstanceName(m_bodyDoc->getDvidTarget());
  if (reader.hasData(instance)) {
    std::string key(std::to_string(m_bodyId));
    std::string url = ZDvidUrl(m_bodyDoc->getDvidTarget()).getKeyUrl(instance, key);
    result = reader.readJsonArray(ZDvidUrl::AppendSourceQuery(url));
  }

  return result;
}

namespace {

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

  QString toString(const QJsonValue &value)
  {
    if (value.isDouble()) {
      return QString::number(value.toDouble());
    } else if (value.isString()) {
      return value.toString();
    } else {
      QJsonDocument doc;
      if (value.isArray()) {
        doc = QJsonDocument(value.toArray());
      } else if (value.isObject()) {
        doc = QJsonDocument(value.toObject());
      }
      return QString(doc.toJson(QJsonDocument::Compact));
    }
    return QString("\'\'");
  }

}

bool TaskBodyCleave::loadSpecific(QJsonObject json)
{
  if (json.contains(KEY_BODY_ID)) {

    // The task may explicitly mention the ID of the body to be cleaved.

    m_bodyId = json[KEY_BODY_ID].toDouble();

    if (json.contains(KEY_BODY_POINT)) {
      QString title = "Cleaving Task Specification Conflict";
      QString text = "The cleaving task for body ID " + QString::number(m_bodyId) +
          " also contains a 3D point, which is being ignored.";
      displayWarning(title, text);
    }
  }
  else if (json.contains(KEY_BODY_POINT)) {

    // Or the task may mention a 3D point (e.g., from a "todo" mark), indicating that the
    // body containing that point is what should be cleaved.

    m_bodyId = 0;
    if (pointFromJSON(json[KEY_BODY_POINT], m_bodyPt)) {
      ZDvidReader reader;
      reader.setVerbose(false);
      if (reader.open(m_bodyDoc->getDvidTarget())) {
        int x = m_bodyPt.x();
        int y = m_bodyPt.y();
        int z = m_bodyPt.z();
        m_bodyId = reader.readBodyIdAt(x, y, z);
      }
      if (m_bodyId == 0) {
        QString title = "Cleaving Task Specification Error";
        QString text = KEY_BODY_POINT + " does not correspond to a valid body ID.";
        displayWarning(title, text);
        return false;
      }
    } else {
      QString title = "Cleaving Task Specification Error";
      QString text = "Unparsable " + KEY_BODY_POINT + ": " + toString(json[KEY_BODY_POINT]);
      displayWarning(title, text);
      return false;
    }
  } else {
    return false;
  }

  m_maxLevel = json[KEY_MAXLEVEL].toDouble();

  m_visibleBodies.insert(ZFlyEmBodyManager::EncodeTar(m_bodyId));

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

ProtocolTaskConfig TaskBodyCleave::getTaskConfig() const
{
  ProtocolTaskConfig config;
  config.setTaskType(taskType());
  config.setDefaultTodo(neutu::EToDoAction::TO_SUPERVOXEL_SPLIT);

  return config;
}

bool TaskBodyCleave::allowingSplit(uint64_t bodyId) const
{
  if (m_meshIdToCleaveIndex.find(bodyId) != m_meshIdToCleaveIndex.end()) {
    return false;
  }

  return true;
}

void TaskBodyCleave::setCleavingShortcutEnabled(bool on)
{
  m_toggleInBodyAction->setEnabled(on);
  m_addToBodyAction->setEnabled(on);
  m_toggleShowChosenCleaveBodyAction->setEnabled(on);
  for (const auto &actionToIndex : m_actionToComboBoxIndex) {
    actionToIndex.first->setEnabled(on);
  }
}

void TaskBodyCleave::disableCleavingShortcut()
{
  setCleavingShortcutEnabled(false);
}

void TaskBodyCleave::enableCleavingShortcut()
{
  setCleavingShortcutEnabled(true);
}

