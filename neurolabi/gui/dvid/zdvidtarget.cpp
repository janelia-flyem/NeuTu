#include "zdvidtarget.h"
#include "zstring.h"
#include "zerror.h"
#include "zjsonparser.h"

const char* ZDvidTarget::m_addressKey = "address";
const char* ZDvidTarget::m_portKey = "port";
const char* ZDvidTarget::m_uuidKey = "uuid";
const char* ZDvidTarget::m_commentKey = "comment";
const char* ZDvidTarget::m_nameKey = "name";

ZDvidTarget::ZDvidTarget() : m_port(-1)
{
}

ZDvidTarget::ZDvidTarget(
    const std::string &address, const std::string &uuid, int port)
{
  set(address, uuid, port);
}

std::string ZDvidTarget::getSourceString(bool withHttpPrefix) const
{
  std::string source;

  if (!getAddress().empty()) {
    source = getAddress() + ":" + ZString::num2str(getPort()) + ":" + getUuid();
    if (withHttpPrefix) {
      source = "http:" + source;
    }
  }

  return source;
}

void ZDvidTarget::set(
    const std::string &address, const std::string &uuid, int port)
{
  setServer(address);
  setUuid(uuid);
  setPort(port);
}

void ZDvidTarget::setServer(const std::string &address)
{
  if (ZString(address).startsWith("http://")) {
    m_address = address.substr(7);
  } else if (ZString(address).startsWith("//")) {
    m_address = address.substr(2);
  } else {
    m_address = address;
  }
}

void ZDvidTarget::setUuid(const std::string &uuid)
{
  m_uuid = uuid;
}

void ZDvidTarget::setPort(int port)
{
  m_port = port;
}


void ZDvidTarget::setFromSourceString(const std::string &sourceString)
{
  set("", "", -1);

  std::vector<std::string> tokens = ZString(sourceString).tokenize(':');

  if (tokens.size() < 4 || tokens[0] != "http") {
    RECORD_WARNING_UNCOND("Invalid source string");
  } else {
    int port = -1;
    if (!tokens[2].empty()) {
      port = ZString::firstInteger(tokens[2]);
      if (tokens[2][0] == '-') {
        port = -port;
      }
    }
    set(tokens[1], tokens[3], port);
  }
}

bool ZDvidTarget::hasPort() const
{
  return getPort() >= 0;
}

bool ZDvidTarget::isValid() const
{
  return !getAddress().empty() && !getUuid().empty();
}

std::string ZDvidTarget::getAddressWithPort() const
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

void ZDvidTarget::print() const
{
  std::cout << getSourceString() << std::endl;
}

std::string ZDvidTarget::getBodyPath(int bodyId) const
{
  return getSourceString() + ":" + ZString::num2str(bodyId);
}

void ZDvidTarget::loadJsonObject(const ZJsonObject &obj)
{
  setServer(ZJsonParser::stringValue(obj[m_addressKey]));
  if (obj.hasKey(m_portKey)) {
    setPort(ZJsonParser::integerValue(obj[m_portKey]));
  } else {
    setPort(-1);
  }
  setUuid(ZJsonParser::stringValue(obj[m_uuidKey]));
  m_comment = ZJsonParser::stringValue(obj[m_commentKey]);
  m_name = ZJsonParser::stringValue(obj[m_nameKey]);
}

std::string ZDvidTarget::getUrl() const
{
  ZString url = "http://" + m_address;
  if (m_port >= 0) {
    url += ":";
    url.appendNumber(m_port);
  }
  url += "/api/node/" + m_uuid;

  return url;
}
