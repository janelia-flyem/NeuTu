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

const QString FlyEmAuthTokenHandler::DEFAULT_APPLICATION = "master";

// is the token present in storage?
bool FlyEmAuthTokenHandler::hasToken(QString application) {
    return m_storage.hasToken(getServer(), application);
}

// get the token either from storage or server
QString FlyEmAuthTokenHandler::getToken(QString application) {

    if (application == DEFAULT_APPLICATION) {
        if (hasToken()) {
            return m_storage.getToken(getServer(), application);
        } else {
            // if we don't have it, would like to pop up dialog, but not now
            return "";
        }
    } else {
        if (hasToken(application)) {
            return m_storage.getToken(getServer(), application);
        } else {
            // get from server
            QString token = m_client.getApplicationToken(getToken(), application);

            // if it's empty, it's an error; otherwise, save it
            if (token.size() > 0) {
                m_storage.saveToken(token, getServer(), application);
            }

            return token;
        }

    }
}

void FlyEmAuthTokenHandler::saveToken(QString token, QString application) {
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
    m_storage.saveToken(bareToken, getServer(), application);
    if (m_storage.status() != FlyEmAuthTokenStorage::OK) {
        showError("Token error!", "Token could not be saved to NeuTu settings directory!");
    } else {
        showMessage("Token saved!", "Token has been saved to NeuTu settings directory.");
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
    if (!hasToken()) {
        // really would prefer to bring up the dialog box and let the user retrieve token
        return result;
    }
    result = m_client.getApplications(getToken());
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

















