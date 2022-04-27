#ifndef TASKMULTIBODYREVIEW_H
#define TASKMULTIBODYREVIEW_H

#include <iostream>
#include <stdlib.h>

#include <QObject>
#include <QSet>
#include <QWidget>

#include "common/neutudefs.h"
#include "protocols/taskprotocoltask.h"

class ZFlyEmBody3dDoc;

class TaskMultiBodyReview : public TaskProtocolTask
{
    Q_OBJECT

public:
    TaskMultiBodyReview(QJsonObject json, ZFlyEmBody3dDoc * bodyDoc);

    // For use with TaskProtocolTaskFactory.
    static QString taskTypeStatic();
    static TaskMultiBodyReview* createFromJson(QJsonObject json, ZFlyEmBody3dDoc *bodyDoc);

    QString taskType() const override;
    QString actionString() override;
    QString targetString() override;

    QWidget * getTaskWidget();

private:
    static const QString KEY_TASKTYPE;
    static const QString VALUE_TASKTYPE;
    static const QString KEY_BODYIDS;

    bool loadSpecific(QJsonObject json) override;
    QJsonObject addToJson(QJsonObject json) override;

    void loadBodyData();
    void setupUI();

    ZFlyEmBody3dDoc * m_bodyDoc;
    QWidget *m_widget;
};

#endif // TASKBODYREVIEW_H
