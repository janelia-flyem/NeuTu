#include "protocolassignmenturl.h"

#include <QMapIterator>

QString ProtocolAssignmentUrl::GetProjects(QString server)
{

}

QString ProtocolAssignmentUrl::GetProjectsFiltered(QString server, QMap<QString, QString> filters)
{

}

ProtocolAssignmentUrl::ProtocolAssignmentUrl()
{

}

QString ProtocolAssignmentUrl::AddParameters(QString url, QMap<QString, QString> params)
{
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
