#include "taskbodyhistory.h"

#include "flyem/zflyembody3ddoc.h"
#include "neu3window.h"
#include "z3dmeshfilter.h"
#include "z3dwindow.h"

#include <iostream>
#include <QSlider>

namespace {

  static const QString KEY_TASKTYPE = "task type";
  static const QString VALUE_TASKTYPE = "body history";
  static const QString KEY_BODYID = "body ID";
  static const QString KEY_MAXLEVEL = "maximum level";

  Z3DMeshFilter *getMeshFilter(ZStackDoc *doc)
  {
    if (Z3DWindow *window = doc->getParent3DWindow()) {
      if (Z3DMeshFilter *filter =
          dynamic_cast<Z3DMeshFilter*>(window->getMeshFilter())) {
          return filter;
       }
    }
    return nullptr;
  }

}

TaskBodyHistory::TaskBodyHistory(QJsonObject json, ZFlyEmBody3dDoc* bodyDoc)
{
  m_bodyDoc = bodyDoc;

  loadJson(json);

  m_widget = new QSlider(Qt::Horizontal);
  m_widget->setMaximum(m_maxLevel);
  m_widget->setTickInterval(1);
  m_widget->setTickPosition(QSlider::TicksBothSides);

  connect(m_widget, SIGNAL(valueChanged(int)), this, SLOT(updateLevel(int)));

  m_widget->setValue(m_maxLevel);

  Neu3Window::enableZoomToLoadedBody(true);

  if (Z3DMeshFilter *filter = getMeshFilter(m_bodyDoc)) {
    filter->enablePreservingSourceColors(true);
  }
}

QString TaskBodyHistory::taskTypeStatic()
{
  return VALUE_TASKTYPE;
}

TaskBodyHistory* TaskBodyHistory::createFromJson(QJsonObject json, ZFlyEmBody3dDoc *doc)
{
  return new TaskBodyHistory(json, doc);
}

QString TaskBodyHistory::taskType() const
{
  return taskTypeStatic();
}

QString TaskBodyHistory::actionString()
{
  return "Body history:";
}

QString TaskBodyHistory::targetString()
{
  return QString::number(m_bodyId);
}

QWidget *TaskBodyHistory::getTaskWidget()
{
  return m_widget;
}

void TaskBodyHistory::updateLevel(int level)
{
  Neu3Window::enableZoomToLoadedBody(false);
  m_bodyDoc->enableGarbageLifetimeLimit(false);

  QSet<uint64_t> visible({ ZFlyEmBodyManager::Encode(m_bodyId, level) });

  updateBodies(visible, QSet<uint64_t>());
}

QJsonObject TaskBodyHistory::addToJson(QJsonObject taskJson)
{
  return taskJson;
}

void TaskBodyHistory::onCompleted()
{
  Neu3Window::enableZoomToLoadedBody(true);

  m_bodyDoc->enableGarbageLifetimeLimit(true);
}

bool TaskBodyHistory::loadSpecific(QJsonObject json)
{
  if (!json.contains(KEY_BODYID)) {
    return false;
  }

  m_bodyId = json[KEY_BODYID].toDouble();
  m_maxLevel = json[KEY_MAXLEVEL].toDouble();

  return true;
}
