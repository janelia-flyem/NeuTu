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
    name = "shift";
    break;
  case Qt::ControlModifier:
    name = "control";
    break;
  case Qt::AltModifier:
    name = "alt";
    break;
  default:
    break;
  }

  return name;
}

void append_mouse_button(std::string &str, const std::string &buttonStr)
{
  if (!buttonStr.empty()) {
    if (str.empty()) {
      str = buttonStr;
    } else {
      str += "+" + buttonStr;
    }
  }
}

void append_mouse_button(std::string &name, Qt::MouseButtons buttons)
{
  if (buttons & Qt::LeftButton) {
    append_mouse_button(name, "left");
  }

  if (buttons & Qt::MidButton) {
    append_mouse_button(name, "middle");
  }

  if (buttons & Qt::RightButton) {
    append_mouse_button(name, "right");
  }
}

void append_key(std::string &str, const std::string &keyStr)
{
  if (!keyStr.empty()) {
    if (str.empty()) {
      str = keyStr;
    } else {
      str += " " + keyStr;
    }
  }
}

}

void neutu::LogMouseEvent(
    QMouseEvent *event, const QString &action, const QString &window)
{
  std::string name = get_modifier_prefix(event->modifiers());

  append_mouse_button(name, event->buttons());

  KLOG << ZLog::Interact()
       << ZLog::Window(window.toStdString())
       << ZLog::Action(action.toStdString())
       << ZLog::Object("mouse", name)
       << ZLog::Level(1);
}

void neutu::LogMouseEvent(QWheelEvent *event, const QString &window)
{
  std::string name = get_modifier_prefix(event->modifiers());
  append_mouse_button(name, "wheel");

  KLOG << ZLog::Interact()
       << ZLog::Window(window.toStdString())
       << ZLog::Action("scroll")
       << ZLog::Tag("value", event->delta())
       << ZLog::Object("mouse", name)
       << ZLog::Level(1);
}

void neutu::LogMouseDragEvent(QMouseEvent *event, const QString &window)
{
  if (event->buttons() != Qt::NoButton) {
    neutu::LogMouseEvent(event, "drag", window);
  }
}

void neutu::LogMouseReleaseEvent(
    Qt::MouseButtons buttons, Qt::KeyboardModifiers modifiers, const QString &window)
{
  if (buttons != Qt::NoButton) {
    std::string name = get_modifier_prefix(modifiers);
    append_mouse_button(name, buttons);

    KLOG << ZLog::Interact()
         << ZLog::Window(window.toStdString())
         << ZLog::Action("release")
         << ZLog::Object("mouse", name)
         << ZLog::Level(1);
  }
}

void neutu::LogKeyEvent(QKeyEvent *event, const QString &action, const QString &window)
{
  if (event->key() != Qt::Key_Shift && event->key() != Qt::Key_Alt &&
      event->key() != Qt::Key_Control && event->key() != Qt::Key_Meta) {
    std::string name = get_modifier_prefix(event->modifiers());
    append_key(name, QKeySequence(event->key()).toString().toStdString());

    KLOG << ZLog::Interact()
         << ZLog::Window(window.toStdString())
         << ZLog::Action(action.toStdString())
         << ZLog::Object("key", name)
         << ZLog::Level(1);
  }
}

void neutu::LogKeyPressEvent(QKeyEvent *event, const QString &window)
{
  LogKeyEvent(event, "press", window);
}

void neutu::LogKeyReleaseEvent(QKeyEvent *event, const QString &window)
{
  LogKeyEvent(event, "release", window);
}
