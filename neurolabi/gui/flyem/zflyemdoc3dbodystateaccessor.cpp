#include "zflyemdoc3dbodystateaccessor.h"

#include "zflyembody3ddoc.h"

ZFlyEmDoc3dBodyStateAccessor::ZFlyEmDoc3dBodyStateAccessor()
{

}

void ZFlyEmDoc3dBodyStateAccessor::setDocument(ZFlyEmBody3dDoc *doc)
{
  m_doc = doc;
}

bool ZFlyEmDoc3dBodyStateAccessor::isProtected(uint64_t bodyId) const
{
  if (m_doc != NULL) {
    return m_doc->isBodyProtected(bodyId);
  }

  return false;
}
