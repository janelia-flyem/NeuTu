#include "taskmultibodyreview.h"

#include <iostream>
#include <stdlib.h>

#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>

#include "logging/zqslog.h"
#include "flyem/zflyembody3ddoc.h"
#include "flyem/zflyembodymanager.h"
#include "dvid/zdvidreader.h"

TaskMultiBodyReview::TaskMultiBodyReview(QJsonObject json, ZFlyEmBody3dDoc * bodyDoc)
{
    if (json[KEY_TASKTYPE] != VALUE_TASKTYPE) {
        // wrong type, don't load the json
        return;
    }

    m_bodyDoc = bodyDoc;

    m_visibleBodies = QSet<uint64_t>();
    m_selectedBodies = QSet<uint64_t>();

    loadJson(json);

    setupUI();

}

// constants
const QString TaskMultiBodyReview::KEY_TASKTYPE = "task type";
const QString TaskMultiBodyReview::VALUE_TASKTYPE = "multibody review";
const QString TaskMultiBodyReview::KEY_BODYIDS = "body IDs";

QString TaskMultiBodyReview::taskTypeStatic()
{
  return VALUE_TASKTYPE;
}

TaskMultiBodyReview* TaskMultiBodyReview::createFromJson(QJsonObject json, ZFlyEmBody3dDoc * bodyDoc)
{
  return new TaskMultiBodyReview(json, bodyDoc);
}

QString TaskMultiBodyReview::taskType() const
{
  return taskTypeStatic();
}

QString TaskMultiBodyReview::actionString() {
    return "Review bodies:";
}

QString TaskMultiBodyReview::targetString() {
    return "(multiple bodies)";
}

QJsonObject TaskMultiBodyReview::addToJson(QJsonObject taskJson) {
    taskJson[KEY_BODYIDS] = "(multiple bodies)";
    taskJson[KEY_TASKTYPE] = VALUE_TASKTYPE;

    return taskJson;
}

bool TaskMultiBodyReview::loadSpecific(QJsonObject /* json */ ) {
    // nothing specific to load for this protocol
    return true;
}

void TaskMultiBodyReview::loadBodyData() {
    ZDvidReader reader;
    reader.setVerbose(false);
    if (!reader.open(m_bodyDoc->getDvidTarget())) {
      LERROR() << "TaskMultiBodyReview::loadBodyData() could not open DVID target for reading";
      return;
    }

    // need to convert m_visibleBodies QSet into a std::vector here
    std::vector<uint64_t> bodyIDs(m_visibleBodies.count());
    for (uint64_t bodyID: m_visibleBodies) {
        bodyIDs.push_back(bodyID);
    }
    std::vector<ZJsonObject> bodyData = reader.readBodyAnnotationJsons(bodyIDs);


}

void TaskMultiBodyReview::setupUI() {
    m_widget = new QWidget();



}

QWidget * TaskMultiBodyReview::getTaskWidget() {
    return m_widget;
}
