#include "zflyembodymanager.h"

#include <iostream>

ZFlyEmBodyManager::ZFlyEmBodyManager()
{
}

bool ZFlyEmBodyManager::isEmpty() const
{
  return m_bodyMap.isEmpty();
}

void ZFlyEmBodyManager::registerBody(uint64_t id, const QSet<uint64_t> &comp)
{
  m_bodyMap[decode(id)] = comp;

#ifdef _DEBUG_2
  print();
#endif
}

void ZFlyEmBodyManager::registerBody(uint64_t id)
{
  if (encodingSupervoxel(id)) {
    registerSupervoxel(id);
  } else {
    id = decode(id);
    if (!contains(id)) {
      registerBody(id, QSet<uint64_t>());
    }
  }
}

void ZFlyEmBodyManager::registerSupervoxel(uint64_t id)
{
  id = decode(id);
  if (!m_bodyMap.contains(0)) {
    m_bodyMap[0] = QSet<uint64_t>();
  }
  m_bodyMap[0].insert(id);
}

void ZFlyEmBodyManager::deregisterSupervoxel(uint64_t id)
{
  uint64_t bodyId = decode(id);
  if (m_bodyMap.contains(0)) {
    m_bodyMap[0].remove(bodyId);
  }
}

void ZFlyEmBodyManager::deregisterBody(uint64_t id)
{
  if (encodingSupervoxel(id)) {
    deregisterSupervoxel(id);
  } else {
    uint64_t bodyId = decode(id);
    m_bodyMap.remove(bodyId);
    m_todoLoaded.remove(bodyId);
    m_synapseLoaded.remove(bodyId);
    m_bodyConfigMap.remove(bodyId);
  }
}

bool ZFlyEmBodyManager::contains(uint64_t id) const
{
  if (encodingSupervoxel(id)) {
    return isOrphanSupervoxel(id);
  }
  return m_bodyMap.contains(decode(id));
}

bool ZFlyEmBodyManager::hasMapping(uint64_t id) const
{
  id = decode(id);
  if (m_bodyMap.contains(id)) {
    return !m_bodyMap[id].isEmpty();
  }

  return false;
}

uint64_t ZFlyEmBodyManager::getAggloId(uint64_t bodyId) const
{
  bodyId = decode(bodyId);
  for (QMap<uint64_t, QSet<uint64_t> >::const_iterator iter = m_bodyMap.begin();
       iter != m_bodyMap.end(); ++iter) {
    if (iter.value().contains(bodyId)) {
      return iter.key();
    }
  }

  return bodyId;
}

bool ZFlyEmBodyManager::isOrphanSupervoxel(uint64_t bodyId) const
{
  return m_bodyMap.value(0).contains(decode(bodyId));
}

bool ZFlyEmBodyManager::isSupervoxel(uint64_t bodyId) const
{
  bodyId = decode(bodyId);
  for (QMap<uint64_t, QSet<uint64_t> >::const_iterator iter = m_bodyMap.begin();
       iter != m_bodyMap.end(); ++iter) {
    if (iter.value().contains(bodyId)) {
      return true;
    }
  }

  return false;
}

QSet<uint64_t> ZFlyEmBodyManager::getMappedSet(uint64_t bodyId) const
{
  return m_bodyMap.value(decode(bodyId));
}

QSet<uint64_t> ZFlyEmBodyManager::getNormalBodySet() const
{
  QSet<uint64_t> bodySet = QSet<uint64_t>::fromList(m_bodyMap.keys());
  bodySet.remove(0);

  return bodySet;
}

QSet<uint64_t> ZFlyEmBodyManager::getUnmappedBodySet() const
{
  QSet<uint64_t> bodySet;

  for (QMap<uint64_t, QSet<uint64_t> >::const_iterator iter = m_bodyMap.begin();
       iter != m_bodyMap.end(); ++iter) {
    if (iter.key() > 0) {
      if (iter.value().empty()) {
        bodySet.insert(iter.key());
      }
    }
  }

  return bodySet;
}

QSet<uint64_t> ZFlyEmBodyManager::getOrphanSupervoxelSet() const
{
  return m_bodyMap.value(0);
}

uint64_t ZFlyEmBodyManager::getSingleBodyId() const
{
  uint64_t bodyId = 0;

  QSet<uint64_t> bodySet;
  for (QMap<uint64_t, QSet<uint64_t> >::const_iterator iter = m_bodyMap.begin();
       iter != m_bodyMap.end(); ++iter) {
    if (iter.key() > 0) {
      if (iter.value().empty()) {
        bodySet.insert(iter.key());
      } else {
        return 0; //Mapped body considered as multiple bodies
      }
    }
  }

  QSet<uint64_t> svSet = getOrphanSupervoxelSet();
  if (bodySet.size() + svSet.size() == 1) {
    if (bodySet.isEmpty()) {
      bodyId = encodeSupervoxel(*svSet.begin());
    } else {
      bodyId = *bodySet.begin();
    }
  }

  return bodyId;
}

void ZFlyEmBodyManager::setTodoLoaded(uint64_t bodyId)
{
  bodyId = decode(bodyId);
  m_todoLoaded.insert(bodyId);
}

void ZFlyEmBodyManager::setSynapseLoaded(uint64_t bodyId)
{
  bodyId = decode(bodyId);
  m_synapseLoaded.insert(bodyId);
}

bool ZFlyEmBodyManager::isTodoLoaded(uint64_t bodyId) const
{
  bodyId = decode(bodyId);
  return m_todoLoaded.contains(bodyId);
}

bool ZFlyEmBodyManager::isSynapseLoaded(uint64_t bodyId) const
{
  bodyId = decode(bodyId);
  return m_synapseLoaded.contains(bodyId);
}

void ZFlyEmBodyManager::addBodyConfig(const ZFlyEmBodyConfig &config)
{
  uint64_t bodyId = decode(config.getBodyId());

  if (bodyId > 0) {
    m_bodyConfigMap[bodyId] = config;
  }
}

ZFlyEmBodyConfig ZFlyEmBodyManager::getBodyConfig(uint64_t bodyId) const
{
  bodyId = decode(bodyId);
  if (m_bodyConfigMap.contains(bodyId)) {
    return m_bodyConfigMap[bodyId];
  }

  return ZFlyEmBodyConfig();
}

QSet<uint64_t> ZFlyEmBodyManager::getSupervoxelToAdd(
    const QSet<uint64_t> &bodySet, bool resultEncoded)
{
  QSet<uint64_t> newSet;
  QSet<uint64_t> supervoxelSet = getOrphanSupervoxelSet();
  foreach (uint64_t bodyId, bodySet) {
    bodyId = decode(bodyId);
    if (!supervoxelSet.contains(bodyId)) {
      if (resultEncoded) {
        newSet.insert(encodeSupervoxel(bodyId));
      } else {
        newSet.insert(bodyId);
      }
    }
  }

  return newSet;
}

QSet<uint64_t> ZFlyEmBodyManager::getSupervoxelToRemove(
    const QSet<uint64_t> &bodySet, bool resultEncoded)
{
  QSet<uint64_t> supervoxelSet = getOrphanSupervoxelSet();
  QSet<uint64_t> decodedSet;
  foreach (uint64_t bodyId, bodySet) {
    decodedSet.insert(decode(bodyId));
  }

  QSet<uint64_t> newSet = supervoxelSet - decodedSet;
  if (resultEncoded) {
    QSet<uint64_t> encodedSet;
    for (uint64_t bodyId : newSet) {
      encodedSet.insert(encodeSupervoxel(bodyId));
    }
    newSet = encodedSet;
  }

  return newSet;
}

/*
void ZFlyEmBodyManager::erase(uint64_t bodyId)
{
  bool removed = false;
  for (QMap<uint64_t, QSet<uint64_t> >::iterator iter = m_bodyMap.begin();
       iter != m_bodyMap.end(); ++iter) {
    if (iter.value().remove(bodyId)) {
      removed = true;
      break;
    }
  }

  if (!removed) {
    m_bodyMap.remove(bodyId);
  }
}
*/

void ZFlyEmBodyManager::eraseSupervoxel(uint64_t bodyId)
{
  for (QMap<uint64_t, QSet<uint64_t> >::iterator iter = m_bodyMap.begin();
       iter != m_bodyMap.end(); ++iter) {
    if (iter.value().remove(bodyId)) {
      break;
    }
  }
}

void ZFlyEmBodyManager::clear()
{
  m_bodyMap.clear();
}

void ZFlyEmBodyManager::print() const
{
  std::cout << "Body manager: " << m_bodyMap.size() << std::endl;
  for (auto iter = m_bodyMap.begin(); iter != m_bodyMap.end(); ++iter) {
    std::cout << iter.key() << ": " << iter.value().size() << std::endl;
  }
}

namespace {
  const uint64_t ENCODING_BASE = 100000000000;
  const uint64_t ENCODING_TAR = 100;
  const uint64_t ENCODING_SUPERVOXEL_LEVEL = 99;
}

uint64_t ZFlyEmBodyManager::encode(uint64_t rawId, unsigned int level, bool tar)
{
  uint64_t tarEncoding = tar ? ENCODING_TAR : 0;
  return (level + tarEncoding) * ENCODING_BASE + rawId;
}

uint64_t ZFlyEmBodyManager::encodeSupervoxel(uint64_t rawId)
{
  return encode(rawId, ENCODING_SUPERVOXEL_LEVEL, false);
}

uint64_t ZFlyEmBodyManager::decode(uint64_t encodedId)
{
  return encodedId % ENCODING_BASE;
}

bool ZFlyEmBodyManager::encodesTar(uint64_t id) {
  uint64_t encoded = id / ENCODING_BASE;
  uint64_t encodedTar = encoded / ENCODING_TAR;
  return (encodedTar != 0);
}

bool ZFlyEmBodyManager::encodingSupervoxel(uint64_t id)
{
  return encodedLevel(id) == ENCODING_SUPERVOXEL_LEVEL;
}

unsigned int ZFlyEmBodyManager::encodedLevel(uint64_t id) {
  uint64_t encoded = id / ENCODING_BASE;
  uint64_t encodedLevel = encoded % ENCODING_TAR;
  return encodedLevel;
}
