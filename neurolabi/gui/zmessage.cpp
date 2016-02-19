#include "zmessage.h"

#include "tz_utilities.h"

ZMessage::ZMessage(QWidget *source) : m_type(ZMessage::TYPE_NULL),
  m_originalSource(source), m_currentSource(source),
  m_isProcessed(false), m_isActive(true)
{
}

void ZMessage::deactivate()
{
  m_isActive = false;
}

void ZMessage::setBodyEntry(const std::string &key, std::string value)
{
  m_body.setEntry(key.c_str(), value);
}

void ZMessage::setBodyEntry(const std::string &key, uint64_t value)
{
  m_body.setEntry(key.c_str(), value);
}

void ZMessage::setBodyEntry(const std::string &key, ZJsonObject obj)
{
  m_body.setEntry(key.c_str(), obj);
}
