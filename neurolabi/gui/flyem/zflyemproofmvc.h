#ifndef ZFLYEMPROOFMVC_H
#define ZFLYEMPROOFMVC_H

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

  ZDvidTileEnsemble* getDvidTileEnsemble();

  void setDvidTarget(const ZDvidTarget &target);

  void clear();

signals:

public slots:
  void mergeSelected();
  void undo();
  void redo();

  void setSegmentationVisible(bool visible);
  void setDvidTarget();
//  void toggleEdgeMode(bool edgeOn);

private:
  bool m_showSegmentation;
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
}


#endif // ZFLYEMPROOFMVC_H
