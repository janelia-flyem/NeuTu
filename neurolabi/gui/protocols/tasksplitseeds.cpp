#include "tasksplitseeds.h"

#include <iostream>
#include <stdlib.h>

#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QsLog.h>

TaskSplitSeeds::TaskSplitSeeds(QJsonObject json)
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
const QString TaskSplitSeeds::KEY_TASKTYPE = "task type";
const QString TaskSplitSeeds::VALUE_TASKTYPE = "split seeds";
const QString TaskSplitSeeds::KEY_BODYID = "body ID";

QString TaskSplitSeeds::tasktype() {
    return VALUE_TASKTYPE;
}

QString TaskSplitSeeds::actionString() {
    return "Add split seeds for body:";
}

QString TaskSplitSeeds::targetString() {
    return QString::number(m_bodyID);
}

QJsonObject TaskSplitSeeds::addToJson(QJsonObject taskJson) {
    // see note in loadJson() re: precision; because we
    //  know the source of the body IDs, the conversions
    //  below should be OK

    taskJson[KEY_BODYID] = static_cast<double>(m_bodyID);
    taskJson[KEY_TASKTYPE] = VALUE_TASKTYPE;

    return taskJson;
}

bool TaskSplitSeeds::loadSpecific(QJsonObject json) {

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
