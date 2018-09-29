#ifndef TaskTestTask_H
#define TaskTestTask_H

#include <iostream>
#include <stdlib.h>

#include <QObject>
#include <QSet>
#include <QWidget>

#include "protocols/taskprotocoltask.h"

class ZFlyEmBody3dDoc;

class TaskTestTask : public TaskProtocolTask
{
    Q_OBJECT

public:
    TaskTestTask(QJsonObject json);

    // For use with TaskProtocolTaskFactory.
    static QString taskTypeStatic();
    static TaskTestTask* createFromJson(QJsonObject json, ZFlyEmBody3dDoc *bodyDoc);

    QString taskType() const;
    QString actionString();
    QString targetString();
    QWidget * getTaskWidget();

private slots:
    void onYes();
    void onNo();

private:
    static const QString KEY_TASKTYPE;
    static const QString VALUE_TASKTYPE;
    static const QString KEY_BODYID;
    uint64_t m_bodyID;
    QWidget * m_widget;

    bool loadSpecific(QJsonObject json);
    QJsonObject addToJson(QJsonObject json);
};

#endif // TaskTestTask_H
