#ifndef PROTOCOLASSIGNMENTDIALOG_H
#define PROTOCOLASSIGNMENTDIALOG_H

#include <QDialog>
#include <QStandardItemModel>
#include <QModelIndex>
#include <QSortFilterProxyModel>

#include "protocolassignmentclient.h"
#include "protocolassignment.h"

namespace Ui {
class ProtocolAssignmentDialog;
}

class ProtocolAssignmentDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ProtocolAssignmentDialog(QWidget *parent = nullptr);
    ~ProtocolAssignmentDialog();

private slots:
    void onRefreshButton();
    void onGetNewButton();
    void onStartButton();
    void onCompleteButton();
    void onClickedTable(QModelIndex index);
    void onClickedFilter();


private:
    Ui::ProtocolAssignmentDialog *ui;

    enum TableColumns {
        ID_COLUMN,
        DISPOSITION_COLUMN,
        PROJECT_COLUMN,
        PROTOCOL_COLUMN,
        NAME_COLUMN,
        STARTED_COLUMN
    };

    QStandardItemModel * m_model;
    QSortFilterProxyModel * m_proxy;
    ProtocolAssignmentClient m_client;
    bool m_clientServerSet;
    QString m_username;
    QList<ProtocolAssignment> m_assignments;
    int m_savedSelectedAssignmentID;

    void loadAssignments();
    bool completeTask(ProtocolAssignmentTask task);
    bool completeAllTasks(ProtocolAssignment assignment);

    void updateAssignmentsTable();
    void clearAssignmentsTable();

    void updateFilter();

    void updateSelectedInfo(ProtocolAssignment assignment);
    void clearSelectedInfo();

    void saveSelection();
    void restoreSelection();

    void setHeaders(QStandardItemModel *model);

    void showEvent(QShowEvent * event);

    void showError(QString title, QString message);
    void showMessage(QString title, QString message);
};

#endif // PROTOCOLASSIGNMENTDIALOG_H
