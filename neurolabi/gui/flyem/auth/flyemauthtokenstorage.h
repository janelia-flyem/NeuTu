#ifndef FLYEMAUTHTOKENSTORAGE_H
#define FLYEMAUTHTOKENSTORAGE_H

#include <QString>
#include <QJsonObject>

class FlyEmAuthTokenStorage
{
public:
    FlyEmAuthTokenStorage();

    bool hasToken(QString server, QString application);
    QString getToken(QString server, QString application);
    void saveToken(QString token, QString server, QString application);


private:
    QJsonObject m_data;

    void loadTokensFile();
    void saveTokensFile();

};

#endif // FLYEMAUTHTOKENSTORAGE_H
