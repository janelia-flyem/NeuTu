#include "zmessage.h"

#include "tz_utilities.h"

ZMessage::ZMessage() : m_type(ZMessage::TYPE_NULL),
  m_originalSource(NULL), m_currentSource(NULL),
  m_isProcessed(false), m_isActive(true)
{
}

void ZMessage::deactivate()
{
  m_isActive = false;
}

void ZMessage::setBodyEntry(std::string key, std::string value)
{
  m_body.setEntry(key.c_str(), value);
}
