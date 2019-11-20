#include "protocolassignmentdialog.h"
#include "ui_protocolassignmentdialog.h"

#include <QMessageBox>
#include <QInputDialog>

#include <QJsonArray>
#include <QJsonObject>

#include <QStandardItem>

#include "neutube.h"

#include "flyem/auth/flyemauthtokenhandler.h"

ProtocolAssignmentDialog::ProtocolAssignmentDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ProtocolAssignmentDialog)
{
    ui->setupUi(this);


    // sites table
    m_model = new QStandardItemModel(0, 4, ui->startedTableView);
    setHeaders(m_model);

    m_proxy = new QSortFilterProxyModel(this);
    m_proxy->setSourceModel(m_model);
    ui->startedTableView->setModel(m_proxy);


    // UI connections
    connect(ui->loadStartedButton, SIGNAL(clicked(bool)), this, SLOT(onLoadStartedButton()));
    connect(ui->getNewButton, SIGNAL(clicked(bool)), this, SLOT(onGetNewButton()));
    connect(ui->completeButton, SIGNAL(clicked(bool)), this, SLOT(onCompleteButton()));

    connect(ui->startedTableView, SIGNAL(clicked(QModelIndex)), this, SLOT(onClickedTable(QModelIndex)));


    // server setup
    // eventually this will be taken from a config; hardcode for now
    m_client.setServer("http://flyem-assignment.int.janelia.org");

    m_username = QString::fromStdString(neutu::GetCurrentUserName());
}

const QString ProtocolAssignmentDialog::ASSIGNMENT_APPLICATION_NAME = "assignment-manager";

bool ProtocolAssignmentDialog::checkForTokens() {
    // check for master token
    FlyEmAuthTokenHandler handler;
    if (!handler.hasMasterToken()) {
        showError("No authentication token!",
            "NeuTu needs your Fly EM services authentication token! Open the authentication dialog via the yellow key icon on the toolbar and follow the instructions, then try this action again.");
        return false;
    }

    // if present, try to get application token
    QString applicationName = ASSIGNMENT_APPLICATION_NAME;
    QString token = handler.getApplicationToken(applicationName);
    if (token.isEmpty()) {
        showError("No application token!", "Could not retrieve application token for " + applicationName);
        return false;
    }
    m_client.setToken(token);
    return true;
}

void ProtocolAssignmentDialog::onLoadStartedButton() {
    if (checkForTokens()) {
        loadStartedAssignments();
        updateStartedTable();
    }
}

void ProtocolAssignmentDialog::onGetNewButton() {
    if (checkForTokens()) {
        QStringList projects = m_client.getEligibleProjects();
        if (projects.size() == 0) {
            showMessage("No projects!", "No eligible projects found!");
            return;
        }

        // now show dialog: user chooses a project
        bool ok;
        QString project = QInputDialog::getItem(this, "Get assignment",
            "Choose a project to get an assignment from:", projects, 0, false, &ok);
        if (ok && !project.isEmpty()) {
            // call endpoint to start assignment

            showMessage("temp", "pretending to start assignment for project " + project);

            // check success
            if (true) {
                loadStartedAssignments();
                updateStartedTable();
            } else {
                showMessage("No assignment", "Failed to start an assignment for project " + project);
            }
        }
    }
}

void ProtocolAssignmentDialog::onCompleteButton() {

    if (ui->startedTableView->selectionModel()->hasSelection()) {
        // single row selection model, so just grab the first/only row:
        QModelIndexList modelIndexList = ui->startedTableView->selectionModel()->selectedRows(0);
        // make sure there is a selected index to avoid unexpected crash (?, copied from other code)
        if (!modelIndexList.isEmpty()) {
            QModelIndex viewIndex = modelIndexList.at(0);
            QModelIndex modelIndex = m_proxy->mapToSource(viewIndex);

             // modelIndex.row() = assignment to complete; do something with it


        }
    } else {
        showMessage("No selection!", "Please select an assignment to complete.");
    }
}

void ProtocolAssignmentDialog::onClickedTable(QModelIndex index) {


    QModelIndex modelIndex = m_proxy->mapToSource(index);

    qDebug() << "table clicked model row " << modelIndex.row();


}

void ProtocolAssignmentDialog::loadStartedAssignments() {


    // temporary; would like to have a better data structure,
    //  maybe a map from id to the structure

    // probably should sort them by id internally?
    // or instead sort them programmatically by id initially (since I
    //  hooked up sort-filter proxy)?

    m_assignments = m_client.getStartedAssignments();



}

void ProtocolAssignmentDialog::updateStartedTable() {
    clearStartedTable();
    int row = 0;
    for (ProtocolAssignment assignment: m_assignments) {

        QStandardItem * idItem = new QStandardItem();
        idItem->setData(assignment.id, Qt::DisplayRole);
        m_model->setItem(row, ID_COLUMN, idItem);

        QStandardItem * projectItem = new QStandardItem();
        projectItem->setData(assignment.project, Qt::DisplayRole);
        m_model->setItem(row, PROJECT_COLUMN, projectItem);

        QStandardItem * protocolItem = new QStandardItem();
        protocolItem->setData(assignment.protocol, Qt::DisplayRole);
        m_model->setItem(row, PROTOCOL_COLUMN, protocolItem);

        QStandardItem * nameItem = new QStandardItem();
        nameItem->setData(assignment.name, Qt::DisplayRole);
        m_model->setItem(row, NAME_COLUMN, nameItem);

        QStandardItem * startedItem = new QStandardItem();
        startedItem->setData(assignment.start_date, Qt::DisplayRole);
        m_model->setItem(row, STARTED_COLUMN, startedItem);

        row++;
    }
    ui->startedTableView->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);

    // this is a guess; not sure which column will end up needing most space as of yet
    ui->startedTableView->horizontalHeader()->setSectionResizeMode(NAME_COLUMN, QHeaderView::Stretch);
}

void ProtocolAssignmentDialog::clearStartedTable() {
    m_model->clear();
    setHeaders(m_model);
}

void ProtocolAssignmentDialog::setHeaders(QStandardItemModel *model) {
    model->setHorizontalHeaderItem(ID_COLUMN, new QStandardItem(QString("ID")));
    model->setHorizontalHeaderItem(PROJECT_COLUMN, new QStandardItem(QString("Project")));
    model->setHorizontalHeaderItem(PROTOCOL_COLUMN, new QStandardItem(QString("Protocol")));
    model->setHorizontalHeaderItem(NAME_COLUMN, new QStandardItem(QString("Name")));
    model->setHorizontalHeaderItem(STARTED_COLUMN, new QStandardItem(QString("Start date")));
}

void ProtocolAssignmentDialog::showEvent(QShowEvent* event) {
    QWidget::showEvent(event);

    // refresh the started assignment list when the dialog is shown
    onLoadStartedButton();
}


void ProtocolAssignmentDialog::showError(QString title, QString message) {
    QMessageBox mb;
    mb.setText(title);
    mb.setIcon(QMessageBox::Warning);
    mb.setInformativeText(message);
    mb.setStandardButtons(QMessageBox::Ok);
    mb.setDefaultButton(QMessageBox::Ok);
    mb.exec();
}

void ProtocolAssignmentDialog::showMessage(QString title, QString message) {
    QMessageBox mb;
    mb.setText(title);
    mb.setInformativeText(message);
    mb.setStandardButtons(QMessageBox::Ok);
    mb.setDefaultButton(QMessageBox::Ok);
    mb.exec();
}

ProtocolAssignmentDialog::~ProtocolAssignmentDialog()
{
    delete ui;
}
