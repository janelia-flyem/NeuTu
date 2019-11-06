#include "flyemauthtokenhandler.h"

#include <QJsonDocument>
#include <QJsonObject>

#include <QMessageBox>

/*
 * this class is the authentication token middleman; it handles all the tasks
 * involved in getting tokens and handing them around; it talks to the auth server
 * etc. so you don't have to
 *
 */
FlyEmAuthTokenHandler::FlyEmAuthTokenHandler()
{

    // the handler knows the server name (hardcoded right now, but will
    //  be retrieved from some kind of config later), but the client
    //  will do all of the actual talking to the auth server
    m_client.setServer(getServer());

}

const QString FlyEmAuthTokenHandler::MASTER_TOKEN_APPLICATION = "master";

bool FlyEmAuthTokenHandler::hasMasterToken() {
    return m_storage.hasToken(getServer(), MASTER_TOKEN_APPLICATION);
}

QString FlyEmAuthTokenHandler::getMasterToken() {
    if (hasMasterToken()) {
        return m_storage.getToken(getServer(), MASTER_TOKEN_APPLICATION);
    } else {
        // at this point I'd like to pop up the dialog, but that's just
        //  not going to be ready any time soon
        return "";
    }
}

void FlyEmAuthTokenHandler::saveMasterToken(QString token) {
    // input could either be a bare token or be in json form {"token": "asfalsjhfdajsf"}
    QString bareToken;
    QJsonDocument doc = QJsonDocument::fromJson(token.toUtf8());
    if (!doc.isNull()) {
        // it parsed == it's json; extract the token
        QJsonObject obj = doc.object();
        if (!obj.contains("token")) {
            showError("Token error!", "Couldn't parse token JSON");
            return;
        } else {
            bareToken = obj["token"].toString();
        }
    } else {
        // it didn't parse; it's just the bare token already
        bareToken = token;
    }
    m_storage.saveToken(bareToken, getServer(), MASTER_TOKEN_APPLICATION);
    if (m_storage.status() != FlyEmAuthTokenStorage::OK) {
        showError("Token error!", "Token could not be saved to NeuTu settings directory!");
    } else {
        showMessage("Token saved!", "Token has been saved to NeuTu settings directory.");
    }
}

// is the token present in storage?
bool FlyEmAuthTokenHandler::hasApplicationToken(QString application) {
    return m_storage.hasToken(getServer(), application);
}

// get the token either from storage or server
QString FlyEmAuthTokenHandler::getApplicationToken(QString application) {
    if (hasApplicationToken(application)) {
        return m_storage.getToken(getServer(), application);
    } else {
        // get from server
        QString token = m_client.getApplicationToken(getMasterToken(), application);

        // if it's empty, it's an error; otherwise, save it
        if (token.size() > 0) {
            m_storage.saveToken(token, getServer(), application);
        }

        return token;
    }
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

QStringList FlyEmAuthTokenHandler::getApplications() {
    QStringList result;

    // check that we have master token first!
    if (!hasMasterToken()) {
        // really would prefer to bring up the dialog box and let the user retrieve token
        return result;
    }
    result = m_client.getApplications(getMasterToken());
    return result;
}

void FlyEmAuthTokenHandler::showError(QString title, QString message) {
    QMessageBox mb;
    mb.setText(title);
    mb.setIcon(QMessageBox::Warning);
    mb.setInformativeText(message);
    mb.setStandardButtons(QMessageBox::Ok);
    mb.setDefaultButton(QMessageBox::Ok);
    mb.exec();
}

void FlyEmAuthTokenHandler::showMessage(QString title, QString message) {
    QMessageBox mb;
    mb.setText(title);
    mb.setInformativeText(message);
    mb.setStandardButtons(QMessageBox::Ok);
    mb.setDefaultButton(QMessageBox::Ok);
    mb.exec();
}

















