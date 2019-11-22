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
    void onCompleteButton();
    void onClickedTable(QModelIndex index);


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

    static const QString ASSIGNMENT_APPLICATION_NAME;

    QStandardItemModel * m_model;
    QSortFilterProxyModel * m_proxy;
    ProtocolAssignmentClient m_client;
    QString m_username;
    QList<ProtocolAssignment> m_assignments;

    bool checkForTokens();

    void loadAssignments();
    bool completeTask(ProtocolAssignmentTask task);
    bool completeAllTasks(ProtocolAssignment assignment);

    void updateAssignmentsTable();
    void clearAssignmentsTable();

    void updateSelectedInfo(ProtocolAssignment assignment);
    void clearSelectedInfo();

    void setHeaders(QStandardItemModel *model);

    void showEvent(QShowEvent * event);

    void showError(QString title, QString message);
    void showMessage(QString title, QString message);
};

#endif // PROTOCOLASSIGNMENTDIALOG_H
