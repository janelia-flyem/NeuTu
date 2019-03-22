#ifndef ZKEYEVENTSWCMAPPER_H
#define ZKEYEVENTSWCMAPPER_H

#include <QMap>
#include "zswctree.h"
#include "neutube.h"

class QKeyEvent;

class ZKeyEventSwcMapper
{
public:
  ZKeyEventSwcMapper(neutu::Document::ETag tag = neutu::Document::ETag::NORMAL);

  ZSwcTree::EOperation getOperation(QKeyEvent *event);
  void setTag(neutu::Document::ETag tag);

private:
  void initKeyMap();
  void updateKeyMap();

private:
  QMap<int, ZSwcTree::EOperation> m_plainKeyMap;
  QMap<int, ZSwcTree::EOperation> m_controlKeyMap;
  QMap<int, ZSwcTree::EOperation> m_altKeyMap;
  QMap<int, ZSwcTree::EOperation> m_shiftKeyMap;

  neutu::Document::ETag m_docTag;
};

#endif // ZKEYEVENTSWCMAPPER_H
