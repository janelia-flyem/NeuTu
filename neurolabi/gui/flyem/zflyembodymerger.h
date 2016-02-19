#ifndef ZFLYEMBODYMERGER_H
#define ZFLYEMBODYMERGER_H

#include <QVector>
#include <QMap>
#include <QQueue>
#include <QList>
#include <QStack>
#include <QSet>
#include <set>

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
  std::set<uint64_t> getFinalLabel(const std::set<uint64_t> labelSet) const;
  template <typename InputIterator>
  std::set<uint64_t> getFinalLabel(InputIterator begin, InputIterator end) const;

  TLabelMap getFinalMap() const;

  void pushMap(uint64_t label1, uint64_t label2);
  void pushMap(const TLabelMap &map);
  void pushMap(const TLabelSet &labelSet);
  TLabelMap undo();
  void redo();

  void print() const;
  void clear();

  bool isMerged(uint64_t label) const;

//  ZJsonObject toJsonObject() const;
  ZJsonArray toJsonArray() const;
//  void loadJsonObject(const ZJsonObject &obj);
  void loadJson(const ZJsonArray &obj);

  std::string toJsonString() const;
  void decodeJsonString(const std::string &str);

  QList<uint64_t> getOriginalLabelList(uint64_t finalLabel) const;
  QSet<uint64_t> getOriginalLabelSet(uint64_t finalLabel) const;
  template <typename InputIterator>
  QSet<uint64_t> getOriginalLabelSet(InputIterator begin,
                                     InputIterator end) const;

  bool isEmpty() const;

private:
  static uint64_t mapLabel(const TLabelMap &labelMap, uint64_t label);
  static uint64_t mapLabel(const TLabelMapList &labelMap, uint64_t label);

private:
  TLabelMapList m_mapList;
  TLabelMapStack m_undoneMapStack;
};

template <typename InputIterator>
QSet<uint64_t> ZFlyEmBodyMerger::getOriginalLabelSet(
    InputIterator begin, InputIterator end) const
{
  QSet<uint64_t> labelSet;
  for (InputIterator iter = begin; iter != end; ++iter) {
    uint64_t label = *iter;
    labelSet.unite(getOriginalLabelSet(label));
  }

  return labelSet;
}

template <typename InputIterator>
std::set<uint64_t> ZFlyEmBodyMerger::getFinalLabel(
    InputIterator begin, InputIterator end) const
{
  std::set<uint64_t> labelSet;
  for (InputIterator iter = begin; iter != end; ++iter) {
    labelSet.insert(getFinalLabel(*iter));
  }

  return labelSet;
}


#endif // ZFLYEMBODYMERGER_H
