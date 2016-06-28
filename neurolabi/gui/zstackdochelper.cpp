#include "zstackdochelper.h"
#include "zstackdoc.h"
#include "dvid/zdvidlabelslice.h"
#include "dvid/zdvidtileensemble.h"
#include "zstackview.h"
#include "zintcuboid.h"
#include "flyem/zflyemproofdoc.h"
#include "zintcuboidobj.h"
#include "dvid/zdvidsparsestack.h"

ZStackDocHelper::ZStackDocHelper()
{
  m_currentZ = 0;
  m_hasCurrentZ = false;
  m_sparseStack = NULL;
}

ZStackDocHelper::~ZStackDocHelper()
{
  delete m_sparseStack;
}

ZStack* ZStackDocHelper::getSparseStack(const ZStackDoc *doc)
{
  if (m_sparseStack != NULL) {
    delete m_sparseStack;
    m_sparseStack = NULL;
    m_sparseStackDsIntv.set(0, 0, 0);
  }

  ZStack *stack = NULL;
  if (doc->getTag() == NeuTube::Document::FLYEM_PROOFREAD) {
    const ZFlyEmProofDoc *cdoc = qobject_cast<const ZFlyEmProofDoc*>(doc);
    if (cdoc != NULL) {
      ZDvidSparseStack *dvidSparseStack = doc->getDvidSparseStack();
      stack = dvidSparseStack->getStack();
      m_sparseStackDsIntv = dvidSparseStack->getDownsampleInterval();
//      box = cdoc->getSplitRoi()->getCuboid();
//      stack = doc->getSparseStack()->getStack(box, &m_sparseStackDsIntv);
    }
  } else {
    const ZSparseStack *spStack = doc->getSparseStack();
    if (!spStack->getBoundBox().isEmpty()) {
      stack = const_cast<ZStack*>(spStack->getStack());
    }
    m_sparseStackDsIntv = spStack->getDownsampleInterval();
  }

  return stack;
}

void ZStackDocHelper::extractCurrentZ(const ZStackDoc *doc)
{
  m_hasCurrentZ = false;
  if (doc != NULL) {
    {
      const TStackObjectList &objList =
          doc->getObjectList(ZStackObject::TYPE_DVID_TILE_ENSEMBLE);
      if (!objList.isEmpty()) {
        ZDvidTileEnsemble *obj =
            dynamic_cast<ZDvidTileEnsemble*>(objList.first());
        if (obj->isVisible()) {
          if (obj->getView() != NULL) {
            m_currentZ = obj->getView()->getCurrentZ();
            m_hasCurrentZ = true;
          }
        }
      }
    }

    if (!m_hasCurrentZ) {
      const TStackObjectList &objList =
          doc->getObjectList(ZStackObject::TYPE_DVID_LABEL_SLICE);
      if (!objList.isEmpty()) {
        ZDvidLabelSlice *obj = dynamic_cast<ZDvidLabelSlice*>(objList.first());
        if (obj->isVisible()) {
          m_currentZ = obj->getViewParam().getZ();
          m_hasCurrentZ = true;
        }
      }
    }
  }
}

int ZStackDocHelper::getCurrentZ() const
{
  return m_currentZ;
}

bool ZStackDocHelper::hasCurrentZ() const
{
  return m_hasCurrentZ;
}

ZIntCuboid getVolumeBoundBox(const ZStackDoc *doc)
{
  ZIntCuboid box;
  if (doc != NULL) {
    if (doc->getTag() == NeuTube::Document::FLYEM_PROOFREAD) {
      const ZFlyEmProofDoc *cdoc = qobject_cast<const ZFlyEmProofDoc*>(doc);
      if (cdoc != NULL) {
        box = cdoc->getSplitRoi()->getCuboid();
      }
      //    doc->getRect2dRoi()
    } else {
      return doc->getStack()->getBoundBox();
    }
  }

  return box;
}
