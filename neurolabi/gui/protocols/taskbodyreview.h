#ifndef TASKBODYREVIEW_H
#define TASKBODYREVIEW_H

#include <iostream>
#include <stdlib.h>

#include <QObject>
#include <QSet>
#include <QWidget>

#include "protocols/taskprotocoltask.h"

class TaskBodyReview : public TaskProtocolTask
{
    Q_OBJECT

public:
    TaskBodyReview(QJsonObject json);
    QString tasktype();
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

#endif // TASKBODYREVIEW_H
