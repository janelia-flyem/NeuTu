#include "flyembodyannotationmanager.h"

#include "zstring.h"
#include "logging/zlog.h"
#include "flyembodyannotationio.h"
#include "zflyembodyannotation.h"

FlyEmBodyAnnotationManager::FlyEmBodyAnnotationManager(QObject *parent) : QObject(parent)
{

}

FlyEmBodyAnnotationManager::~FlyEmBodyAnnotationManager()
{
}

void FlyEmBodyAnnotationManager::setIO(
    std::shared_ptr<FlyEmBodyAnnotationIO> io)
{
  invalidateCache();
  m_io = io;
}

ZJsonObject FlyEmBodyAnnotationManager::getAnnotation(
    uint64_t bodyId, neutu::ECacheOption option)
{
  if (option == neutu::ECacheOption::SOURCE_ONLY) {
    invalidateCache(bodyId);
  }

  bool updatingCache = (option == neutu::ECacheOption::SOURCE_FIRST) ||
      !m_annotationCache.contains(bodyId);
  if (updatingCache && m_io) {
    try {
      m_annotationCache[bodyId] = m_io->readBodyAnnotation(bodyId);
    } catch (std::exception &e) {
      KWARN << std::string("Failed to read body annotation: ") + e.what();
    }
  }

  if (m_annotationCache.contains(bodyId)) {
    return m_annotationCache.value(bodyId).clone();
  }

  return ZJsonObject();
}

void FlyEmBodyAnnotationManager::saveAnnotation(
    uint64_t bodyId, const ZJsonObject &obj)
{
  if (m_io) {
    try {
      ZJsonObject oldAnnotation = getAnnotation(bodyId);
      ZJsonObject newAnnotation = obj.clone();
      std::vector<std::string> keysToRemove;
      newAnnotation.forEachValue([&](const std::string &key, ZJsonValue value) {
        if (!oldAnnotation.hasKey(key)) {
          if (value.isString()) {
            if (value.toString().empty()) {
              keysToRemove.push_back(key);
            }
          }
        }
      });
      for (const auto &key : keysToRemove) {
        newAnnotation.removeKey(key.c_str());
      }
      m_io->writeBodyAnnotation(bodyId, newAnnotation);
      m_annotationCache[bodyId] = newAnnotation;
    }  catch (std::exception &e) {
      KWARN << std::string("Failed to write body annotation: ") + e.what();
    }
  } else {
    m_annotationCache[bodyId] = obj.clone();
  }
}

void FlyEmBodyAnnotationManager::removeAnnotation(uint64_t bodyId)
{
  if (m_io) {
    try {
      m_io->deleteBodyAnnotation(bodyId);
      invalidateCache(bodyId);
    }  catch (std::exception &e) {
      KWARN << std::string("Failed to write body annotation: ") + e.what();
    }
  } else {
    invalidateCache(bodyId);
  }
}

void FlyEmBodyAnnotationManager::invalidateCache(uint64_t bodyId)
{
  m_annotationCache.remove(bodyId);
}

void FlyEmBodyAnnotationManager::invalidateCache()
{
  m_annotationCache.clear();
}

bool FlyEmBodyAnnotationManager::isCached(uint64_t bodyId) const
{
  return m_annotationCache.count(bodyId) > 0;
}

bool FlyEmBodyAnnotationManager::usingGenericAnnotation() const
{
  return !m_bodyAnnotationSchema.isEmpty();
}

ZFlyEmBodyAnnotation FlyEmBodyAnnotationManager::getParsedAnnotation(
    uint64_t bodyId)
{
  ZFlyEmBodyAnnotation annot;
  ZJsonObject obj = getAnnotation(bodyId);
  if (!obj.isEmpty()) {
    annot.loadJsonObject(obj);
  }

  return annot;
}

void FlyEmBodyAnnotationManager::setBodyStatusProtocol(
    const ZFlyEmBodyAnnotationProtocol &protocol)
{
  m_bodyStatusProtocol = protocol;
}

ZFlyEmBodyAnnotationProtocol
FlyEmBodyAnnotationManager::getBodyStatusProtocol() const
{
  return m_bodyStatusProtocol;
}

std::string FlyEmBodyAnnotationManager::getBodyStatus(uint64_t bodyId)
{
  return ZFlyEmBodyAnnotation::GetStatus(getAnnotation(bodyId));
}

std::string FlyEmBodyAnnotationManager::getBodyName(uint64_t bodyId)
{
  return ZFlyEmBodyAnnotation::GetName(getAnnotation(bodyId));
}

bool FlyEmBodyAnnotationManager::hasName(uint64_t bodyId)
{
  return !getBodyName(bodyId).empty();
}

bool FlyEmBodyAnnotationManager::canMerge(uint64_t bodyId)
{
  return isMergableStatus(getBodyStatus(bodyId));
}

bool FlyEmBodyAnnotationManager::preserved(uint64_t bodyId)
{
  return preservingId(getBodyStatus(bodyId));
}

void FlyEmBodyAnnotationManager::setBodyAnnotationSchema(
    const ZJsonObject &schema)
{
  m_bodyAnnotationSchema = schema;
}

QList<QString> FlyEmBodyAnnotationManager::getBodyStatusList(
    std::function<bool(const ZFlyEmBodyStatus&)> pred) const
{
  const std::vector<ZFlyEmBodyStatus> &bodyStatusList =
      m_bodyStatusProtocol.getStatusList();

  QList<QString> statusList;
  for (const ZFlyEmBodyStatus &status : bodyStatusList) {
    if (pred(status)) {
      statusList.append(status.getName().c_str());
    }
  }

  return statusList;
}

void FlyEmBodyAnnotationManager::setAdmin(bool isAdmin)
{
  m_isAdmin = isAdmin;
}

QMap<uint64_t, std::string> FlyEmBodyAnnotationManager::getStatusMap(
    const std::set<uint64_t> &bodySet)
{
  QMap<uint64_t, std::string> result;
  for (uint64_t body : bodySet) {
    result[body] = getBodyStatus(body);
  }

  return result;
}

namespace {

QString compose_body_status_message(
    const std::vector<uint64_t> &bodyArray,
    const QMap<uint64_t, std::string> &statusMap, int &itemCount)
{
  QString msg;
  if (!bodyArray.empty()) {
    msg += "<ul>";
    for (uint64_t bodyId : bodyArray) {
      msg += QString("<li>%1: %2</li>").
          arg(bodyId).arg(statusMap[bodyId].c_str());
      ++itemCount;
      if (itemCount >= 5) {
        msg += "<li>...</li>";
        break;
      }
    }
    msg += "</ul>";
    msg += "<br>";
  }

  return msg;
}

}

QString FlyEmBodyAnnotationManager::composeStatusExclusionMessage(
    const std::set<uint64_t> &bodySet)
{
  QString msg;
  auto statusMap = getStatusMap(bodySet);
  const std::vector<std::vector<uint64_t>> &conflictSet =
      m_bodyStatusProtocol.getExclusionBody(statusMap);
  int itemCount = 0;
  for (const std::vector<uint64_t> &bodyArray : conflictSet) {
    msg += compose_body_status_message(bodyArray, statusMap, itemCount);
    if (itemCount >= 5) {
      break;
    }
  }

  if (!msg.isEmpty()) {
    msg = "The following bodies have statuses that cannot be merged: " + msg +
        "<font color=\"#FF0000\">No merge will be done.</font>";
  }

  return msg;
}

QString FlyEmBodyAnnotationManager::composeStatusConflictMessage(
    const std::set<uint64_t> &bodySet)
{
  QString msg;
  auto statusMap = getStatusMap(bodySet);
  const std::vector<std::vector<uint64_t>> &conflictSet =
      getBodyStatusProtocol().getConflictBody(statusMap);
  int itemCount = 0;
  for (const std::vector<uint64_t> &bodyArray : conflictSet) {
    msg += compose_body_status_message(bodyArray, statusMap, itemCount);
    if (itemCount >= 5) {
      break;
    }
  }

  if (!msg.isEmpty()) {
    msg = "The following bodies have conflicting statuses: " + msg +
        "<font color=\"#FF0000\">You should NOT merge them "
        "unless you want to be resposible for any side effects.</font>";
  }

  return msg;
}

QString FlyEmBodyAnnotationManager::composeFinalStatusMessage(
    const std::set<uint64_t> &bodySet)
{
  std::vector<uint64_t> bodyArray;
  for (uint64_t body : bodySet) {
    if (isFinalStatus(getBodyStatus(body))) {
      bodyArray.push_back(body);
    }
  }

  int itemCount = 0;
  QString msg = compose_body_status_message(
        bodyArray, getStatusMap(bodySet), itemCount);

  if (!msg.isEmpty()) {
    msg = "The following bodies have final statuses: " + msg +
        "<font color=\"#FF0000\">You should NOT merge them "
        "unless you want to be resposible for any side effects.</font>";
  }

  return msg;
}

QList<QString> FlyEmBodyAnnotationManager::getBodyStatusList() const
{
  const std::vector<ZFlyEmBodyStatus> &bodyStatusList =
      m_bodyStatusProtocol.getStatusList();

  QList<QString> statusList;
  for (const ZFlyEmBodyStatus &status : bodyStatusList) {
    if (status.isAccessible(m_isAdmin)) {
      statusList.append(status.getName().c_str());
    }
  }

  return statusList;
}

QList<QString> FlyEmBodyAnnotationManager::getAdminStatusList() const
{
  return getBodyStatusList([](const ZFlyEmBodyStatus &status) {
    return status.isAdminAccessible();
  });

//  const std::vector<ZFlyEmBodyStatus> &bodyStatusList =
//      m_annotMerger.getStatusList();

//  QList<QString> statusList;
//  for (const ZFlyEmBodyStatus &status : bodyStatusList) {
//    if (status.isAdminAccessible()) {
//      statusList.append(status.getName().c_str());
//    }
//  }

//  return statusList;
}

int FlyEmBodyAnnotationManager::getStatusRank(const std::string &status) const
{
  if (!m_bodyStatusProtocol.isEmpty()) {
    return m_bodyStatusProtocol.getStatusRank(status);
  }

  return ZFlyEmBodyAnnotation::GetStatusRank(status);
}

bool FlyEmBodyAnnotationManager::isFinalStatus(const std::string &status) const
{
  if (m_bodyStatusProtocol.isEmpty()) {
    return ZString(status).lower() == "finalized";
  }

  return m_bodyStatusProtocol.isFinal(status);
}

bool FlyEmBodyAnnotationManager::isExpertStatus(const std::string &status) const
{
  return m_bodyStatusProtocol.isExpertStatus(status);
}

bool FlyEmBodyAnnotationManager::isMergableStatus(const std::string &status) const
{
  return m_bodyStatusProtocol.isMergable(status);
}

bool FlyEmBodyAnnotationManager::preservingId(const std::string &status) const
{
  return m_bodyStatusProtocol.preservingId(status);
}


void FlyEmBodyAnnotationManager::mergeAnnotation(
    uint64_t targetId, const std::vector<uint64_t> &bodyIdArray)
{
  ZJsonObject annotation = getAnnotation(targetId);
  for (uint64_t bodyId : bodyIdArray) {
    if (bodyId != targetId) {
      ZJsonObject subann = getAnnotation(bodyId);
      if (!subann.isEmpty()) {
        if (m_bodyStatusProtocol.isEmpty()) {
          annotation = ZFlyEmBodyAnnotation::MergeAnnotation(
                annotation, subann, &ZFlyEmBodyAnnotation::GetStatusRank);
        } else {
          annotation = ZFlyEmBodyAnnotation::MergeAnnotation(
                annotation, subann, [=](const std::string &status) {
            return m_bodyStatusProtocol.getStatusRank(status);
          });
        }
      }
    }
  }

  saveAnnotation(targetId, annotation);
}

QMap<uint64_t, ZFlyEmBodyAnnotation> FlyEmBodyAnnotationManager::GetAnnotationMap(
      const QMap<uint64_t, ZJsonObject> &amap)
{
  QMap<uint64_t, ZFlyEmBodyAnnotation> result;

  ZFlyEmBodyAnnotation annot;
  for (auto iter = amap.constBegin(); iter != amap.constEnd(); ++iter) {
    if (!iter.value().isEmpty()) {
      annot.loadJsonObject(iter.value());
    }
    result[iter.key()] = annot;
  }

  return result;
}

QString FlyEmBodyAnnotationManager::toString() const
{
  return QString("Annotation Manager: Cached x %1; Admin: %2").
      arg(m_annotationCache.size()).arg(m_isAdmin);
}
