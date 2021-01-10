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

    // calls via browser
    void openLoginInBrowser();
    QString getLoginUrl();
    void openTokenInBrowser();
    QString getTokenUrl();

    // calls to server (which set had/last error calls)
    bool hadError();
    QString lastError();
    QString getApplicationToken(QString masterToken, QString application);
    QStringList getApplications(QString masterToken);

private:
    QString m_server;
    QNetworkAccessManager * m_networkManager;
    bool m_hadError;
    QString m_lastErrorMessage;

    QString getApplicationTokenUrl(QString application);
    QString getApplicationsUrl();

    QNetworkReply * get(QString url, QString token);
    QNetworkReply * post(QString url, QJsonObject data, QString token);
    QNetworkReply * call(QNetworkReply * reply);

};

#endif // FLYEMAUTHSERVERCLIENT_H
