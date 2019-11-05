#include "flyemauthserverclient.h"

#include <QDesktopServices>
#include <QUrl>

/*
 * this class talks to the authentication server; for login
 * stuff, it opens your browser; for other stuff, it uses
 * the rest api
 */
FlyEmAuthServerClient::FlyEmAuthServerClient()
{

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
