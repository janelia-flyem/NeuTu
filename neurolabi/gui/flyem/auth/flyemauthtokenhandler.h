#ifndef FLYEMAUTHTOKENHANDLER_H
#define FLYEMAUTHTOKENHANDLER_H

#include <QString>

#include "flyemauthserverclient.h"
#include "flyemauthtokenstorage.h"

class FlyEmAuthTokenHandler
{
public:
    FlyEmAuthTokenHandler();

    static const QString DEFAULT_APPLICATION;

    QString getServer();
    void openLoginInBrowser();
    QString getLoginUrl();
    void openTokenInBrowser();
    QString getTokenUrl();

    bool hasToken(QString application=DEFAULT_APPLICATION);
    QString getToken(QString application=DEFAULT_APPLICATION);
    void saveToken(QString token, QString application=DEFAULT_APPLICATION);

    QStringList getApplications();

private:
    FlyEmAuthServerClient m_client;
    FlyEmAuthTokenStorage m_storage;

};

#endif // FLYEMAUTHTOKENHANDLER_H
