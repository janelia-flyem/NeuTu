#include "zflyembodyannotationprotocol.h"

#include <iostream>
#include <algorithm>

#include "zjsonobject.h"
#include "zjsonarray.h"
#include "zjsonparser.h"
#include "zflyembodyannotation.h"
#include "zstring.h"

const char* ZFlyEmBodyAnnotationProtocal::KEY_STATUS = "status";
const char* ZFlyEmBodyAnnotationProtocal::KEY_CONFILICT = "conflict";

ZFlyEmBodyAnnotationProtocal::ZFlyEmBodyAnnotationProtocal()
{
}

void ZFlyEmBodyAnnotationProtocal::reset()
{
  m_statusList.clear();
  m_statusMap.clear();
  m_conflictStatus.clear();
}

bool ZFlyEmBodyAnnotationProtocal::isEmpty() const
{
  return m_statusList.empty();
}

ZJsonObject ZFlyEmBodyAnnotationProtocal::toJsonObject() const
{
  ZJsonObject obj;

  ZJsonArray statusArray;
  for (const ZFlyEmBodyStatus &status : m_statusList) {
    statusArray.append(status.toJsonObject());
  }
  obj.setEntry(KEY_STATUS, statusArray);

  ZJsonArray conflictArray;
  for (auto bodySet : m_conflictStatus) {
    ZJsonArray bodyArray;
    for (const std::string& body : bodySet) {
      bodyArray.append(body);
    }
    if (!bodyArray.isEmpty()) {
      conflictArray.append(bodyArray);
    }
  }

  obj.setEntry(KEY_CONFILICT, conflictArray);

  return obj;
}

void ZFlyEmBodyAnnotationProtocal::loadJsonObject(const ZJsonObject &statusJson)
{
  reset();

  ZJsonArray statusListJson(statusJson.value(KEY_STATUS));

  for (size_t i = 0; i < statusListJson.size(); ++i) {
    ZFlyEmBodyStatus status;
    status.loadJsonObject(ZJsonObject(statusListJson.value(i)));
    m_statusList.push_back(status);
    m_statusMap[ZString(status.getName()).lower()] = status;
  }

  if (statusJson.hasKey(KEY_CONFILICT)) {
    ZJsonArray arrayJson(statusJson.value(KEY_CONFILICT));
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

const ZFlyEmBodyStatus& ZFlyEmBodyAnnotationProtocal::getBodyStatus(
    const std::string &name) const
{
  std::string nameKey = ZString(name).lower();

  if (m_statusMap.count(nameKey) > 0) {
    return m_statusMap.at(nameKey);
  }

  return m_emptyStatus;
}

void ZFlyEmBodyAnnotationProtocal::print() const
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

const std::vector<ZFlyEmBodyStatus>& ZFlyEmBodyAnnotationProtocal::getStatusList() const
{
  return m_statusList;
}

int ZFlyEmBodyAnnotationProtocal::getStatusRank(const std::string &status) const
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

bool ZFlyEmBodyAnnotationProtocal::isFinal(const std::string &status) const
{
  return getBodyStatus(status).isFinal();
}

bool ZFlyEmBodyAnnotationProtocal::isMergable(const std::string &status) const
{
  return getBodyStatus(status).isMergable();
}

bool ZFlyEmBodyAnnotationProtocal::isAdminAccessible(const std::string &status) const
{
  return getBodyStatus(status).isAdminAccessible();
}

bool ZFlyEmBodyAnnotationProtocal::isExpertStatus(const std::string &status) const
{
  return getBodyStatus(status).isExpertStatus();
}

bool ZFlyEmBodyAnnotationProtocal::preservingId(const std::string &status) const
{
  return getBodyStatus(status).presevingId();
}

std::string ZFlyEmBodyAnnotationProtocal::getColorCode(
    const std::string &status) const
{
  return getBodyStatus(status).getColorCode();
}

std::vector<std::vector<uint64_t>> ZFlyEmBodyAnnotationProtocal::getConflictBody(
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
