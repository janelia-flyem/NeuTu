#include "data3d/zstackobjecthelper.h"

#include "geometry/zaffineplane.h"
#include "zstackobject.h"
#include "zswctree.h"
#include "flyem/zflyembookmark.h"

ZStackObjectHelper::ZStackObjectHelper()
{

}

bool ZStackObjectHelper::IsOverSize(const ZStackObject &obj)
{
  return obj.getSource() == "oversize" || obj.getObjectId() == "oversize";
}

void ZStackObjectHelper::SetOverSize(ZStackObject *obj)
{
  if (obj != NULL) {
    obj->setObjectId("oversize");
  }
}

bool ZStackObjectHelper::IsEmptyTree(const ZStackObject *obj)
{
  bool passed = false;
  if (obj != NULL) {
    if (obj->getType() == ZStackObject::EType::SWC) {
      const ZSwcTree *tree = dynamic_cast<const ZSwcTree*>(obj);
      if (tree != NULL) {
        passed = tree->isEmpty();
      }
    }
  }

  return passed;
}

ZStackObject* ZStackObjectHelper::Clone(ZStackObject *obj)
{
  if (obj) {
    return obj->clone();
  }

  return nullptr;
}

void ZStackObjectHelper::Align(ZStackObject *obj, const ZAffinePlane &cutPlane)
{
  ZStackBall *ball = dynamic_cast<ZStackBall*>(obj);
  if (ball) {
    ball->setCenter(cutPlane.align(ball->getCenter()));
  }
}
