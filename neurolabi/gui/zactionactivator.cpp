#include "zactionactivator.h"
#include "zstackdoc.h"
#include "zstackframe.h"
#include "zstackpresenter.h"
#include "z3dwindow.h"

ZActionActivator::ZActionActivator()
{
}

ZActionActivator::~ZActionActivator()
{

}

bool ZActionActivator::isPositive(const ZStackFrame *frame) const
{
  if (frame == NULL) {
    return false;
  }

  return isPositive(frame->document().get()) && isPositive(frame->presenter());
}

bool ZActionActivator::isPositive(const ZStackDoc *doc) const
{
  if (doc == NULL) {
    return false;
  }

  return true;
}

bool ZActionActivator::isPositive(const ZStackPresenter *presenter) const
{
  if (presenter == NULL){
    return false;
  }

  return true;
}

bool ZActionActivator::isPositive(const Z3DWindow *window) const
{
  if (window == NULL) {
    return false;
  }

  return isPositive(window->getDocument());
}

void ZActionActivator::update(bool positive)
{
  foreach (QAction *action, m_postiveActionList) {
    //qDebug() << action->text();
    action->setEnabled(positive);
  }

  foreach (QAction *action, m_negativeActionList) {
    //qDebug() << action->text();
    action->setEnabled(!positive);
  }
}

void ZActionActivator::update(const ZStackFrame *frame)
{
  update(isPositive(frame));
}

void ZActionActivator::update(const ZStackDoc *doc)
{
  update(isPositive(doc));
}

void ZActionActivator::update(const Z3DWindow *window)
{
  update(isPositive(window));
}

void ZActionActivator::registerAction(QAction *action, bool positive)
{
  if (action != NULL) {
    if (positive) {
      m_postiveActionList.insert(action);
    } else {
      m_negativeActionList.insert(action);
    }
  }
}

/////////////////////////////////////
ZStackActionActivator::ZStackActionActivator()
{

}

ZStackActionActivator::~ZStackActionActivator()
{

}

bool ZStackActionActivator::isPositive(const ZStackDoc *doc) const
{
  if (doc == NULL) {
    return false;
  }

  return doc->hasStackData();
}

/////////////////////////
ZSingleSwcNodeActionActivator::ZSingleSwcNodeActionActivator()
{

}

ZSingleSwcNodeActionActivator::~ZSingleSwcNodeActionActivator()
{

}

bool ZSingleSwcNodeActionActivator::isPositive(const ZStackDoc *doc) const
{
  if (doc == NULL) {
    return false;
  }

  return doc->getSelectedSwcNodeNumber() == 1;
}

/////////////////////////////////////////
ZSwcActionActivator::ZSwcActionActivator()
{

}

ZSwcActionActivator::~ZSwcActionActivator()
{

}

bool ZSwcActionActivator::isPositive(const ZStackDoc *doc) const
{
  if (doc == NULL) {
    return false;
  }

  return doc->hasSwc();
}
