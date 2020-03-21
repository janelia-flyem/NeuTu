#include "protocolchooseassignmentdialog.h"
#include "ui_protocolchooseassignmentdialog.h"

#include <QJsonObject>

/*
 * this dialog presents a given list of assignments to the user so they
 * can select one
 */
ProtocolChooseAssignmentDialog::ProtocolChooseAssignmentDialog(QList<ProtocolAssignment> assignments,
    QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ProtocolChooseAssignmentDialog)
{
    ui->setupUi(this);

    m_model = new QStandardItemModel(0, 3, ui->tableView);
    setHeaders(m_model);
    ui->tableView->setModel(m_model);

    connect(ui->tableView, SIGNAL(clicked(QModelIndex)), this, SLOT(onClickedTable(QModelIndex)));

    m_assignments = assignments;
    loadAssignments();

}

void ProtocolChooseAssignmentDialog::loadAssignments() {
    m_model->clear();

    int row = 0;
    for (ProtocolAssignment assignment: m_assignments) {
        QStandardItem * idItem = new QStandardItem();
        idItem->setData(assignment.id, Qt::DisplayRole);
        m_model->setItem(row, ID_COLUMN, idItem);

        QStandardItem * projectItem = new QStandardItem();
        projectItem->setData(assignment.project, Qt::DisplayRole);
        m_model->setItem(row, PROJECT_COLUMN, projectItem);

        QStandardItem * nameItem = new QStandardItem();
        nameItem->setData(assignment.name, Qt::DisplayRole);
        m_model->setItem(row, NAME_COLUMN, nameItem);

        QStandardItem * startedItem = new QStandardItem();
        startedItem->setData(assignment.start_date, Qt::DisplayRole);
        m_model->setItem(row, STARTED_COLUMN, startedItem);

        row++;
    }
    ui->tableView->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);

    // this is a guess; not sure which column will end up needing most space as of yet
    ui->tableView->horizontalHeader()->setSectionResizeMode(NAME_COLUMN, QHeaderView::Stretch);
}

void ProtocolChooseAssignmentDialog::setHeaders(QStandardItemModel *model) {
    model->setHorizontalHeaderItem(ID_COLUMN, new QStandardItem(QString("ID")));
    model->setHorizontalHeaderItem(PROJECT_COLUMN, new QStandardItem(QString("Project")));
    model->setHorizontalHeaderItem(NAME_COLUMN, new QStandardItem(QString("Name")));
    model->setHorizontalHeaderItem(STARTED_COLUMN, new QStandardItem(QString("Start date")));
}

void ProtocolChooseAssignmentDialog::onClickedTable(QModelIndex index) {
    m_selectedIndex = index;
}

ProtocolAssignment ProtocolChooseAssignmentDialog::getAssignment() {
    if (m_selectedIndex.isValid()) {
        return m_assignments[m_selectedIndex.row()];
    } else {
        QJsonObject empty;
        return ProtocolAssignment(empty);
    }
}

ProtocolChooseAssignmentDialog::~ProtocolChooseAssignmentDialog()
{
    delete ui;
}
