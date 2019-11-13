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

QString ProtocolAssignmentUrl::GetEligibleProjects(QString server) {
    return server + "/projects/eligible";
}

QString ProtocolAssignmentUrl::GenerateAssignment(QString server, QString projectName) {
    return server + "/assignment/" + projectName;
}

QString ProtocolAssignmentUrl::StartAssignment(QString server, int assignmentID) {
    return server + "/assignment/" + QString::number(assignmentID) + "/start";
}

/*
 * input: server and username; if username is empty, get URL for started assigments for all users
 */
QString ProtocolAssignmentUrl::GetStartedAssigments(QString server, QString username) {
    QString url = server + "/assignments_started";
    if (username.isEmpty()) {
        return url;
    } else {
        return AddParameter(url, "user", username);
    }
}

QString ProtocolAssignmentUrl::GetUsers(QString server) {
    return server + "/users";
}

/*
 * url for getting user data from Janelia username
 */
QString ProtocolAssignmentUrl::GetJaneliaUser(QString server, QString username) {
    QString url = GetUsers(server);
    return AddParameter(url, "janelia_id", username);
}

/*
 * add a single parameter=value to a url in place
 */
QString ProtocolAssignmentUrl::AddParameter(QString url, QString parameter, QString value) {
    if (!url.contains("?")) {
        url += "?";
    } else {
        url += "&";
    }
    url += parameter;
    url += "=";
    url += value;
    return url;
}

/*
 * add several parameter=value to a url
 */
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
