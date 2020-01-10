#ifndef PROTOCOLASSIGNMENTCLIENT_H
#define PROTOCOLASSIGNMENTCLIENT_H

#include <QObject>

#include <QNetworkReply>

#include <QJsonArray>
#include <QJsonObject>

#include "protocolassignment.h"
#include "protocolassignmenttask.h"

class ProtocolAssignmentClient : public QObject
{
    Q_OBJECT
public:
    explicit ProtocolAssignmentClient(QObject *parent = nullptr);

    enum AssigmentProtocols {
        ORPHAN_LINK
    };

    void setServer(QString server);
    void setToken(QString token);

    QMap<QString, int> getProjectsForProtocol(AssigmentProtocols protocol);
    QMap<QString, QString> getEligibleProjects();

    ProtocolAssignment getAssignment(int assignmentID);
    QList<ProtocolAssignment> getAssignments();
    QList<ProtocolAssignment> getStartedAssignments();
    bool completeAssignment(ProtocolAssignment assignment);

    int generateAssignment(QString projectName);
    bool startAssignment(int assignmentID);

    QList<ProtocolAssignmentTask> getAssignmentTasks(ProtocolAssignment assignment);
    bool startTask(ProtocolAssignmentTask task);
    bool completeTask(ProtocolAssignmentTask task);

    QString getLocalUsername(QString janeliaUsername);

signals:

public slots:

private:
    QString m_server;
    QString m_token;
    QNetworkAccessManager * m_networkManager;
    QNetworkReply * m_datasetReply = nullptr;

    QNetworkReply * get(QString url);
    QNetworkReply * post(QString url, QJsonObject data);
    QNetworkReply * call(QNetworkReply * reply);

    bool hadError(QNetworkReply * reply);
    QString getErrorString(QNetworkReply * reply);
    int getReplyStatus(QNetworkReply * reply);
    QJsonObject getReplyJSON(QNetworkReply * reply);
    QJsonObject getReplyJsonObject(QNetworkReply * reply, QString key);
    QJsonArray getReplyJsonArray(QNetworkReply * reply, QString key);

    void showError(QString title, QString message);

};

#endif // PROTOCOLASSIGNMENTCLIENT_H
