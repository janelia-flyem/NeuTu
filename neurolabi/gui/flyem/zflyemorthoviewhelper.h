#ifndef ZFLYEMORTHOVIEWHELPER_H
#define ZFLYEMORTHOVIEWHELPER_H

#include "common/neutube_def.h"

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
  neutube::EAxis getAlignAxis(neutube::EAxis a1, neutube::EAxis a2);
  neutube::EAxis getAlignAxis(const ZFlyEmOrthoMvc *mvc);
  ZPoint getCrossCenter() const;

private:
  ZFlyEmOrthoMvc *m_mvc;
};

#endif // ZFLYEMORTHOVIEWHELPER_H
