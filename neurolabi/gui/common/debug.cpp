#include "debug.h"

HighligthDebug::HighligthDebug()
{
}

std::string HighligthDebug::getIcon(const std::string &key) const
{
  if (!topicDisabled(key)) {
    std::lock_guard<std::mutex> guard(m_mutex);

    if (m_iconMap.count(key) == 0) {
      m_iconMap[key] = m_iconList[m_iconMap.size() % m_iconList.size()];
    }

    return m_iconMap.at(key);
  }

  return "";
}

bool HighligthDebug::topicDisabled(const std::string &key) const
{
  return key.empty() || (m_disabledTopicSet.count(key) > 0);
}

HighligthDebug& HLDebug(const std::string &key)
{
  HighligthDebug& hd = HighligthDebug::GetInstance();
  std::string icon = hd.getIcon(key);
  if (!icon.empty()) {
    hd << hd.getIcon(key);
  }

  return hd;
}

// Handle endl
// https://www.cplusplus.com/forum/general/49590/
HighligthDebug& HighligthDebug::operator <<(std::ostream& (*os)(std::ostream&))
{
  std::cout << os;
  return *this;
}

