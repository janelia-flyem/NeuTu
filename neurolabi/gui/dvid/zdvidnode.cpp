#include "zdvidnode.h"

#include <iostream>
#include <cstdlib>

#if _QT_APPLICATION_
#include <QtDebug>
#include "logging/zqslog.h"
#include "dvid/zdvidbufferreader.h"
#endif
#include "neutubeconfig.h"
#include "zstring.h"
#include "zjsonobject.h"
#include "zjsonparser.h"

const char* ZDvidNode::m_addressKey = "address";
const char* ZDvidNode::m_portKey = "port";
const char* ZDvidNode::m_uuidKey = "uuid";

/* Implementation details
 *
 * The member m_uuid in this class can be
 *   > explicit, which is just a normal DVID UUID
 *   > reference, which starts with "ref:", it is
 *       expected to followed by a link where the actual UUID is stored
 *   > alias, which starts with "@", referring to the master node of the root
 *       alias configured in the flyem configuration file.
 *
 * setUuid() can take any kind of UUID, but it will only try to translate the
 * reference one.
 *
 */

ZDvidNode::ZDvidNode()
{
}

ZDvidNode::ZDvidNode(
    const std::string &address, const std::string &uuid, int port)
{
  set(address, uuid, port);
}

bool ZDvidNode::hasDvidUuid() const
{
  if (!m_uuid.empty()) {
    if (m_uuid[0] == '@' || ZString(m_uuid).startsWith("ref:")) {
      return false;
    }
  }

  return true;
}

std::string ZDvidNode::getScheme() const
{
  return m_scheme.empty() ? "http" : m_scheme;
}

void ZDvidNode::setScheme(const std::string &scheme)
{
  m_scheme = scheme;
}

void ZDvidNode::setMock(bool on)
{
  m_scheme = on ? "mock" : "";
//  m_isMocked = on;
}

bool ZDvidNode::isMock() const
{
  return getScheme() == "mock";
}

std::string ZDvidNode::getSourceString(bool withScheme, size_t uuidBrief) const
{
  std::string source;

  if (!getAddress().empty()) {
    std::string uuid = getUuid();
    if (uuidBrief > 0 && int(uuid.size()) > uuidBrief) {
      uuid = uuid.substr(0, uuidBrief);
    } else if (int(uuid.size()) < uuidBrief) {
#if defined(_QT_APPLICATION_)
      LWARN() << "Out-of-bound uuid brief (" << uuidBrief << ") for" << uuid;
#endif
    }

    source = getAddress() + ":" + ZString::num2str(getPort()) + ":" + uuid;
    if (withScheme) {
      source = getScheme() + ":" + source;
    }
  }

  return source;
}

bool ZDvidNode::operator ==(const ZDvidNode &node) const
{
  return m_address == node.m_address && m_port == node.m_port &&
      m_uuid == node.m_uuid && getScheme() == node.getScheme();
}

bool ZDvidNode::operator !=(const ZDvidNode &node) const
{
  return m_address != node.m_address || m_port != node.m_port ||
      m_uuid != node.m_uuid || getScheme() != node.getScheme();
}


void ZDvidNode::set(
    const std::string &address, const std::string &uuid, int port)
{
  ZString s(address);
  std::string pureAddress = address;
  if (port < 0) { //parsing address
    bool hasPrefix = false;
    if (s.startsWith("http://", ZString::CASE_INSENSITIVE)) {
      hasPrefix = true;
      setScheme("http");
    } else if (s.startsWith("mock://", ZString::CASE_INSENSITIVE)) {
      hasPrefix = true;
      setScheme("mock");
//      setMock(true);
    }
    if (hasPrefix) {
      s = s.substr(7);
      std::vector<std::string> tokenArray = s.tokenize(':');
      if (tokenArray.empty()) {
        pureAddress = "";
      } else {
        pureAddress = tokenArray[0];
        if (tokenArray.size() > 1 && port < 0) {
          std::string portStr = tokenArray[1];
          std::string::size_type pos = portStr.find_first_of('/');
          if (pos != std::string::npos) {
            if (pos > 0) {
              portStr = portStr.substr(0, pos + 1);
            } else {
              portStr = "";
            }
          }

          if (!portStr.empty()) {
            int tmpPort = std::atoi(portStr.c_str());
            if (tmpPort > 0) {
              port = tmpPort;
            }
          }
        }
      }
    }
  }

  setServer(pureAddress);
  setPort(port);
  setUuid(uuid);
}

void ZDvidNode::clear()
{
  *this = ZDvidNode();
  /*
  set("", "", -1);
  m_isMocked = false;
  */
}

void ZDvidNode::setServer(const std::string &address)
{
  m_address = address;

  if (!address.empty()) {
    ZString addressObj(address);

    if (addressObj.startsWith("http://")) {
      addressObj = address.substr(7);
      setScheme("http");
    } else if (addressObj.startsWith("mock://")) {
      addressObj = address.substr(7);
      setScheme("mock");
    } else if (ZString(address).startsWith("//")) {
      addressObj = address.substr(2);
    } else {
      addressObj = address;
    }

    std::vector<std::string> strArray = addressObj.toWordArray("/");
    if (!strArray.empty()) {
      addressObj = strArray[0];
      strArray = addressObj.toWordArray(":");

      if (strArray.size() > 1) {
        std::vector<int> intArray = ZString(strArray[1]).toIntegerArray();
        if (!intArray.empty()) {
          setPort(intArray[0]);
        }
      }
    }

    m_address = strArray[0];

#if defined(_FLYEM_)
    m_address = GET_FLYEM_CONFIG.mapAddress(m_address);
#endif
  }
}

void ZDvidNode::setInferredUuid(const std::string &uuid)
{
  m_uuid = uuid;
}

void ZDvidNode::setMappedUuid(
    const std::string &original, const std::string &mapped)
{
  m_originalUuid = original;
  m_uuid = mapped;
}

void ZDvidNode::setUuid(const std::string &uuid)
{
  m_uuid.clear();
  m_originalUuid = uuid;

  if (ZString(uuid).startsWith("ref:")) {
#if _QT_APPLICATION_
    std::string uuidLink = uuid.substr(4);
    ZDvidBufferReader reader;
    reader.read(uuidLink.c_str());
    m_uuid = reader.getBuffer().constData();
#else
    m_uuid = "";
    std::cout << "Unsupported uuid ref in non-GUI application. No uuid is set."
              << std::endl;
#endif
  } else {
    m_uuid = uuid;
  }
  /*
  if (m_uuid.size() > 4) {
    if (m_uuid[0] != '@') { //skip reference
      m_uuid = m_uuid.substr(0, 4);
    }
  }
  */
}

void ZDvidNode::setPort(int port)
{
  m_port = port;
}

void ZDvidNode::setFromUrl_deprecated(const std::string &url)
{
  clear();
  if (url.empty()) {
    return;
  }

  ZString zurl(url);
  if (zurl.startsWith("http:")) {
    zurl.replace("http://", "");
    setScheme("http");
  } else if (zurl.startsWith("mock://")) {
    zurl.replace("mock://", "");
    setMock(true);
  }

  std::vector<std::string> tokens = zurl.tokenize('/');
  ZString addressToken = tokens[0];
  std::vector<std::string> tokens2 = addressToken.tokenize(':');
  int port = -1;
  if (tokens2.size() > 1) {
    if (!tokens2[1].empty()) {
      port = ZString::FirstInteger(tokens2[1]);
      if (tokens2[1][0] == '-') {
        port = -port;
      }
    }
  }
  std::string uuid;
  if (tokens.size() > 3) {
    if (tokens[1] == "api" && tokens[2] == "node") {
      uuid = tokens[3];
    }
  }
  set(tokens2[0], uuid, port);
}

void ZDvidNode::setFromSourceString(const std::string &sourceString)
{
  std::vector<std::string> tokens = ZString(sourceString).tokenize(':');

  if (setFromSourceToken(tokens)) {
#if defined(_QT_APPLICATION_)
    LWARN() << "Invalid source string for dvid target:" << sourceString.c_str();
#endif
  }
}

bool ZDvidNode::setFromSourceToken(const std::vector<std::string> &tokens)
{
  bool succ = false;

 clear();

  if (tokens.size() >= 4 && (tokens[0] == "http" || tokens[0] == "mock")) {
    int port = -1;
    if (!tokens[2].empty()) {
      port = ZString::FirstInteger(tokens[2]);
      if (tokens[2][0] == '-') {
        port = -port;
      }
    }
    set(tokens[1], tokens[3], port);
    setScheme(tokens[0]);
    succ = true;
  }

  return succ;
}

bool ZDvidNode::hasPort() const
{
  return getPort() >= 0;
}

bool ZDvidNode::isValid() const
{
  return !getAddress().empty() && !getUuid().empty();
}

std::string ZDvidNode::getAddressWithPort() const
{
  std::string address;

  if (!getAddress().empty()) {
    address = getAddress();
    if (hasPort()) {
      address += ":" + ZString::num2str(getPort());
    }
  }

  return address;
}

std::string ZDvidNode::getUrl() const
{
  ZString url = "";
  if (isValid()) {
    url = getScheme() + "://" + m_address;
    if (m_port >= 0) {
      url += ":";
      url.appendNumber(m_port);
    }
    url += "/api/node/" + m_uuid;
  }

  return std::move(url);
}


void ZDvidNode::print() const
{
  std::cout << getSourceString() << std::endl;
}

void ZDvidNode::loadJsonObject(const ZJsonObject &obj)
{
  clear();

  bool isValidJson = true;


  if (isValidJson) {
    setServer(ZJsonParser::stringValue(obj[m_addressKey]));
    if (obj.hasKey(m_portKey)) {
      setPort(int(ZJsonParser::integerValue(obj[m_portKey])));
    } else {
      setPort(-1);
    }
    setUuid(ZJsonParser::stringValue(obj[m_uuidKey]));
  }
}

ZJsonObject ZDvidNode::toJsonObject() const
{
  ZJsonObject obj;
  if (m_port >= 0) {
    obj.setEntry(m_portKey, m_port);
  }

  obj.setEntry(m_addressKey, m_address);
  obj.setEntry(m_uuidKey, m_uuid);

  return obj;
}


