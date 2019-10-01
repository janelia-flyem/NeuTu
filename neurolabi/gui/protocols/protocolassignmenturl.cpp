#include "protocolassignmenturl.h"

#include <QMap>
#include <QMapIterator>

QString ProtocolAssignmentUrl::GetProjects(QString server)
{
    if (!server.endsWith('/')) {
        server += '/';
    }
    return server + "projects";
}

QString ProtocolAssignmentUrl::GetProjectsForProtocol(QString server, QString protocol)
{    
    QMap<QString, QString> filter;
    filter["protocol"] = protocol;
    return AddParameters(GetProjects(server), filter);
}

ProtocolAssignmentUrl::ProtocolAssignmentUrl()
{

}

QString ProtocolAssignmentUrl::AddParameters(QString url, QMap<QString, QString> params)
{
    if (params.size() == 0) {
        return url;
    }

    if (!url.contains("?")) {
        url += "?";
    } else {
        url += "&";
    }

    QStringList paramList;
    QMapIterator<QString, QString> iter(params);
    while (iter.hasNext()) {
        iter.next();
        QString temp;
        temp = iter.key() + "=" + iter.value();
        paramList << temp;
    }
    url += paramList.join("&");

    return url;
}
