#ifndef FLYEMAUTHTOKENSTORAGE_H
#define FLYEMAUTHTOKENSTORAGE_H

#include <QString>
#include <QJsonObject>

class FlyEmAuthTokenStorage
{
public:
    FlyEmAuthTokenStorage();

    enum StorageStatus {
        OK,
        NOT_LOADED,
        NOT_SAVED
    };

    StorageStatus status();
    bool hasToken(QString server, QString application);
    QString getToken(QString server, QString application);
    void saveToken(QString token, QString server, QString application);
    void clearTokens(QString server);


private:
    QJsonObject m_data;
    StorageStatus m_status;

    void loadTokensFile();
    void saveTokensFile();

};

#endif // FLYEMAUTHTOKENSTORAGE_H
