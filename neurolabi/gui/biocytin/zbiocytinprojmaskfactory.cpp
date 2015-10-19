#include "zbiocytinprojmaskfactory.h"
#include "zstackview.h"
#include "zstackdoc.h"
#include "zstroke2d.h"

ZBiocytinProjMaskFactory::ZBiocytinProjMaskFactory()
{
}

ZStack* ZBiocytinProjMaskFactory::MakeMask(ZStackView *view, int label)
{
  ZStack *mask = NULL;
  if (view != NULL) {
    ZSharedPointer<ZStackDoc> doc = view->buddyDocument();
    if (doc.get() != NULL) {
      QList<ZDocPlayer*> playerList =
          doc->getPlayerList(ZStackObjectRole::ROLE_SKELETON_MASK);
      QList<ZStackObject*> objList;
      for (QList<ZDocPlayer*>::iterator iter = playerList.begin();
           iter != playerList.end(); ++iter) {
        ZDocPlayer *player = *iter;
        ZStackObject *obj = player->getData();
        if (player->getLabel() == label) {
          objList.append(obj);
        } else {
          if (obj->getType() == ZStackObject::TYPE_STROKE) {
            ZStroke2d *stroke = dynamic_cast<ZStroke2d*>(obj);
            if (stroke->isEraser()) {
              objList.append(obj);
            }
          }
        }
      }

      std::sort(objList.begin(), objList.end(), ZStackObject::ZOrderLessThan());

      mask = new ZStack(GREY, doc->getStackWidth(), doc->getStackHeight(), 1, 1);
      mask->setZero();

      for (QList<ZStackObject*>::const_iterator iter = objList.begin();
           iter != objList.end(); ++iter) {
        const ZStackObject *obj = *iter;
        switch (obj->getType()) {
        case ZStackObject::TYPE_STROKE:
        {
          const ZStroke2d *stroke = dynamic_cast<const ZStroke2d*>(obj);
          stroke->labelStack(mask);
        }
          break;
        case ZStackObject::TYPE_OBJECT3D_SCAN:
        {
          const ZObject3dScan *sparseObj =
              dynamic_cast<const ZObject3dScan*>(obj);
          sparseObj->drawStack(mask, 1);
        }
          break;
        default:
          break;
        }
//        if (obj->getL)
      }
      mask->binarize();
    }
  }

  return mask;
}
