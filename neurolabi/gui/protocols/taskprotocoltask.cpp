#include "taskprotocoltask.h"



/*
 * this is the abstract base class for tasks used by the TaskProtocolWindow in Neu3
 *
 */
TaskProtocolTask::TaskProtocolTask()
{

    m_completed = false;

}

bool TaskProtocolTask::completed() const
{
    return m_completed;
}

void TaskProtocolTask::setCompleted(bool completed)
{
    m_completed = completed;
}

/*
 * utility; when having trouble with input json, nice
 * to embed that json in the error
 */
QString TaskProtocolTask::objectToString(QJsonObject json) {
    QJsonDocument doc(json);
    QString jsonString(doc.toJson(QJsonDocument::Compact));
    return jsonString;
}

