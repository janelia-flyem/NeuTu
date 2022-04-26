#include "taskmultibodyreview.h"

#include <iostream>
#include <stdlib.h>

#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include "logging/zqslog.h"
#include "flyem/zflyembodymanager.h"

TaskMultiBodyReview::TaskMultiBodyReview(QJsonObject json)
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
const QString TaskMultiBodyReview::KEY_TASKTYPE = "task type";
const QString TaskMultiBodyReview::VALUE_TASKTYPE = "multibody review";
const QString TaskMultiBodyReview::KEY_BODYID = "body ID";
const QString TaskMultiBodyReview::KEY_SVID = "supervoxel ID";

QString TaskMultiBodyReview::taskTypeStatic()
{
  return VALUE_TASKTYPE;
}

TaskMultiBodyReview* TaskMultiBodyReview::createFromJson(QJsonObject json, ZFlyEmBody3dDoc *)
{
  return new TaskMultiBodyReview(json);
}

QString TaskMultiBodyReview::taskType() const
{
  return taskTypeStatic();
}

QString TaskMultiBodyReview::actionString() {
    return "Review body:";
}

uint64_t TaskMultiBodyReview::getEncodedBodyId() const
{
  return (m_bodyType == neutu::EBodyLabelType::SUPERVOXEL)
      ? ZFlyEmBodyManager::EncodeSupervoxel(m_bodyID)
      : m_bodyID;
}

QString TaskMultiBodyReview::targetString() {
    return QString::number(getEncodedBodyId());
}

QString TaskMultiBodyReview::getBodyKey() const
{
  return (m_bodyType == neutu::EBodyLabelType::SUPERVOXEL)
      ? KEY_SVID : KEY_BODYID;
}

QJsonObject TaskMultiBodyReview::addToJson(QJsonObject taskJson) {
    // see note in loadJson() re: precision; because we
    //  know the source of the body IDs, the conversions
    //  below should be OK

    taskJson[getBodyKey()] = static_cast<double>(m_bodyID);
    taskJson[KEY_TASKTYPE] = VALUE_TASKTYPE;

    return taskJson;
}

bool TaskMultiBodyReview::loadSpecific(QJsonObject json) {

    if (!json.contains(KEY_BODYID) && !json.contains(KEY_SVID)) {
        return false;
    }

    // see note on body IDs in base class loadStandard() method
    if (json.contains(KEY_SVID)) {
      m_bodyID = json[KEY_SVID].toDouble();
      m_bodyType = neutu::EBodyLabelType::SUPERVOXEL;
    } else {
      m_bodyID = json[KEY_BODYID].toDouble();
      m_bodyType = neutu::EBodyLabelType::BODY;
    }
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
    m_visibleBodies.insert(getEncodedBodyId());

    return true;
}

