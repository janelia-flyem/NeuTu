#include "zflyembodymanager.h"

ZFlyEmBodyManager::ZFlyEmBodyManager()
{
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
  if (contains(id)) {
    return !m_bodyMap[id].isEmpty();
  }

  return false;
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
