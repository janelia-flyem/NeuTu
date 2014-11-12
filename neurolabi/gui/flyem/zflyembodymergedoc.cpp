#include "zflyembodymergedoc.h"
#include <QMap>
#include <QColor>
#include "zobject3dscan.h"
#include "zarray.h"

ZFlyEmBodyMergeDoc::ZFlyEmBodyMergeDoc(ZStack *stack, QObject *parent) :
  ZStackDoc(stack, parent), m_originalLabel(NULL)
{
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
    QColor color = m_objColorSheme.getColor(abs((int) obj->getLabel()));
    color.setAlpha(64);
    obj->setColor(color);
    objList.append(obj);
  }

  return objList;
}

void ZFlyEmBodyMergeDoc::updateBodyObject()
{
  if (m_originalLabel != NULL) {
    QList<ZObject3dScan*> objList = extractAllObject();
    foreach (ZObject3dScan *obj, objList) {
      addObject(obj);
    }
  }
}
