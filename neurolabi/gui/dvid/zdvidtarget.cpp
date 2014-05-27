#include "zdvidtarget.h"
#include "zstring.h"
#include "zerror.h"

ZDvidTarget::ZDvidTarget()
{
}


std::string ZDvidTarget::getSourceString() const
{
  return "http:" + getAddress() + ":" + ZString::num2str(getPort()) + ":" +
      getUuid();
}

void ZDvidTarget::set(
    const std::string &address, const std::string &uuid, int port)
{
  if (ZString(address).startsWith("http://")) {
    m_address = address.substr(7);
  } else if (ZString(address).startsWith("//")) {
    m_address = address.substr(2);
  } else {
    m_address = address;
  }
  m_uuid = uuid;
  m_port = port;
}

void ZDvidTarget::set(const std::string &sourceString)
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

bool ZDvidTarget::isValid() const
{
  return !getAddress().empty() && !getUuid().empty();
}

std::string ZDvidTarget::getAddressWithPort() const
{
  if (getPort() < 0) {
    return getAddress();
  }

  return getAddress() + ":" + ZString::num2str(getPort());
}

void ZDvidTarget::print() const
{
  std::cout << getSourceString() << std::endl;
}

std::string ZDvidTarget::getBodyPath(int bodyId) const
{
  return getSourceString() + ":" + ZString::num2str(bodyId);
}
