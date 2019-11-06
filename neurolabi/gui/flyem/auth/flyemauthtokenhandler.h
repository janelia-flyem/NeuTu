#ifndef FLYEMAUTHTOKENHANDLER_H
#define FLYEMAUTHTOKENHANDLER_H

#include <QString>

#include "flyemauthserverclient.h"
#include "flyemauthtokenstorage.h"

class FlyEmAuthTokenHandler
{
public:
    FlyEmAuthTokenHandler();

    static const QString MASTER_TOKEN_APPLICATION;

    QString getServer();
    void openLoginInBrowser();
    QString getLoginUrl();
    void openTokenInBrowser();
    QString getTokenUrl();

    bool hasMasterToken();
    QString getMasterToken();
    void saveMasterToken(QString token);

    bool hasApplicationToken(QString application);
    QString getApplicationToken(QString application);

    QStringList getApplications();

private:
    FlyEmAuthServerClient m_client;
    FlyEmAuthTokenStorage m_storage;

    void showError(QString title, QString message);
    void showMessage(QString title, QString message);
};

#endif // FLYEMAUTHTOKENHANDLER_H
