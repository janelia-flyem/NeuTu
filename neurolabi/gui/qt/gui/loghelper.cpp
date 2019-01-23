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

void append_mouse_button(std::string &str, const std::string &buttonStr)
{
  if (str.empty()) {
    str = buttonStr;
  } else {
    str += "+" + buttonStr;
  }
}

}

void neutu::LogMouseEvent(
    QMouseEvent *event, const QString &action, const QString &window)
{
  std::string idstr = get_modifier_prefix(event->modifiers());

  if (event->buttons() & Qt::LeftButton) {
    append_mouse_button(idstr, "left");
  }

  if (event->buttons() & Qt::MidButton) {
    append_mouse_button(idstr, "middle");
  }

  if (event->buttons() & Qt::RightButton) {
    append_mouse_button(idstr, "right");
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
  if (event->key() != Qt::Key_Shift && event->key() != Qt::Key_Alt &&
      event->key() != Qt::Key_Control && event->key() != Qt::Key_Meta) {
    std::string idstr = get_modifier_prefix(event->modifiers()) +
        QKeySequence(event->key()).toString().toStdString();

    KLOG << ZLog::Info()
         << ZLog::Window(window.toStdString())
         << ZLog::Action("press")
         << ZLog::Object("key", "", idstr);
  }
}
