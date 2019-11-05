#include "flyemauthtokenstorage.h"

#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>

#include "neutubeconfig.h"

/*
 * this class stores Fly EM authentication server tokens on disk,
 * so the user doesn't have to keep retrieving them
 *
 * token data dictionary: map servers to dictionaries with tokens for that server (master) and
 *  any applications it supports (eg, dvid, or assignment manager, or whatever)
 *  {"server name": {
 *      "master": "token",
 *      "application name": "token"
 *      }
 *  }
 *
 */
FlyEmAuthTokenStorage::FlyEmAuthTokenStorage()
{

    // the tokens file is small, so we'll read it into this object as soon as it's created
    loadTokensFile();


}

bool FlyEmAuthTokenStorage::hasToken(QString server, QString application) {
    // this is more for convenience anyway...there's no real difference in getting
    //  the token vs. checking if it's there
    QString result = getToken(server, application);
    return !result.isEmpty();
}

QString FlyEmAuthTokenStorage::getToken(QString server, QString application) {
    if (!m_data.contains(server)) {
        return "";
    }
    QJsonObject serverData = m_data[server].toObject();
    if (!serverData.contains(application)) {
        return "";
    } else {
        return serverData[application].toString();
    }
}

void FlyEmAuthTokenStorage::saveToken(QString token, QString server, QString application) {
    QJsonObject serverData;
    if (!m_data.contains(server)) {
        m_data[server] = serverData;
    } else {
        serverData = m_data[server].toObject();
    }
    serverData[application] = token;
    m_data[server] = serverData;
    saveTokensFile();
}

void FlyEmAuthTokenStorage::loadTokensFile() {
    QString authFilePath = QString::fromStdString(NeutubeConfig::getInstance().getPath(NeutubeConfig::EConfigItem::FLYEM_SERVICES_AUTH));
    QFile authFile(authFilePath);
    if (authFile.exists()) {
        // if file exists, read its data
        authFile.open(QIODevice::ReadOnly | QIODevice::Text);
        QString fileData = authFile.readAll();
        authFile.close();
        QJsonDocument doc = QJsonDocument::fromJson(fileData.toUtf8());
        if (doc.isNull()) {
            // error; not sure how I want to handle this; just return no data silently?

            // do that for now...

        } else {
            m_data = doc.object();
        }
    } else {
        // if file doesn't exist, save empty data to file
        saveTokensFile();
    }
}

void FlyEmAuthTokenStorage::saveTokensFile() {
    QString authFilePath = QString::fromStdString(NeutubeConfig::getInstance().getPath(NeutubeConfig::EConfigItem::FLYEM_SERVICES_AUTH));
    QFile authFile(authFilePath);
    if (!authFile.open(QIODevice::WriteOnly)) {


        // error handling

    }
    QJsonDocument jsonDoc(m_data);
    authFile.write(jsonDoc.toJson());
}
