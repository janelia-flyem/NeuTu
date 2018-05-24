#include "taskbodyreview.h"

#include <iostream>
#include <stdlib.h>

#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QsLog.h>

TaskBodyReview::TaskBodyReview(QJsonObject json)
{
    if (json[KEY_TASKTYPE] != VALUE_TASKTYPE) {
        // wrong type, don't load the json
        return;
    }

    m_visibleBodies = QSet<uint64_t>();
    m_selectedBodies = QSet<uint64_t>();

    // I split the loading out for now
    loadJson(json);

}

// constants
const QString TaskBodyReview::KEY_TASKTYPE = "task type";
const QString TaskBodyReview::VALUE_TASKTYPE = "body review";
const QString TaskBodyReview::KEY_BODYID = "body ID";

QString TaskBodyReview::tasktype() {
    return VALUE_TASKTYPE;
}

QString TaskBodyReview::actionString() {
    return "Review body:";
}

QString TaskBodyReview::targetString() {
    return QString::number(m_bodyID);
}

QJsonObject TaskBodyReview::addToJson(QJsonObject taskJson) {
    // see note in loadJson() re: precision; because we
    //  know the source of the body IDs, the conversions
    //  below should be OK

    taskJson[KEY_BODYID] = static_cast<double>(m_bodyID);
    taskJson[KEY_TASKTYPE] = VALUE_TASKTYPE;

    return taskJson;
}

bool TaskBodyReview::loadSpecific(QJsonObject json) {

    if (!json.contains(KEY_BODYID)) {
        return false;
    }

    // see note on body IDs in base class loadStandard() method
    m_bodyID = json[KEY_BODYID].toDouble();
    if (m_bodyID == 0) {
        // 0 indicates a conversion failure; we don't
        //  anticipate reviewing body 0
        LINFO() << "error converting task json; body ID = 0 not allowed";
        return false;
    }
    if (m_bodyID > 4503599627370496) {
        // that number is 2^52
        LINFO() << "error converting task json; found body ID > 2^52";
        return false;
    }

    // if it's OK, put it in visible set:
    m_visibleBodies.insert(m_bodyID);

    return true;
}

