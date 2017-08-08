#ifndef TASKBODYREVIEW_H
#define TASKBODYREVIEW_H

#include <QObject>

#include "protocols/taskprotocoltask.h"

class TaskBodyReview : public TaskProtocolTask
{
public:
    TaskBodyReview(QJsonObject json);
    uint64_t bodyID() const;
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
    uint64_t m_bodyID;
};

#endif // TASKBODYREVIEW_H
