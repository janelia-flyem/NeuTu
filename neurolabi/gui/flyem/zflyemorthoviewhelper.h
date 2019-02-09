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
  neutu::EAxis getAlignAxis(neutu::EAxis a1, neutu::EAxis a2);
  neutu::EAxis getAlignAxis(const ZFlyEmOrthoMvc *mvc);
  ZPoint getCrossCenter() const;

private:
  ZFlyEmOrthoMvc *m_mvc;
};

#endif // ZFLYEMORTHOVIEWHELPER_H
