#include "zmessagemanager.h"

#include<QWidget>
#include <iostream>
#include <sstream>

#include "zmessage.h"
#include "zmessageprocessor.h"
#include "ztextlinecompositer.h"

ZMessageManager::ZMessageManager(QObject *parent) :
  QObject(parent), m_widget(NULL)
{
}

void ZMessageManager::registerWidget(QWidget *widget)
{
  if (m_widget != NULL) {
    detachWidget();
  }

  m_widget = widget;
  if (widget != NULL) {
    connect(static_cast<QObject*>(m_widget), SIGNAL(destroyed()),
            this, SLOT(detachWidget()));
  }
  updateParent();
}

void ZMessageManager::detachWidget()
{
  disconnect(dynamic_cast<QObject*>(m_widget), SIGNAL(destroyed()),
             this, SLOT(detachWidget()));
  m_widget = NULL;
}

void ZMessageManager::processMessage(ZMessage *message)
{
  if (message == NULL) {
    return;
  }

  if (m_processor != NULL && message->isActive()) {
    m_processor->processMessage(message, m_widget);
    if (message->isActive()) {
      dispatchMessage(message);
    }
    if (message->isActive()) {
      reportMessage(message);
    }
  }
}

void ZMessageManager::dispatchMessage(ZMessage *message)
{
  if (message == NULL) {
    return;
  }

  const QObjectList& childList = children();
  foreach (const QObject *child, childList) {
    ZMessageManager *manager = const_cast<ZMessageManager*>(
          dynamic_cast<const ZMessageManager*>(child));
    if (manager != NULL) {
      manager->processMessage(message);
    }
  }
}

void ZMessageManager::reportMessage(ZMessage *message)
{
  if (message == NULL) {
    return;
  }

  ZMessageManager *manager = dynamic_cast<ZMessageManager*>(parent());
  if (manager != NULL) {
    manager->processMessage(message);
  }
}

void ZMessageManager::updateParent()
{
  ZMessageManager &root = getRootManager();
  ZMessageManager *manager =
      root.findChildManager(dynamic_cast<QWidget*>(m_widget->parent()));
  if (manager != NULL) {
    setParent(manager);
  } else {
    setParent(&root);
  }
}

ZMessageManager* ZMessageManager::findChildManager(QWidget *widget) const
{
  QObjectList objList = children();
  ZMessageManager *target = NULL;
  foreach (QObject *obj, objList) {
    ZMessageManager *child = dynamic_cast<ZMessageManager*>(obj);
    if (child != NULL) {
      if (child->getWidget() == widget) {
        target = child;
      } else {
        target = findChildManager(widget);
      }
    }
    if (target != NULL) {
      break;
    }
  }

  return target;
}

void ZMessageManager::setProcessor(ZSharedPointer<ZMessageProcessor> processor)
{
  m_processor = processor;
}

bool ZMessageManager::hasProcessor() const
{
  return m_processor != NULL;
}

std::string ZMessageManager::toLine() const
{
  if (hasProcessor()) {
    return m_processor->toString();
  }

  return "Null processor";
}

ZTextLineCompositer ZMessageManager::toLineCompositer(int level) const
{
  ZTextLineCompositer text;
  text.appendLine(toLine());

  QObjectList objList = children();
  foreach (QObject *obj, objList) {
    ZMessageManager *child = dynamic_cast<ZMessageManager*>(obj);
    if (child != NULL) {
      text.appendLine(child->toLineCompositer(1));
    }
  }

  text.setLevel(level);

  return text;
}

void ZMessageManager::print() const
{
  toLineCompositer().print(2);
}


