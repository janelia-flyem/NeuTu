#ifndef ORPHANLINKPROTOCOL_H
#define ORPHANLINKPROTOCOL_H

#include <QDialog>
#include <QStandardItemModel>
#include <QModelIndex>
#include <QSortFilterProxyModel>

#include "protocoldialog.h"
#include "protocolassignmentclient.h"
#include "protocolassignmenttask.h"

#include "zjsonobject.h"

namespace Ui {
class OrphanLinkProtocol;
}

class OrphanLinkProtocol : public ProtocolDialog
{
    Q_OBJECT

public:
    explicit OrphanLinkProtocol(QWidget *parent = 0);
    ~OrphanLinkProtocol();
    bool initialize();

signals:
    void protocolCompleting();
    void protocolExiting();
    void requestSaveProtocol(ZJsonObject data);

public slots:
    void loadDataRequested(ZJsonObject data);

private slots:
    void onExitButton();
    void onCompleteButton();
    void onCommentButton();
    void onClickedTable(QModelIndex index);

private:
    static const std::string KEY_VERSION;
    static const std::string KEY_ASSIGNMENT_ID;
    static const std::string KEY_COMMENTS;
    static const QString TASK_KEY_BODY_ID;

    static const int fileVersion;

    enum TableColumns {
        TASK_ID_COLUMN,
        BODY_ID_COLUMN,
        STATUS_COLUMN,
        COMMENT_COLUMN
    };

    Ui::OrphanLinkProtocol *ui;
    QStandardItemModel * m_model;
    QSortFilterProxyModel * m_proxy;
    ProtocolAssignmentClient m_client;

    int m_assignmentID;
    QMap<int, QString> m_comments;
    QList<ProtocolAssignmentTask> m_tasks;

    void saveState();
    void showError(QString title, QString message);

    void loadTasks();
    static bool compareTasks(const ProtocolAssignmentTask task1, const ProtocolAssignmentTask task2);

    bool hasSelection();
    ProtocolAssignmentTask getSelectedTask();

    void setHeaders(QStandardItemModel *model);
    void updateTable();
    void updateLabels();
    void updateCurrentBodyLabel();
    void updateProgressLabel();

};

#endif // ORPHANLINKPROTOCOL_H
