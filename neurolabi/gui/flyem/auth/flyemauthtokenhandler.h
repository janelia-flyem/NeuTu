#ifndef FLYEMAUTHTOKENHANDLER_H
#define FLYEMAUTHTOKENHANDLER_H

#include <QString>

class FlyEmAuthTokenHandler
{
public:
    FlyEmAuthTokenHandler();

    QString getServer();

private:
    static const QString AUTH_SERVER;

};

#endif // FLYEMAUTHTOKENHANDLER_H
