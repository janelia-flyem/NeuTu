#include <iostream>
#include <stdlib.h>

#include <QDateTime>
#include <QFileDialog>
#include <QMessageBox>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QsLog.h>

#include "neutube.h"
#include "flyem/zflyemproofdoc.h"
#include "flyem/zflyembody3ddoc.h"
#include "protocols/taskbodyreview.h"
#include "protocols/tasksplitseeds.h"
#include "protocols/tasktesttask.h"

#include "taskprotocolwindow.h"
#include "ui_taskprotocolwindow.h"

TaskProtocolWindow::TaskProtocolWindow(ZFlyEmProofDoc *doc, ZFlyEmBody3dDoc *bodyDoc, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::TaskProtocolWindow)
{
    ui->setupUi(this);

    m_proofDoc = doc;
    m_body3dDoc = bodyDoc;

    m_protocolInstanceStatus = UNCHECKED;

    // control connections
    // connect prefetch signals to BodyPrefetchQueue here, if applicable


    // UI connections
    connect(ui->nextButton, SIGNAL(clicked(bool)), this, SLOT(onNextButton()));
    connect(ui->prevButton, SIGNAL(clicked(bool)), this, SLOT(onPrevButton()));
    connect(ui->doneButton, SIGNAL(clicked(bool)), this, SLOT(onDoneButton()));
    connect(ui->loadTasksButton, SIGNAL(clicked(bool)), this, SLOT(onLoadTasksButton()));
    connect(ui->completedCheckBox, SIGNAL(stateChanged(int)), this, SLOT(onCompletedStateChanged(int)));
    connect(ui->reviewCheckBox, SIGNAL(stateChanged(int)), this, SLOT(onReviewStateChanged(int)));
    connect(ui->showCompletedCheckBox, SIGNAL(stateChanged(int)), this, SLOT(onShowCompletedStateChanged(int)));

}

// constants
const QString TaskProtocolWindow::KEY_DESCRIPTION = "file type";
const QString TaskProtocolWindow::VALUE_DESCRIPTION = "Neu3 task list";
const QString TaskProtocolWindow::KEY_VERSION = "file version";
const int TaskProtocolWindow::currentVersion = 1;
const QString TaskProtocolWindow::KEY_ID = "ID";
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

    ZDvidReader reader;
    if (!reader.open(m_proofDoc->getDvidTarget())) {
        showError("Couldn't open DVID", "DVID couldn't be opened!  Check your network connections.");
        setWindowConfiguration(LOAD_BUTTON);
        return;
    }
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

void TaskProtocolWindow::onNextButton() {
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
        showInfo("Not all tasks complete!", "You have not completed all tasks!");
        return;
    }

    QMessageBox messageBox;
    messageBox.setText("Complete task protocol?");
    messageBox.setInformativeText("Do you want to complete the task protocol? If you do, the data in DVID will be renamed, and you will not be able to continue.\n\nComplete protocol?");
    messageBox.setStandardButtons(QMessageBox::Ok | QMessageBox::Cancel);
    messageBox.setDefaultButton(QMessageBox::Ok);
    int ret = messageBox.exec();
    if (ret != QMessageBox::Ok) {
        return;
    }

    // new key is old key + either identifier or datetime stamp
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

    // delete old key
    m_writer.deleteKey(PROTOCOL_INSTANCE.toStdString(), generateDataKey().toStdString());

    LINFO() << "Task protocol: saved completed protocol data to DVID:" << PROTOCOL_INSTANCE.toStdString()
            << "," << key.toStdString();

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

void TaskProtocolWindow::onCompletedStateChanged(int state) {
    if (m_currentTaskIndex >= 0) {
        m_taskList[m_currentTaskIndex]->setCompleted(ui->completedCheckBox->isChecked());
        saveState();
        updateLabel();
    }
}

void TaskProtocolWindow::onReviewStateChanged(int state) {
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

void TaskProtocolWindow::onShowCompletedStateChanged(int state) {
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
        m_currentTaskIndex = 0;
        updateCurrentTaskLabel();
        updateBodyWindow();
        updateLabel();
    }
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


    // save the unique identifier, if present:
    if (json.contains(KEY_ID)) {
        m_ID = json[KEY_ID].toString();
    }


    // load tasks from json into internal data structures; save to DVID if needed
    loadTasks(json);
    if (save) {
        saveState();
    }

    // load first task; enable UI and go
    if (ui->showCompletedCheckBox->isChecked()) {
        m_currentTaskIndex = 0;
    } else {
        m_currentTaskIndex = getFirstUncompleted();
    }
    if (m_currentTaskIndex < 0) {
        showInfo("No tasks to do!", "All tasks have been completed!");
    }

    updateCurrentTaskLabel();
    updateBodyWindow();
    updateLabel();
    setWindowConfiguration(TASK_UI);
}

/*
 * returns index of first uncompleted task, or -1
 */
int TaskProtocolWindow::getFirstUncompleted() {
    for (int i=0; i<m_taskList.size(); i++) {
        if (!m_taskList[i]->completed()) {
            return i;
        }
    }
    return -1;
}

/*
 * returns index of previous task before current task
 */
int TaskProtocolWindow::getPrev() {
    int index = m_currentTaskIndex - 1;
    if (index < 0) {
        index = m_taskList.size() - 1;
    }
    return index;
}

/*
 * returns index of next task after current task
 */
int TaskProtocolWindow::getNext() {
    int index = m_currentTaskIndex + 1;
    if (index >= m_taskList.size()) {
        index = 0;
    }
    return index;
}

/*
 * returns index of previous uncompleted task before
 * current task, or -1
 */
int TaskProtocolWindow::getPrevUncompleted() {
    int startIndex = m_currentTaskIndex;
    int index = startIndex - 1;
    while (index != startIndex) {
        if (index < 0) {
            index = m_taskList.size() - 1;
            continue;
        }
        if (!m_taskList[index]->completed()) {
            return index;
        }
        index--;
    }
    // we're back at current index
    if (m_taskList[index]->completed()) {
        return -1;
    } else {
        return index;
    }
}

/*
 * returns index of next uncompleted task after current
 * task, or -1
 */
int TaskProtocolWindow::getNextUncompleted() {
    int startIndex = m_currentTaskIndex;
    int index = startIndex + 1;
    while (index != startIndex) {
        if (index >= m_taskList.size()) {
            index = 0;
            continue;
        }
        if (!m_taskList[index]->completed()) {
            return index;
        }
        index++;
    }
    // we're back at current index
    if (m_taskList[index]->completed()) {
        return -1;
    } else {
        return index;
    }
}

/*
 * prefetch the bodies for a task
 */
void TaskProtocolWindow::prefetchForTaskIndex(int index) {
    // each task may have bodies that it wants visible and selected;
    //  add those, selected first (which are presumably more important?)
    if (m_taskList[index]->selectedBodies().size() > 0) {
        prefetch(m_taskList[index]->selectedBodies());
    }
    if (m_taskList[index]->visibleBodies().size() > 0) {
        prefetch(m_taskList[index]->visibleBodies());
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
        }
    }
}

/*
 * update the body view window for current index
 */
void TaskProtocolWindow::updateBodyWindow() {
    // update the body window so the required bodies are visible and/or selected
    if (m_currentTaskIndex >= 0) {

        // remove existing bodies; proof doc "selected" corresponds to "visible"
        //  I'm taking a bit of a guess that I want "MAPPED" (not "ORIGINAL")
        foreach (uint64_t bodyID, m_proofDoc->getSelectedBodySet(NeuTube::BODY_LABEL_MAPPED)) {
            emit bodyRemoved(bodyID);
        }

        QSet<uint64_t> visible = m_taskList[m_currentTaskIndex]->visibleBodies();
        QSet<uint64_t> selected = m_taskList[m_currentTaskIndex]->selectedBodies();

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
            LINFO() << "Task protocol: json loaded from DVID:" + instance + "," + key;
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

        // also, need to collect these keys and values better; hard-code this one
        //  for now
        QString taskType = taskJson.toObject()[KEY_TASKTYPE].toString();
        if (taskType == "body review") {
            QSharedPointer<TaskProtocolTask> task(new TaskBodyReview(taskJson.toObject()));
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
    return QString::fromStdString(NeuTube::GetCurrentUserName()) + "-" + TASK_PROTOCOL_KEY;
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
    }
}

/*
 * input: title and message for error dialog
 * effect: shows error dialog (convenience function)
 */
void TaskProtocolWindow::showError(QString title, QString message) {
    QMessageBox errorBox;
    errorBox.setText(title);
    errorBox.setInformativeText(message);
    errorBox.setStandardButtons(QMessageBox::Ok);
    errorBox.setIcon(QMessageBox::Warning);
    errorBox.exec();
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

TaskProtocolWindow::~TaskProtocolWindow()
{
    delete ui;
}
