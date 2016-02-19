#include "zstackviewmanager.h"
#include <iostream>
#include "zstackframe.h"
#include "z3dwindow.h"
#include "zstackview.h"

ZStackViewManager::ZStackViewManager(QObject *parent) :
  QObject(parent)
{
}

void ZStackViewManager::registerWidgetPair(QWidget *sender, QWidget *receiver)
{
  if (!m_windowGraph.contains(sender, receiver)) {
    m_windowGraph.insert(sender, receiver);
  }
}

void ZStackViewManager::registerWindowPair(
    ZStackFrame *sender, Z3DWindow *receiver)
{
  registerWindow(sender);
  registerWindow(receiver);
  registerWidgetPair(sender, receiver);
}

void ZStackViewManager::registerWindowPair(
    Z3DWindow *sender, ZStackFrame *receiver)
{
  registerWindow(sender);
  registerWindow(receiver);
  registerWidgetPair(sender, receiver);
}

void ZStackViewManager::registerWindowPair(
    ZStackFrame *sender, ZStackFrame *receiver)
{
  registerWindow(sender);
  registerWindow(receiver);

  receiver->view()->setView(sender->view()->getViewParameter());

  registerWidgetPair(sender, receiver);
}


void ZStackViewManager::print() const
{
  std::cout << m_windowGraph.size() << " pairs." << std::endl;
  for (QMultiMap<QWidget*, QWidget*>::const_iterator iter = m_windowGraph.begin();
       iter != m_windowGraph.end(); ++iter) {
    std::cout << "  " << iter.key() << " --> " << iter.value() << std::endl;
  }
}

void ZStackViewManager::removeWidget()
{
  removeWidget(qobject_cast<QWidget*>(sender()));
}

void ZStackViewManager::removeWidget(QWidget *widget)
{
  if (widget != NULL) {
    m_windowGraph.remove(widget);

    QList<QWidget*> senderList;
    for (QMultiMap<QWidget*, QWidget*>::const_iterator iter = m_windowGraph.begin();
         iter != m_windowGraph.end(); ++iter) {
      if (iter.value() == widget) {
        senderList.append(iter.key());
      }
    }

    for (QList<QWidget*>::iterator iter = senderList.begin();
         iter != senderList.end(); ++iter) {
      m_windowGraph.remove(*iter, widget);
    }
  }
}

void ZStackViewManager::updateView(const ZStackViewParam &param)
{
#if 1
  std::cout << "Slot: ZStackViewManager::updateView" << std::endl;
#endif

  QWidget *senderWidget = qobject_cast<QWidget*>(sender());

  QList<QWidget*> receiverList = m_windowGraph.values(senderWidget);
  for (QList<QWidget*>::iterator iter = receiverList.begin();
       iter != receiverList.end(); ++iter) {
    ZStackFrame *frame = qobject_cast<ZStackFrame*>(*iter);
    if (frame != NULL) {
      frame->view()->setView(param);
    }
  }
}

bool ZStackViewManager::hasWidget(QWidget *widget)
{
  if (m_windowGraph.contains(widget)) {
    return true;
  }

  for (QMultiMap<QWidget*, QWidget*>::const_iterator iter = m_windowGraph.begin();
       iter != m_windowGraph.end(); ++iter) {
    if (iter.value() == widget) {
      return true;
    }
  }

  return false;
}

void ZStackViewManager::registerWindow(Z3DWindow *window)
{
  if (!hasWidget(window)) {
    connect(window, SIGNAL(closed()), this, SLOT(removeWidget()));
  }
}

void ZStackViewManager::slotTest()
{
  std::cout << "ZStackViewManager::slotTest triggered" << std::endl;
}

void ZStackViewManager::registerWindow(ZStackFrame *window)
{
  if (!hasWidget(window)) {
    connect(window, SIGNAL(closed(ZStackFrame*)), this, SLOT(removeWidget()));
    connect(window, SIGNAL(viewChanged(ZStackViewParam)),
            this, SLOT(updateView(ZStackViewParam)));
    connect(window, SIGNAL(viewChanged(ZStackViewParam)),
            this, SLOT(slotTest()));
  }
}


