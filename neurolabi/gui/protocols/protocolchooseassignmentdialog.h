#ifndef PROTOCOLCHOOSEASSIGNMENTDIALOG_H
#define PROTOCOLCHOOSEASSIGNMENTDIALOG_H

#include <QDialog>
#include <QStandardItemModel>
#include <QModelIndex>

#include "protocolassignment.h"

namespace Ui {
class ProtocolChooseAssignmentDialog;
}

class ProtocolChooseAssignmentDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ProtocolChooseAssignmentDialog(QList<ProtocolAssignment> assignments,
        QWidget *parent = nullptr);
    ~ProtocolChooseAssignmentDialog();

    ProtocolAssignment getAssignment();

private slots:
    void onClickedTable(QModelIndex index);

private:
    Ui::ProtocolChooseAssignmentDialog *ui;

    enum TableColumns {
        ID_COLUMN,
        PROJECT_COLUMN,
        NAME_COLUMN,
        STARTED_COLUMN
    };


    QStandardItemModel * m_model;
    QModelIndex m_selectedIndex;
    QList<ProtocolAssignment> m_assignments;

    void loadAssignments();
    void setHeaders(QStandardItemModel *model);

};

#endif // PROTOCOLCHOOSEASSIGNMENTDIALOG_H
