#include "protocolassignment.h"

/*
 * this class is just a convenient wrapper for the json data; all the
 * actual functionality will be in the PAClient
 */
ProtocolAssignment::ProtocolAssignment(QJsonObject data)
{

    /* this would be more elegant in Python, but C++ apparently
     * doesn't have any kind of useful introspection */
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

    if (data.contains("project")) {
        project = data["project"].toString();
    } else {
        project = "";
    }

    if (data.contains("protocol")) {
        protocol = data["protocol"].toString();
    } else {
        protocol = "";
    }

    if (data.contains("note")) {
        note = data["note"].toString();
    } else {
        note = "";
    }

    if (data.contains("disposition")) {
        disposition = data["disposition"].toString();
    } else {
        disposition = "";
    }

    if (data.contains("user")) {
        user = data["user"].toString();
    } else {
        user = "";
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

    if (data.contains("duration")) {
        duration = data["duration"].toString();
    } else {
        duration = "";
    }

    if (data.contains("working_duration")) {
        working_duration = data["working_duration"].toString();
    } else {
        working_duration = "";
    }

    if (data.contains("create_date")) {
        create_date = data["create_date"].toString();
    } else {
        create_date = "";
    }

}
