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
    m_model = new QStandardItemModel(0, 5, ui->assignmentTableView);
    setHeaders(m_model);

    m_proxy = new QSortFilterProxyModel(this);
    m_proxy->setSourceModel(m_model);
    ui->assignmentTableView->setModel(m_proxy);


    // UI connections
    connect(ui->refreshButton, SIGNAL(clicked(bool)), this, SLOT(onRefreshButton()));
    connect(ui->getNewButton, SIGNAL(clicked(bool)), this, SLOT(onGetNewButton()));
    connect(ui->completeButton, SIGNAL(clicked(bool)), this, SLOT(onCompleteButton()));
    connect(ui->startButton, SIGNAL(clicked(bool)), this, SLOT(onStartButton()));

    connect(ui->assignmentTableView, SIGNAL(clicked(QModelIndex)), this, SLOT(onClickedTable(QModelIndex)));


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

void ProtocolAssignmentDialog::onRefreshButton() {
    if (checkForTokens()) {
        loadAssignments();
        updateAssignmentsTable();
    }
}

void ProtocolAssignmentDialog::onGetNewButton() {
    if (checkForTokens()) {

        QMap<QString, QString> projects = m_client.getEligibleProjects();
        if (projects.size() == 0) {
            showMessage("No projects!", "No eligible projects found!");
            return;
        }

        // show dialog: user chooses a project from list of "project name (procotol)"
        QStringList options;
        QStringList nameList;
        for (QString projectName: projects.keys()) {
            nameList << projectName;
            options << projectName + " (" + projects[projectName] + ")";
        }

        bool ok;
        QString choice = QInputDialog::getItem(this, "Get assignment",
            "Choose a project to get an assignment from:", options, 0, false, &ok);
        if (ok && !choice.isEmpty()) {
            QString projectName = nameList[options.indexOf(choice)];
            int assignmentID = m_client.generateAssignment(projectName);


            // check success
            if (assignmentID == -1) {
                showMessage("No assignment", "Failed to start an assignment for project " + projectName);
            } else {
                loadAssignments();
                updateAssignmentsTable();
            }
        }
    }
}

void ProtocolAssignmentDialog::onStartButton() {
    if (ui->assignmentTableView->selectionModel()->hasSelection()) {
        // single row selection model, so just grab the first/only row:
        QModelIndexList modelIndexList = ui->assignmentTableView->selectionModel()->selectedRows(0);
        // make sure there is a selected index to avoid unexpected crash (copied from other code,
        //      not sure what the issue is)
        if (!modelIndexList.isEmpty()) {
            QModelIndex viewIndex = modelIndexList.at(0);
            QModelIndex modelIndex = m_proxy->mapToSource(viewIndex);
            ProtocolAssignment assignment = m_assignments[modelIndex.row()];

            // is it not started?  I wish we had an actual disposition for this...
            if (assignment.disposition != ProtocolAssignment::DISPOSITION_SKIPPED &&
                assignment.disposition != ProtocolAssignment::DISPOSITION_COMPLETE &&
                assignment.disposition != ProtocolAssignment::DISPOSITION_IN_PROGRESS) {
                bool status = m_client.startAssignment(assignment.id);
                if (status) {
                    // refresh table if successful; if not, error should have already
                    //  been displayed
                    loadAssignments();
                    updateAssignmentsTable();
                }
            } else {
                showError("Not eligible!", "Assignment may not be in progress, skipped, or complete to start.");
                return;
            }
        }
    } else {
        showMessage("No selection!", "Please select an assignment to start.");
    }
}

void ProtocolAssignmentDialog::onCompleteButton() {

    if (ui->assignmentTableView->selectionModel()->hasSelection()) {
        // single row selection model, so just grab the first/only row:
        QModelIndexList modelIndexList = ui->assignmentTableView->selectionModel()->selectedRows(0);
        // make sure there is a selected index to avoid unexpected crash (copied from other code,
        //      not sure what the issue is)
        if (!modelIndexList.isEmpty()) {
            QModelIndex viewIndex = modelIndexList.at(0);
            QModelIndex modelIndex = m_proxy->mapToSource(viewIndex);
            ProtocolAssignment assignment = m_assignments[modelIndex.row()];

            // are all the tasks complete?
            QList<ProtocolAssignmentTask> tasks = m_client.getAssignmentTasks(assignment);
            int nCompleted = 0;
            for (ProtocolAssignmentTask t: tasks) {
                if (t.disposition == ProtocolAssignmentTask::DISPOSITION_COMPLETE) {
                    nCompleted++;
                }
            }

            // offer to complete all tasks; right now, this is kind of mandatory,
            //  as we initially haven't provided a way to complete individual tasks
            //  as you go (this is planned for the future)
            if (nCompleted < tasks.size()) {
                int ans = QMessageBox::question(this, "Uncompleted tasks",
                    QString("There are %1 uncompleted tasks; complete them now?").arg(tasks.size() - nCompleted),
                    QMessageBox::Ok|QMessageBox::Cancel,
                    QMessageBox::Cancel);
                if (ans == QMessageBox::Ok) {
                    // complete all tasks; also completes the assignment (happens automatically
                    //  when the last task is completed)
                    bool status = completeAllTasks(assignment);
                    if (!status) {
                        showError("Problem completing tasks", "At least one task could not be completed. Assignment not completed.");
                        return;
                    }
                } else {
                    return;
                }
            } else {
                // complete assignment explicitly; this may never happen, as completing all tasks
                //  from an assignment completes the assignment automatically
                bool status = m_client.completeAssignment(assignment);
                if (!status) {
                    showError("Problem completing assignment", "There was a problem completing the assignment.");
                    // don't return here; let the table refresh so the user can look at all the info
                }
            }

            // refresh table
            loadAssignments();
            updateAssignmentsTable();
        }
    } else {
        showMessage("No selection!", "Please select an assignment to complete.");
    }
}

void ProtocolAssignmentDialog::onClickedTable(QModelIndex index) {
    QModelIndex modelIndex = m_proxy->mapToSource(index);
    ProtocolAssignment assignment = m_assignments[modelIndex.row()];
    updateSelectedInfo(assignment);
}

void ProtocolAssignmentDialog::loadAssignments() {
    m_assignments = m_client.getAssignments();
    clearSelectedInfo();
}

bool ProtocolAssignmentDialog::completeTask(ProtocolAssignmentTask task) {

    // if not started, start it
    if (task.disposition != ProtocolAssignmentTask::DISPOSITION_IN_PROGRESS) {
        bool status = m_client.startTask(task);
        if (!status) {
            return status;
        }
    }

    bool status = m_client.completeTask(task);
    return status;

}

bool ProtocolAssignmentDialog::completeAllTasks(ProtocolAssignment assignment) {
    // NOTE: this has the effect of completing the assignment, too; completing
    //  the final task of an assignment automatically completes the assignment
    QList<ProtocolAssignmentTask> tasks = m_client.getAssignmentTasks(assignment);
    for (ProtocolAssignmentTask t: tasks) {
        if (t.disposition != ProtocolAssignmentTask::DISPOSITION_COMPLETE) {
            bool status = completeTask(t);
            if (!status) {
                return false;
            }
        }
    }
    return true;
}

void ProtocolAssignmentDialog::updateAssignmentsTable() {
    clearAssignmentsTable();
    int row = 0;
    for (ProtocolAssignment assignment: m_assignments) {

        QStandardItem * idItem = new QStandardItem();
        idItem->setData(assignment.id, Qt::DisplayRole);
        m_model->setItem(row, ID_COLUMN, idItem);

        QStandardItem * dispositionItem = new QStandardItem();
        dispositionItem->setData(assignment.disposition, Qt::DisplayRole);
        m_model->setItem(row, DISPOSITION_COLUMN, dispositionItem);

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
    ui->assignmentTableView->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);

    // this is a guess; not sure which column will end up needing most space as of yet
    ui->assignmentTableView->horizontalHeader()->setSectionResizeMode(NAME_COLUMN, QHeaderView::Stretch);

    // maybe sort them by id initially?

}

void ProtocolAssignmentDialog::updateSelectedInfo(ProtocolAssignment assignment) {
    ui->nameLabel->setText(assignment.name);
    ui->idLabel->setText(QString::number(assignment.id));
    ui->dispositionLabel->setText(assignment.disposition);
    ui->createdLabel->setText(assignment.create_date);
    ui->startedLabel->setText(assignment.start_date);
    ui->completedLabel->setText(assignment.completion_date);
    ui->noteLabel->setText(assignment.note);


    QList<ProtocolAssignmentTask> tasks = m_client.getAssignmentTasks(assignment);
    int nCompleted = 0;
    for (ProtocolAssignmentTask t: tasks) {
        if (t.disposition == ProtocolAssignmentTask::DISPOSITION_COMPLETE) {
            nCompleted++;
        }
    }
    ui->tasksLabel->setText(QString::number(nCompleted) + "/" + QString::number(tasks.size()));
}

void ProtocolAssignmentDialog::clearSelectedInfo() {
    ui->nameLabel->clear();
    ui->idLabel->clear();
    ui->createdLabel->clear();
    ui->startedLabel->clear();
    ui->completedLabel->clear();
    ui->tasksLabel->clear();
}

void ProtocolAssignmentDialog::clearAssignmentsTable() {
    m_model->clear();
    setHeaders(m_model);
}

void ProtocolAssignmentDialog::setHeaders(QStandardItemModel *model) {
    model->setHorizontalHeaderItem(ID_COLUMN, new QStandardItem(QString("ID")));
    model->setHorizontalHeaderItem(DISPOSITION_COLUMN, new QStandardItem(QString("Disposition")));
    model->setHorizontalHeaderItem(PROJECT_COLUMN, new QStandardItem(QString("Project")));
    model->setHorizontalHeaderItem(PROTOCOL_COLUMN, new QStandardItem(QString("Protocol")));
    model->setHorizontalHeaderItem(NAME_COLUMN, new QStandardItem(QString("Name")));
    model->setHorizontalHeaderItem(STARTED_COLUMN, new QStandardItem(QString("Start date")));
}

void ProtocolAssignmentDialog::showEvent(QShowEvent* event) {
    QWidget::showEvent(event);

    // refresh the started assignment list when the dialog is shown
    onRefreshButton();
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
