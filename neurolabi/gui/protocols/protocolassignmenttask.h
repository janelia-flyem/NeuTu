#ifndef PROTOCOLASSIGNMENTTASK_H
#define PROTOCOLASSIGNMENTTASK_H

#include <QJsonObject>

class ProtocolAssignmentTask
{
public:
    ProtocolAssignmentTask(QJsonObject data);

    static const QString DISPOSITION_IN_PROGRESS;
    static const QString DISPOSITION_SKIPPED;
    static const QString DISPOSITION_COMPLETE;

    // all of the field names match the json names
    int id;
    QString name;
    QString disposition;
    QString start_date;
    QString completion_date;
};

#endif // PROTOCOLASSIGNMENTTASK_H
