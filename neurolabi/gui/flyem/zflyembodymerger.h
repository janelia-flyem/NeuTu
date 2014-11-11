#ifndef ZFLYEMBODYMERGER_H
#define ZFLYEMBODYMERGER_H

#include <QVector>
#include <QMap>
#include <QQueue>
#include <QList>
#include <QStack>

#include "tz_stdint.h"

/*!
 * \brief The ZFlyEmBodyMerger class
 *
 * label 0 is treated as null.
 */
class ZFlyEmBodyMerger
{
public:
  ZFlyEmBodyMerger();
  typedef QMap<uint64_t, uint64_t> TLabelMap;
//  typedef QVector<TLabelMap> TLabelMapGroup;
  typedef QList<TLabelMap> TLabelMapList;
  typedef QStack<TLabelMap> TLabelMapStack;

  uint64_t getFinalLabel(uint64_t label);

  void pushMap(uint64_t label1, uint64_t label2);
  void pushMap(const TLabelMap &map);
  void undo();
  void redo();

  void print() const;

private:
  static uint64_t mapLabel(const TLabelMap &labelMap, uint64_t label);
  static uint64_t mapLabel(const TLabelMapList &labelMap, uint64_t label);

private:
  TLabelMapList m_mapList;
  TLabelMapStack m_undoneMapStack;

};

#endif // ZFLYEMBODYMERGER_H
