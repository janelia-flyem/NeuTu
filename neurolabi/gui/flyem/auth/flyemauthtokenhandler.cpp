#include "flyemauthtokenhandler.h"

FlyEmAuthTokenHandler::FlyEmAuthTokenHandler()
{



}

// hard coding this for now
const QString FlyEmAuthTokenHandler::AUTH_SERVER = "https://emdata1.int.janelia.org:15000";

QString FlyEmAuthTokenHandler::getServer() {
    return AUTH_SERVER;
}
