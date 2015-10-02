#ifndef ZFLYEMPROOFPRESENTER_H
#define ZFLYEMPROOFPRESENTER_H

#include "zstackpresenter.h"

class QKeyEvent;
class ZFlyEmBookmark;

class ZFlyEmProofPresenter : public ZStackPresenter
{
  Q_OBJECT

protected:
//  explicit ZFlyEmProofPresenter(ZStackFrame *parent = 0);
  explicit ZFlyEmProofPresenter(QWidget *parent = 0);

public:
  static ZFlyEmProofPresenter* Make(QWidget *parent);

  bool customKeyProcess(QKeyEvent *event);

  void toggleHighlightMode();
  bool isHighlight() const;
  void setHighlightMode(bool hl);
  bool isSplitOn() const;
  bool highTileContrast() const;

  void setHighTileContrast(bool high);

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

  void processRectRoiUpdate(ZRect2d *rect);

  ZKeyOperationConfig* getKeyConfig();
  void configKeyMap();

private:
  void tryAddBookmarkMode();
  void tryAddBookmarkMode(double x, double y);
  void addActiveStrokeAsBookmark();
  void init();

signals:
  void highlightingSelected(bool);
  void selectingBodyAt(int x, int y, int z);
  void deselectingAllBody();
  void runningSplit();
  void goingToBody();
  void selectingBody();
  void bookmarkAdded(ZFlyEmBookmark*);
  void annotatingBookmark(ZFlyEmBookmark*);
  void mergingBody();
  void goingToBodyBottom();
  void goingToBodyTop();

public slots:

private:
  bool m_isHightlightMode;
  bool m_splitWindowMode;
  bool m_highTileContrast;

  ZKeyOperationMap m_bookmarkKeyOperationMap;
};

#endif // ZFLYEMPROOFPRESENTER_H
