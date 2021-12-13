#include "zstackdochelper.h"

#include <string>
#include <QColor>

#include "common/debug.h"

#include "zstackdoc.h"
#include "zstackview.h"

#include "dvid/zdvidlabelslice.h"
#include "dvid/zdvidtileensemble.h"
//#include "dvid/zdvidsparsestack.h"

#include "geometry/zintcuboid.h"
#include "flyem/zflyemproofdoc.h"
#include "zintcuboidobj.h"

#include "zstack.hxx"
#include "zsparsestack.h"

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

ZStack* ZStackDocHelper::makeBoundedSparseStack(const ZStackDoc *doc)
{
  if (m_sparseStack != NULL) {
    delete m_sparseStack;
    m_sparseStack = NULL;
    m_sparseStackDsIntv.set(0, 0, 0);
  }


//  if (doc->getCuboidRoi())

  //Todo: remove const_cast
  ZSparseStack *spStack =
      const_cast<ZStackDoc*>(doc)->getSparseStack(doc->getCuboidRoi());

  ZStack *stack = NULL;

  if (spStack) {
    stack = spStack->makeStack(doc->getCuboidRoi(), false);
  }

  if (stack) {
    m_sparseStackDsIntv = stack->getDsIntv();
  }

  return stack;
}

ZStack* ZStackDocHelper::getSparseStack(const ZStackDoc *doc)
{
  if (m_sparseStack != NULL) {
    delete m_sparseStack;
    m_sparseStack = NULL;
    m_sparseStackDsIntv.set(0, 0, 0);
  }

  const ZSparseStack *spStack = doc->getSparseStack();

  const ZStack* stack = nullptr;

  if (spStack) {
    stack = spStack->getStack();
  }

  if (stack) {
    m_sparseStackDsIntv = stack->getDsIntv();
  }

  /*
  ZStack *stack = NULL;
  if (doc->getTag() == neutu::Document::ETag::FLYEM_PROOFREAD) {
    const ZFlyEmProofDoc *cdoc = qobject_cast<const ZFlyEmProofDoc*>(doc);
    if (cdoc != NULL) {
      ZDvidSparseStack *dvidSparseStack = doc->getDvidSparseStack();
      stack = dvidSparseStack->getStack();
      if (stack != NULL) {
        m_sparseStackDsIntv = stack->getDsIntv();
      }
//      m_sparseStackDsIntv = dvidSparseStack->getDownsampleInterval();
//      box = cdoc->getSplitRoi()->getCuboid();
//      stack = doc->getSparseStack()->getStack(box, &m_sparseStackDsIntv);
    }
  } else {
    const ZSparseStack *spStack = doc->getSparseStack();
    if (!spStack->getBoundBox().isEmpty()) {
      stack = const_cast<ZStack*>(spStack->getStack());
    }
    m_sparseStackDsIntv = spStack->getDenseDsIntv();
  }
  */

  return const_cast<ZStack*>(stack);
}

void ZStackDocHelper::extractCurrentZ(const ZStackDoc *doc)
{
  m_hasCurrentZ = false;
  if (doc != NULL) {
    {
      const TStackObjectList &objList =
          doc->getObjectList(ZStackObject::EType::DVID_TILE_ENSEMBLE);
      if (!objList.isEmpty()) {
        ZDvidTileEnsemble *obj =
            dynamic_cast<ZDvidTileEnsemble*>(objList.first());
        if (obj->isVisible()) {
          m_currentZ = obj->getCurrentZ();
          m_hasCurrentZ = true;
        }
      }
    }

    if (!m_hasCurrentZ) {
      const TStackObjectList &objList =
          doc->getObjectList(ZStackObject::EType::DVID_LABEL_SLICE);
      if (!objList.isEmpty()) {
        ZDvidLabelSlice *obj = dynamic_cast<ZDvidLabelSlice*>(objList.first());
        if (obj->isVisible()) {
          m_currentZ = obj->getCurrentZ();
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

#if 0
ZIntCuboid ZStackDocHelper::GetVolumeBoundBox(const ZStackDoc *doc)
{
  ZIntCuboid box;
  if (doc != NULL) {
    if (doc->getTag() == neutu::Document::ETag::FLYEM_PROOFREAD) {
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
#endif

bool ZStackDocHelper::HasMultipleBodySelected(
    const ZFlyEmProofDoc *doc, neutu::ELabelSource type)
{
  return CountSelectedBody(doc, type) > 1;
}

int ZStackDocHelper::CountSelectedBody(
    const ZFlyEmProofDoc *doc, neutu::ELabelSource type)
{
  return doc->getSelectedBodySet(type).size();
}

bool ZStackDocHelper::HasBodySelected(const ZFlyEmProofDoc *doc)
{
  return CountSelectedBody(doc, neutu::ELabelSource::ORIGINAL) > 0;
}

void ZStackDocHelper::ClearBodySelection(ZFlyEmProofDoc *doc)
{
  QList<ZDvidLabelSlice*> sliceList = doc->getFrontDvidLabelSliceList();
  for (QList<ZDvidLabelSlice*>::iterator iter = sliceList.begin();
       iter != sliceList.end(); ++iter) {
    ZDvidLabelSlice *slice = *iter;
    if (slice != NULL) {
      slice->startSelection();
      slice->deselectAll();
      slice->endSelection();
      doc->processObjectModified(slice);
    }
  }
//  doc->clearBodyAnnotationMap();
  //    updateBodySelection();
  doc->notifyBodySelectionChanged();
}

#if 0
QColor ZStackDocHelper::GetBodyColor(
    const ZFlyEmProofDoc *doc, uint64_t bodyId)
{
  QColor color;
  ZDvidLabelSlice *labelSlice = doc->getDvidLabelSlice(neutu::EAxis::Z);
  if (labelSlice != NULL) {
    color = labelSlice->getLabelColor(bodyId, neutu::ELabelSource::ORIGINAL);
  }

  return color;
}
#endif

bool ZStackDocHelper::AllowingBodySplit(const ZStackDoc *doc)
{
  return doc->getTag() == neutu::Document::ETag::FLYEM_PROOFREAD;
}

bool ZStackDocHelper::AllowingBodyAnnotation(const ZStackDoc *doc)
{
  return doc->getTag() == neutu::Document::ETag::FLYEM_PROOFREAD ||
      doc->getTag() == neutu::Document::ETag::FLYEM_ORTHO;
}

bool ZStackDocHelper::AllowingBatchBodyAnnotation(const ZStackDoc *doc)
{
  const ZFlyEmProofDoc *pdoc = qobject_cast<const ZFlyEmProofDoc*>(doc);
  if (pdoc) {
    return pdoc->allowingBatchBodyAnnotation();
  }
  return false;
}

bool ZStackDocHelper::AllowingBodyMerge(const ZStackDoc *doc)
{
  return doc->getTag() == neutu::Document::ETag::FLYEM_PROOFREAD ||
      doc->getTag() == neutu::Document::ETag::FLYEM_ORTHO;
}

bool ZStackDocHelper::AllowingBodyLock(const ZStackDoc *doc)
{
  if (doc->getTag() == neutu::Document::ETag::FLYEM_PROOFREAD ||
      doc->getTag() == neutu::Document::ETag::FLYEM_ORTHO) {
    const ZFlyEmProofDoc *cdoc = qobject_cast<const ZFlyEmProofDoc*>(doc);
    if (cdoc->getSupervisor()) {
      return true;
    }
  }

  return false;
}

bool ZStackDocHelper::AllowingBodySelection(const ZFlyEmProofDoc *doc)
{
  return (doc->getTag() == neutu::Document::ETag::FLYEM_PROOFREAD) &&
      doc->hasSegmentation();
}
