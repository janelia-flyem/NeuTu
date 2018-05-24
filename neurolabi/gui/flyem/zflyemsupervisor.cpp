#include "zflyemsupervisor.h"
#include "neutube.h"
#include "neutubeconfig.h"
#include <QsLog.h>
#include "dvid/libdvidheader.h"
#include "dvid/zdvidwriter.h"
#include "dvid/zdvidreader.h"
#include "zrandomgenerator.h"
#include "dvid/zdvidbufferreader.h"
#include "dvid/zdvidurl.h"
#include "zdvidutil.h"

ZFlyEmSupervisor::ZFlyEmSupervisor(QObject *parent) :
  QObject(parent)
{
  m_userName = neutube::GetCurrentUserName();
//  m_server = "emdata2.int.janelia.org:9100";
}


void ZFlyEmSupervisor::setDvidTarget(const ZDvidTarget &target)
{
  m_dvidTarget = target;
#if defined(_FLYEM_)
  if (!target.getSupervisor().empty()) {
    setSever(m_dvidTarget.getSupervisor());
  } else {
    setSever(GET_FLYEM_CONFIG.getDefaultLibrarian());
  }
#endif
}

int ZFlyEmSupervisor::testServer()
{
  int statusCode = 0;
#if defined(_ENABLE_LIBDVIDCPP_)
  if (!m_server.empty()) {
    if (m_connection.get() != NULL) {
      ZDvid::MakeRequest(*m_connection, "/", "HEAD",
                         libdvid::BinaryDataPtr(), libdvid::DEFAULT,
                         statusCode);
    }
  }
#endif

  return statusCode;
}

const ZDvidTarget& ZFlyEmSupervisor::getDvidTarget() const
{
  return m_dvidTarget;
}

void ZFlyEmSupervisor::setUserName(const std::string userName)
{
  m_userName = userName;
}

std::string ZFlyEmSupervisor::GetUserName(
    const std::string &userName, flyem::EBodySplitMode mode)
{
  if (mode == flyem::BODY_SPLIT_OFFLINE) {
    return userName + "-offline";
  }

  return userName;
}

std::string ZFlyEmSupervisor::getUserName(flyem::EBodySplitMode mode) const
{
  return GetUserName(m_userName, mode);
}

bool ZFlyEmSupervisor::checkIn(uint64_t bodyId, flyem::EBodySplitMode mode)
{
  if (m_server.empty()) {
    return false;
  }

  ZDvidWriter writer;
  writer.put(getCheckinUrl(bodyId, mode));

//  writer.writeUrl(getCheckinUrl(bodyId), "PUT");

  return writer.getStatusCode() == 200;
}

bool ZFlyEmSupervisor::checkInAdmin(uint64_t bodyId)
{
  if (m_server.empty()) {
    return false;
  }

  ZDvidWriter writer;
  writer.put(getCheckinUrl(getUuid(), bodyId, getOwner(bodyId)));
//  writer.writeUrl(getCheckinUrl(getUuid(), bodyId, getOwner(bodyId)), "PUT");

  return writer.getStatusCode() == 200;
}

bool ZFlyEmSupervisor::checkOut(uint64_t bodyId, flyem::EBodySplitMode mode)
{
  ZOUT(LINFO(), 3) << "Checking out body:" << bodyId;

  if (m_server.empty()) {
    return false;
  }

  ZDvidWriter writer;
  writer.put(getCheckoutUrl(bodyId, mode));
//  writer.writeUrl(getCheckoutUrl(bodyId), "PUT");

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

bool ZFlyEmSupervisor::isEmpty() const
{
  return m_server.empty();
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
    const std::string &uuid, uint64_t bodyId, flyem::EBodySplitMode mode) const
{
  return QString("%1/%2/%3").arg(getCheckinUrl(uuid).c_str()).arg(bodyId).
      arg(getUserName(mode).c_str()).toStdString();
}

std::string ZFlyEmSupervisor::getCheckoutUrl(
    const std::string &uuid, uint64_t bodyId, flyem::EBodySplitMode mode) const
{
  return QString("%1/%2/%3").arg(getCheckoutUrl(uuid).c_str()).arg(bodyId).
      arg(getUserName(mode).c_str()).toStdString();
}

std::string ZFlyEmSupervisor::getCheckinUrl(
    uint64_t bodyId, flyem::EBodySplitMode mode) const
{
  return getCheckinUrl(getUuid(), bodyId, mode);
}

std::string ZFlyEmSupervisor::getCheckoutUrl(
    uint64_t bodyId, flyem::EBodySplitMode mode) const
{
  return getCheckoutUrl(getUuid(), bodyId, mode);
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
#if defined(_ENABLE_LIBDVIDCPP_)
  m_connection = ZDvid::MakeDvidConnection(server);
#endif
}
