#ifndef ZFLYEMBODYMERGER_H
#define ZFLYEMBODYMERGER_H

#include <QVector>
#include <QMap>
#include <QQueue>

class ZFlyEmBodyMerger
{
public:
  ZFlyEmBodyMerger();
  typedef QMap<uint64_t, uint64_t> TLabelMap;
  typedef QVector<TLabelMap> TLableMapGroup;
  typedef QQueue<TLableMapGroup> TLabelMapStack;

private:

};

#endif // ZFLYEMBODYMERGER_H
