#include "zflyembodymanager.h"

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
}

void ZFlyEmBodyManager::registerBody(uint64_t id)
{
  if (!contains(id)) {
    registerBody(id, QSet<uint64_t>());
  }
}

void ZFlyEmBodyManager::deregisterBody(uint64_t id)
{
  m_bodyMap.remove(decode(id));
}

bool ZFlyEmBodyManager::contains(uint64_t id) const
{
  return m_bodyMap.contains(decode(id));
}

bool ZFlyEmBodyManager::hasMapping(uint64_t id) const
{
  uint64_t decodedId = decode(id);
  if (m_bodyMap.contains(decodedId)) {
    return !m_bodyMap[decodedId].isEmpty();
  }

  return false;
}

uint64_t ZFlyEmBodyManager::getHostId(uint64_t bodyId) const
{
  for (QMap<uint64_t, QSet<uint64_t> >::const_iterator iter = m_bodyMap.begin();
       iter != m_bodyMap.end(); ++iter) {
    if (iter.value().contains(bodyId)) {
      return iter.key();
    }
  }

  return bodyId;
}

QSet<uint64_t> ZFlyEmBodyManager::getMappedSet(uint64_t bodyId) const
{
  return m_bodyMap.value(decode(bodyId));
}

QSet<uint64_t> ZFlyEmBodyManager::getBodySet() const
{
  return QSet<uint64_t>::fromList(m_bodyMap.keys());
}

uint64_t ZFlyEmBodyManager::getSingleBodyId() const
{
  uint64_t bodyId = 0;
  if (m_bodyMap.size() == 1) {
    bodyId = m_bodyMap.firstKey();
  }

  return bodyId;
}

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

void ZFlyEmBodyManager::clear()
{
  m_bodyMap.clear();
}

namespace {
  const uint64_t ENCODING_BASE = 100000000000;
  const uint64_t ENCODING_TAR = 100;
}

uint64_t ZFlyEmBodyManager::encode(uint64_t rawId, unsigned int level, bool tar)
{
  uint64_t tarEncoding = tar ? ENCODING_TAR : 0;
  return (level + tarEncoding) * ENCODING_BASE + rawId;
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

unsigned int ZFlyEmBodyManager::encodedLevel(uint64_t id) {
  uint64_t encoded = id / ENCODING_BASE;
  uint64_t encodedLevel = encoded % ENCODING_TAR;
  return encodedLevel;
}
