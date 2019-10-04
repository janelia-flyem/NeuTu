#ifndef PROTOCOLASSIGNMENTURL_H
#define PROTOCOLASSIGNMENTURL_H

#include <QString>

class ProtocolAssignmentUrl
{
public:
    static QString GetProjects(QString server);
    static QString GetProjectsForProtocol(QString server, QString protocol);

    static QString GenerateAssignment(QString server, QString projectName);
    static QString StartAssignment(QString server, int assignmentID);

private:
    ProtocolAssignmentUrl();

    static QString AddParameters(QString url, QMap<QString, QString>);

};

#endif // PROTOCOLASSIGNMENTURL_H
