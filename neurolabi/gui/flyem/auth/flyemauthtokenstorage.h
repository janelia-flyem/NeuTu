#ifndef FLYEMAUTHTOKENSTORAGE_H
#define FLYEMAUTHTOKENSTORAGE_H

#include <QString>
#include <QJsonObject>

class FlyEmAuthTokenStorage
{
public:
    FlyEmAuthTokenStorage();

    static const QString DEFAULT_APPLICATION;

    bool hasToken(QString server, QString application=DEFAULT_APPLICATION);
    QString getToken(QString server, QString application=DEFAULT_APPLICATION);
    void saveToken(QString token, QString server, QString application=DEFAULT_APPLICATION);


private:
    QJsonObject m_data;

    void loadTokensFile();
    void saveTokensFile();

};

#endif // FLYEMAUTHTOKENSTORAGE_H
