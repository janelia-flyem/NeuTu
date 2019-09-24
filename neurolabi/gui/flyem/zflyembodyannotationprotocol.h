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

class ZFlyEmBodyAnnotationProtocal
{
public:
  ZFlyEmBodyAnnotationProtocal();

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
//  bool hasConflict(const std::string &s1, const std::string &s2) const;

  std::vector<std::vector<uint64_t>> getConflictBody(
      const QMap<uint64_t, ZFlyEmBodyAnnotation> &annotMap) const;

public:
  static const char* KEY_STATUS;
  static const char* KEY_CONFILICT;

private:
  std::vector<ZFlyEmBodyStatus> m_statusList;

  std::unordered_map<std::string, ZFlyEmBodyStatus> m_statusMap;
  std::vector<std::set<std::string>> m_conflictStatus;
  ZFlyEmBodyStatus m_emptyStatus;
};

#endif // ZFLYEMBODYANNOTATIONPROTOCOL_H
