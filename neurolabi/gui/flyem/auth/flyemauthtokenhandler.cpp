#include "flyemauthtokenhandler.h"

#include <QJsonDocument>
#include <QJsonObject>

/*
 * this class is the authentication token middleman; it handles all the tasks
 * involved in getting tokens and handing them around; it talks to the auth server
 * etc. so you don't have to
 */
FlyEmAuthTokenHandler::FlyEmAuthTokenHandler()
{

    // the handler knows the server name (hardcoded right now, but will
    //  be retrieved from some kind of config later), but the client
    //  will do all of the actual talking to the auth server
    m_client.setServer(getServer());

}

const QString FlyEmAuthTokenHandler::DEFAULT_APPLICATION = "master";

bool FlyEmAuthTokenHandler::hasToken(QString application) {
    return m_storage.hasToken(getServer(), application);
}

QString FlyEmAuthTokenHandler::getToken(QString application) {
    return m_storage.getToken(getServer(), application);
}

void FlyEmAuthTokenHandler::saveToken(QString token, QString application) {
    // input could either be a bare token or be in json form {"token": "asfalsjhfdajsf"}
    QString bareToken;
    QJsonDocument doc = QJsonDocument::fromJson(token.toUtf8());
    if (!doc.isNull()) {
        // it parsed == it's json; extract the token
        QJsonObject obj = doc.object();
        if (!obj.contains("token")) {


            // error


            return;



        } else {
            bareToken = obj["token"].toString();
        }
    } else {
        // it didn't parse; it's just the bare token already
        bareToken = token;
    }
    m_storage.saveToken(bareToken, getServer(), application);
}

QString FlyEmAuthTokenHandler::getServer() {
    // hard coding this for now
    return "https://emdata1.int.janelia.org:15000";
}

QString FlyEmAuthTokenHandler::getLoginUrl() {
    return m_client.getLoginUrl();
}

QString FlyEmAuthTokenHandler::getTokenUrl() {
    return m_client.getTokenUrl();
}

void FlyEmAuthTokenHandler::openLoginInBrowser() {
    m_client.openLoginInBrowser();
}

void FlyEmAuthTokenHandler::openTokenInBrowser() {
    m_client.openTokenInBrowser();
}
