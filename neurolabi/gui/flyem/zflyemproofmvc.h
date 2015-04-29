#ifndef ZFLYEMPROOFMVC_H
#define ZFLYEMPROOFMVC_H

#include <QString>

#include "zstackmvc.h"

class QWidget;
class ZFlyEmProofDoc;
class ZDvidTileEnsemble;
class ZDvidTarget;
class ZDvidDialog;

class ZFlyEmProofMvc : public ZStackMvc
{
  Q_OBJECT
public:
  explicit ZFlyEmProofMvc(QWidget *parent = 0);

  static ZFlyEmProofMvc* Make(
      QWidget *parent, ZSharedPointer<ZFlyEmProofDoc> doc);
  static ZFlyEmProofMvc* Make(const ZDvidTarget &target);

  ZFlyEmProofDoc* getCompleteDocument() const;

  template <typename T>
  void connectControlPanel(T *panel);

  template <typename T>
  void connectSplitControlPanel(T *panel);

  ZDvidTileEnsemble* getDvidTileEnsemble();

  void setDvidTarget(const ZDvidTarget &target);

  void clear();

signals:
  void launchingSplit(const QString &message);
  void launchingSplit(int64_t bodyId);

public slots:
  void mergeSelected();
  void undo();
  void redo();

  void setSegmentationVisible(bool visible);
  void setDvidTarget();
  const ZDvidTarget &getDvidTarget() const;
  void launchSplit(int64_t bodyId);
  void processMessageSlot(const QString &message);
  void notifySplitTriggered();
  void exitSplit();
//  void toggleEdgeMode(bool edgeOn);

protected:
  void customInit();

private:
  bool m_showSegmentation;
  bool m_splitOn;
  ZDvidDialog *m_dvidDlg;
};

template <typename T>
void ZFlyEmProofMvc::connectControlPanel(T *panel)
{
  connect(panel, SIGNAL(segmentVisibleChanged(bool)),
          this, SLOT(setSegmentationVisible(bool)));
  connect(panel, SIGNAL(mergingSelected()), this, SLOT(mergeSelected()));
  connect(panel, SIGNAL(edgeModeToggled(bool)),
          this, SLOT(toggleEdgeMode(bool)));
  connect(panel, SIGNAL(dvidSetTriggered()), this, SLOT(setDvidTarget()));
  connect(this, SIGNAL(launchingSplit(int64_t)),
          panel, SIGNAL(splitTriggered(int64_t)));
}

template <typename T>
void ZFlyEmProofMvc::connectSplitControlPanel(T *panel)
{
  connect(panel, SIGNAL(quickViewTriggered()), this, SLOT(showBodyQuickView()));
  connect(panel, SIGNAL(exitingSplit()), this, SLOT(exitSplit));
}


#endif // ZFLYEMPROOFMVC_H
