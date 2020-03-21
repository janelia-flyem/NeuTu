#include "protocolassignmenttask.h"

/*
 * this class is a convenient wrapper for task json data returned by
 * the assignment manager; all the actual functionality is
 * in the PAClient
 *
 * this class only provides a useful subset of the data fields
 * as class members; the rest, many of which may vary depending on
 * the protocol, must be accessed from the data json object
 */
ProtocolAssignmentTask::ProtocolAssignmentTask(QJsonObject data)
{

    if (data.contains("id")) {
        id = data["id"].toInt();
    } else {
        id = -1;
    }

    if (data.contains("name")) {
        name = data["name"].toString();
    } else {
        name = "";
    }

    if (data.contains("disposition")) {
        disposition = data["disposition"].toString();
    } else {
        disposition = "";
    }

    if (data.contains("start_date")) {
        start_date = data["start_date"].toString();
    } else {
        start_date = "";
    }

    if (data.contains("completion_date")) {
        completion_date = data["completion_date"].toString();
    } else {
        completion_date = "";
    }

    // keep all the data
    m_data = data;
}

bool ProtocolAssignmentTask::has(QString key) {
    return m_data.contains(key);
}

QJsonValue ProtocolAssignmentTask::get(QString key) {
    return m_data[key];
}

const QString ProtocolAssignmentTask::DISPOSITION_SKIPPED = "Skipped";
const QString ProtocolAssignmentTask::DISPOSITION_IN_PROGRESS = "In progress";
const QString ProtocolAssignmentTask::DISPOSITION_COMPLETE = "Complete";
