#include "zflyembodyannotationprotocol.h"

#include <iostream>
#include <algorithm>

#include "zjsonobject.h"
#include "zjsonarray.h"
#include "zjsonparser.h"
#include "zflyembodyannotation.h"
#include "zstring.h"
#include "zgraph.h"

const char* ZFlyEmBodyAnnotationProtocol::KEY_STATUS = "status";
const char* ZFlyEmBodyAnnotationProtocol::KEY_CONFILICT = "conflict";
const char* ZFlyEmBodyAnnotationProtocol::KEY_EXCLUSION = "exclusion";

ZFlyEmBodyAnnotationProtocol::ZFlyEmBodyAnnotationProtocol()
{
}

void ZFlyEmBodyAnnotationProtocol::reset()
{
  m_statusList.clear();
  m_statusMap.clear();
  m_conflictStatus.clear();
  m_exclusionStatus.clear();
}

bool ZFlyEmBodyAnnotationProtocol::isEmpty() const
{
  return m_statusList.empty();
}

ZJsonObject ZFlyEmBodyAnnotationProtocol::toJsonObject() const
{
  ZJsonObject obj;

  ZJsonArray statusArray;
  for (const ZFlyEmBodyStatus &status : m_statusList) {
    statusArray.append(status.toJsonObject());
  }
  obj.setEntry(KEY_STATUS, statusArray);

  ZJsonArray conflictArray;
  for (const auto &bodySet : m_conflictStatus) {
    ZJsonArray bodyArray;
    for (const std::string& body : bodySet) {
      bodyArray.append(body);
    }
    if (!bodyArray.isEmpty()) {
      conflictArray.append(bodyArray);
    }
  }
  obj.setEntry(KEY_CONFILICT, conflictArray);

  ZJsonArray exclusionArray;
  for (const auto &bodySet : m_exclusionStatus) {
    ZJsonArray bodyArray;
    for (const std::string& body : bodySet) {
      bodyArray.append(body);
    }
    if (!bodyArray.isEmpty()) {
      conflictArray.append(bodyArray);
    }
  }
  obj.setEntry(KEY_EXCLUSION, exclusionArray);

  return obj;
}

void ZFlyEmBodyAnnotationProtocol::loadJsonObject(const ZJsonObject &statusJson)
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

  if (statusJson.hasKey(KEY_EXCLUSION)) {
    ZJsonArray arrayJson(statusJson.value(KEY_EXCLUSION));
    for (size_t i = 0; i < arrayJson.size(); ++i) {
      ZJsonArray conflictJson(arrayJson.value(i));
      std::set<std::string> conflict;
      for (size_t j = 0; j < conflictJson.size(); ++j) {
        conflict.insert(
              ZString(ZJsonParser().getValue<std::string>(
                        conflictJson.at(j))).lower());
      }
      if (!conflict.empty()) {
        m_exclusionStatus.push_back(conflict);
      }
    }
  }
}

const ZFlyEmBodyStatus& ZFlyEmBodyAnnotationProtocol::getBodyStatus(
    const std::string &name) const
{
  std::string nameKey = ZString(name).lower();

  if (m_statusMap.count(nameKey) > 0) {
    return m_statusMap.at(nameKey);
  }

  return m_emptyStatus;
}

void ZFlyEmBodyAnnotationProtocol::print() const
{
  toJsonObject().print();
}

const std::vector<ZFlyEmBodyStatus>& ZFlyEmBodyAnnotationProtocol::getStatusList() const
{
  return m_statusList;
}

int ZFlyEmBodyAnnotationProtocol::getStatusRank(const std::string &status) const
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

bool ZFlyEmBodyAnnotationProtocol::isFinal(const std::string &status) const
{
  return getBodyStatus(status).isFinal();
}

bool ZFlyEmBodyAnnotationProtocol::isMergable(const std::string &status) const
{
  return getBodyStatus(status).isMergable();
}

bool ZFlyEmBodyAnnotationProtocol::isAdminAccessible(const std::string &status) const
{
  return getBodyStatus(status).isAdminAccessible();
}

bool ZFlyEmBodyAnnotationProtocol::isExpertStatus(const std::string &status) const
{
  return getBodyStatus(status).isExpertStatus();
}

bool ZFlyEmBodyAnnotationProtocol::preservingId(const std::string &status) const
{
  return getBodyStatus(status).presevingId();
}

std::string ZFlyEmBodyAnnotationProtocol::getColorCode(
    const std::string &status) const
{
  return getBodyStatus(status).getColorCode();
}

std::vector<std::vector<uint64_t>> ZFlyEmBodyAnnotationProtocol::MapBody(
    const QMap<uint64_t, std::string> &statusMap,
    const std::vector<std::set<std::string>> &groupList, bool selfConflict)
{
  std::vector<uint64_t> bodyIdList(statusMap.size());
  size_t index = 0;
  for (auto iter = statusMap.constBegin(); iter != statusMap.constEnd(); ++iter) {
    bodyIdList[index++] = iter.key();
  }

  ZGraph graph;

  for (size_t i = 0; i < bodyIdList.size(); ++i) {
    for (size_t j = i + 1; j < bodyIdList.size(); ++j) {
      std::string s1 = ZString(statusMap[bodyIdList[i]]).lower();
      std::string s2 = ZString(statusMap[bodyIdList[j]]).lower();
      for (const auto &group : groupList) {
        bool conflict = false;
        if (s1 != s2) {
          if (group.count(s1) > 0 && group.count(s2) > 0) {
            conflict = true;
          }
        } else {
          conflict = (selfConflict || group.size() == 1) && (group.count(s1) > 0);
        }
        if (conflict) {
          graph.addEdge(i, j);
          break;
        }
      }
    }
  }

#ifdef _DEBUG_0
  std::cout << "Conflict graph: " << std::endl;
  graph.print();
#endif

  const std::vector<ZGraph*>& subgraphs = graph.getConnectedSubgraph();
  std::vector<std::vector<uint64_t>> groupBodies(subgraphs.size());
  for (size_t i = 0; i < subgraphs.size(); ++i) {
    std::vector<uint64_t> &group = groupBodies[i];
    ZGraph *subgraph = subgraphs[i];
    std::set<int> vertexSet = subgraph->getConnectedVertexSet();
    for (int v : vertexSet) {
      group.push_back(bodyIdList[v]);
    }
  }

  return groupBodies;

  /*
  for (auto iter = statusMap.constBegin(); iter != statusMap.constEnd(); ++iter) {
    uint64_t bodyId = iter.key();
    const std::string status = iter.value();

    for (size_t i = 0; i < groupList.size(); ++i) {
      const auto &statusSet = groupList[i];
      if (statusSet.count(ZString(status).lower()) > 0) {
        groupBodies[i].push_back(bodyId);
      }
    }
  }

  std::set<std::string> selfConflictSet;
  for (const auto &group : groupList) {
    if (group.size() == 1) {
      selfConflictSet.insert(*group.begin());
    }
  }

  std::vector<std::vector<uint64_t>> finalGroupBodies;
  for (auto &group : groupBodies) {
    if (group.size() > 1) {
      if (!selfConflict) {
        std::set<std::string> statusSet;
        for (auto body : group) {
          statusSet.insert(ZString(statusMap[body]).lower());
        }
        if (statusSet.size() > 1) {
          finalGroupBodies.push_back(group);
        } else if (selfConflictSet.count(*statusSet.begin()) > 0) {
          finalGroupBodies.push_back(group);
        }
      } else {
        finalGroupBodies.push_back(group);
      }
    }
  }

  return finalGroupBodies;
  */
}

std::vector<std::vector<uint64_t>> ZFlyEmBodyAnnotationProtocol::getConflictBody(
    const QMap<uint64_t, ZFlyEmBodyAnnotation> &annotMap) const
{
  QMap<uint64_t, std::string> statusMap;
  for (auto iter = annotMap.constBegin(); iter != annotMap.constEnd(); ++iter) {
    uint64_t bodyId = iter.key();
    const auto &anno = iter.value();
    statusMap[bodyId] = anno.getStatus();
  }

  return getConflictBody(statusMap);
  /*
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
  */
}

std::vector<std::vector<uint64_t>> ZFlyEmBodyAnnotationProtocol::getConflictBody(
    const QMap<uint64_t, ZJsonObject> &annotMap) const
{
  QMap<uint64_t, std::string> statusMap;
  for (auto iter = annotMap.constBegin(); iter != annotMap.constEnd(); ++iter) {
    uint64_t bodyId = iter.key();
    const ZJsonObject &anno = iter.value();
    statusMap[bodyId] = ZFlyEmBodyAnnotation::GetStatus(anno);
  }

  return getConflictBody(statusMap);
/*
  std::vector<std::vector<uint64_t>> potentialConflict(m_conflictStatus.size());

  for (auto iter = annotMap.constBegin(); iter != annotMap.constEnd(); ++iter) {
    uint64_t bodyId = iter.key();
    const ZJsonObject &anno = iter.value();

    for (size_t i = 0; i < m_conflictStatus.size(); ++i) {
      const auto &conflictSet = m_conflictStatus[i];
      if (conflictSet.count(
            ZString(ZFlyEmBodyAnnotation::GetStatus(anno)).lower()) > 0) {
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
  */
}

std::vector<std::vector<uint64_t>>
ZFlyEmBodyAnnotationProtocol::getConflictBody(
    const QMap<uint64_t, std::string> &statusMap) const
{
  return MapBody(statusMap, m_conflictStatus);
}

std::vector<std::vector<uint64_t>>
ZFlyEmBodyAnnotationProtocol::getExclusionBody(
    const QMap<uint64_t, std::string> &statusMap) const
{
  return MapBody(statusMap, m_exclusionStatus, false);
}
