#include "zflyembodymergedoc.h"
#include <QMap>
#include <QColor>
#include "zobject3dscan.h"
#include "zarray.h"
#include "zarrayfactory.h"

ZFlyEmBodyMergeDoc::ZFlyEmBodyMergeDoc(ZStack *stack, QObject *parent) :
  ZStackDoc(stack, parent), m_originalLabel(NULL)
{
}

ZFlyEmBodyMergeDoc::~ZFlyEmBodyMergeDoc()
{
  delete m_originalLabel;
}

QList<ZObject3dScan*> ZFlyEmBodyMergeDoc::extractAllObject()
{
  QList<ZObject3dScan*> objList;

  if (m_originalLabel == NULL) {
    return objList;
  }

  if (m_originalLabel->isEmpty()) {
    return objList;
  }

  ZFlyEmBodyMerger::TLabelMap finalMap = m_bodyMerger.getFinalMap();

  std::map<uint64_t, ZObject3dScan*> bodySet;
  uint64_t *array = m_originalLabel->getDataPointer<uint64_t>();
  int width = m_originalLabel->getDim(0);
  int height = m_originalLabel->getDim(1);
  int depth = m_originalLabel->getDim(2);
  int x0 = m_originalLabel->getStartCoordinate(0);
  int y0 = m_originalLabel->getStartCoordinate(1);
  int z0 = m_originalLabel->getStartCoordinate(2);


  ZObject3dScan *obj = NULL;
  for (int z = 0; z < depth; ++z) {
    for (int y = 0; y < height; ++y) {
      int x = 0;
      while (x < width) {
        uint64_t v = array[x];
        if (finalMap.contains(v)) {
          v = finalMap[v];
        }
        std::map<uint64_t, ZObject3dScan*>::iterator iter = bodySet.find(v);
        if (iter == bodySet.end()) {
          obj = new ZObject3dScan;
          obj->setLabel(v);
          //(*bodySet)[v] = obj;
          bodySet.insert(std::map<uint64_t, ZObject3dScan*>::value_type(v, obj));
        } else {
          obj = iter->second;
        }
        int length = obj->scanArray(array, x, y + y0, z + z0, width, x0);
        x += length;
      }
      array += width;
    }
  }

  for (std::map<uint64_t, ZObject3dScan*>::iterator iter = bodySet.begin();
       iter != bodySet.end(); ++iter) {
    ZObject3dScan *obj = iter->second;
    obj->setRole(ZStackObjectRole::ROLE_SEGMENTATION);
    QColor color = m_objColorSheme.getColor(abs((int) obj->getLabel()));
    color.setAlpha(64);
    obj->setColor(color);
    objList.append(obj);
  }

  return objList;
}

void ZFlyEmBodyMergeDoc::updateBodyObject()
{
  removeObject(ZStackObjectRole::ROLE_SEGMENTATION, true);
  if (m_originalLabel != NULL) {
    QList<ZObject3dScan*> objList = extractAllObject();
    foreach (ZObject3dScan *obj, objList) {
      addObject(obj);
    }
  }
}

void ZFlyEmBodyMergeDoc::mergeSelected()
{
  TStackObjectSet m_objSet =
      getObjectGroup().getSelectedSet(ZStackObject::TYPE_OBJECT3D_SCAN);
  ZFlyEmBodyMerger::TLabelSet labelSet;
  for (TStackObjectSet::const_iterator iter = m_objSet.begin();
       iter != m_objSet.end(); ++iter) {
    const ZObject3dScan *obj = dynamic_cast<const ZObject3dScan*>(*iter);
    if (obj->getLabel() > 0) {
      labelSet.insert(obj->getLabel());
    }
  }
  m_bodyMerger.pushMap(labelSet);
  m_bodyMerger.undo();

  ZFlyEmBodyMergerDocCommand::MergeBody *command =
      new ZFlyEmBodyMergerDocCommand::MergeBody(this);
  pushUndoCommand(command);

  //return true;

  //updateBodyObject();
}

void ZFlyEmBodyMergeDoc::setOriginalLabel(const ZStack *stack)
{
  updateOriginalLabel(ZArrayFactory::MakeArray(stack));
}

void ZFlyEmBodyMergeDoc::updateOriginalLabel(ZArray *array)
{
  delete m_originalLabel;
  m_originalLabel = array;
  updateBodyObject();
}


ZFlyEmBodyMergerDocCommand::MergeBody::MergeBody(
    ZStackDoc *doc, QUndoCommand *parent)
  : ZUndoCommand(parent), m_doc(doc)
{

}

ZFlyEmBodyMergeDoc* ZFlyEmBodyMergerDocCommand::MergeBody::getCompleteDocument()
{
  return dynamic_cast<ZFlyEmBodyMergeDoc*>(m_doc);
}

void ZFlyEmBodyMergerDocCommand::MergeBody::redo()
{
  getCompleteDocument()->getBodyMerger()->redo();
  getCompleteDocument()->updateBodyObject();
}

void ZFlyEmBodyMergerDocCommand::MergeBody::undo()
{
  getCompleteDocument()->getBodyMerger()->undo();
  getCompleteDocument()->updateBodyObject();
}
