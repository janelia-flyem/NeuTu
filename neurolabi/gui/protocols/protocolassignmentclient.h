#ifndef PROTOCOLASSIGNMENTCLIENT_H
#define PROTOCOLASSIGNMENTCLIENT_H

#include <QObject>

#include <QNetworkReply>

#include <QJsonObject>


class ProtocolAssignmentClient : public QObject
{
    Q_OBJECT
public:
    explicit ProtocolAssignmentClient(QObject *parent = nullptr);

    enum AssigmentProtocols {
        ORPHAN_LINK
    };

    void setServer(QString server);
    QMap<QString, int> getProjectsForProtocol(AssigmentProtocols protocol);

    // QJsonObject generateAssignment(QString project);
    // QJsonObject startAssignment(QString assignmentormaybeid);


signals:

public slots:

private:


    QString m_server;
    QNetworkAccessManager * m_networkManager;
    QNetworkReply * m_datasetReply = nullptr;

    QJsonObject get(QString url);
    QJsonObject post(QString url, QJsonObject data);

    void showError(QString title, QString message);

};

#endif // PROTOCOLASSIGNMENTCLIENT_H
