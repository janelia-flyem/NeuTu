#ifndef ZKEYEVENTSWCMAPPER_H
#define ZKEYEVENTSWCMAPPER_H

#include <QKeyEvent>
#include <QMap>
#include "zswctree.h"

class ZKeyEventSwcMapper
{
public:
  ZKeyEventSwcMapper();

  ZSwcTree::EOperation getOperation(QKeyEvent *event);

private:
  void initKeyMap();

private:
  QMap<int, ZSwcTree::EOperation> m_plainKeyMap;
  QMap<int, ZSwcTree::EOperation> m_controlKeyMap;
  QMap<int, ZSwcTree::EOperation> m_altKeyMap;
  QMap<int, ZSwcTree::EOperation> m_shiftKeyMap;
};

#endif // ZKEYEVENTSWCMAPPER_H
