#include "zflyembodymanager.h"

#include <iostream>

/*
 * Impelementation details:
 *
 * An ID number can be a raw ID, encoded normal body ID, or encoded supervoxel ID.
 * There is no overlap between two different types of ID. When an ID is
 * registered in the manager, it is always stored as its decoded form (raw ID).
 * A body ID can be encoded with a level, but currently we assume that a normal
 * body has level 0. Therefore its encoding is the same as its raw form, unless
 * it is encoded additionally with a tar flag, which is mainly used by
 * ZFlyEmBody3dDoc to determine how to load a body.
 *
 * The body set mapped from key 0 in m_bodyMap is treated as a set of orphan
 * supervoxels, which are supervoxels that have unknown parents. An orphan
 * supervoxel is decoded in the manager, but always encoded when returned from an
 * API unless there is an option for specifying encoding or not.
 *
 * A manager object can be something like:
 * {
 *   0 -> {10, 20, 30}
 *   1 -> {1, 2, 3}
 *   2 -> {4, 5}
 *   3 -> {}
 * }
 *
 * To distinguish between an orphan supervoxel and an normal body, encoding is
 * applied on supervoxel IDs.
 *
 * Encoding specification:
 *   An ID can be encoded with a level and/or a tar flag. An encoded ID can
 *   always be distinguished from a non-encoded one.
 *
 * Catches:
 *   There is no guarantee that supervoxels are unique across mapped sets.
 *   A supervoxel may also have the same ID as a normal body. This can cause
 *   confusion while erasing an ID. Therefore the function erase() has been
 *   replaced by more explicit eraseSupervoxel().
 */

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

void ZFlyEmBodyManager::registerBody(uint64_t aggloId, uint64_t bodyId)
{
  uint64_t decodedAggloId = decode(aggloId);

  m_bodyMap[decodedAggloId].insert(decode(bodyId));
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
    if (m_bodyMap[0].isEmpty()) {
      m_bodyMap.remove(0);
    }
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
  if (encodesTar(bodyId) || !couldBeSupervoxelLevel(bodyId)) {
    return 0;
  }

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
  if (couldBeSupervoxel(bodyId)) {
    return m_bodyMap.value(0).contains(decode(bodyId));
  }

  return false;
}

bool ZFlyEmBodyManager::isSupervoxel(uint64_t bodyId) const
{
  if (couldBeSupervoxel(bodyId)) {
    bodyId = decode(bodyId);
    for (QMap<uint64_t, QSet<uint64_t> >::const_iterator iter = m_bodyMap.begin();
         iter != m_bodyMap.end(); ++iter) {
      if (iter.value().contains(bodyId)) {
        return true;
      }
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

QSet<uint64_t> ZFlyEmBodyManager::getOrphanSupervoxelSet(
    bool resultEncoded) const
{
  if (resultEncoded) {
    QSet<uint64_t> bodySet;
    for (uint64_t bodyId : m_bodyMap.value(0)) {
      bodySet.insert(EncodeSupervoxel(bodyId));
    }
    return bodySet;
  }

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

  QSet<uint64_t> svSet = getOrphanSupervoxelSet(false);
  if (bodySet.size() + svSet.size() == 1) {
    if (bodySet.isEmpty()) {
      bodyId = EncodeSupervoxel(*svSet.begin());
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

void ZFlyEmBodyManager::setSynapseLoaded(uint64_t bodyId, bool on)
{
  bodyId = decode(bodyId);
  if (on) {
    m_synapseLoaded.insert(bodyId);
  } else {
    m_synapseLoaded.remove(bodyId);
  }
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
  QSet<uint64_t> supervoxelSet = getOrphanSupervoxelSet(false);
  foreach (uint64_t bodyId, bodySet) {
    bodyId = decode(bodyId);
    if (!supervoxelSet.contains(bodyId)) {
      if (resultEncoded) {
        newSet.insert(EncodeSupervoxel(bodyId));
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
  QSet<uint64_t> supervoxelSet = getOrphanSupervoxelSet(false);
  QSet<uint64_t> decodedSet;
  foreach (uint64_t bodyId, bodySet) {
    decodedSet.insert(decode(bodyId));
  }

  QSet<uint64_t> newSet = supervoxelSet - decodedSet;
  if (resultEncoded) {
    QSet<uint64_t> encodedSet;
    for (uint64_t bodyId : newSet) {
      encodedSet.insert(EncodeSupervoxel(bodyId));
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
  m_bodyConfigMap.clear();
  m_todoLoaded.clear();
  m_synapseLoaded.clear();
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
  if (IsEncoded(rawId)) {
    return 0;
  }

  uint64_t tarEncoding = tar ? ENCODING_TAR : 0;
  return (level + tarEncoding) * ENCODING_BASE + rawId;
}

uint64_t ZFlyEmBodyManager::EncodeSupervoxel(uint64_t rawId)
{
  return encode(rawId, ENCODING_SUPERVOXEL_LEVEL, false);
}

uint64_t ZFlyEmBodyManager::decode(uint64_t encodedId)
{
  return encodedId % ENCODING_BASE;
}

bool ZFlyEmBodyManager::encodesTar(uint64_t bodyId) {
  uint64_t encoded = bodyId / ENCODING_BASE;
  uint64_t encodedTar = encoded / ENCODING_TAR;
  return (encodedTar != 0);
}

bool ZFlyEmBodyManager::encodingSupervoxel(uint64_t bodyId)
{
  return encodedLevel(bodyId) == ENCODING_SUPERVOXEL_LEVEL;
}

bool ZFlyEmBodyManager::encodingSupervoxelTar(uint64_t bodyId)
{
  return encodesTar(bodyId) && (encodedLevel(bodyId) == 0);
}

unsigned int ZFlyEmBodyManager::encodedLevel(uint64_t bodyId) {
  uint64_t encoded = bodyId / ENCODING_BASE;
  uint64_t encodedLevel = encoded % ENCODING_TAR;
  return encodedLevel;
}

bool ZFlyEmBodyManager::couldBeSupervoxelLevel(uint64_t bodyId)
{
  unsigned int level = encodedLevel(bodyId);

  return (level == 0) || (level == ENCODING_SUPERVOXEL_LEVEL);
}

bool ZFlyEmBodyManager::couldBeSupervoxel(uint64_t bodyId)
{
  return !encodesTar(bodyId) && couldBeSupervoxelLevel(bodyId);
}

bool ZFlyEmBodyManager::IsEncoded(uint64_t bodyId)
{
  return encodesTar(bodyId) || (encodedLevel(bodyId) > 0);
}
