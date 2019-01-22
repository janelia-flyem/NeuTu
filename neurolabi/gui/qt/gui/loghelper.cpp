#include "loghelper.h"

#include <QJsonObject>
#include <QMouseEvent>
#include <QWheelEvent>
#include <QKeyEvent>

#include "logging/zlog.h"

namespace {
std::string get_modifier_prefix(const Qt::KeyboardModifiers &m)
{
  std::string name;
  switch (m) {
  case Qt::ShiftModifier:
    name = "shift+";
    break;
  case Qt::ControlModifier:
    name = "control+";
    break;
  case Qt::AltModifier:
    name = "alt+";
    break;
  default:
    break;
  }

  return name;
}
}

void neutu::LogMouseEvent(
    QMouseEvent *event, const QString &action, const QString &window)
{
  std::string idstr = get_modifier_prefix(event->modifiers());

  switch (event->buttons()) {
  case Qt::LeftButton:
    idstr += "left";
    break;
  case Qt::RightButton:
    idstr += "right";
    break;
  case Qt::RightButton | Qt::LeftButton:
    idstr += "left+right";
    break;
  default:
    break;
  }

  KLOG << ZLog::Info()
       << ZLog::Window(window.toStdString())
       << ZLog::Action(action.toStdString())
       << ZLog::Object("mouse", "", idstr);
}

void neutu::LogMouseEvent(QWheelEvent *event, const QString &window)
{
  std::string idstr = get_modifier_prefix(event->modifiers()) + "wheel";

  KLOG << ZLog::Info()
       << ZLog::Window(window.toStdString())
       << ZLog::Action("scroll")
       << ZLog::Object("mouse", "", idstr);
}

void neutu::LogKeyEvent(QKeyEvent *event, const QString &window)
{
  std::string idstr = get_modifier_prefix(event->modifiers()) +
      QKeySequence(event->key()).toString().toStdString();

  KLOG << ZLog::Info()
       << ZLog::Window(window.toStdString())
       << ZLog::Action("press")
       << ZLog::Object("key", "", idstr);
}
