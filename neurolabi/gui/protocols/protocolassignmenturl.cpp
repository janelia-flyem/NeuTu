#include "protocolassignmenturl.h"

#include <QMap>
#include <QMapIterator>

ProtocolAssignmentUrl::ProtocolAssignmentUrl()
{

}

QString ProtocolAssignmentUrl::GetProjects(QString server)
{
    return server + "/projects";
}

QString ProtocolAssignmentUrl::GetProjectsForProtocol(QString server, QString protocol)
{    
    QMap<QString, QString> filter;
    filter["protocol"] = protocol;
    return AddParameters(GetProjects(server), filter);
}

QString ProtocolAssignmentUrl::GenerateAssignment(QString server, QString projectName) {
    return server + "/assignment/" + projectName;
}

QString ProtocolAssignmentUrl::StartAssignment(QString server, int assignmentID) {
    return server + "/assignment/" + QString::number(assignmentID) + "/start";
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
