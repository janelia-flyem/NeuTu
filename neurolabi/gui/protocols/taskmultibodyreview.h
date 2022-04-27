#ifndef TASKMULTIBODYREVIEW_H
#define TASKMULTIBODYREVIEW_H

#include <iostream>
#include <stdlib.h>

#include <QObject>
#include <QSet>

#include "common/neutudefs.h"
#include "protocols/taskprotocoltask.h"

class ZFlyEmBody3dDoc;

class TaskMultiBodyReview : public TaskProtocolTask
{

public:
    TaskMultiBodyReview(QJsonObject json);

    // For use with TaskProtocolTaskFactory.
    static QString taskTypeStatic();
    static TaskMultiBodyReview* createFromJson(QJsonObject json, ZFlyEmBody3dDoc *bodyDoc);

    QString taskType() const override;
    QString actionString() override;
    QString targetString() override;

private:
    static const QString KEY_TASKTYPE;
    static const QString VALUE_TASKTYPE;
    static const QString KEY_BODYIDS;

    bool loadSpecific(QJsonObject json) override;
    QJsonObject addToJson(QJsonObject json) override;
};

#endif // TASKBODYREVIEW_H
