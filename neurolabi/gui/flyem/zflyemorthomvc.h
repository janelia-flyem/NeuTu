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
      neutu::EAxis axis = neutu::EAxis::Z);
  static ZFlyEmOrthoMvc* Make(const ZDvidTarget &target, neutu::EAxis axis);
  static ZFlyEmOrthoMvc* Make(
      const ZDvidTarget &target, neutu::EAxis axis,
      int width, int height, int depth);

  ZFlyEmOrthoDoc* getCompleteDocument() const;

  void setDvidTarget(const ZDvidTarget &target);
  ZDvidTarget getDvidTarget() const;
  void updateDvidTargetFromDoc();

  void updateStack(const ZIntPoint &center);
  void updateStackFromCrossHair();

//  void syncView(const ZIntPoint &center, double zoomRatio);

  void setCrossHairCenter(double x, double y);

signals:
  void crossHairChanged();

public slots:
  void moveCrossHairTo(int x, int y);


private:
  void init();

};

#endif // ZFLYEMORTHOMVC_H
