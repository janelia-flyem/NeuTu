#include "protocolassignmenttask.h"

/*
 * this class is a convenient wrapper for task json data returned by
 * the assignment manager; all the actual functionality is
 * in the PAClient
 *
 * this class only contains a useful subset of the data fields!
 * many fields are omitted, especially those that can be found in
 * the task's assignment
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

}
