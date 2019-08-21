#ifndef PROTOCOLASSIGNMENTURL_H
#define PROTOCOLASSIGNMENTURL_H

#include <QString>

class ProtocolAssignmentUrl
{
public:
    static QString GetProjects(QString server);
    static QString GetProjectsFiltered(QString server, QMap<QString, QString> filters);

private:
    ProtocolAssignmentUrl();

    static QString AddParameters(QString url, QMap<QString, QString>);

};

#endif // PROTOCOLASSIGNMENTURL_H
