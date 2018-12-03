#include "zflyembodyannotationmerger.h"

#include <iostream>
#include <algorithm>

#include "zjsonobject.h"
#include "zjsonarray.h"
#include "zjsonparser.h"

ZFlyEmBodyAnnotationMerger::ZFlyEmBodyAnnotationMerger()
{
}

void ZFlyEmBodyAnnotationMerger::reset()
{
  m_statusList.clear();
  m_statusRank.clear();
  m_conflictStatus.clear();
}

bool ZFlyEmBodyAnnotationMerger::isEmpty() const
{
  return m_statusList.empty();
}

void ZFlyEmBodyAnnotationMerger::loadJsonObject(const ZJsonObject &statusJson)
{
  reset();

  ZJsonArray statusListJson(statusJson.value("status"));

  for (size_t i = 0; i < statusListJson.size(); ++i) {
    ZFlyEmBodyStatus status;
    status.loadJsonObject(ZJsonObject(statusListJson.value(i)));
    m_statusList.push_back(status);
    m_statusRank[status.getName()] = status.getPriority();
  }

  if (statusJson.hasKey("conflict")) {
    ZJsonArray arrayJson(statusJson.value("conflict"));
    for (size_t i = 0; i < arrayJson.size(); ++i) {
      ZJsonArray conflictJson(arrayJson.value(i));
      std::set<std::string> conflict;
      for (size_t j = 0; j < conflictJson.size(); ++j) {
        conflict.insert(ZJsonParser().getValue<std::string>(conflictJson.at(j)));
      }
      if (!conflict.empty()) {
        m_conflictStatus.push_back(conflict);
      }
    }
  }
}

void ZFlyEmBodyAnnotationMerger::print()
{
  std::cout << "Statuses: " << std::endl;
  for (const auto &rank : m_statusRank) {
    std::cout << "  " << rank.first << ": " << rank.second << std::endl;
  }

  std::cout << "Conflict:" << std::endl;
  for (const auto &confSet : m_conflictStatus) {
    for (const std::string &status : confSet) {
      std::cout << "  " << status << std::endl;
    }
  }
}

const std::vector<ZFlyEmBodyStatus>& ZFlyEmBodyAnnotationMerger::getStatusList() const
{
  return m_statusList;
}

int ZFlyEmBodyAnnotationMerger::getStatusRank(const std::string &status) const
{
  if (status.empty()) {
    return 9999;
  }

  std::string statusLowerCase = status;
  std::transform(statusLowerCase.begin(), statusLowerCase.end(),
                 statusLowerCase.begin(), ::tolower);

  if (m_statusRank.count(status) > 0) {
    return m_statusRank.at(status);
  }

  return 999;
}
