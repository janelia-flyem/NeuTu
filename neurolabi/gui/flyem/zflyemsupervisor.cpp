#include "zflyemsupervisor.h"
#include "neutube.h"
#include "dvid/zdvidwriter.h"
#include "dvid/zdvidreader.h"
#include "zrandomgenerator.h"
#include "dvid/zdvidbufferreader.h"
#include "dvid/zdvidurl.h"

ZFlyEmSupervisor::ZFlyEmSupervisor(QObject *parent) :
  QObject(parent)
{
  m_userName = NeuTube::GetCurrentUserName();
  m_server = "emdata2.int.janelia.org:9100";
}


void ZFlyEmSupervisor::setDvidTarget(const ZDvidTarget &target)
{
  m_dvidTarget = target;
  if (!target.getSupervisor().empty()) {
    setSever(m_dvidTarget.getSupervisor());
  }
}

const ZDvidTarget& ZFlyEmSupervisor::getDvidTarget() const
{
  return m_dvidTarget;
}

void ZFlyEmSupervisor::setUserName(const std::string userName)
{
  m_userName = userName;
}

bool ZFlyEmSupervisor::checkIn(uint64_t bodyId)
{
  if (m_server.empty()) {
    return false;
  }

  ZDvidWriter writer;
  writer.writeUrl(getCheckinUrl(bodyId), "PUT");

  return writer.getStatusCode() == 200;
}

bool ZFlyEmSupervisor::checkInAdmin(uint64_t bodyId)
{
  if (m_server.empty()) {
    return false;
  }

  ZDvidWriter writer;
  writer.writeUrl(getCheckinUrl(getUuid(), bodyId, getOwner(bodyId)), "PUT");

  return writer.getStatusCode() == 200;
}

bool ZFlyEmSupervisor::checkOut(uint64_t bodyId)
{
  if (m_server.empty()) {
    return false;
  }

  ZDvidWriter writer;
  writer.writeUrl(getCheckoutUrl(bodyId), "PUT");

  return writer.getStatusCode() == 200;

  /*
  static ZRandomGenerator generator;

  return generator.rndint(2) > 0;
  */
}

std::vector<std::string> ZFlyEmSupervisor::getUuidList() const
{
  ZDvidBufferReader reader;

  reader.read(getUuidsUrl().c_str());

  const QByteArray &buffer = reader.getBuffer();

  std::vector<std::string> uuidList;
  if (!buffer.isEmpty()) {
    ZJsonArray obj;
    obj.decodeString(buffer.constData());
    for (size_t i = 0; i < obj.size(); ++i) {
      uuidList.push_back(ZJsonParser::stringValue(obj.at(i)));
    }
  }

  return uuidList;
//  reader.read(ZDvidUrl)
}

std::string ZFlyEmSupervisor::getMainUrl() const
{
  return "http://" + m_server;
}

std::string ZFlyEmSupervisor::getUuidsUrl() const
{
  return getMainUrl() + "/uuids";
}

std::string ZFlyEmSupervisor::getStateUrl(const std::string &uuid) const
{
  return getMainUrl() + "/state/" + uuid;
}

std::string ZFlyEmSupervisor::getCheckinUrl(const std::string &uuid) const
{
  return getMainUrl() + "/checkin/" + uuid;
}

std::string ZFlyEmSupervisor::getCheckoutUrl(const std::string &uuid) const
{
  return getMainUrl() + "/checkout/" + uuid;
}

std::string ZFlyEmSupervisor::getCheckinUrl(
    const std::string &uuid, uint64_t bodyId, const std::string &userName) const
{
  return QString("%1/%2/%3").arg(getCheckinUrl(uuid).c_str()).arg(bodyId).
      arg(userName.c_str()).toStdString();
}

std::string ZFlyEmSupervisor::getCheckinUrl(
    const std::string &uuid, uint64_t bodyId) const
{
  return QString("%1/%2/%3").arg(getCheckinUrl(uuid).c_str()).arg(bodyId).
      arg(getUserName().c_str()).toStdString();
}

std::string ZFlyEmSupervisor::getCheckoutUrl(
    const std::string &uuid, uint64_t bodyId) const
{
  return QString("%1/%2/%3").arg(getCheckoutUrl(uuid).c_str()).arg(bodyId).
      arg(getUserName().c_str()).toStdString();
}

std::string ZFlyEmSupervisor::getCheckinUrl(uint64_t bodyId) const
{
  return getCheckinUrl(getUuid(), bodyId);
}

std::string ZFlyEmSupervisor::getCheckoutUrl(uint64_t bodyId) const
{
  return getCheckoutUrl(getUuid(), bodyId);
}

std::string ZFlyEmSupervisor::getUuid() const
{
  return getDvidTarget().getUuid();
}

std::string ZFlyEmSupervisor::getOwner(uint64_t bodyId) const
{
  std::string owner;

  ZDvidBufferReader bufferReader;
  bufferReader.read(
        QString("%1/%2").arg(getCheckoutUrl(getDvidTarget().getUuid()).c_str()).
        arg(bodyId));
  ZJsonObject obj;
  obj.decodeString(bufferReader.getBuffer());
  if (obj.hasKey("Client")) {
    owner = ZJsonParser::stringValue(obj["Client"]);
  }

  return owner;
}

bool ZFlyEmSupervisor::isLocked(uint64_t bodyId) const
{
  return !getOwner(bodyId).empty();
}

void ZFlyEmSupervisor::setSever(const std::string &server)
{
  m_server = server;
}
