#ifndef ZFLYEMDOC3DBODYSTATEACCESSOR_H
#define ZFLYEMDOC3DBODYSTATEACCESSOR_H

#include "zflyembodystateaccessor.h"

class ZFlyEmBody3dDoc;

class ZFlyEmDoc3dBodyStateAccessor : public ZFlyEmBodyStateAccessor
{
public:
  ZFlyEmDoc3dBodyStateAccessor();

  void setDocument(ZFlyEmBody3dDoc *doc);

  bool isProtected(uint64_t bodyId) const;

private:
  ZFlyEmBody3dDoc *m_doc = nullptr;
};

#endif // ZFLYEMDOC3DBODYSTATEACCESSOR_H
