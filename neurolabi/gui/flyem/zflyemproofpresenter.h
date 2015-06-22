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

  bool processKeyPressEvent(QKeyEvent *event);
  void processCustomOperator(const ZStackOperator &op);

  inline bool isSplitWindow() const {
    return m_splitWindowMode;
  }

  void setSplitWindow(bool state) {
    m_splitWindowMode = state;
  }

signals:
  void highlightingSelected(bool);
  void selectingBodyAt(int x, int y, int z);
  void deselectingAllBody();
  void runningSplit();

public slots:

private:
  bool m_isHightlightMode;
  bool m_splitWindowMode;
};

#endif // ZFLYEMPROOFPRESENTER_H
