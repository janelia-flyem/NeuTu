#ifndef ZFLYEMPROOFPRESENTER_H
#define ZFLYEMPROOFPRESENTER_H

#include "zstackpresenter.h"
#include "dvid/zdvidsynapse.h"

class QKeyEvent;
class ZFlyEmBookmark;
class ZFlyEmProofDoc;

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
  void processCustomOperator(
      const ZStackOperator &op, ZInteractionEvent *e = NULL);

  inline bool isSplitWindow() const {
    return m_splitWindowMode;
  }

  void setSplitWindow(bool state) {
    m_splitWindowMode = state;
  }

  void processRectRoiUpdate(ZRect2d *rect, bool appending);

  ZKeyOperationConfig* getKeyConfig();
  void configKeyMap();

//  void createBodyContextMenu();

  ZStackDocMenuFactory* getMenuFactory();

  ZFlyEmProofDoc* getCompleteDocument() const;

  void createSynapseContextMenu();
  QMenu* getSynapseContextMenu();

  QMenu* getContextMenu();

signals:
  void highlightingSelected(bool);
  void selectingBodyAt(int x, int y, int z);
  void deselectingAllBody();
  void selectingBodyInRoi(bool appending);
  void runningSplit();
  void goingToBody();
  void selectingBody();
  void bookmarkAdded(ZFlyEmBookmark*);
  void annotatingBookmark(ZFlyEmBookmark*);
  void mergingBody();
  void goingToBodyBottom();
  void goingToBodyTop();

public slots:
  void deleteSelectedSynapse();
  void linkSelectedSynapse();
  void unlinkSelectedSynapse();
  void tryAddSynapseMode(ZDvidSynapse::EKind kind);
  void tryAddPreSynapseMode();
  void tryAddPostSynapseMode();
  void tryMoveSynapseMode();

private:
  void tryAddBookmarkMode();
  void tryAddBookmarkMode(double x, double y);
  void addActiveStrokeAsBookmark();
  void init();
  void tryAddSynapse(const ZIntPoint &pt, ZDvidSynapse::EKind kind);
  void tryAddSynapse(const ZIntPoint &pt);
  void tryMoveSynapse(const ZIntPoint &pt);
  bool updateActiveObjectForSynapseMove();
  bool updateActiveObjectForSynapseMove(const ZPoint &currentPos);
  void updateActiveObjectForSynapseAdd();
  void updateActiveObjectForSynapseAdd(const ZPoint &currentPos);

private:
  bool m_isHightlightMode;
  bool m_splitWindowMode;
  bool m_highTileContrast;

  QMenu *m_synapseContextMenu;

  ZKeyOperationMap m_bookmarkKeyOperationMap;
};

#endif // ZFLYEMPROOFPRESENTER_H
