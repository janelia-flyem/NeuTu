#include "flyemdvidbodymeshcache.h"

#include <exception>

#include "neulib/core/stringbuilder.h"
#include "common/debug.h"
#include "zmesh.h"
#include "zmeshio.h"
#include "dvid/zdvidtarget.h"
#include "dvid/zdvidwriter.h"

FlyEmDvidBodyMeshCache::FlyEmDvidBodyMeshCache()
{
  m_defaultFormat = "drc";
}

namespace {

std::string get_cache_name(const ZDvidTarget &target) {
  return target.getSegmentationName() + "_mesh_cache";
}

}

ZDvidReader* FlyEmDvidBodyMeshCache::getDvidReader() const
{
  if (m_writer) {
    return &(m_writer->getDvidReader());
  }

  return nullptr;
}

std::string FlyEmDvidBodyMeshCache::getCacheName() const
{
  if (m_writer) {
    return get_cache_name(m_writer->getDvidTarget());
  }

  return "";
}

void FlyEmDvidBodyMeshCache::setDvidTarget(const ZDvidTarget &target)
{
  std::string error;

  std::lock_guard<std::mutex> guard(m_ioMutex);
  if (target.isValid() && target.hasSegmentation()) {
    m_writer = std::shared_ptr<ZDvidWriter>(new ZDvidWriter);
    if (m_writer->open(target)) {
      HLDEBUG("mesh cache") << "Set dvid body mesh cache: "
                            << target.getSourceString() << std::endl;
      std::string cacheName = get_cache_name(target);
      if (!m_writer->getDvidReader().hasData(cacheName)) {
        m_writer->createData("keyvalue", cacheName, false);
        if (!m_writer->isStatusOk()) {
          m_writer.reset();
          error = "Failed to create " + cacheName + " in " +
              target.getSourceString();
        }
        HLDEBUG("mesh cache") << "Cache store created: " << cacheName << "@"
                              << target.getSourceString() << std::endl;
      }
    } else {
      m_writer.reset();
      error = "Failed to connect to " + target.getSourceString();
    }
  } else {
    m_writer.reset();
    error = "Invalid target for mesh cache: " + target.getSourceString();
  }

  if (!error.empty()) {
    throw std::runtime_error("Failed to set up the mesh cache: " + error);
  }
}

std::string FlyEmDvidBodyMeshCache::getBodyKey(int bodyId, int mutationId) const
{
  if (bodyId > 0 && mutationId >= 0) {
    return neulib::StringBuilder("[$]_[$]").arg(bodyId).arg(mutationId);
  }
  return "";
}

std::string FlyEmDvidBodyMeshCache::getKey(
    const MeshIndex &index, const std::string &format) const
{
  if (index.isSolidValid()) {
    std::string key = neulib::StringBuilder("[$]_[$]")
        .arg(getBodyKey(index.bodyId, index.mutationId))
        .arg(index.resLevel);
    if (!format.empty()) {
      key += "." + format;
    }
    return key;
  }

  return "";
}

int FlyEmDvidBodyMeshCache::getLatestMutationId(uint64_t bodyId) const
{
  std::lock_guard<std::mutex> guard(m_ioMutex);
  ZDvidReader *reader = getDvidReader();
  if (reader) {
    return reader->readBodyMutationId(bodyId);
  }

  return -1;
}

std::vector<std::string>
FlyEmDvidBodyMeshCache::getCachedKeys(uint64_t bodyId, int mutationId) const
{
  std::vector<std::string> result;

  std::lock_guard<std::mutex> guard(m_ioMutex);
  ZDvidReader *reader = getDvidReader();
  if (reader) {
    QString bodyKey = QString::fromStdString(getBodyKey(bodyId, mutationId));
    QStringList keyList = reader->readKeys(
          getCacheName().c_str(), bodyKey + "_0", bodyKey + "_9");
    foreach (const QString &key, keyList) {
      if (key.endsWith(m_defaultFormat.c_str())) {
        result.push_back(key.toStdString());
      }
    }
  }

  return result;
}

ZMesh* FlyEmDvidBodyMeshCache::getFromSolidIndex(const MeshIndex &index) const
{
  std::lock_guard<std::mutex> guard(m_ioMutex);
  ZDvidReader *reader = getDvidReader();
  if (reader) {
    std::string key = getKey(index, m_defaultFormat);
    HLDEBUG("mesh cache") << "Trying to get mesh from " << key << std::endl;
    if (reader->hasKey(getCacheName().c_str(), key.c_str())) {
//      ZMeshIO mio = ZMeshIO::instance();
//      mio.setDeduplicatingDraco(false);
      ZMesh *mesh = reader->readMesh(getCacheName(), key);
      return mesh;
    } else {
      HLDEBUG("mesh cache") << "No mesh exists at " << key << std::endl;
    }
  }
  return nullptr;
}

void FlyEmDvidBodyMeshCache::set(const MeshIndex &index, const ZMesh *mesh)
{
  std::lock_guard<std::mutex> guard(m_ioMutex);
  if (m_writer && mesh) {
    std::string key = getKey(index, m_defaultFormat);
    if (!key.empty()) {
      QByteArray buffer = mesh->writeToMemory(m_defaultFormat);
      if (!buffer.isEmpty()) {
        m_writer->writeDataToKeyValue(getCacheName(), key, buffer);
      }
    }
  }
}
