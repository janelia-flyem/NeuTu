#include "zmessagemanager.h"

#include<QWidget>

#include "zmessage.h"
#include "zmessageprocessor.h"

ZMessageManager::ZMessageManager(QObject *parent) :
  QObject(parent), m_widget(NULL), m_processor(NULL)
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


