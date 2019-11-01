#include "flyemauthtokenhandler.h"

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
