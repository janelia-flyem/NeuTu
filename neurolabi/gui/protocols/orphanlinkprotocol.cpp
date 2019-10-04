#include "orphanlinkprotocol.h"
#include "ui_orphanlinkprotocol.h"

#include <iostream>
#include <stdlib.h>

#include <QMessageBox>

#include "orphanlinkinputdialog.h"

#include "zjsonparser.h"

OrphanLinkProtocol::OrphanLinkProtocol(QWidget *parent) :
    ProtocolDialog(parent),
    ui(new Ui::OrphanLinkProtocol)
{
    ui->setupUi(this);


    // currently hard-coded, but we expect to get from some config source later

    // must not end with / character!  enforce that when we un-hard-code it
    m_assignments.setServer("http://flyem-assignment.int.janelia.org");


    // UI connections
    connect(ui->exitButton, SIGNAL(clicked(bool)), this, SLOT(onExitButton()));
    connect(ui->completeButton, SIGNAL(clicked(bool)), this, SLOT(onCompleteButton()));

}

// constants
const std::string OrphanLinkProtocol::KEY_VERSION = "version";
const std::string OrphanLinkProtocol::KEY_PARAMETERS = "parameters";
const int OrphanLinkProtocol::fileVersion = 1;

bool OrphanLinkProtocol::initialize() {

    // Qt really really wants network calls to be asynchronous, which is a real pain
    //  for me;


    QMap<QString, int> projects = m_assignments.getProjectsForProtocol(ProtocolAssignmentClient::ORPHAN_LINK);

    if (projects.size() == 0) {
        showError("No projects!", "No projects were found for the orphan link protocol!");
        return false;
    }

    OrphanLinkInputDialog inputDialog(QStringList(projects.keys()));
    int ans = inputDialog.exec();
    if (ans == QDialog::Rejected) {
        return false;
    }


    // get the user's chosen project; generate the assignment and start it
    QString projectName = inputDialog.getProject();


    int assignmentID = m_assignments.generateAssignment(projectName);

    // what's the error condition?  ID < 0?
    if (assignmentID <= 0) {
        showError("Assignment generation failed!", "Could not generate an assignment for project " + projectName);
        return false;
    }


    bool status = m_assignments.startAssignment(assignmentID);


    if (!status) {
        showError("Assignment start failed!", "Assigment ID " + QString::number(assignmentID) + " did not start correctly!");
        return false;
    }


    // get all the tasks?  or just the number of tasks and the next one?  not
    //  clear what we intend at this point

    // init UI


    // advance to first task



    return true;
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


    // load data here





    // if, in the future, you need to update to a new save version,
    //  remember to do a save here


    // start work

}

void OrphanLinkProtocol::saveState() {
    ZJsonObject data;

    // always version your output files!
    data.setEntry(KEY_VERSION.c_str(), fileVersion);








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

