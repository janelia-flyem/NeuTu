#include "zstackdochelper.h"

#include <string>
#include <QColor>

#include "zstackdoc.h"
#include "dvid/zdvidlabelslice.h"
#include "dvid/zdvidtileensemble.h"
#include "zstackview.h"
#include "zintcuboid.h"
#include "flyem/zflyemproofdoc.h"
#include "zintcuboidobj.h"
#include "dvid/zdvidsparsestack.h"
#include "zstack.hxx"

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
  if (doc->getTag() == neutube::Document::FLYEM_PROOFREAD) {
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

ZIntCuboid ZStackDocHelper::GetVolumeBoundBox(const ZStackDoc *doc)
{
  ZIntCuboid box;
  if (doc != NULL) {
    if (doc->getTag() == neutube::Document::FLYEM_PROOFREAD) {
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

ZIntCuboid ZStackDocHelper::GetStackSpaceRange(
    const ZStackDoc *doc, neutube::EAxis sliceAxis)
{
  ZIntCuboid box;

  if (doc->hasStack()) {
    box = doc->getStack()->getBoundBox();
    box.shiftSliceAxis(sliceAxis);
  }

  return box;
}

bool ZStackDocHelper::HasMultipleBodySelected(
    const ZFlyEmProofDoc *doc, neutube::EBodyLabelType type)
{
  return CountSelectedBody(doc, type) > 1;
}

int ZStackDocHelper::CountSelectedBody(
    const ZFlyEmProofDoc *doc, neutube::EBodyLabelType type)
{
  return doc->getSelectedBodySet(type).size();
}

bool ZStackDocHelper::HasBodySelected(const ZFlyEmProofDoc *doc)
{
  return CountSelectedBody(doc, neutube::BODY_LABEL_ORIGINAL) > 0;
}

void ZStackDocHelper::ClearBodySelection(ZFlyEmProofDoc *doc)
{
  QList<ZDvidLabelSlice*> sliceList = doc->getDvidLabelSliceList();
  for (QList<ZDvidLabelSlice*>::iterator iter = sliceList.begin();
       iter != sliceList.end(); ++iter) {
    ZDvidLabelSlice *slice = *iter;
    if (slice != NULL) {
      slice->recordSelection();
      slice->deselectAll();
      slice->processSelection();
    }
  }
  //    updateBodySelection();
  doc->notifyBodySelectionChanged();
}

QColor ZStackDocHelper::GetBodyColor(
    const ZFlyEmProofDoc *doc, uint64_t bodyId)
{
  QColor color;
  ZDvidLabelSlice *labelSlice = doc->getDvidLabelSlice(neutube::Z_AXIS);
  if (labelSlice != NULL) {
    color = labelSlice->getLabelColor(bodyId, neutube::BODY_LABEL_ORIGINAL);
  }

  return color;
}

std::string ZStackDocHelper::SaveStack(
    const ZStackDoc *doc, const std::string &path)
{
  std::string  resultPath;
  if (doc->hasStackData()) {
    resultPath = doc->getStack()->save(path);
  }

  return resultPath;
}
