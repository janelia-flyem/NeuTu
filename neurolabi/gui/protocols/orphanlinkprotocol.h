#ifndef ORPHANLINKPROTOCOL_H
#define ORPHANLINKPROTOCOL_H

#include <QDialog>

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

private:
    static const std::string KEY_VERSION;
    static const std::string KEY_ASSIGNMENT_ID;
    static const std::string KEY_COMMENTS;

    static const int fileVersion;

    Ui::OrphanLinkProtocol *ui;
    ProtocolAssignmentClient m_client;

    int m_assignmentID;
    QMap<int, QString> m_comments;
    QList<ProtocolAssignmentTask> m_tasks;

    void saveState();
    void showError(QString title, QString message);

    void loadTasks();

    void updateTable();
    void updateLabels();
    void updateCurrentLabel();
    void updateProgressLabel();

};

#endif // ORPHANLINKPROTOCOL_H
