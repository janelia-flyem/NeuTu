#ifndef ZFLYEMORTHOVIEWHELPER_H
#define ZFLYEMORTHOVIEWHELPER_H

#include "neutube_def.h"

class ZFlyEmOrthoMvc;
class ZFlyEmOrthoDoc;
class ZStackView;
class ZPoint;

class ZFlyEmOrthoViewHelper
{
public:
  ZFlyEmOrthoViewHelper();

  void attach(ZFlyEmOrthoMvc *mvc);

  void syncCrossHair(ZFlyEmOrthoMvc *mvc);
  void syncViewPort(ZFlyEmOrthoMvc *mvc);

  ZFlyEmOrthoMvc* getMasterMvc() const;
  ZFlyEmOrthoDoc* getMasterDoc() const;
  ZStackView* getMasterView() const;

private:
  NeuTube::EAxis getAlignAxis(NeuTube::EAxis a1, NeuTube::EAxis a2);
  NeuTube::EAxis getAlignAxis(const ZFlyEmOrthoMvc *mvc);
  ZPoint getCrossCenter() const;

private:
  ZFlyEmOrthoMvc *m_mvc;
};

#endif // ZFLYEMORTHOVIEWHELPER_H
