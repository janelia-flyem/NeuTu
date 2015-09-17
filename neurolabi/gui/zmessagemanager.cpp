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

void ZMessageManager::registerWidget(
    QWidget *widget, ZMessageManager *parent, bool updatingParent)
{
  detachWidget();
  m_widget = widget;

  if (parent == NULL && updatingParent) {
    parent = getPotentialParent();
  }

  if (m_widget != NULL) {
    connect(static_cast<QObject*>(m_widget), SIGNAL(destroyed()),
            this, SLOT(deleteLater()));
  }

  setParent(parent);
}

void ZMessageManager::detachWidget()
{
  disconnect(qobject_cast<QObject*>(m_widget), SIGNAL(destroyed()),
             this, SLOT(detachWidget()));
  m_widget = NULL;
  setParent(&(getRootManager()));
}

void ZMessageManager::processMessage(ZMessage *message, bool reporting)
{
  if (message == NULL) {
    return;
  }

  if (message->isActive()) {
    if (m_processor.get() != NULL) {
      m_processor->processMessage(message, m_widget);
    }
    if (message->isActive()) {
      dispatchMessage(message);
    }
    if (reporting && message->isActive()) {
      message->setCurrentSource(getWidget());
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
          qobject_cast<const ZMessageManager*>(child));
    if (manager != NULL) {
      if ((message->getCurrentSource() == NULL) ||
          (manager->getWidget() != message->getCurrentSource())) {
        manager->processMessage(message, false);
        if (!message->isActive()) {
          break;
        }
      }
    }
  }
}

void ZMessageManager::reportMessage(ZMessage *message)
{
  if (message == NULL) {
    return;
  }

  ZMessageManager *manager = qobject_cast<ZMessageManager*>(parent());
  if (manager != NULL) {
    if ((message->getOriginalSource() == NULL) ||
        (manager->getWidget() != message->getOriginalSource())) {
      manager->processMessage(message, true);
    }
  }
}

ZMessageManager* ZMessageManager::getPotentialParent() const
{
  ZMessageManager &root = getRootManager();

  ZMessageManager *parent = NULL;
  if (m_widget == NULL) {
    parent = &root;
  } else {
    QWidget *parentWidget = m_widget->parentWidget();
    parent = root.findChildManager(parentWidget);
    if (parent == NULL) {
      parent = &root;
    }
  }

  return parent;
}

void ZMessageManager::updateParent()
{
  setParent(getPotentialParent());
}

ZMessageManager* ZMessageManager::findChildManager(const QWidget *widget) const
{
  QObjectList objList = children();
  ZMessageManager *target = NULL;
  foreach (QObject *obj, objList) {
    ZMessageManager *child = qobject_cast<ZMessageManager*>(obj);
    if (child != NULL) {
      if (child->getWidget() == widget) {
        target = child;
      } else {
        target = child->findChildManager(widget);
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

void ZMessageManager::setProcessor(ZMessageProcessor *processor)
{
  setProcessor(ZSharedPointer<ZMessageProcessor>(processor));
}

bool ZMessageManager::hasProcessor() const
{
  return m_processor.get() != NULL;
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
    ZMessageManager *child = qobject_cast<ZMessageManager*>(obj);
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

ZMessageManager* ZMessageManager::Make(QWidget *widget, ZMessageManager *parent)
{
  ZMessageManager *messageManager = new ZMessageManager();
  messageManager->registerWidget(widget, parent);

  return messageManager;
}

bool ZMessageManager::hasParent() const
{
  return parent() != NULL;
}
