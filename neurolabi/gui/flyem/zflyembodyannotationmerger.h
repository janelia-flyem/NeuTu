#ifndef ZFLYEMBODYANNOTATIONMERGER_H
#define ZFLYEMBODYANNOTATIONMERGER_H

#include <unordered_map>
#include <string>
#include <vector>
#include <set>

#include "zflyembodystatus.h"

class ZJsonObject;

class ZFlyEmBodyAnnotationMerger
{
public:
  ZFlyEmBodyAnnotationMerger();

  void loadJsonObject(const ZJsonObject &obj);

  bool isEmpty() const;
//  int getStatusRank() const;

  void reset();

  void print();

  const std::vector<ZFlyEmBodyStatus>& getStatusList() const;

  int getStatusRank(const std::string &status) const;

private:
  std::vector<ZFlyEmBodyStatus> m_statusList;

  std::unordered_map<std::string, int> m_statusRank;
  std::vector<std::set<std::string>> m_conflictStatus;
};

#endif // ZFLYEMBODYANNOTATIONMERGER_H
