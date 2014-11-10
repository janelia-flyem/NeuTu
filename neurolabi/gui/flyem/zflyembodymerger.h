#ifndef ZFLYEMBODYMERGER_H
#define ZFLYEMBODYMERGER_H

#include <QVector>
#include <QMap>
#include <QQueue>

#include "tz_stdint.h"

class ZFlyEmBodyMerger
{
public:
  ZFlyEmBodyMerger();
  typedef QMap<uint64_t, uint64_t> TLabelMap;
  typedef QVector<TLabelMap> TLableMapGroup;
  typedef QQueue<TLableMapGroup> TLabelMapStack;

private:
  TLabelMapStack m_mapStack;
  TLabelMapStack m_undoneMapStack;

};

#endif // ZFLYEMBODYMERGER_H
