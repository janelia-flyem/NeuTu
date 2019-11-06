#include "flyemauthserverclient.h"

#include <QDesktopServices>
#include <QEventLoop>

#include <QUrl>
#include <QNetworkRequest>
#include <QNetworkAccessManager>

#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>

/*
 * this class talks to the authentication server; for login
 * stuff, it opens your browser; for other stuff, it uses
 * the rest api
 */
FlyEmAuthServerClient::FlyEmAuthServerClient()
{
    m_networkManager = new QNetworkAccessManager(this);

}

QString FlyEmAuthServerClient::getServer() {
    return m_server;
}

void FlyEmAuthServerClient::setServer(QString server) {
    m_server = server;
}

QString FlyEmAuthServerClient::getLoginUrl() {
    return getServer() + "/login";
}

QString FlyEmAuthServerClient::getTokenUrl() {
    return getServer() + "/token";
}

void FlyEmAuthServerClient::openLoginInBrowser() {
    QDesktopServices::openUrl(QUrl(getLoginUrl()));
}

void FlyEmAuthServerClient::openTokenInBrowser() {
    QDesktopServices::openUrl(QUrl(getTokenUrl()));
}

QString FlyEmAuthServerClient::getApplicationsUrl() {
    return getServer() + "/api/applications";
}

QString FlyEmAuthServerClient::getApplicationTokenUrl(QString application) {
    return getServer() + "/api/token/" + application;
}

QStringList FlyEmAuthServerClient::getApplications(QString masterToken) {
    QStringList results;
    QNetworkReply * reply = get(getApplicationsUrl(), masterToken);
    if (reply->error() == QNetworkReply::NoError) {
        // reply is a json array
        QString stringReply = (QString) reply->readAll();
        QJsonDocument jsonResponse = QJsonDocument::fromJson(stringReply.toUtf8());
        QJsonArray array = jsonResponse.array();
        for (QJsonValue val: array) {
            results << val.toString();
        }
    }

    // note that error returns empty list

    return results;
}

QString FlyEmAuthServerClient::getApplicationToken(QString masterToken, QString application) {
    QNetworkReply * reply = get(getApplicationTokenUrl(application), masterToken);
    if (reply->error() == QNetworkReply::NoError) {
        // reply is a json dict
        QString stringReply = (QString) reply->readAll();
        QJsonDocument jsonResponse = QJsonDocument::fromJson(stringReply.toUtf8());
        QJsonObject object = jsonResponse.object();
        if (object.contains("token")) {
            // if it's an unknown app: {"error":"cannot find provided app"}
            // (not handled separately yet)


            return object["token"].toString();
        }
    }

    // error returns empty string
    return "";
}

QNetworkReply * FlyEmAuthServerClient::get(QString url, QString token) {
    qDebug() << "FlyEmAuthServerClient::get: url = " << url;

    QUrl requestUrl;
    requestUrl.setUrl(url);
    QNetworkRequest request = QNetworkRequest(requestUrl);
    QString authString = "Bearer " + token;
    request.setRawHeader(QByteArray("Authorization"), authString.toUtf8());
    QNetworkReply *reply = m_networkManager->get(request);
    return call(reply);
}

QNetworkReply *  FlyEmAuthServerClient::post(QString url, QJsonObject jsonData, QString token) {
    qDebug() << "FlyEmAuthServerClient::put: url = " << url;

    QUrl requestUrl;
    requestUrl.setUrl(url);
    QJsonDocument jsonDataDoc(jsonData);
    QByteArray byteData(jsonDataDoc.toJson());
    QNetworkRequest request = QNetworkRequest(requestUrl);
    QString authString = "Bearer " + token;
    request.setRawHeader(QByteArray("Authorization"), authString.toUtf8());
    QNetworkReply *reply = m_networkManager->post(request, byteData);
    return call(reply);
}

QNetworkReply * FlyEmAuthServerClient::call(QNetworkReply * reply) {

    // this is implemented synchronously because calls are short and
    //  the user has to wait for results anyway

    // Qt synchronous network call requires mini-event loop, ugh; from examples online:
    QEventLoop loop;
    connect(reply, SIGNAL(finished()), &loop, SLOT(quit()));
    connect(reply, SIGNAL(error(QNetworkReply::NetworkError)), &loop, SLOT(quit()));
    loop.exec();

    return reply;
}

