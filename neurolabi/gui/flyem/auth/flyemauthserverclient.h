#ifndef FLYEMAUTHSERVERCLIENT_H
#define FLYEMAUTHSERVERCLIENT_H

#include <QObject>

class FlyEmAuthServerClient
{
public:
    FlyEmAuthServerClient();

    void setServer(QString server);
    QString getServer();

    void openLoginInBrowser();
    QString getLoginUrl();
    void openTokenInBrowser();
    QString getTokenUrl();

private:
    QString m_server;


};

#endif // FLYEMAUTHSERVERCLIENT_H
