#include "zflyembodyannotationmerger.h"

#include <iostream>
#include <algorithm>

#include "zjsonobject.h"
#include "zjsonarray.h"
#include "zjsonparser.h"
#include "zflyembodyannotation.h"
#include "zstring.h"

ZFlyEmBodyAnnotationMerger::ZFlyEmBodyAnnotationMerger()
{
}

void ZFlyEmBodyAnnotationMerger::reset()
{
  m_statusList.clear();
  m_statusMap.clear();
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
    m_statusMap[ZString(status.getName()).lower()] = status;
  }

  if (statusJson.hasKey("conflict")) {
    ZJsonArray arrayJson(statusJson.value("conflict"));
    for (size_t i = 0; i < arrayJson.size(); ++i) {
      ZJsonArray conflictJson(arrayJson.value(i));
      std::set<std::string> conflict;
      for (size_t j = 0; j < conflictJson.size(); ++j) {
        conflict.insert(
              ZString(ZJsonParser().getValue<std::string>(
                        conflictJson.at(j))).lower());
      }
      if (!conflict.empty()) {
        m_conflictStatus.push_back(conflict);
      }
    }
  }
}

const ZFlyEmBodyStatus& ZFlyEmBodyAnnotationMerger::getBodyStatus(
    const std::string &name) const
{
  std::string nameKey = ZString(name).lower();

  if (m_statusMap.count(nameKey) > 0) {
    return m_statusMap.at(nameKey);
  }

  return m_emptyStatus;
}

void ZFlyEmBodyAnnotationMerger::print() const
{
  std::cout << "Statuses: " << std::endl;
  for (const auto &status : m_statusList) {
    std::cout << "  ";
    status.print();
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

  return getBodyStatus(status).getPriority();

  /*
  std::string statusLowerCase = status;
  std::transform(statusLowerCase.begin(), statusLowerCase.end(),
                 statusLowerCase.begin(), ::tolower);

  if (m_statusRank.count(status) > 0) {
    return m_statusRank.at(status);
  }

  return 999;
  */
}

bool ZFlyEmBodyAnnotationMerger::isFinal(const std::string &status) const
{
  return getBodyStatus(status).isFinal();
}

bool ZFlyEmBodyAnnotationMerger::isMergable(const std::string &status) const
{
  return getBodyStatus(status).isMergable();
}

bool ZFlyEmBodyAnnotationMerger::isAdminAccessible(const std::string &status) const
{
  return getBodyStatus(status).isAdminAccessible();
}

bool ZFlyEmBodyAnnotationMerger::isExpertStatus(const std::string &status) const
{
  return getBodyStatus(status).isExpertStatus();
}

std::vector<std::vector<uint64_t>> ZFlyEmBodyAnnotationMerger::getConflictBody(
    const QMap<uint64_t, ZFlyEmBodyAnnotation> &annotMap) const
{
  std::vector<std::vector<uint64_t>> potentialConflict(m_conflictStatus.size());

  for (auto iter = annotMap.constBegin(); iter != annotMap.constEnd(); ++iter) {
    uint64_t bodyId = iter.key();
    const ZFlyEmBodyAnnotation &anno = iter.value();

    for (size_t i = 0; i < m_conflictStatus.size(); ++i) {
      const auto &conflictSet = m_conflictStatus[i];
      if (conflictSet.count(ZString(anno.getStatus()).lower()) > 0) {
        potentialConflict[i].push_back(bodyId);
        break;
      }
    }
  }

  for (auto &conflict : potentialConflict) {
    if (conflict.size() == 1) {
      conflict.clear();
    }
  }

  return potentialConflict;
}
