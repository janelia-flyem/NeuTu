#ifndef TASKBODYREVIEW_H
#define TASKBODYREVIEW_H

#include <iostream>
#include <stdlib.h>

#include <QObject>
#include <QSet>

#include "protocols/taskprotocoltask.h"

class TaskBodyReview : public TaskProtocolTask
{
public:
    TaskBodyReview(QJsonObject json);
    QString tasktype();
    QString actionString();
    QString targetString();
    QSet<uint64_t> visibleBodies();
    QSet<uint64_t> selectedBodies();
    QJsonObject toJson();
    bool loadJson(QJsonObject json);

private:
    static const QString KEY_TASKTYPE;
    static const QString VALUE_TASKTYPE;
    static const QString KEY_COMPLETED;
    static const QString KEY_BODYID;
    static const QString KEY_VISIBLE;
    static const QString KEY_SELECTED;
    uint64_t m_bodyID;
    QSet<uint64_t> m_visibleBodies;
    QSet<uint64_t> m_selectedBodies;
};

#endif // TASKBODYREVIEW_H
