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
const QString TaskBodyReview::KEY_VISIBLE = "visible";
const QString TaskBodyReview::KEY_SELECTED= "selected";
const QString TaskBodyReview::KEY_COMPLETED = "completed";

QString TaskBodyReview::tasktype() {
    return VALUE_TASKTYPE;
}

QString TaskBodyReview::actionString() {
    return "Review body:";
}

QString TaskBodyReview::targetString() {
    return QString::number(m_bodyID);
}

uint64_t TaskBodyReview::bodyID() const
{
    return m_bodyID;
}

QSet<uint64_t> TaskBodyReview::visibleBodies() {
    QSet<uint64_t> bodies;
    bodies.unite(m_visibleBodies);
    bodies.insert(m_bodyID);
    return bodies;
}

QSet<uint64_t> TaskBodyReview::selectedBodies() {
    // note that we don't select the target body; it's probably
    //  clearer that way
    QSet<uint64_t> bodies;
    bodies.unite(m_selectedBodies);
    return bodies;
}

QJsonObject TaskBodyReview::toJson() {
    // see note in loadJson() re: precision; because we
    //  know the source of the body IDs, the conversions
    //  below should be OK

    QJsonObject taskJson;
    taskJson[KEY_BODYID] = static_cast<double>(m_bodyID);

    taskJson[KEY_COMPLETED] = m_completed;
    taskJson[KEY_TASKTYPE] = VALUE_TASKTYPE;

    if (m_visibleBodies.size() > 0) {
        QJsonArray array;
        foreach (uint64_t bodyID, m_visibleBodies) {
            array.append(static_cast<double>(bodyID));
        }
        taskJson[KEY_VISIBLE] = array;
    }
    if (m_selectedBodies.size() > 0) {
        QJsonArray array;
        foreach (uint64_t bodyID, m_selectedBodies) {
            array.append(static_cast<double>(bodyID));
        }
        taskJson[KEY_SELECTED] = array;
    }

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

    if (json.contains(KEY_COMPLETED)) {
        m_completed = json[KEY_COMPLETED].toBool();
    }

    m_visibleBodies.clear();
    if (json.contains(KEY_VISIBLE)) {
        foreach (QJsonValue value, json[KEY_VISIBLE].toArray()) {
            m_visibleBodies.insert(value.toDouble());
        }
    }

    m_selectedBodies.clear();
    if (json.contains(KEY_SELECTED)) {
        foreach (QJsonValue value, json[KEY_SELECTED].toArray()) {
            m_selectedBodies.insert(value.toDouble());
        }
    }

    // check all the body IDs at once:
    QSet<uint64_t> allBodies;
    allBodies.unite(m_visibleBodies);
    allBodies.unite(m_selectedBodies);
    allBodies.insert(m_bodyID);
    foreach (uint64_t bodyID, allBodies) {
        if (bodyID == 0) {
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
    }
    return true;
}

