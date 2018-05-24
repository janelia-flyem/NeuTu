#ifndef ZKEYEVENTSWCMAPPER_H
#define ZKEYEVENTSWCMAPPER_H

#include <QMap>
#include "zswctree.h"
#include "neutube.h"

class QKeyEvent;

class ZKeyEventSwcMapper
{
public:
  ZKeyEventSwcMapper(neutube::Document::ETag tag = neutube::Document::NORMAL);

  ZSwcTree::EOperation getOperation(QKeyEvent *event);
  void setTag(neutube::Document::ETag tag);

private:
  void initKeyMap();
  void updateKeyMap();

private:
  QMap<int, ZSwcTree::EOperation> m_plainKeyMap;
  QMap<int, ZSwcTree::EOperation> m_controlKeyMap;
  QMap<int, ZSwcTree::EOperation> m_altKeyMap;
  QMap<int, ZSwcTree::EOperation> m_shiftKeyMap;

  neutube::Document::ETag m_docTag;
};

#endif // ZKEYEVENTSWCMAPPER_H
