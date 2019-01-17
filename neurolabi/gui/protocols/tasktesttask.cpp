#include "tasktesttask.h"

#include <iostream>
#include <stdlib.h>

#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>

#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>

#include "logging/zqslog.h"

TaskTestTask::TaskTestTask(QJsonObject json)
{
    if (json[KEY_TASKTYPE] != VALUE_TASKTYPE) {
        // wrong type, don't load the json
        return;
    }

    m_visibleBodies = QSet<uint64_t>();
    m_selectedBodies = QSet<uint64_t>();

    // I split the loading out for now
    loadJson(json);

    // testing: set up UI


    // for testing, be sloppy;

    m_widget = new QWidget();
    QHBoxLayout * layout = new QHBoxLayout;
    layout->setContentsMargins(0, 0, 0, 0);

    QLabel * label = new QLabel("Body " + QString::number(m_bodyID) + ":", m_widget);
    layout->addWidget(label);

    layout->addItem(new QSpacerItem(0, 10, QSizePolicy::Expanding));

    QPushButton * yesButton = new QPushButton("Yes");
    layout->addWidget(yesButton);

    QPushButton * noButton = new QPushButton("No");
    layout->addWidget(noButton);

    m_widget->setLayout(layout);
    m_widget->setVisible(false);

    connect(yesButton, SIGNAL(clicked(bool)), this, SLOT(onYes()));
    connect(noButton, SIGNAL(clicked(bool)), this, SLOT(onNo()));
}

void TaskTestTask::onYes() {
    std::cout << "yes button, body ID " << m_bodyID << std::endl;
}

void TaskTestTask::onNo() {
    std::cout << "no button, body ID " << m_bodyID << std::endl;
}

// constants
const QString TaskTestTask::KEY_TASKTYPE = "task type";
const QString TaskTestTask::VALUE_TASKTYPE = "test task";
const QString TaskTestTask::KEY_BODYID = "body ID";

QString TaskTestTask::taskTypeStatic()
{
  return VALUE_TASKTYPE;
}

TaskTestTask* TaskTestTask::createFromJson(QJsonObject json, ZFlyEmBody3dDoc *)
{
  return new TaskTestTask(json);
}

QString TaskTestTask::taskType() const
{
  return taskTypeStatic();
}

QString TaskTestTask::actionString() {
    return "Review test thing:";
}

QString TaskTestTask::targetString() {
    return QString::number(m_bodyID);
}

QJsonObject TaskTestTask::addToJson(QJsonObject taskJson) {
    // see note in loadJson() re: precision; because we
    //  know the source of the body IDs, the conversions
    //  below should be OK

    taskJson[KEY_BODYID] = static_cast<double>(m_bodyID);
    taskJson[KEY_TASKTYPE] = VALUE_TASKTYPE;

    return taskJson;
}

bool TaskTestTask::loadSpecific(QJsonObject json) {

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

QWidget * TaskTestTask::getTaskWidget() {





    return m_widget;

}
