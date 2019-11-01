#ifndef FLYEMAUTHTOKENHANDLER_H
#define FLYEMAUTHTOKENHANDLER_H

#include <QString>

#include "flyemauthserverclient.h"

class FlyEmAuthTokenHandler
{
public:
    FlyEmAuthTokenHandler();

    QString getServer();
    void openLoginInBrowser();
    QString getLoginUrl();
    void openTokenInBrowser();
    QString getTokenUrl();

private:
    FlyEmAuthServerClient m_client;

};

#endif // FLYEMAUTHTOKENHANDLER_H
