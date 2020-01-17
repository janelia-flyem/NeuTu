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


    // UI connections
    connect(ui->exitButton, SIGNAL(clicked(bool)), this, SLOT(onExitButton()));
    connect(ui->completeButton, SIGNAL(clicked(bool)), this, SLOT(onCompleteButton()));

}

// constants
const std::string OrphanLinkProtocol::KEY_VERSION = "version";
const std::string OrphanLinkProtocol::KEY_ASSIGNMENT_ID = "assignment ID";
const std::string OrphanLinkProtocol::KEY_COMMENTS = "comments";
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

void OrphanLinkProtocol::updateTable() {

    // should tasks be sorted first, or should we just hook up a sorter and let the user have a say?
    // probably ought to do it right...but note it's not so easy, if we also have "go to first
    //  pending task", right?  maybe just sort by ID initially and force that order?



}

void OrphanLinkProtocol::updateLabels() {
    updateCurrentLabel();
    updateProgressLabel();
}

void OrphanLinkProtocol::updateCurrentLabel() {


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

void OrphanLinkProtocol::loadTasks() {
    ProtocolAssignment assignment = m_client.getAssignment(m_assignmentID);
    m_tasks = m_client.getAssignmentTasks(assignment);
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

