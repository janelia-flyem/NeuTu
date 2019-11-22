#ifndef PROTOCOLASSIGNMENT_H
#define PROTOCOLASSIGNMENT_H

#include <QJsonObject>

class ProtocolAssignment
{
public:
    ProtocolAssignment(QJsonObject data);

    static const QString DISPOSITION_IN_PROGRESS;
    static const QString DISPOSITION_SKIPPED;
    static const QString DISPOSITION_COMPLETE;

    // all of these fields match names in the json
    int id;
    QString name;
    QString project;
    QString protocol;
    QString note;
    QString disposition;
    QString user;
    QString start_date;
    QString completion_date;
    QString duration;
    QString working_duration;
    QString create_date;

};

#endif // PROTOCOLASSIGNMENT_H
