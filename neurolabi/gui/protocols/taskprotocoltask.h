#ifndef TASKPROTOCOLTASK_H
#define TASKPROTOCOLTASK_H

#include <QObject>
#include <QJsonDocument>
#include <QJsonObject>

class TaskProtocolTask
{
public:
    TaskProtocolTask();

    bool completed() const;
    void setCompleted(bool completed);

    virtual QString tasktype() = 0;
    virtual QJsonObject toJson() = 0;
    virtual bool loadJson(QJsonObject json) = 0;

protected:
    bool m_completed;
    QString objectToString(QJsonObject json);
};

#endif // TASKPROTOCOLTASK_H
