#include "zmessage.h"

#include "tz_utilities.h"


ZMessage::ZMessage() : m_originalSource(NULL), m_currentSource(NULL),
  m_isProcessed(false), m_isActive(true)
{
}
