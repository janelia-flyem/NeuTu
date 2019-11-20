#ifndef PROTOCOLASSIGNMENTCLIENT_H
#define PROTOCOLASSIGNMENTCLIENT_H

#include <QObject>

#include <QNetworkReply>

#include <QJsonArray>
#include <QJsonObject>

#include "protocolassignment.h"

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
    QStringList getEligibleProjects();

    QList<ProtocolAssignment> getStartedAssignments();
    int generateAssignment(QString projectName);
    bool startAssignment(int assignmentID);

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
    QJsonObject getReplyDataObject(QNetworkReply * reply);
    QJsonArray getReplyDataArray(QNetworkReply * reply);

    void showError(QString title, QString message);

};

#endif // PROTOCOLASSIGNMENTCLIENT_H
