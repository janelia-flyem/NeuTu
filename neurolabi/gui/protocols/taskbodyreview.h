#ifndef TASKBODYREVIEW_H
#define TASKBODYREVIEW_H

#include <iostream>
#include <stdlib.h>

#include <QObject>
#include <QSet>

#include "common/neutudefs.h"
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
    uint64_t getEncodedBodyId() const;
    QString getBodyKey() const;

private:
    static const QString KEY_TASKTYPE;
    static const QString VALUE_TASKTYPE;
    static const QString KEY_BODYID;
    static const QString KEY_SVID;
    uint64_t m_bodyID = 0;
    neutu::EBodyLabelType m_bodyType = neutu::EBodyLabelType::BODY;

    bool loadSpecific(QJsonObject json);
    QJsonObject addToJson(QJsonObject json);
};

#endif // TASKBODYREVIEW_H
