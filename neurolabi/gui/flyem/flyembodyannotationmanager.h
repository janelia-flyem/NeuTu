#ifndef FLYEMBODYANNOTATIONMANAGER_H
#define FLYEMBODYANNOTATIONMANAGER_H

#include <QObject>
#include <QMap>

#include <memory>

#include "zjsonobject.h"
#include "zflyembodyannotationprotocol.h"
#include "mvc/annotation/zsegmentannotationstore.h"

class FlyEmBodyAnnotationIO;
class ZFlyEmBodyAnnotation;

class FlyEmBodyAnnotationManager : public QObject, public ZSegmentAnnotationStore
{
  Q_OBJECT
public:
  explicit FlyEmBodyAnnotationManager(QObject *parent = nullptr);
  ~FlyEmBodyAnnotationManager() override;

  void setIO(std::shared_ptr<FlyEmBodyAnnotationIO> io);

//  enum class ECacheOption {
//    CACHE_FIRST, // Always use cached value first.
//    SOURCE_ONLY, // Always get annotation from source and refresh the cache.
//    SOURCE_FIRST, // Similar to SOURCE_ONLY, but keep using the cached when
//                  // source retrieval fails.
//  };

  ZJsonObject getAnnotation(
      uint64_t bodyId,
      neutu::ECacheOption option = neutu::ECacheOption::CACHE_FIRST) override;
  void saveAnnotation(uint64_t bodyId, const ZJsonObject &obj) override;;
  void removeAnnotation(uint64_t bodyId) override;;

  std::vector<std::pair<uint64_t, ZJsonObject>> getAnnotations(
      const std::vector<uint64_t> &ids,
      neutu::ECacheOption option = neutu::ECacheOption::CACHE_FIRST) override;

  ZFlyEmBodyAnnotation getParsedAnnotation(uint64_t bodyId);
  std::string getBodyStatus(uint64_t bodyId);
  std::string getBodyName(uint64_t bodyId);
  bool hasName(uint64_t bodyId);

  bool isCached(uint64_t bodyId) const;
  bool usingGenericAnnotation() const;

  void setBodyStatusProtocol(const ZFlyEmBodyAnnotationProtocol &protocol);
  ZFlyEmBodyAnnotationProtocol getBodyStatusProtocol() const;
  void setBodyAnnotationSchema(const ZJsonObject &schema);
  QList<QString> getBodyStatusList(
      std::function<bool(const ZFlyEmBodyStatus&)> pred) const;
  QList<QString> getBodyStatusList() const;
  QList<QString> getAdminStatusList() const;
  int getStatusRank(const std::string &status) const;
  bool isFinalStatus(const std::string &status) const;
  bool isExpertStatus(const std::string &status) const;
  bool isMergableStatus(const std::string &status) const;
  bool preservingId(const std::string &status) const;
  bool canMerge(uint64_t bodyId);
  bool preserved(uint64_t bodyId);

  void setAdmin(bool isAdmin);

  void mergeAnnotation(
      uint64_t targetId, const std::vector<uint64_t> &bodyIdArray);

  QString composeStatusExclusionMessage(const std::set<uint64_t> &bodySet);
  QString composeStatusConflictMessage(const std::set<uint64_t> &bodySet);
  QString composeFinalStatusMessage(const std::set<uint64_t> &bodySet);

  QMap<uint64_t, std::string> getStatusMap(const std::set<uint64_t> &bodySet);

  template<template<class...> class C>
  QMap<uint64_t, ZJsonObject> getAnnotationMap(const C<uint64_t> & bodies);

  static QMap<uint64_t, ZFlyEmBodyAnnotation> GetAnnotationMap(
      const QMap<uint64_t, ZJsonObject> &amap);

  QString toString() const;

public:
  void invalidateCache(uint64_t bodyId);
  void invalidateCache(const std::vector<uint64_t> &bodyIds);
  void invalidateCache();

signals:


private:
  std::shared_ptr<FlyEmBodyAnnotationIO> m_io;
  ZFlyEmBodyAnnotationProtocol m_bodyStatusProtocol;
  ZJsonObject m_bodyAnnotationSchema;
  bool m_isAdmin = false;
};

template<template<class...> class C>
QMap<uint64_t, ZJsonObject> FlyEmBodyAnnotationManager::getAnnotationMap(
    const C<uint64_t> & bodies)
{
  QMap<uint64_t, ZJsonObject> result;
  for (uint64_t body : bodies) {
    result[body] = getAnnotation(body);
  }

  return result;
}

#endif // FLYEMBODYANNOTATIONMANAGER_H
