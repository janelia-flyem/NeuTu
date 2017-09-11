#ifndef TaskSplitSeeds_H
#define TaskSplitSeeds_H

#include <iostream>
#include <stdlib.h>

#include <QObject>
#include <QSet>

#include "protocols/taskprotocoltask.h"

class TaskSplitSeeds : public TaskProtocolTask
{
public:
    TaskSplitSeeds(QJsonObject json);
    QString tasktype();
    QString actionString();
    QString targetString();

private:
    static const QString KEY_TASKTYPE;
    static const QString VALUE_TASKTYPE;
    static const QString KEY_BODYID;
    uint64_t m_bodyID;

    bool loadSpecific(QJsonObject json);
    QJsonObject addToJson(QJsonObject json);
};

#endif // TaskSplitSeeds_H
