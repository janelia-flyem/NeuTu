#ifndef ZFLYEMPROOFPRESENTER_H
#define ZFLYEMPROOFPRESENTER_H

#include "zstackpresenter.h"

class QKeyEvent;

class ZFlyEmProofPresenter : public ZStackPresenter
{
  Q_OBJECT
public:
  explicit ZFlyEmProofPresenter(ZStackFrame *parent = 0);
  explicit ZFlyEmProofPresenter(QWidget *parent = 0);

  bool customKeyProcess(QKeyEvent *event);

  void toggleHighlightMode();
  bool isHighlight() const;
  void setHighlightMode(bool hl);
  bool isSplitOn() const;

  void enableSplit();
  void disableSplit();
  void setSplitEnabled(bool s);

  void processCustomOperator(const ZStackOperator &op);

signals:
  void highlightingSelected(bool);
  void selectingBodyAt(int x, int y, int z);
  void deselectingAllBody();

public slots:

private:
  bool m_isHightlightMode;
};

#endif // ZFLYEMPROOFPRESENTER_H
