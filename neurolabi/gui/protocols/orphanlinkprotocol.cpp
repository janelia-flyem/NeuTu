#include "orphanlinkprotocol.h"
#include "ui_orphanlinkprotocol.h"

#include <iostream>
#include <stdlib.h>

#include <QMessageBox>

#include "protocolchooseassignmentdialog.h"
#include "protocolassignment.h"

#include "zjsonparser.h"

OrphanLinkProtocol::OrphanLinkProtocol(QWidget *parent) :
    ProtocolDialog(parent),
    ui(new Ui::OrphanLinkProtocol)
{
    ui->setupUi(this);


    // currently hard-coded, but we expect to get from some config source later
    // must not end with / character!  enforce that when we un-hard-code it
    m_client.setServer("http://flyem-assignment.int.janelia.org");


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
    connect(ui->gotoCurrentButton, SIGNAL(clicked(bool)), this, SLOT(onGotoCurrentButton()));

    connect(ui->tableView, SIGNAL(clicked(QModelIndex)), this, SLOT(onClickedTable(QModelIndex)));
    connect(ui->tableView, SIGNAL(doubleClicked(QModelIndex)), this, SLOT(onDoubleClickedTable(QModelIndex)));

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

    // onFirstButton();




    return true;
}

void OrphanLinkProtocol::onGotoCurrentButton() {
    gotoCurrentBody();
}

void OrphanLinkProtocol::onCommentButton() {
    if (hasSelection()) {
        ProtocolAssignmentTask selectedTask = getSelectedTask();
        if (selectedTask.id > 0) {
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
    updateCurrentBodyLabel();
    updateCommentField();
}

void OrphanLinkProtocol::onDoubleClickedTable(QModelIndex /*index*/) {
    // this works because the double-click will also select the table row
    gotoCurrentBody();
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

void OrphanLinkProtocol::gotoCurrentBody() {
    QString bodyIDString = getSelectedTask().get(TASK_KEY_BODY_ID).toString();

    bool ok;
    uint64_t bodyID = bodyIDString.toLong(&ok);
    if (ok) {
        emit requestDisplayBody(bodyID);
    }
}

void OrphanLinkProtocol::updateTable() {
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
}

void OrphanLinkProtocol::updateLabels() {
    updateCurrentBodyLabel();
    updateProgressLabel();
}

void OrphanLinkProtocol::updateCurrentBodyLabel() {
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
        if (task.disposition == ProtocolAssignmentTask::DISPOSITION_COMPLETE) {
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

bool OrphanLinkProtocol::compareTasks(const ProtocolAssignmentTask task1, const ProtocolAssignmentTask task2) {
    // compare by ID:
    return task1.id < task2.id;
}

void OrphanLinkProtocol::loadDataRequested(ZJsonObject data) {
    // check version of saved data here
    if (!data.hasKey(KEY_VERSION.c_str())) {

        // report error

        return;
    }
    int version = ZJsonParser::integerValue(data[KEY_VERSION.c_str()]);
    if (version > fileVersion) {


        // report error


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

    // onFirstButton();

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

void OrphanLinkProtocol::setHeaders(QStandardItemModel *model) {
    model->setHorizontalHeaderItem(TASK_ID_COLUMN, new QStandardItem(QString("Task ID")));
    model->setHorizontalHeaderItem(BODY_ID_COLUMN, new QStandardItem(QString("Body ID")));
    model->setHorizontalHeaderItem(STATUS_COLUMN, new QStandardItem(QString("Status")));
    model->setHorizontalHeaderItem(COMMENT_COLUMN, new QStandardItem(QString("Comment")));
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

