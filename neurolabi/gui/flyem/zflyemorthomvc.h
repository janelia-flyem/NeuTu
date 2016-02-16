#ifndef ZFLYEMORTHOMVC_H
#define ZFLYEMORTHOMVC_H

#include "flyem/zflyemproofmvc.h"

class ZFlyEmOrthoDoc;

class ZFlyEmOrthoMvc : public ZFlyEmProofMvc
{
  Q_OBJECT
public:
  explicit ZFlyEmOrthoMvc(QWidget *parent = 0);

  static ZFlyEmOrthoMvc* Make(
      QWidget *parent, ZSharedPointer<ZFlyEmOrthoDoc> doc,
      NeuTube::EAxis axis = NeuTube::Z_AXIS);
  static ZFlyEmOrthoMvc* Make(const ZDvidTarget &target, NeuTube::EAxis axis);

  ZFlyEmOrthoDoc* getCompleteDocument() const;

  void setDvidTarget(const ZDvidTarget &target);
  ZDvidTarget getDvidTarget() const;
  void updateDvidTargetFromDoc();

  void updateStack(const ZIntPoint &center);

//  void syncView(const ZIntPoint &center, double zoomRatio);

signals:

public slots:

private:
  void init();

};

#endif // ZFLYEMORTHOMVC_H
