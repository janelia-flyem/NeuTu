#ifndef ZKEYEVENTSTROKEMAPPER_H
#define ZKEYEVENTSTROKEMAPPER_H

#include <QMap>
#include "zstroke2d.h"
#include "neutube.h"

class QKeyEvent;

class ZKeyEventStrokeMapper
{
public:
  ZKeyEventStrokeMapper(neutube::Document::ETag tag);

  ZStroke2d::EOperation getOperation(QKeyEvent *event);
  void setTag(neutube::Document::ETag tag);

private:
  void initKeyMap();
  void updateKeyMap();

private:
  QMap<int, ZStroke2d::EOperation> m_plainKeyMap;
  QMap<int, ZStroke2d::EOperation> m_controlKeyMap;
  QMap<int, ZStroke2d::EOperation> m_altKeyMap;
  QMap<int, ZStroke2d::EOperation> m_shiftKeyMap;

  neutube::Document::ETag m_docTag;

};

#endif // ZKEYEVENTSTROKEMAPPER_H
