#ifndef ZFLYEMBODYMERGER_H
#define ZFLYEMBODYMERGER_H

#include <QVector>
#include <QMap>
#include <QQueue>
#include <QList>
#include <QStack>
#include <QSet>

#include "tz_stdint.h"

class ZJsonObject;
class ZJsonArray;

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
  typedef QSet<uint64_t> TLabelSet;
//  typedef QVector<TLabelMap> TLabelMapGroup;
  typedef QList<TLabelMap> TLabelMapList;
  typedef QStack<TLabelMap> TLabelMapStack;

  uint64_t getFinalLabel(uint64_t label) const;
  TLabelMap getFinalMap() const;

  void pushMap(uint64_t label1, uint64_t label2);
  void pushMap(const TLabelMap &map);
  void pushMap(const TLabelSet &labelSet);
  void undo();
  void redo();

  void print() const;
  void clear();

  bool isMapped(uint64_t label) const;

//  ZJsonObject toJsonObject() const;
  ZJsonArray toJsonArray() const;
//  void loadJsonObject(const ZJsonObject &obj);
  void loadJson(const ZJsonArray &obj);

  std::string toJsonString() const;
  void decodeJsonString(const std::string &str);

private:
  static uint64_t mapLabel(const TLabelMap &labelMap, uint64_t label);
  static uint64_t mapLabel(const TLabelMapList &labelMap, uint64_t label);

private:
  TLabelMapList m_mapList;
  TLabelMapStack m_undoneMapStack;

};

#endif // ZFLYEMBODYMERGER_H
