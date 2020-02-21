#ifndef ZFLYEMORTHOMVC_H
#define ZFLYEMORTHOMVC_H

#include "flyem/zflyemproofmvc.h"

class ZFlyEmOrthoDoc;

class ZFlyEmOrthoMvc : public ZFlyEmProofMvc
{
  Q_OBJECT
public:
  explicit ZFlyEmOrthoMvc(QWidget *parent = nullptr);

  static ZFlyEmOrthoMvc* Make(
      QWidget *parent, ZSharedPointer<ZFlyEmOrthoDoc> doc,
      neutu::EAxis axis = neutu::EAxis::Z);

  static ZFlyEmOrthoMvc* Make(const ZDvidEnv &env, neutu::EAxis axis);
  static ZFlyEmOrthoMvc* Make(
      const ZDvidEnv &env, neutu::EAxis axis,
      int width, int height, int depth);

  ZFlyEmOrthoDoc* getCompleteDocument() const;

//  void setDvidTarget(const ZDvidTarget &target) override;
  void setDvid(const ZDvidEnv &env) override;
//  ZDvidTarget getDvidTarget() const override;
  void updateDvidTargetFromDoc();

  void updateStack(const ZIntPoint &center);
  void updateStackFromCrossHair();

//  void syncView(const ZIntPoint &center, double zoomRatio);

  void setCrossHairCenter(double x, double y);
  void processViewChangeCustom(const ZStackViewParam &viewParam) override;
  void setAutoReload(bool on) {
    m_autoReload = on;
  }

signals:
  void crossHairChanged();

public slots:
  void moveCrossHairTo(int x, int y);


private:
  void init();

private:
  bool m_autoReload = false;

};

#endif // ZFLYEMORTHOMVC_H
