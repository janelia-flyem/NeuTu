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
    ui->startedTableView->setModel(m_model);


    // UI connections
    connect(ui->loadStartedButton, SIGNAL(clicked(bool)), this, SLOT(onLoadStartedButton()));
    connect(ui->getNewButton, SIGNAL(clicked(bool)), this, SLOT(onGetNewButton()));


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
        updateStartedTable();
    }
}

void ProtocolAssignmentDialog::onGetNewButton() {
    if (checkForTokens()) {

        // not sure how I'm doing errors right now...I suppose the best way
        //  might be signals and slots?  one for success, one for failure?


        QStringList projects = m_client.getEligibleProjects();
        if (projects.size() == 0) {
            showMessage("No projects!", "No eligible projects found!");
            return;
        }

        // dialog: user chooses a project

        bool ok;
        QString project = QInputDialog::getItem(this, "Get assignment",
            "Choose a project to get an assignment from:", projects, 0, false, &ok);
        if (ok && !project.isEmpty()) {
            // call endpoint to start assignment

            showMessage("temp", "pretending to start assignment for project " + project);

            // check success
            if (true) {
                updateStartedTable();
            } else {
                showMessage("No assignment", "Failed to start an assignment for project " + project);
            }
        }
    }
}

void ProtocolAssignmentDialog::updateStartedTable() {
    // get started assignments from server, then update table
    QJsonArray assignments = m_client.getStartedAssignments();
    clearStartedTable();
    int row = 0;
    for (QJsonValue val: assignments) {
        QJsonObject a = val.toObject();

        QStandardItem * idItem = new QStandardItem();
        idItem->setData(QString::number(a["id"].toInt()), Qt::DisplayRole);
        m_model->setItem(row, ID_COLUMN, idItem);

        QStandardItem * projectItem = new QStandardItem();
        projectItem->setData(a["project"].toString(), Qt::DisplayRole);
        m_model->setItem(row, PROJECT_COLUMN, projectItem);

        QStandardItem * protocolItem = new QStandardItem();
        protocolItem->setData(a["protocol"].toString(), Qt::DisplayRole);
        m_model->setItem(row, PROTOCOL_COLUMN, protocolItem);

        QStandardItem * nameItem = new QStandardItem();
        nameItem->setData(a["name"].toString(), Qt::DisplayRole);
        m_model->setItem(row, NAME_COLUMN, nameItem);

        QStandardItem * startedItem = new QStandardItem();
        startedItem->setData(a["start_date"].toString(), Qt::DisplayRole);
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
