#ifndef TASKBODYREVIEW_H
#define TASKBODYREVIEW_H

#include <iostream>
#include <stdlib.h>

#include <QObject>
#include <QSet>

#include "protocols/taskprotocoltask.h"

class ZFlyEmBody3dDoc;

class TaskBodyReview : public TaskProtocolTask
{

public:
    TaskBodyReview(QJsonObject json);

    // For use with TaskProtocolTaskFactory.
    static QString taskTypeStatic();
    static TaskBodyReview* createFromJson(QJsonObject json, ZFlyEmBody3dDoc *bodyDoc);

    QString taskType() const;
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

#endif // TASKBODYREVIEW_H
