#include "taskbodyreview.h"

#include <iostream>
#include <stdlib.h>

#include <QJsonDocument>
#include <QJsonObject>
#include <QsLog.h>


TaskBodyReview::TaskBodyReview(QJsonObject json)
{
    if (json[KEY_TASKTYPE] != VALUE_TASKTYPE) {
        // wrong type, don't load the json
        return;
    }

    // I split the loading out for now
    loadJson(json);
}

// constants
const QString TaskBodyReview::KEY_TASKTYPE = "task type";
const QString TaskBodyReview::VALUE_TASKTYPE = "body review";
const QString TaskBodyReview::KEY_BODYID = "body ID";
const QString TaskBodyReview::KEY_COMPLETED = "completed";

QString TaskBodyReview::tasktype() {
    return VALUE_TASKTYPE;
}

uint64_t TaskBodyReview::bodyID() const
{
    return m_bodyID;
}

QJsonObject TaskBodyReview::toJson() {
    QJsonObject taskJson;

    // see note in loadJson() re: precision; because we
    //  know the source of the body ID, this conversion should be OK
    taskJson[KEY_BODYID] = static_cast<double>(m_bodyID);

    taskJson[KEY_COMPLETED] = m_completed;
    taskJson[KEY_TASKTYPE] = VALUE_TASKTYPE;
    return taskJson;
}

bool TaskBodyReview::loadJson(QJsonObject json) {

    if (!json.contains(KEY_BODYID)) {
        return false;
    }

    // this is a little iffy; our body IDs are 64-bit; json numbers
    //  are floats and technnically not limited; however, Qt only
    //  converts them to doubles; that means if we have a body ID
    //  above about 2^52, we won't be able to convert it without
    //  losing precision; that seems unlikely in the near future,
    //  so test and log, but don't handle it yet
    // alternate possibility is to store the body IDs as strings are parse,
    //  but that seems just as inelegant
    m_bodyID = json[KEY_BODYID].toDouble();
    if (m_bodyID == 0) {
        // 0 indicates a conversion failure; we don't
        //  anticipate reviewing body 0
        LINFO() << "error converting task json; 0 not allowed: " << objectToString(json);
        return false;
    }
    if (m_bodyID > 4503599627370496) {
        // that number is 2^52
        LINFO() << "error converting task json; can't handle body ID > 2^52 " << objectToString(json);
        return false;
    }

    if (json.contains(KEY_COMPLETED)) {
        m_completed = json[KEY_COMPLETED].toBool();
    }

    return true;
}

