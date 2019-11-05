#ifndef FLYEMAUTHTOKENHANDLER_H
#define FLYEMAUTHTOKENHANDLER_H

#include <QString>

#include "flyemauthserverclient.h"
#include "flyemauthtokenstorage.h"

class FlyEmAuthTokenHandler
{
public:
    FlyEmAuthTokenHandler();

    QString getServer();
    void openLoginInBrowser();
    QString getLoginUrl();
    void openTokenInBrowser();
    QString getTokenUrl();

    bool hasToken(QString application=FlyEmAuthTokenStorage::DEFAULT_APPLICATION);
    QString getToken(QString application=FlyEmAuthTokenStorage::DEFAULT_APPLICATION);
    void saveToken(QString token, QString application=FlyEmAuthTokenStorage::DEFAULT_APPLICATION);

private:
    FlyEmAuthServerClient m_client;
    FlyEmAuthTokenStorage m_storage;

};

#endif // FLYEMAUTHTOKENHANDLER_H
