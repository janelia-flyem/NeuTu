#include "orphanlinkprotocol.h"
#include "ui_orphanlinkprotocol.h"

#include <iostream>
#include <stdlib.h>

#include <QMessageBox>
#include <QAbstractItemModel>

#include "neutubeconfig.h"

#include "protocolchooseassignmentdialog.h"
#include "protocolassignment.h"

#include "zjsonparser.h"

OrphanLinkProtocol::OrphanLinkProtocol(QWidget *parent) :
    ProtocolDialog(parent),
    ui(new Ui::OrphanLinkProtocol)
{
    ui->setupUi(this);

    // must not end with / character!  enforce that when we un-hard-code it
    m_client.setServer(QString::fromStdString(GET_FLYEM_CONFIG.getDefaultAssignmentManager()));

    m_model = new QStandardItemModel(0, 3, ui->tableView);
    // setHeaders(m_model);

    m_proxy = new QSortFilterProxyModel(this);
    m_proxy->setSourceModel(m_model);
    ui->tableView->setModel(m_proxy);
    m_proxy->setFilterKeyColumn(TASK_ID_COLUMN);




    // UI connections
    connect(ui->exitButton, SIGNAL(clicked(bool)), this, SLOT(onExitButton()));
    connect(ui->completeButton, SIGNAL(clicked(bool)), this, SLOT(onCompleteButton()));

    connect(ui->commentButton, SIGNAL(clicked(bool)), this, SLOT(onCommentButton()));
    connect(ui->gotoSelectedButton, SIGNAL(clicked(bool)), this, SLOT(onGotoSelectedButton()));

    connect(ui->tableView, SIGNAL(clicked(QModelIndex)), this, SLOT(onClickedTable(QModelIndex)));

    connect(ui->nextTaskButton, SIGNAL(clicked(bool)), this, SLOT(onNextTaskButton()));
    connect(ui->startTaskButton, SIGNAL(clicked(bool)), this, SLOT(onStartTaskButton()));
    connect(ui->completeTaskButton, SIGNAL(clicked(bool)), this, SLOT(onCompleteTaskButton()));
    connect(ui->skipTaskButton, SIGNAL(clicked(bool)), this, SLOT(onSkipTaskButton()));

}

// constants
const std::string OrphanLinkProtocol::KEY_VERSION = "version";
const std::string OrphanLinkProtocol::KEY_ASSIGNMENT_ID = "assignment ID";
const std::string OrphanLinkProtocol::KEY_COMMENTS = "comments";
const QString OrphanLinkProtocol::TASK_KEY_BODY_ID = "key_text";
const int OrphanLinkProtocol::fileVersion = 1;

bool OrphanLinkProtocol::initialize() {

    // the protocol is not in the business of generating or starting assignments; that's the
    //  responsibility of the ProtocolAssignmentDialog (green inbox icon); we grab the started
    //  assignments and have the user pick an appropriate one for this protocol
    QList<ProtocolAssignment> assignments;
    for (ProtocolAssignment a: m_client.getStartedAssignments()) {
        if (a.protocol == "orphan_link") {
            assignments << a;
        }
    }
    if (assignments.size() == 0) {
        showError("No assignments!", "There are no started assignments for this protocol!");
        return false;
    }

    ProtocolChooseAssignmentDialog dialog(assignments);
    int ans = dialog.exec();
    if (ans == QDialog::Rejected) {
        return false;
    }
    ProtocolAssignment assignment = dialog.getAssignment();
    if (assignment.id == -1) {
        showError("No chosen assignment!", "You didn't choose an assignment!");
        return false;
    }

    m_assignmentID = assignment.id;

    // save state and start work
    saveState();

    loadTasks();
    updateTable();
    updateLabels();

    startProtocol();

    return true;
}

void OrphanLinkProtocol::startProtocol() {
    if (hasPendingTasks()) {
        ui->assignmentIDLabel->setText("Assignment ID: " + QString::number(m_assignmentID));

        m_allTasksCompleted = false;

        // no selection yet, so disable start/complete
        enable(NEXT_TASK_BUTTON);
        disable(START_TASK_BUTTON);
        disable(COMPLETE_SKIP_TASK_BUTTONS);

        // do nothing at start; we could immediately go to next,
        //  but let's have the user do that explicity for now
    } else {
        allTasksCompleted();
    }
}

void OrphanLinkProtocol::onNextTaskButton() {
    // this should not happen; next button should be disabled if no tasks
    if (!hasPendingTasks()) {
        allTasksCompleted();
        return;
    }

    ProtocolAssignmentTask next = findNextTask();
    if (next.id < 0) {
        // no valid task; we're done (also should not happen)
        allTasksCompleted();
        return;
    }

    // select task in table programmatically
    int nextRow = findTaskIndex(next);
    if (nextRow < 0) {
        // not found, should be impossible
        return;
    }

    QModelIndex nextModelIndex = m_model->index(nextRow, 0);
    QModelIndex viewIndex = m_proxy->mapFromSource(nextModelIndex);
    if (viewIndex.isValid()) {
        ui->tableView->selectionModel()->setCurrentIndex(viewIndex, QItemSelectionModel::ClearAndSelect|QItemSelectionModel::Rows);
        taskSelected();
    }
}

void OrphanLinkProtocol::onStartTaskButton() {
    ProtocolAssignmentTask task = getSelectedTask();
    if (task.id < 0) {
        return;
    }

    // should be impossible:
    if (task.disposition == ProtocolAssignmentTask::DISPOSITION_IN_PROGRESS ||
            task.disposition == ProtocolAssignmentTask::DISPOSITION_SKIPPED ||
            task.disposition == ProtocolAssignmentTask::DISPOSITION_COMPLETE) {
        return;
    }

    bool status = m_client.startTask(task);
    if (!status) {
        showError("Error starting task!", "Task could not be started!");
        return;
    }


    // update task disposition in memory
    task.disposition = ProtocolAssignmentTask::DISPOSITION_IN_PROGRESS;
    updateTask(task);

    disable(START_TASK_BUTTON);
    enable(COMPLETE_SKIP_TASK_BUTTONS);

    updateTable();
    updateLabels();
}

void OrphanLinkProtocol::onCompleteTaskButton() {
    ProtocolAssignmentTask task = getSelectedTask();
    if (task.id < 0) {
        return;
    }

    // should be impossible:
    if (task.disposition != ProtocolAssignmentTask::DISPOSITION_IN_PROGRESS) {
        return;
    }

    bool status = m_client.completeTask(task, false, m_comments[task.id]);
    if (!status) {
        showError("Error completing task!", "Task could not be completed!");
        return;
    }

    // update task disposition in memory
    task.disposition = ProtocolAssignmentTask::DISPOSITION_COMPLETE;
    updateTask(task);

    disable(COMPLETE_SKIP_TASK_BUTTONS);
    if (!hasPendingTasks()) {
        allTasksCompleted();
    }

    updateTable();
    updateLabels();
}

void OrphanLinkProtocol::onSkipTaskButton() {
    ProtocolAssignmentTask task = getSelectedTask();
    if (task.id < 0) {
        return;
    }

    // should be impossible:
    if (task.disposition != ProtocolAssignmentTask::DISPOSITION_IN_PROGRESS) {
        return;
    }

    // you must provide a comment on skips
    if (m_comments[task.id].size() == 0) {
        showMessage("Add comment!", "You must add a comment if you skip a task!");
        return;
    }


    bool status = m_client.completeTask(task, true, m_comments[task.id]);
    if (!status) {
        showError("Error skipping task!", "Task could not be skipped!");
        return;
    }

    // update task disposition in memory
    task.disposition = ProtocolAssignmentTask::DISPOSITION_SKIPPED;
    updateTask(task);


    if (hasPendingTasks()) {
        disable(START_TASK_BUTTON);
        enable(COMPLETE_SKIP_TASK_BUTTONS);
    } else {
        allTasksCompleted();
    }

    updateTable();
    updateLabels();
}

void OrphanLinkProtocol::onGotoSelectedButton() {
    // if the user is explicitly pressing the go to button, we need to
    //  be explicit about why it might not be working
    ProtocolAssignmentTask task = getSelectedTask();
    if (task.id > 0) {
        if (!hasBody(getTaskBodyID(task))) {
            showError("No such body!", "Selected body no longer exists!");
            return;
        }
    }
    gotoSelectedTaskBody();
}

void OrphanLinkProtocol::onCommentButton() {
    if (hasSelection()) {
        ProtocolAssignmentTask selectedTask = getSelectedTask();
        if (selectedTask.id > 0) {
            if (selectedTask.disposition == ProtocolAssignmentTask::DISPOSITION_SKIPPED ||
                selectedTask.disposition == ProtocolAssignmentTask::DISPOSITION_COMPLETE) {
                // reset to the saved comment
                ui->commentEntry->setText(m_comments[selectedTask.id]);
                showError("Can't update completed task!", "You can't change the comment on a completed or skipped task.");
                return;
            }

            m_comments[selectedTask.id] = ui->commentEntry->text();
            saveState();

            // update just the single item instead of rebuilding the table;
            //  I've had trouble with this before, let's see if I can make it work:

            // this duplicates code in getSelectedTask(); if we get this far,
            //  the index is always valid (because the selected task above is valid)
            QModelIndex index = ui->tableView->selectionModel()->currentIndex();
            QModelIndex modelIndex = m_proxy->mapToSource(index);
            m_model->item(modelIndex.row(), COMMENT_COLUMN)->setData(m_comments[selectedTask.id], Qt::DisplayRole);
        }
    }
}

void OrphanLinkProtocol::onClickedTable(QModelIndex /*index*/) {
    taskSelected();
}

bool OrphanLinkProtocol::hasSelection() {
    return ui->tableView->selectionModel()->hasSelection();
}

ProtocolAssignmentTask OrphanLinkProtocol::getSelectedTask() {
    QModelIndex index = ui->tableView->selectionModel()->currentIndex();
    if (index.isValid()) {
        QModelIndex modelIndex = m_proxy->mapToSource(index);
        return m_tasks[modelIndex.row()];
    } else {
        QJsonObject empty;
        return ProtocolAssignmentTask(empty);
    }
}

/*
 * called when a task is selected either via click or programmatically
 */
void OrphanLinkProtocol::taskSelected() {
    ProtocolAssignmentTask task = getSelectedTask();
    if (!hasPendingTasks()) {
        allTasksCompleted();
    } else {
        if (task.disposition == ProtocolAssignmentTask::DISPOSITION_COMPLETE ||
                task.disposition == ProtocolAssignmentTask::DISPOSITION_SKIPPED) {
            disable(START_TASK_BUTTON);
            disable(COMPLETE_SKIP_TASK_BUTTONS);
        } else if (task.disposition == ProtocolAssignmentTask::DISPOSITION_IN_PROGRESS) {
            disable(START_TASK_BUTTON);
            enable(COMPLETE_SKIP_TASK_BUTTONS);
        } else {
            // must be not started, which doesn't have a disposition
            enable(START_TASK_BUTTON);
            disable(COMPLETE_SKIP_TASK_BUTTONS);
        }
    }

    // check that the body still exists:
    if (hasBody(getTaskBodyID(task))) {
        gotoSelectedTaskBody();
    } else {
        skipNoBodyTask();
    }

    // whether all tasks are completed or or not:
    updateSelectedBodyLabel();
    updateCommentField();
}

/*
 * called when the body for the selected task doesn't exist
 */
void OrphanLinkProtocol::skipNoBodyTask() {
    ProtocolAssignmentTask task = getSelectedTask();
    if (task.id == 0) {
        // no selected task, should never happen
        return;
    }

    if (task.disposition != ProtocolAssignmentTask::DISPOSITION_COMPLETE &&
            task.disposition != ProtocolAssignmentTask::DISPOSITION_SKIPPED) {

        // we're basically just triggering the routines the user would trigger
        //  from the UI; this will ensure everything gets updated correctly;
        //  note that this relies on the selected task remaining selected
        //  through the whole process, which ought to be the case
        if (task.disposition != ProtocolAssignmentTask::DISPOSITION_IN_PROGRESS) {
            onStartTaskButton();
        }

        ui->commentEntry->setText("body doesn't exist");
        onCommentButton();
        onSkipTaskButton();

        showMessage("No body!", "Body doesn't exist!  The task has been skipped.");
    }
}

uint64_t OrphanLinkProtocol::getTaskBodyID(ProtocolAssignmentTask task) {
    QString bodyIDString = task.get(TASK_KEY_BODY_ID).toString();

    bool ok;
    uint64_t bodyID = bodyIDString.toLong(&ok);
    if (ok) {
        return bodyID;
    } else {
        return 0;
    }
}

bool OrphanLinkProtocol::hasBody(uint64_t bodyID) {
    const ZDvidReader &reader = m_dvidReader;
    if (reader.good()) {
        return reader.hasBody(bodyID);
    } else {
        return false;
    }
}

/*
 * in NeuTu, go to the body in the selected task
 */
void OrphanLinkProtocol::gotoSelectedTaskBody() {
    ProtocolAssignmentTask selectedTask = getSelectedTask();
    if (selectedTask.id > 0) {
        uint64_t bodyID = getTaskBodyID(selectedTask);
        if (bodyID > 0) {
            emit requestDisplayBody(bodyID);
        }
    }
}

bool OrphanLinkProtocol::hasPendingTasks() {
    for (ProtocolAssignmentTask task: m_tasks) {
        if (task.disposition != ProtocolAssignmentTask::DISPOSITION_COMPLETE &&
            task.disposition != ProtocolAssignmentTask::DISPOSITION_SKIPPED) {
            return true;
        }
    }
    return false;
}

void OrphanLinkProtocol::allTasksCompleted() {
    if (!m_allTasksCompleted) {
        m_allTasksCompleted = true;

        disable(NEXT_TASK_BUTTON);
        disable(START_TASK_BUTTON);
        disable(COMPLETE_SKIP_TASK_BUTTONS);

        // dialog (once only)
        showMessage("All tasks completed!", "All tasks have been completed. You may now complete the protocol.");
    }
}

/*
 * returns next not complete task; if there are none, returns
 * invalid task (you should just check before calling that there
 * are pending tasks)
 */
ProtocolAssignmentTask OrphanLinkProtocol::findNextTask() {
    QJsonObject empty;
    if (!hasPendingTasks()) {
        // should never happen
        return ProtocolAssignmentTask(empty);
    }
    // now we can assume there is always an incomplete task to find

    // this cycles through the table in view order, even if the sort
    //  order is changed; I *believe* it will work with filtering, too,
    //  but we don't have that currently, and it has not been tested

    // start at the top or the task after the selected task
    QModelIndex startIndex = m_proxy->index(0, TASK_ID_COLUMN);
    if (hasSelection()) {
        QModelIndex selectedIndex = ui->tableView->selectionModel()->currentIndex();
        if (selectedIndex.isValid()) {
            startIndex = selectedIndex.sibling(selectedIndex.row() + 1, TASK_ID_COLUMN);
            if (!startIndex.isValid()) {
                // back to top again
                startIndex = m_proxy->index(0, TASK_ID_COLUMN);
            }
        }
        // if it's invalid, it's probably off the bottom, but in any case,
        //  if it's invalid, stick with the top of the list
    }

    QModelIndex currentIndex = startIndex;
    int n = 0;
    while (m_tasks[m_proxy->mapToSource(currentIndex).row()].disposition == ProtocolAssignmentTask::DISPOSITION_COMPLETE ||
           m_tasks[m_proxy->mapToSource(currentIndex).row()].disposition == ProtocolAssignmentTask::DISPOSITION_SKIPPED) {
        // qDebug() << "view row " << currentIndex.row();
        // qDebug() << "model row " << m_proxy->mapToSource(currentIndex).row();
        // qDebug() << "ID: " << m_tasks[m_proxy->mapToSource(currentIndex).row()].id;
        // increment the index
        currentIndex = currentIndex.sibling(currentIndex.row() + 1, TASK_ID_COLUMN);
        if (!currentIndex.isValid()) {
            // again assuming invalid = off the bottom
            currentIndex = startIndex.sibling(0, TASK_ID_COLUMN);
        }
        n++;
        if (n > m_tasks.size() + 1) {
            // shouldn't happen unless something goes very wrong
            return ProtocolAssignmentTask(empty);
        }
    }

    // qDebug() << "next task: ID = " << m_tasks[m_proxy->mapToSource(currentIndex).row()].id;
    return m_tasks[m_proxy->mapToSource(currentIndex).row()];
}

/*
 * given a task ID, return its task's index in the task list
 */
int OrphanLinkProtocol::findTaskIndex(int taskID) {
    int index = -1;
    for (int i=0; i<m_tasks.size(); i++) {
        if (m_tasks[i].id == taskID) {
            index = i;
            break;
        }
    }
    return index;
}

/*
 * given a task, return its index in the task list
 */
int OrphanLinkProtocol::findTaskIndex(ProtocolAssignmentTask task) {
    return findTaskIndex(task.id);
}

void OrphanLinkProtocol::updateTable() {
    saveSelection();

    m_model->clear();
    setHeaders(m_model);

    int row = 0;
    for (ProtocolAssignmentTask task: m_tasks) {
        QStandardItem * taskIDItem = new QStandardItem();
        taskIDItem->setData(task.id, Qt::DisplayRole);
        m_model->setItem(row, TASK_ID_COLUMN, taskIDItem);

        QStandardItem * bodyIDItem = new QStandardItem();
        bodyIDItem->setData(task.get(TASK_KEY_BODY_ID), Qt::DisplayRole);
        m_model->setItem(row, BODY_ID_COLUMN, bodyIDItem);

        QStandardItem * statusItem = new QStandardItem();
        statusItem->setData(task.disposition, Qt::DisplayRole);
        m_model->setItem(row, STATUS_COLUMN, statusItem);

        QStandardItem * commentItem = new QStandardItem();
        commentItem->setData(m_comments[task.id], Qt::DisplayRole);
        m_model->setItem(row, COMMENT_COLUMN, commentItem);

        row++;
    }
    ui->tableView->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    ui->tableView->horizontalHeader()->setSectionResizeMode(COMMENT_COLUMN, QHeaderView::Stretch);

    restoreSelection();
}

void OrphanLinkProtocol::updateLabels() {
    updateSelectedBodyLabel();
    updateProgressLabel();
}

void OrphanLinkProtocol::updateSelectedBodyLabel() {
    QString text;
    if (hasSelection()) {
        text = getSelectedTask().get(TASK_KEY_BODY_ID).toString();
    } else {
        text = "";
    }
    ui->currentBodyLabel->setText(text);
}

void OrphanLinkProtocol::updateProgressLabel() {
    int nComplete = 0;
    for (ProtocolAssignmentTask task: m_tasks) {
        if (task.disposition == ProtocolAssignmentTask::DISPOSITION_COMPLETE ||
            task.disposition == ProtocolAssignmentTask::DISPOSITION_SKIPPED) {
            nComplete++;
        }
    }
    float percent = 100 * float(nComplete) / m_tasks.size();
    ui->progressLabel->setText(QString("%1/%2 (%3%)").arg(nComplete).arg(m_tasks.size()).arg(percent, 1, 'f', 1));
}

void OrphanLinkProtocol::updateCommentField() {
    if (hasSelection()) {
        ui->commentEntry->setText(m_comments[getSelectedTask().id]);
    } else {
        ui->commentEntry->clear();
    }
}

void OrphanLinkProtocol::loadTasks() {
    ProtocolAssignment assignment = m_client.getAssignment(m_assignmentID);
    m_tasks = m_client.getAssignmentTasks(assignment);
    std::sort(m_tasks.begin(), m_tasks.end(), compareTasks);
}

/*
 * given a task object, find the (probably stale) task object with
 * the same ID in the task list and replace it with the new object
 */
void OrphanLinkProtocol::updateTask(ProtocolAssignmentTask task) {
    int index = findTaskIndex(task);
    m_tasks.replace(index, task);
}

bool OrphanLinkProtocol::compareTasks(const ProtocolAssignmentTask task1, const ProtocolAssignmentTask task2) {
    // compare by ID:
    return task1.id < task2.id;
}

void OrphanLinkProtocol::loadDataRequested(ZJsonObject data) {
    // check version of saved data here
    if (!data.hasKey(KEY_VERSION.c_str())) {
        showError("Error parsing protocol information!", "The protocol information in DVID is missing its version information!  Data could not be loaded.");
        return;
    }
    int version = ZJsonParser::integerValue(data[KEY_VERSION.c_str()]);
    if (version > fileVersion) {
        showError("Newer version required!", "The protocol information in DVID from a newer version of NeuTu!  Data could not be loaded.");
        return;
    }

    // convert old versions to current version here, when it becomes necessary


    // load data here; assignment ID and comments
    m_assignmentID = ZJsonParser::integerValue(data[KEY_ASSIGNMENT_ID.c_str()]);
    ZJsonObject comments(data.value(KEY_COMMENTS.c_str()));
    for (std::string key: comments.getAllKey()) {
        int taskID = std::stoi(key);
        m_comments[taskID] = QString::fromStdString(ZJsonParser::stringValue(comments[key.c_str()]));
    }


    // if, in the future, you need to update to a new save version,
    //  remember to do a saveState() here


    // start work
    loadTasks();
    updateTable();
    updateLabels();

    startProtocol();

}

void OrphanLinkProtocol::saveState() {
    ZJsonObject data;

    // always version your output files!
    data.setEntry(KEY_VERSION.c_str(), fileVersion);

    // only assignment ID and comments right now
    data.setEntry(KEY_ASSIGNMENT_ID.c_str(), m_assignmentID);
    ZJsonObject comments;
    for (int taskID: m_comments.keys()) {
        comments.setEntry(std::to_string(taskID).c_str(), m_comments[taskID].toStdString().c_str());
    }
    data.setEntry(KEY_COMMENTS.c_str(), comments);

    emit requestSaveProtocol(data);
}

void OrphanLinkProtocol::onExitButton() {
    emit protocolExiting();
}

void OrphanLinkProtocol::onCompleteButton() {
    QMessageBox mb;
    mb.setText("Complete protocol");
    mb.setInformativeText("When you complete the protocol, it will save and exit immediately.  You will not be able to reopen it.\n\nComplete protocol now?");
    mb.setStandardButtons(QMessageBox::Cancel | QMessageBox::Ok);
    mb.setDefaultButton(QMessageBox::Cancel);
    int ans = mb.exec();

    if (ans == QMessageBox::Ok) {
        // save here if needed
        // saveState();
        emit protocolCompleting();
    }
}

void OrphanLinkProtocol::saveSelection() {
    m_savedTaskID = getSelectedTask().id;
}

void OrphanLinkProtocol::restoreSelection() {
    if (m_savedTaskID > 0) {

        // this is adapted from onNextTaskButton(); factor some of the
        //  code out at some point?

        int nextRow = findTaskIndex(m_savedTaskID);
        if (nextRow < 0) {
            // not found, should be impossible?
            return;
        }

        QModelIndex nextModelIndex = m_model->index(nextRow, 0);
        QModelIndex viewIndex = m_proxy->mapFromSource(nextModelIndex);
        if (viewIndex.isValid()) {
            ui->tableView->selectionModel()->setCurrentIndex(viewIndex, QItemSelectionModel::ClearAndSelect|QItemSelectionModel::Rows);
            // since we're restoring selection, do NOT trigger taskSelected();
        }
    }
}

void OrphanLinkProtocol::enable(DisenableElements element) {
    switch (element) {
    case NEXT_TASK_BUTTON:
        ui->nextTaskButton->setEnabled(true);
        break;
    case START_TASK_BUTTON:
        ui->startTaskButton->setEnabled(true);
        break;
    case COMPLETE_SKIP_TASK_BUTTONS:
        ui->completeTaskButton->setEnabled(true);
        ui->skipTaskButton->setEnabled(true);
        break;
    }
}

void OrphanLinkProtocol::disable(DisenableElements element) {
    switch (element) {
    case NEXT_TASK_BUTTON:
        ui->nextTaskButton->setEnabled(false);
        break;
    case START_TASK_BUTTON:
        ui->startTaskButton->setEnabled(false);
        break;
    case COMPLETE_SKIP_TASK_BUTTONS:
        ui->completeTaskButton->setEnabled(false);
        ui->skipTaskButton->setEnabled(false);
        break;
    }
}

void OrphanLinkProtocol::setHeaders(QStandardItemModel *model) {
    model->setHorizontalHeaderItem(TASK_ID_COLUMN, new QStandardItem(QString("Task ID")));
    model->setHorizontalHeaderItem(BODY_ID_COLUMN, new QStandardItem(QString("Body ID")));
    model->setHorizontalHeaderItem(STATUS_COLUMN, new QStandardItem(QString("Status")));
    model->setHorizontalHeaderItem(COMMENT_COLUMN, new QStandardItem(QString("Comment")));
}

/*
 * input: title and message fordialog
 * effect: shows dialog (convenience function)
 */
void OrphanLinkProtocol::showMessage(QString title, QString message) {
    QMessageBox messageBox;
    messageBox.setText(title);
    messageBox.setInformativeText(message);
    messageBox.setStandardButtons(QMessageBox::Ok);
    messageBox.setIcon(QMessageBox::Information);
    messageBox.exec();
}

/*
 * input: title and message for error dialog
 * effect: shows error dialog (convenience function)
 */
void OrphanLinkProtocol::showError(QString title, QString message) {
    QMessageBox errorBox;
    errorBox.setText(title);
    errorBox.setInformativeText(message);
    errorBox.setStandardButtons(QMessageBox::Ok);
    errorBox.setIcon(QMessageBox::Warning);
    errorBox.exec();
}

OrphanLinkProtocol::~OrphanLinkProtocol()
{
    delete ui;
}

