#include <iostream>
#include <stdlib.h>

#include <QDateTime>
#include <QFileDialog>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QMenu>
#include <QMenuBar>
#include <QMessageBox>
#include <QsLog.h>
#include <QShortcut>


#include "neutube_def.h"
#include "neutubeconfig.h"
#include "flyem/zflyemproofdoc.h"
#include "flyem/zflyembody3ddoc.h"
#include "protocols/bodyprefetchqueue.h"
#include "protocols/taskbodyhistory.h"
#include "protocols/taskbodycleave.h"
#include "protocols/taskbodymerge.h"
#include "protocols/taskbodyreview.h"
#include "protocols/tasksplitseeds.h"
#include "protocols/tasktesttask.h"
#include "z3dwindow.h"
#include "zstackdocproxy.h"
#include "zwidgetmessage.h"

#include "taskprotocolwindow.h"
#include "ui_taskprotocolwindow.h"

TaskProtocolWindow::TaskProtocolWindow(ZFlyEmProofDoc *doc, ZFlyEmBody3dDoc *bodyDoc, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::TaskProtocolWindow)
{
    ui->setupUi(this);

    m_proofDoc = doc;
    m_body3dDoc = bodyDoc;

    m_currentTaskWidget = NULL;

    m_protocolInstanceStatus = UNCHECKED;

    // prefetch queue, setup
    // following https://mayaposch.wordpress.com/2011/11/01/how-to-really-truly-use-qthreads-the-full-explanation/
    m_prefetchQueue = new BodyPrefetchQueue();
    m_prefetchThread = new QThread();
    m_prefetchQueue->setDocument(m_body3dDoc);

    m_prefetchQueue->moveToThread(m_prefetchThread);
    connect(m_prefetchQueue, SIGNAL(finished()), m_prefetchThread, SLOT(quit()));
    connect(m_prefetchQueue, SIGNAL(finished()), m_prefetchQueue, SLOT(deleteLater()));
    connect(m_prefetchThread, SIGNAL(finished()), m_prefetchThread, SLOT(deleteLater()));

    // prefetch queue, item management
    connect(this, SIGNAL(prefetchBody(QSet<uint64_t>)), m_prefetchQueue, SLOT(add(QSet<uint64_t>)));
    connect(this, SIGNAL(prefetchBody(uint64_t)), m_prefetchQueue, SLOT(add(uint64_t)));
    connect(this, SIGNAL(unprefetchBody(QSet<uint64_t>)), m_prefetchQueue, SLOT(remove(QSet<uint64_t>)));
    connect(this, SIGNAL(clearBodyQueue()), m_prefetchQueue, SLOT(clear()));


    // UI connections
    connect(QApplication::instance(), SIGNAL(aboutToQuit()), this, SLOT(applicationQuitting()));
    connect(ui->nextButton, SIGNAL(clicked(bool)), this, SLOT(onNextButton()));
    connect(ui->prevButton, SIGNAL(clicked(bool)), this, SLOT(onPrevButton()));
    connect(ui->doneButton, SIGNAL(clicked(bool)), this, SLOT(onDoneButton()));
    connect(ui->loadTasksButton, SIGNAL(clicked(bool)), this, SLOT(onLoadTasksButton()));
    connect(ui->completedCheckBox, SIGNAL(stateChanged(int)), this, SLOT(onCompletedStateChanged(int)));
    connect(ui->reviewCheckBox, SIGNAL(stateChanged(int)), this, SLOT(onReviewStateChanged(int)));
    connect(ui->showCompletedCheckBox, SIGNAL(stateChanged(int)), this, SLOT(onShowCompletedStateChanged(int)));

    ui->nextButton->setShortcut(Qt::Key_E);
    ui->prevButton->setShortcut(Qt::Key_Q);

    // there are two keyboard shortcuts for completed+next, for different hand sizes
    QShortcut *shortcut1 = new QShortcut(Qt::SHIFT + Qt::Key_E, this);
    QShortcut *shortcut2 = new QShortcut(Qt::SHIFT + Qt::Key_X, this);
    connect(shortcut1, SIGNAL(activated()), this, SLOT(onCompletedAndNext()));
    connect(shortcut2, SIGNAL(activated()), this, SLOT(onCompletedAndNext()));

    connect(m_body3dDoc, &ZFlyEmBody3dDoc::bodyMeshesAdded,
            this, &TaskProtocolWindow::onBodyMeshesAdded);
    connect(m_body3dDoc, &ZFlyEmBody3dDoc::bodyMeshLoaded,
            this, &TaskProtocolWindow::onBodyMeshLoaded);
    connect(m_body3dDoc, &ZFlyEmBody3dDoc::bodyRecycled,
            this, &TaskProtocolWindow::onBodyRecycled);
}

// constants
const QString TaskProtocolWindow::KEY_DESCRIPTION = "file type";
const QString TaskProtocolWindow::VALUE_DESCRIPTION = "Neu3 task list";
const QString TaskProtocolWindow::KEY_VERSION = "file version";
const int TaskProtocolWindow::currentVersion = 1;
const QString TaskProtocolWindow::KEY_ID = "ID";
const QString TaskProtocolWindow::KEY_DVID_SERVER = "DVID server";
const QString TaskProtocolWindow::KEY_UUID = "UUID";
const QString TaskProtocolWindow::KEY_TASKLIST = "task list";
const QString TaskProtocolWindow::KEY_TASKTYPE = "task type";
const QString TaskProtocolWindow::PROTOCOL_INSTANCE = "Neu3-protocols";
const QString TaskProtocolWindow::TASK_PROTOCOL_KEY = "task-protocol";
const QString TaskProtocolWindow::TAG_NEEDS_REVIEW = "needs review";

/*
 * init() performs tasks that have to occur after UI connections are
 * made from this class to other things in Neu3; if we do them
 * in the constructor, they happen too soon
 */
void TaskProtocolWindow::init() {
    // start to do stuff
    if (!m_writer.open(m_proofDoc->getDvidTarget())) {
        showError("Couldn't open DVID", "DVID couldn't be opened!  Check your network connections.");
        setWindowConfiguration(LOAD_BUTTON);
        return;
    }

    const ZDvidReader &reader = m_writer.getDvidReader();
    /*
    if (!reader.open(m_proofDoc->getDvidTarget())) {
        showError("Couldn't open DVID", "DVID couldn't be opened!  Check your network connections.");
        setWindowConfiguration(LOAD_BUTTON);
        return;
    }
    */

    ZDvid::ENodeStatus status = reader.getNodeStatus();
    if (status == ZDvid::NODE_INVALID || status == ZDvid::NODE_OFFLINE) {
        showError("Couldn't open DVID", "DVID node is invalid or offline!  Check your DVID server or settings.");
        setWindowConfiguration(LOAD_BUTTON);
        return;
    }
    if (status == ZDvid::NODE_LOCKED) {
        m_nodeLocked = true;
        ui->completedCheckBox->setEnabled(false);
    } else {
        // NODE_NORMAL
        m_nodeLocked = false;
        ui->completedCheckBox->setEnabled(true);
    }

    // check DVID; if user has a started task list, load it immediately
    QJsonObject json = loadJsonFromDVID(PROTOCOL_INSTANCE, generateDataKey());
    if (!json.isEmpty()) {
        if (m_nodeLocked) {
            QMessageBox messageBox;
            messageBox.setText("DVID node locked");
            messageBox.setInformativeText("This DVID node is locked! Do you want to load the task protocol? If you do, you will not be able to complete any tasks.\n\nContinue loading protocol?");
            messageBox.setStandardButtons(QMessageBox::Ok | QMessageBox::Cancel);
            messageBox.setDefaultButton(QMessageBox::Ok);
            int ret = messageBox.exec();
            if (ret != QMessageBox::Ok) {
                setWindowConfiguration(LOAD_BUTTON);
                return;
            }
        }
        startProtocol(json, false);
    } else {
        // otherwise, show the load task file button
        setWindowConfiguration(LOAD_BUTTON);
    } 
}

void TaskProtocolWindow::onPrevButton() {
    // warn the task we're about to move away
    if (m_currentTaskIndex >= 0) {
        m_taskList[m_currentTaskIndex]->beforePrev();
    }

    // for now, just clear the body prefetch queue; see
    //  longer note in onNextButton()
    emit clearBodyQueue();

    if (ui->showCompletedCheckBox->isChecked()) {
        m_currentTaskIndex = getPrev();
    } else {
        m_currentTaskIndex = getPrevUncompleted();
        if (m_currentTaskIndex < 0) {
            showInfo("No tasks to do!", "All tasks have been completed!");
        }
    }

    // no prefetching is performed here; if we're backing up in the list,
    //  the next body should already be in memory; it's the responsibility of
    //  the rest of the application not to throw it out too soon (yes, this
    //  is a debatable position)

    updateCurrentTaskLabel();
    updateBodyWindow();
    updateLabel();
}

void TaskProtocolWindow::test()
{
  QJsonObject json =
      loadJsonFromFile((GET_TEST_DATA_DIR + "/_system/neu3_protocol.json").c_str());
  startProtocol(json, false);

  std::cout << "#Tasks: " << m_taskList.size() << std::endl;
  for (int i = -1; i <= m_taskList.size() + 1; ++i) {
    std::cout << "Prev index of " << i << ": " << getPrevIndex(i) << std::endl;
    std::cout << "Next index of " << i << ": " << getNextIndex(i) << std::endl;
  }
  int index = 0;
  for (int i = 0; i < 5; ++i) {
    std::cout << "Prev index of " << index << ": " << getPrevIndex(index) << std::endl;
    index = getPrevIndex(index);
  }

  index = 0;
  for (int i = 0; i < 5; ++i) {
    std::cout << "Next index of " << index << ": " << getNextIndex(index) << std::endl;
    index = getNextIndex(index);
  }

  m_currentTaskIndex = 0;
  for (int i = 0; i < 3; ++i) {
    std::cout << "Prev uncompleted (" << m_currentTaskIndex << "): ";
    m_currentTaskIndex = getPrevUncompleted();
    std::cout << m_currentTaskIndex << std::endl;
  }

  m_currentTaskIndex = 0;
  for (int i = 0; i < 3; ++i) {
    std::cout << "Next uncompleted (" << m_currentTaskIndex << "): ";
    m_currentTaskIndex = getNextUncompleted();
    std::cout << m_currentTaskIndex << std::endl;
  }

  m_taskList[0]->setCompleted(true);
  m_currentTaskIndex = 0;
  for (int i = 0; i < 3; ++i) {
    std::cout << "Prev uncompleted (" << m_currentTaskIndex << "): ";
    m_currentTaskIndex = getPrevUncompleted();
    std::cout << m_currentTaskIndex << std::endl;
  }

  m_currentTaskIndex = 0;
  for (int i = 0; i < 3; ++i) {
    std::cout << "Next uncompleted (" << m_currentTaskIndex << "): ";
    m_currentTaskIndex = getNextUncompleted();
    std::cout << m_currentTaskIndex << std::endl;
  }
}

void TaskProtocolWindow::onNextButton() {
    // warn the task we're about to move away
    if (m_currentTaskIndex >= 0) {
        m_taskList[m_currentTaskIndex]->beforeNext();
    }

    // currently the prefetching is unsophisticated--we only
    //  look one task ahead; so for now, we'll just clear the queue
    //  as we move; in the future, we could get fancier and both
    //  fetch farther ahead and be smarter about detecting which
    //  bodies are or aren't needed "soon"
    emit clearBodyQueue();

    if (ui->showCompletedCheckBox->isChecked()) {
        m_currentTaskIndex = getNext();
    } else {
        m_currentTaskIndex = getNextUncompleted();
        if (m_currentTaskIndex < 0) {
            showInfo("No tasks to do!", "All tasks have been completed!");
        }
    }

    // for now, simplest possible prefetching: just prefetch for the next task,
    //  as long as there is one and it's not the current one
    int nextTaskIndex = getNext();
    if (nextTaskIndex >= 0 && nextTaskIndex != m_currentTaskIndex) {
        prefetchForTaskIndex(nextTaskIndex);
    }

    updateCurrentTaskLabel();
    updateBodyWindow();
    updateLabel();
}

void TaskProtocolWindow::onDoneButton() {
    bool allComplete = true;
    foreach (QSharedPointer<TaskProtocolTask> task, m_taskList) {
        if (!task->completed()) {
            allComplete = false;
            break;
        }
    }
    if (!allComplete) {
        QMessageBox messageBox;
        messageBox.setText("Exit task protocol?");
        messageBox.setInformativeText("Not all tasks are complete!\n\nDo you want to exit the task protocol with incomplete tasks? If you do, your progress data will be saved, but you will not be able to continue.\n\nExit protocol?");
        messageBox.setStandardButtons(QMessageBox::Ok | QMessageBox::Cancel);
        messageBox.setDefaultButton(QMessageBox::Ok);
        int ret = messageBox.exec();
        if (ret != QMessageBox::Ok) {
            return;
        }
    } else {
        QMessageBox messageBox;
        messageBox.setText("Complete task protocol?");
        messageBox.setInformativeText("Do you want to complete the task protocol? If you do, your progress data will be stored in DVID, and you will not be able to continue.\n\nComplete protocol?");
        messageBox.setStandardButtons(QMessageBox::Ok | QMessageBox::Cancel);
        messageBox.setDefaultButton(QMessageBox::Ok);
        int ret = messageBox.exec();
        if (ret != QMessageBox::Ok) {
            return;
        }
    }

    if (m_currentTaskIndex >= 0) {
      m_taskList[m_currentTaskIndex]->beforeDone();
    }

    // re-save the data, complete or not; the new key is old key + either an identifier
    //  or datetime stamp
    QString key;
    if (m_ID.size() > 0) {
        key = generateDataKey() + "-" + m_ID;
    } else {
        key = generateDataKey() + "-" + QDateTime::currentDateTime().toString("yyyyMMddhhmm");
    }

    QJsonDocument doc(storeTasks());
    QString jsonString(doc.toJson(QJsonDocument::Compact));
    m_writer.writeJsonString(PROTOCOL_INSTANCE.toStdString(), key.toStdString(),
        jsonString.toStdString());

    LINFO() << "Task protocol: saved completed protocol data to DVID:" << PROTOCOL_INSTANCE.toStdString()
            << "," << key.toStdString();

    // delete old key in either case
    m_writer.deleteKey(PROTOCOL_INSTANCE.toStdString(), generateDataKey().toStdString());
    LINFO() << "Task protocol: deleted working protocol data from DVID";

    setWindowConfiguration(LOAD_BUTTON);
}

void TaskProtocolWindow::onLoadTasksButton() {
    // prompt for file path; might need to adjust this after testing on
    //  Linux; not sure what default file type filter is?
    QString result = QFileDialog::getOpenFileName(this, "Open task json file");
    if (result.size() == 0) {
        // canceled
        return;
    }

    if (m_nodeLocked) {
        showError("DVID node locked", "This DVID node is locked! Protocol not loaded.");
        return;
    }

    // load json from file (for now; eventually, allow user to browse from DVID,
    //  or maybe enter an assignment ID or something)
    QJsonObject json = loadJsonFromFile(result);
    startProtocol(json, true);
}

void TaskProtocolWindow::onBodiesUpdated() {
  updateBodyWindow();
}

namespace {
  struct BlockSignals
  {
    BlockSignals(QObject *o) : m_o(o) { m_blocked = o->blockSignals(true); }
    ~BlockSignals() { m_o->blockSignals(m_blocked); }
    QObject *m_o;
    bool m_blocked;
  };
}

void TaskProtocolWindow::onCompletedStateChanged(int state) {
    if (m_currentTaskIndex >= 0) {
      if (!state || m_taskList[m_currentTaskIndex]->allowCompletion()) {
          m_taskList[m_currentTaskIndex]->setCompleted(ui->completedCheckBox->isChecked());
          saveState();
          updateLabel();
      } else {
        BlockSignals blockCallingThisAgain(ui->completedCheckBox);
        ui->completedCheckBox->setCheckState(Qt::Unchecked);
      }
    }
}

void TaskProtocolWindow::onCompletedAndNext()
{
  ui->completedCheckBox->setCheckState(Qt::Checked);
  if (ui->nextButton->isEnabled()) {
    onNextButton();
  }
}

void TaskProtocolWindow::onReviewStateChanged(int /*state*/) {
    if (m_currentTaskIndex >= 0) {
        if (ui->reviewCheckBox->isChecked()) {
            m_taskList[m_currentTaskIndex]->addTag(TAG_NEEDS_REVIEW);
        } else {
            m_taskList[m_currentTaskIndex]->removeTag(TAG_NEEDS_REVIEW);
        }
        saveState();
        updateLabel();
    }
}

void TaskProtocolWindow::onShowCompletedStateChanged(int /*state*/) {
    // if we go from "show completed" to not, it's possible we need
    //  to advance away from the current task, if it's completed
    if (!ui->showCompletedCheckBox->isChecked() &&
        m_taskList[m_currentTaskIndex]->completed()) {
        m_currentTaskIndex = getNextUncompleted();
        if (m_currentTaskIndex < 0) {
            showInfo("No tasks to do!", "All tasks have been completed!");
        }
        updateCurrentTaskLabel();
        updateBodyWindow();
        updateLabel();
    }
    // likewise, if there is nothing showing (all complete) and
    //  we go to "show completed", advance and show something
    if (ui->showCompletedCheckBox->isChecked() &&
        m_currentTaskIndex < 0) {
        m_currentTaskIndex = getFirst(true);
        updateCurrentTaskLabel();
        updateBodyWindow();
        updateLabel();
    }
    updateButtonsEnabled();
}

/*
 * input: json from file or dvid; flag whether to save immediately back to dvid
 * output: none
 * effect: start up the UI for the protocol; finds first uncompleted task and loads
 *      into UI
 */
void TaskProtocolWindow::startProtocol(QJsonObject json, bool save) {
    // validate json; this call displays errors itself
    if (!isValidJson(json)) {
        return;
    }

    // at the point in time we have older versions hanging around, this is where you
    //  would convert them


    // save various metadata, if present:
    if (json.contains(KEY_ID)) {
        m_ID = json[KEY_ID].toString();
    }
    if (json.contains(KEY_DVID_SERVER)) {
        m_DVIDServer = json[KEY_DVID_SERVER].toString();
    }
    if (json.contains(KEY_UUID)) {
        m_UUID = json[KEY_UUID].toString();
    }

    // check that the server and UUID are correct if they are both present
    if (m_DVIDServer.size() > 0 && m_UUID.size() > 0) {
        if (!checkDVIDTarget()) {
            ZDvidTarget target = m_proofDoc->getDvidTarget();
            showError("Wrong DVID server or UUID",
                "This task list expects server " + m_DVIDServer + " and UUID " + m_UUID +
                ". You have opened " + QString::fromStdString(target.getAddressWithPort()) +
                " and " + QString::fromStdString(target.getUuid()) + ".");
            return;
        }
    }

    // load tasks from json into internal data structures; save to DVID if needed
    loadTasks(json);
    if (save && (m_currentTaskIndex >= 0)) {
      saveState();
    }

    // load first task; enable UI and go
    m_currentTaskIndex = getFirst(ui->showCompletedCheckBox->isChecked());
    if (m_currentTaskIndex < 0) {
        showInfo("No tasks to do!", "All tasks have been completed!");
    }

    // first prefetch
    int nextTaskIndex = getNext();
    if (nextTaskIndex >= 0 && nextTaskIndex != m_currentTaskIndex) {
        prefetchForTaskIndex(nextTaskIndex);
    }


    updateCurrentTaskLabel();
    updateBodyWindow();
    updateLabel();
    setWindowConfiguration(TASK_UI);

    m_prefetchThread->start();
}

/*
 * returns index of first (uncompleted) task, or -1
 */
int TaskProtocolWindow::getFirst(bool includeCompleted)
{
  for (int i = 0; i < m_taskList.size(); i++) {
      if ((includeCompleted || !m_taskList[i]->completed()) && (!m_taskList[i]->skip())) {
          return i;
      }
  }
  return -1;
}

/*
 * returns index of previous task before current task
 */
int TaskProtocolWindow::getPrev() {
  return getPrevIndex(m_currentTaskIndex);
  /*
    int index = m_currentTaskIndex - 1;
    if (index < 0) {
        index = m_taskList.size() - 1;
    }
    return index;
    */
}

/*
 * returns index of next task after current task
 */
int TaskProtocolWindow::getNext() {
  return getNextIndex(m_currentTaskIndex);
  /*
    int index = m_currentTaskIndex + 1;
    if (index >= m_taskList.size()) {
        index = 0;
    }
    return index;
    */
}

/*
 * returns index of previous uncompleted task before
 * current task, or -1
 */
int TaskProtocolWindow::getPrevUncompleted() {
    int startIndex = m_currentTaskIndex;
    int index = getPrevIndex(startIndex);
    while (index >= 0) {
      if (!m_taskList[index]->completed()) {
        break;
      } else {
        index = getPrevIndex(index);
        if (index == m_currentTaskIndex) {
          index = -1;
          break;
        }
      }
    }

    return index;
}

int TaskProtocolWindow::getPrevIndex(int currentIndex) const
{
  int index = currentIndex;

  do {
    index--;
    if (index < 0 && currentIndex >= 0) {
      index = m_taskList.size() - 1;
    } else if (index == currentIndex) {
      break;
    } else if (index < 0 || index >= m_taskList.size()) {
      index = -1;
      break;
    }
  } while (m_taskList[index]->skip());

  return index;
}

int TaskProtocolWindow::getNextIndex(int currentIndex) const
{
  int index = currentIndex;

  do {
    index++;
    if (index == m_taskList.size() && currentIndex >= 0) {
      index = 0;
    } else if (index == currentIndex) {
      break;
    } else if (index <= 0 || index >= m_taskList.size()) {
      index = -1;
      break;
    }
  } while (m_taskList[index]->skip());

  return index;
}
/*
 * returns index of next uncompleted task after current
 * task, or -1
 */
int TaskProtocolWindow::getNextUncompleted() {
  int startIndex = m_currentTaskIndex;
  int index = getNextIndex(startIndex);
  while (index >= 0) {
    if (!m_taskList[index]->completed()) {
      break;
    } else {
      index = getNextIndex(index);
      if (index == m_currentTaskIndex) {
        index = -1;
        break;
      }
    }
  }

  return index;
}

/*
 * prefetch the bodies for a task
 */
void TaskProtocolWindow::prefetchForTaskIndex(int index) {
    if (m_taskList[index]->usePrefetching()) {
        // each task may have bodies that it wants visible and selected;
        //  add those, selected first (which are presumably more important?)
        if (m_taskList[index]->selectedBodies().size() > 0) {
            prefetch(m_taskList[index]->selectedBodies());
        }
        if (m_taskList[index]->visibleBodies().size() > 0) {
            prefetch(m_taskList[index]->visibleBodies());
        }
    }
}

/*
 * request prefetch of bodies that you know are coming up next
 */
void TaskProtocolWindow::prefetch(QSet<uint64_t> bodyIDs) {
    emit prefetchBody(bodyIDs);
}

/*
 * request prefetch of a body that you know is coming up next
 */
void TaskProtocolWindow::prefetch(uint64_t bodyID) {
    emit prefetchBody(bodyID);
}

/*
 * updates the task label for current index
 */
void TaskProtocolWindow::updateCurrentTaskLabel() {
    // if there is a current task widget, remove it from the layout:
    if (m_currentTaskWidget != NULL) {
        ui->verticalLayout_3->removeWidget(m_currentTaskWidget);
        // ui->horizontalLayout->removeWidget(m_currentTaskWidget);
        m_currentTaskWidget->setVisible(false);
    }

    if (m_currentTaskIndex < 0) {
        ui->taskActionLabel->setText("(no task)");
        ui->taskTargetLabel->setText("n/a");
        ui->completedCheckBox->setChecked(false);
        ui->reviewCheckBox->setChecked(false);
    } else {
        ui->taskActionLabel->setText(m_taskList[m_currentTaskIndex]->actionString());
        ui->taskTargetLabel->setText(m_taskList[m_currentTaskIndex]->targetString());

        // make the "completed" checkbox match the current task, but prevent the signal from
        // being triggered, so that task does not do somehing like save its results an extra time
        BlockSignals blockOnCompleted(ui->completedCheckBox);
        ui->completedCheckBox->setChecked(m_taskList[m_currentTaskIndex]->completed());

        if (m_taskList[m_currentTaskIndex]->hasTag(TAG_NEEDS_REVIEW)) {
            ui->reviewCheckBox->setChecked(true);
        } else {
            ui->reviewCheckBox->setChecked(false);
        }
        // show task-specific UI if it exist
        m_currentTaskWidget = m_taskList[m_currentTaskIndex]->getTaskWidget();
        if (m_currentTaskWidget != NULL) {
            ui->verticalLayout_3->addWidget(m_currentTaskWidget);
            // ui->horizontalLayout->addWidget(m_currentTaskWidget);
            m_currentTaskWidget->setVisible(true);
            updateButtonsEnabled();
        }

        updateMenu(true);
    }
}

/*
 * ensures that the "Next" and "Prev" buttons are enabled only when there are
 * next or previous tasks to go to
 */
void TaskProtocolWindow::updateButtonsEnabled() {
    bool nextPrevEnabled = ((m_taskList.size() > 1) &&
                            ((getNextUncompleted() != -1) || ui->showCompletedCheckBox->isChecked()));
    ui->nextButton->setEnabled(nextPrevEnabled);
    ui->prevButton->setEnabled(nextPrevEnabled);
}

/*
 * removes the task menu from the main menu bar, and if "add" is true
 * also adds a new menu for the current task
 */
void TaskProtocolWindow::updateMenu(bool add) {
    if (Z3DWindow *window = m_body3dDoc->getParent3DWindow()) {
        if (m_currentTaskMenuAction) {
            window->menuBar()->removeAction(m_currentTaskMenuAction);
        }
        if (add && (m_currentTaskIndex >= 0)) {
            QMenu *menu = m_taskList[m_currentTaskIndex]->getTaskMenu();
            if (menu != NULL) {
              m_currentTaskMenuAction = window->menuBar()->addMenu(menu);
            }
        }
    }
}


/*
 * update the body view window for current index
 */
void TaskProtocolWindow::updateBodyWindow() {
    // update the body window so the required bodies are visible and/or selected
    if (m_currentTaskIndex >= 0) {
        disableButtonsWhileUpdating();

        emit allBodiesRemoved();

        const QSet<uint64_t> &visible = m_taskList[m_currentTaskIndex]->visibleBodies();
        const QSet<uint64_t> &selected = m_taskList[m_currentTaskIndex]->selectedBodies();

#ifdef _DEBUG_
        std::cout << "Visible bodies:";
        foreach (uint64_t body, visible) {
          std::cout << body << ", ";
        }
        std::cout << std::endl;

        std::cout << "Selected bodies:";
        foreach (uint64_t body, selected) {
          std::cout << body << ", ";
        }
        std::cout << std::endl;
#endif

#ifdef _DEBUG_2
        m_body3dDoc->setMinDsLevel(5);
        ZFlyEmBodyConfig config(1);
        config.setDsLevel(5);
        config.setLocalDsLevel(0);
        config.setRange(ZIntCuboid(ZIntPoint(2775, 6048, 2742),
                                   ZIntPoint(3287, 6560, 3254)));
        m_body3dDoc->addBodyConfig(config);
#endif

        // if something is selected, it should be visible, too
        foreach (uint64_t bodyID, visible) {
            emit bodyAdded(bodyID);
        }
        // I don't assume all selected bodies are already added
        foreach (uint64_t bodyID, selected) {
            if (!visible.contains(bodyID)) {
                emit bodyAdded(bodyID);
            }
        }
        emit bodySelectionChanged(selected);
    }
}

void TaskProtocolWindow::disableButtonsWhileUpdating()
{
    // If the user triggers several calls to updateBodyWindow() in rapid succession, by
    // quickly moving between tasks and using controls from the task widget that also change
    // the loaded bodies, then the bodies loaded by one call may not be fully cleared by the
    // next call, due to the way bodies are added and removed asynchronously in a background
    // thread. The buttons for moving between tasks, and the whole task widget, will be disabled
    // until signals connected to the onBodyRecycled, onBodyMeshesAdded and onBodyMeshLoaded
    // slots indicate that all the old meshes have been fully deleted and all the new meshes
    // have been loaded.

    QList<ZMesh*> meshes = ZStackDocProxy::GetGeneralMeshList(m_body3dDoc);
    m_bodyRecycledExpected = meshes.size();
    m_bodyRecycledReceived = 0;

    m_bodyMeshesAddedExpected = 0;
    m_bodyMeshesAddedReceived = 0;

    bool usingTars = false;
    const QSet<uint64_t> &visible = m_taskList[m_currentTaskIndex]->visibleBodies();
    foreach (uint64_t bodyID, visible) {
        if (ZFlyEmBodyManager::encodesTar(bodyID)) {
            m_bodyMeshesAddedExpected++;
            usingTars = true;
        }
    }

    const QSet<uint64_t> &selected = m_taskList[m_currentTaskIndex]->selectedBodies();
    foreach (uint64_t bodyID, selected) {
        if (ZFlyEmBodyManager::encodesTar(bodyID)) {
            m_bodyMeshesAddedExpected++;
            usingTars = true;
        }
    }

    m_bodiesReused = 0;

    if (usingTars) {
        m_bodyMeshLoadedExpected = 0;
    } else {
        m_bodyMeshLoadedExpected = (visible + selected).size();

        // Bodies reused from the previous task may not generate onBodyRecycled amd
        // onBodyMeshLoaded signals, which needs to be considered when counting
        // these signals.

        for (auto it = meshes.cbegin(); it != meshes.cend(); it++) {
            ZMesh *mesh = *it;
            uint64_t id = mesh->getLabel();
            m_bodiesReused += (visible.contains(id) || selected.contains(id));
        }
    }

    m_bodyMeshLoadedReceived = 0;

    ui->nextButton->setEnabled(false);
    ui->prevButton->setEnabled(false);
    if (m_currentTaskWidget) {
        m_currentTaskWidget->setEnabled(false);
    }
    if (m_currentTaskMenuAction) {
        m_currentTaskMenuAction->setEnabled(false);
    }

    // Check for immediate reenabling, in the case of all bodies being reused.

    enableButtonsAfterUpdating();
}

void TaskProtocolWindow::enableButtonsAfterUpdating()
{
  LDEBUG() << "m_bodyRecycledExpected =" << m_bodyRecycledExpected << ";"
           << "m_bodyRecycledReceived =" << m_bodyRecycledReceived << ";"
           << "m_bodyMeshLoadedExpected =" << m_bodyMeshLoadedExpected << ";"
           << "m_bodyMeshLoadedReceived =" << m_bodyMeshLoadedReceived << ";"
           << "m_bodiesReused =" << m_bodiesReused;
    if ((m_bodyRecycledExpected - m_bodiesReused <= m_bodyRecycledReceived) &&
        (m_bodyMeshesAddedExpected == m_bodyMeshesAddedReceived) &&
        (m_bodyMeshLoadedExpected - m_bodiesReused <= m_bodyMeshLoadedReceived)) {

        bool justEnabled = (m_currentTaskWidget && !m_currentTaskWidget->isEnabled());

        updateButtonsEnabled();
        if (m_currentTaskWidget) {
            m_currentTaskWidget->setEnabled(true);
        }
        if (m_currentTaskMenuAction) {
            m_currentTaskMenuAction->setEnabled(true);
        }

        if ((m_currentTaskIndex >= 0) && justEnabled) {

            // Call this function on the task only once, when the task's UI was
            // just enabled.

            m_taskList[m_currentTaskIndex]->onLoaded();
        }
    }
}

/*
 * updates any progress labels
 */
void TaskProtocolWindow::updateLabel() {
    int ncomplete = 0;
    foreach (QSharedPointer<TaskProtocolTask> task, m_taskList) {
        if (task->completed()) {
            ncomplete++;
        }
    }
    int ntasks = m_taskList.size();
    float percent = (100.0 * ncomplete) / ntasks;
    ui->progressLabel->setText(QString("%1 / %2 (%3%)").arg(ncomplete).arg(ntasks).arg(percent));

    // whenever we update the label, also log the progress; this is not useful as
    //  an activity tracker, as the label gets updated not always in response to user action
    LINFO() << "Task protocol: progress updated:" << ncomplete << "/" << ntasks;
}

/*
 * save the internal data to dvid in predetermined instance and key
 * of current dvid target
 */
void TaskProtocolWindow::saveState() {
    QJsonObject tasks = storeTasks();
    saveJsonToDvid(tasks);
}

/*
 * input: filepath
 * output: json object of json in file; empty json if error
 * effect: shows error dialogs on errors
 */
QJsonObject TaskProtocolWindow::loadJsonFromFile(QString filepath) {
    QJsonObject emptyResult;

    QFile file(filepath);
    if (!file.open(QIODevice::ReadOnly)) {
        showError("Error loading file", "Couldn't open file " + filepath + "!");
        return emptyResult;
        }

    QByteArray data = file.readAll();
    QJsonDocument doc = QJsonDocument::fromJson(data);
    if (doc.isNull() or !doc.isObject()) {
        showError("Error parsing file", "Couldn't parse file " + filepath + "!");
        return emptyResult;
    } else {
        LINFO() << "Task protocol: json loaded from file" + filepath;
        TaskProtocolTask::setJsonSource(filepath);
        return doc.object();
    }
}

/*
 * input: dvid instance and key names
 * output: json object of json in from dvid; empty json if error
 * effect: shows error dialogs on errors
 */
QJsonObject TaskProtocolWindow::loadJsonFromDVID(QString instance, QString key) {
    QJsonObject emptyResult;
    ZDvidReader reader;
    if (!reader.open(m_proofDoc->getDvidTarget())) {
        return emptyResult;
    }
    if (!reader.hasKey(instance, key)) {
        return emptyResult;
    }

    // we got something!  reel it in...
    QByteArray data = reader.readKeyValue(instance, key);
    QJsonDocument doc = QJsonDocument::fromJson(data);
        if (doc.isNull() or !doc.isObject()) {
            showError("Error parsing JSON", "Couldn't parse JSON from " + instance +
                ", " + key + "!");
            return emptyResult;
        } else {
          QString msg =
              "Task protocol: json loaded from DVID:" + instance + "," + key;
          LINFO() << msg;
          emit messageGenerated(ZWidgetMessage(msg));
#if 0
          //For testing
          emit messageGenerated(
              ZWidgetMessage(
                "test", msg, neutube::MSG_WARNING, ZWidgetMessage::TARGET_DIALOG));
#endif
          return doc.object();
        }
}

/*
 * check input json for validity; not comprehensive, but good enough for our purposes
 */
bool TaskProtocolWindow::isValidJson(QJsonObject json) {

    // check file description and version
    if (!json.contains(KEY_DESCRIPTION) || json[KEY_DESCRIPTION].toString() != VALUE_DESCRIPTION) {
        showError("Json parsing error", "This file does not appear to be a Neu3 task list file!");
        return false;
    }

    if (!json.contains(KEY_VERSION)) {
        showError("Json parsing error", "No version info in file!");
        return false;
    }

    int fileVersion = json[VALUE_DESCRIPTION].toInt();
    if (fileVersion > currentVersion) {
        showError("Json parsing issue", "This file is from a newer version of this software!  Update and try again.");
        return false;
    }

    if (!json.contains(KEY_TASKLIST)) {
        showError("Json parsing issue", "Can't find list of tasks in json file!");
        return false;
    }
    // could validate that it's a list and each element is a map, but I'll
    //  draw the line here for now

    return true;
}

/*
 * input: json object
 * effect: load data from json into internal data structures
 */
void TaskProtocolWindow::loadTasks(QJsonObject json) {

    m_taskList.clear();
    foreach(QJsonValue taskJson, json[KEY_TASKLIST].toArray()) {
        if (!taskJson.isObject()) {
            LWARN() << "Task protocol: found task json that is not an object; skipping";
            continue;
        }

        // this if-else tree will get more awkward with more types...
        // TODO: Switch to a factory pattern that produces an instance of the appropriate
        // TaskProtocolTask subclass given the KEY_TASKTYPE value.

        // also, need to collect these keys and values better; hard-code this one
        //  for now
        QString taskType = taskJson.toObject()[KEY_TASKTYPE].toString();
        if (taskType == "body review") {
            QSharedPointer<TaskProtocolTask> task(new TaskBodyReview(taskJson.toObject()));
            m_taskList.append(task);
        } else if (taskType == "body history") {
            QSharedPointer<TaskProtocolTask> task(new TaskBodyHistory(taskJson.toObject(), m_body3dDoc));
            m_taskList.append(task);
        } else if (taskType == "body cleave") {
            QSharedPointer<TaskProtocolTask> task(new TaskBodyCleave(taskJson.toObject(), m_body3dDoc));
            m_taskList.append(task);
        } else if (taskType == "body merge") {
            QSharedPointer<TaskProtocolTask> task(new TaskBodyMerge(taskJson.toObject(), m_body3dDoc));
            m_taskList.append(task);
        } else if (taskType == "split seeds") {
            // I'm not really fond of this task having a different constructor signature, but
            //  neither do I want to pass in both docs to every task just because a few might
            //  need one or the other of them
            QSharedPointer<TaskProtocolTask> task(new TaskSplitSeeds(taskJson.toObject(), m_body3dDoc));
            m_taskList.append(task);
        } else if (taskType == "test task") {
            QSharedPointer<TaskProtocolTask> task(new TaskTestTask(taskJson.toObject()));
            m_taskList.append(task);
        } else {
            // unknown task type; log it and move on
            LWARN() << "Task protocol: found unknown task type " << taskType << " in task json; skipping";
        }

        if (!m_taskList.empty()) {
            connect(m_taskList.back().data(), SIGNAL(bodiesUpdated()),
                    this, SLOT(onBodiesUpdated()));
            connect(m_taskList.back().data(), SIGNAL(browseGrayscale(double,double,double,const QHash<uint64_t, QColor>&)),
                    this, SIGNAL(browseGrayscale(double,double,double,const QHash<uint64_t, QColor>&)));
            connect(m_taskList.back().data(), SIGNAL(updateGrayscaleColor(const QHash<uint64_t, QColor>&)),
                    this, SIGNAL(updateGrayscaleColor(QHash<uint64_t,QColor>)));
        }
    }

    LINFO() << "Task protocol: loaded" << m_taskList.size() << "tasks";
}

/*
 * output: json object containing data from internal data structures
 */
QJsonObject TaskProtocolWindow::storeTasks() {

    QJsonObject json;
    json[KEY_DESCRIPTION] = VALUE_DESCRIPTION;
    json[KEY_VERSION] = currentVersion;
    if (m_ID.size() > 0) {
        json[KEY_ID] = m_ID;
    }
    if (m_DVIDServer.size() > 0) {
        json[KEY_DVID_SERVER] = m_DVIDServer;
    }
    if (m_UUID.size() > 0) {
        json[KEY_UUID] = m_UUID;
    }

    QJsonArray tasks;
    foreach (QSharedPointer<TaskProtocolTask> task, m_taskList) {
        tasks.append(task->toJson());
    }
    json[KEY_TASKLIST] = tasks;

    return json;
}

/*
 * input: json object
 * effect: save json to dvid in predetermined instance and key
 */
void TaskProtocolWindow::saveJsonToDvid(QJsonObject json) {
    // check that instance exists; if not, create it
    if (!checkCreateDataInstance()) {
        showError("DVID error", "Could not create the protocol instance in DVID!  Data is not saved!");
        return;
    }

    QJsonDocument doc(json);
    QString jsonString(doc.toJson(QJsonDocument::Compact));
    m_writer.writeJsonString(PROTOCOL_INSTANCE.toStdString(), generateDataKey().toStdString(),
        jsonString.toStdString());

    LINFO() << "Task protocol: saved data to DVID:" << PROTOCOL_INSTANCE.toStdString()
            << "," << generateDataKey().toStdString();
}

/*
 * output: key under which protocol data should be stored in dvid
 */
QString TaskProtocolWindow::generateDataKey() {
    return QString::fromStdString(neutube::GetCurrentUserName()) + "-" + TASK_PROTOCOL_KEY;
}

/*
 * output: success or not
 * effect: check to see if predetermined instance exists in
 * dvid in current target; if not, attempt to create it (once)
 */
bool TaskProtocolWindow::checkCreateDataInstance() {
    if (m_protocolInstanceStatus == CHECKED_PRESENT) {
        return true;
    } else if (m_protocolInstanceStatus == CHECKED_ABSENT) {
        // only check once; always an error if we can't create
        //  the first time
        return false;
    }

    // m_protocolInstanceStatus = UNCHECKED:
    ZDvidReader reader;
    reader.setVerbose(false);
    if (reader.open(m_proofDoc->getDvidTarget())) {
        if (!reader.hasData(PROTOCOL_INSTANCE.toStdString())) {
            m_writer.createKeyvalue(PROTOCOL_INSTANCE.toStdString());
            // did it actually create?  I'm only going to try once
            if (reader.hasData(PROTOCOL_INSTANCE.toStdString())) {
                m_protocolInstanceStatus = CHECKED_PRESENT;
                return true;
            } else {
                m_protocolInstanceStatus = CHECKED_ABSENT;
                return false;
            }
        } else {
            m_protocolInstanceStatus = CHECKED_PRESENT;
            return true;
        }
    } else {
        m_protocolInstanceStatus = CHECKED_ABSENT;
        return false;
    }
}

/*
 * check that the current DVID target matches the one
 * input in the task json
 */
bool TaskProtocolWindow::checkDVIDTarget() {
    ZDvidTarget target = m_proofDoc->getDvidTarget();

    // UUID: we don't always specify the full UUID; just compare
    //  the digits we have (ie, the shorter of the two)
    QString targetUUID = QString::fromStdString(target.getUuid());
    if (targetUUID.size() > m_UUID.size()) {
        if (!targetUUID.startsWith(m_UUID)) {
            return false;
        }
    } else {
        if (!m_UUID.startsWith(targetUUID)) {
            return false;
        }
    }

    // server: the server name should always include port, and it should
    //  always be fully qualified
    if (m_DVIDServer != QString::fromStdString(target.getAddressWithPort())) {
        return false;
    }

    return true;
}

/*
 * hide and show UI panels to correspond to "ready for loading data"
 * and "data loaded, time to do work"
 */
void TaskProtocolWindow::setWindowConfiguration(WindowConfigurations config) {
    // note that size constraint = fixed size, set in designer,
    //  ensures the widget resizes when child widgets are hidden/shown

    if (config == TASK_UI) {
        ui->loadTasksWidget->hide();
        ui->taskButtonsWidget->show();
        ui->taskDetailsWidget->show();
        ui->tasksProgressWidget->show();
    } else if (config == LOAD_BUTTON) {
        ui->loadTasksWidget->show();
        ui->taskButtonsWidget->hide();
        ui->taskDetailsWidget->hide();
        ui->tasksProgressWidget->hide();
        updateMenu(false);
    }
}

/*
 * input: title and message for error dialog
 * effect: shows error dialog (convenience function)
 */
void TaskProtocolWindow::showError(QString title, QString message) {
//    QMessageBox errorBox;
//    errorBox.setText(title);
//    errorBox.setInformativeText(message);
//    errorBox.setStandardButtons(QMessageBox::Ok);
//    errorBox.setIcon(QMessageBox::Warning);
//    errorBox.exec();

    emit messageGenerated(
        ZWidgetMessage(
          title, message, neutube::MSG_WARNING, ZWidgetMessage::TARGET_DIALOG));
}

/*
 * input: title and message for info dialog
 * effect: shows info dialog (convenience function)
 */
void TaskProtocolWindow::showInfo(QString title, QString message) {
    QMessageBox infoBox;
    infoBox.setText(title);
    infoBox.setInformativeText(message);
    infoBox.setStandardButtons(QMessageBox::Ok);
    infoBox.setIcon(QMessageBox::Information);
    infoBox.exec();
}

void TaskProtocolWindow::onBodyMeshesAdded(int numMeshes)
{
  LDEBUG() << "onBodyMeshesAdded:" << numMeshes;

    m_bodyMeshesAddedReceived++;
    m_bodyMeshLoadedExpected += numMeshes;
    enableButtonsAfterUpdating();
}

void TaskProtocolWindow::onBodyMeshLoaded(int numMeshes)
{
  m_bodyMeshLoadedReceived += numMeshes;
//    m_bodyMeshLoadedReceived++;
    enableButtonsAfterUpdating();
}

void TaskProtocolWindow::onBodyRecycled()
{
    m_bodyRecycledReceived++;
    enableButtonsAfterUpdating();
}

void TaskProtocolWindow::applicationQuitting() {
    m_prefetchQueue->finish();
}

BodyPrefetchQueue *TaskProtocolWindow::getPrefetchQueue() const
{
    return m_prefetchQueue;
}

TaskProtocolWindow::~TaskProtocolWindow()
{
    delete ui;
}
