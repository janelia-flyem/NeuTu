#ifndef ZFLYEMBODYANNOTATIONPROTOCOL_H
#define ZFLYEMBODYANNOTATIONPROTOCOL_H

#include <unordered_map>
#include <string>
#include <vector>
#include <set>
#include <QMap>

#include "zflyembodystatus.h"

class ZJsonObject;
class ZFlyEmBodyAnnotation;

class ZFlyEmBodyAnnotationProtocol
{
public:
  ZFlyEmBodyAnnotationProtocol();

  void loadJsonObject(const ZJsonObject &obj);
  ZJsonObject toJsonObject() const;

  bool isEmpty() const;
//  int getStatusRank() const;

  void reset();

  void print() const;

  const std::vector<ZFlyEmBodyStatus>& getStatusList() const;

  const ZFlyEmBodyStatus &getBodyStatus(const std::string &name) const;

  int getStatusRank(const std::string &status) const;
  bool isFinal(const std::string &status) const;
  bool isMergable(const std::string &status) const;
  bool isAdminAccessible(const std::string &status) const;
  bool isExpertStatus(const std::string &status) const;
  bool preservingId(const std::string &status) const;
  std::string getColorCode(const std::string &status) const;
//  bool hasConflict(const std::string &s1, const std::string &s2) const;

  std::vector<std::vector<uint64_t>> getConflictBody(
      const QMap<uint64_t, ZFlyEmBodyAnnotation> &annotMap) const;
  std::vector<std::vector<uint64_t>> getConflictBody(
      const QMap<uint64_t, ZJsonObject> &annotMap) const;
  std::vector<std::vector<uint64_t>> getConflictBody(
      const QMap<uint64_t, std::string> &statusMap) const;

  std::vector<std::vector<uint64_t>> getExclusionBody(
      const QMap<uint64_t, std::string> &statusMap) const;

private:
  static std::vector<std::vector<uint64_t>> MapBody(
      const QMap<uint64_t, std::string> &statusMap,
      const std::vector<std::set<std::string>> &groupList,
      bool selfConflict = true);

public:
  static const char* KEY_STATUS;
  static const char* KEY_CONFILICT;
  static const char* KEY_EXCLUSION;

private:
  std::vector<ZFlyEmBodyStatus> m_statusList;

  std::unordered_map<std::string, ZFlyEmBodyStatus> m_statusMap;
  std::vector<std::set<std::string>> m_conflictStatus;
  std::vector<std::set<std::string>> m_exclusionStatus;
  ZFlyEmBodyStatus m_emptyStatus;
};

#endif // ZFLYEMBODYANNOTATIONPROTOCOL_H
