#include "tasksplitseeds.h"

#include <iostream>
#include <stdlib.h>

#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>

#include <QCheckBox>
#include <QHBoxLayout>
#include <QLabel>

#include "logging/zqslog.h"
#include "flyem/zflyembody3ddoc.h"

TaskSplitSeeds::TaskSplitSeeds(QJsonObject json, ZFlyEmBody3dDoc * bodyDoc)
{
    if (json[KEY_TASKTYPE] != VALUE_TASKTYPE) {
        // wrong type, don't load the json
        return;
    }

    m_bodyDoc = bodyDoc;

    m_visibleBodies = QSet<uint64_t>();
    m_selectedBodies = QSet<uint64_t>();

    // set up this task's custom UI
    // currently it only has a checkbox, right justified;
    //  I expect to add a label at left (I'm planning a
    //  message re: auto-save seeds once it's in)

    m_widget = new QWidget();
    QHBoxLayout * layout = new QHBoxLayout;
    layout->setContentsMargins(0, 0, 0, 0);

    // QLabel * label = new QLabel("Split seeds saved automatically on complete");
    // layout->addWidget(label);

    layout->addItem(new QSpacerItem(0, 10, QSizePolicy::Expanding));

    // one could argue that clicking the complete box without adding
    //  seeds is enough to indicate that it doesn't need splitting; but
    //  it's expected to be rare, and Steve requested it; it's not that
    //  bad to have a specific affirmative step in that case
    m_noSplitCheck = new QCheckBox("No split needed");
    layout->addWidget(m_noSplitCheck);

    m_widget->setLayout(layout);
    m_widget->setVisible(false);

    connect (m_noSplitCheck, SIGNAL(stateChanged(int)), this, SLOT(onNoSplitStateChanged(int)));

    connect (this, SIGNAL(saveSplitTask()), m_bodyDoc, SLOT(saveSplitTask()));

    // I split the loading out for now
    loadJson(json);
}

// constants
const QString TaskSplitSeeds::KEY_TASKTYPE = "task type";
const QString TaskSplitSeeds::VALUE_TASKTYPE = "split seeds";
const QString TaskSplitSeeds::KEY_BODYID = "body ID";
const QString TaskSplitSeeds::TAG_SEEDS_ADDED = "seeds added";

QString TaskSplitSeeds::taskTypeStatic()
{
  return VALUE_TASKTYPE;
}

TaskSplitSeeds* TaskSplitSeeds::createFromJson(QJsonObject json, ZFlyEmBody3dDoc *bodyDoc)
{
  return new TaskSplitSeeds(json, bodyDoc);
}

QString TaskSplitSeeds::taskType() const
{
  return taskTypeStatic();
}

QString TaskSplitSeeds::actionString() {
    return "Add split seeds for body:";
}

QString TaskSplitSeeds::targetString() {
    return QString::number(m_bodyID);
}

void TaskSplitSeeds::onNoSplitStateChanged(int /*state*/) {
    updateSeedsTag();
}

void TaskSplitSeeds::onCompleted() {
    // be sure the tag is right; there's one sequence by which it won't
    //  be set until here
    updateSeedsTag();

    // if there are seeds, be sure they are saved
    if (hasTag(TAG_SEEDS_ADDED)) {
        emit saveSplitTask();
    }
}

void TaskSplitSeeds::beforeNext() {
    // save seeds before the user moves to another task
    updateSeedsTag();
    if (hasTag(TAG_SEEDS_ADDED)) {
        emit saveSplitTask();
    }
}

void TaskSplitSeeds::beforePrev() {
    // save seeds before the user moves to another task
    updateSeedsTag();
    if (hasTag(TAG_SEEDS_ADDED)) {
        emit saveSplitTask();
    }
}

void TaskSplitSeeds::updateSeedsTag() {
    if (m_noSplitCheck->isChecked()) {
        removeTag(TAG_SEEDS_ADDED);
    } else {
        addTag(TAG_SEEDS_ADDED);
    }
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


    // update the "no split" check box, but only on completed tasks; you
    //  don't want an uncompleted task already having that box checked
    if (completed()) {
        if (hasTag(TAG_SEEDS_ADDED)) {
            m_noSplitCheck->setChecked(false);
        } else {
            m_noSplitCheck->setChecked(true);
        }
    }

    return true;
}

QWidget * TaskSplitSeeds::getTaskWidget() {
    return m_widget;
}
