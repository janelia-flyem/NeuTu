#ifndef FLYEMAUTHSERVERCLIENT_H
#define FLYEMAUTHSERVERCLIENT_H

#include <QObject>

#include <QNetworkReply>

class FlyEmAuthServerClient : public QObject
{
    Q_OBJECT
public:
    FlyEmAuthServerClient();

    void setServer(QString server);
    QString getServer();

    void openLoginInBrowser();
    QString getLoginUrl();
    void openTokenInBrowser();
    QString getTokenUrl();

    QString getApplicationToken(QString masterToken, QString application);
    QStringList getApplications(QString masterToken);

private:
    QString m_server;
    QNetworkAccessManager * m_networkManager;

    QString getApplicationTokenUrl(QString application);
    QString getApplicationsUrl();

    QNetworkReply * get(QString url, QString token);
    QNetworkReply * post(QString url, QJsonObject data, QString token);
    QNetworkReply * call(QNetworkReply * reply);

};

#endif // FLYEMAUTHSERVERCLIENT_H
