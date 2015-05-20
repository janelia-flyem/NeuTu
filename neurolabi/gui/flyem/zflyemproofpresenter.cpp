#include "zflyemproofpresenter.h"

#include <QKeyEvent>

ZFlyEmProofPresenter::ZFlyEmProofPresenter(ZStackFrame *parent) :
  ZStackPresenter(parent), m_isHightlightMode(false)
{
}

ZFlyEmProofPresenter::ZFlyEmProofPresenter(QWidget *parent) :
  ZStackPresenter(parent), m_isHightlightMode(false)
{

}

bool ZFlyEmProofPresenter::customKeyProcess(QKeyEvent *event)
{
  bool processed = false;

  switch (event->key()) {
  case Qt::Key_H:
    toggleHighlightMode();
    emit highlightingSelected(isHighlight());
    processed = true;
    break;
  default:
    break;
  }

  return processed;
}

bool ZFlyEmProofPresenter::isHighlight() const
{
  return m_isHightlightMode;
}

void ZFlyEmProofPresenter::setHighlightMode(bool hl)
{
  m_isHightlightMode = hl;
}

void ZFlyEmProofPresenter::toggleHighlightMode()
{
  setHighlightMode(!isHighlight());
}
