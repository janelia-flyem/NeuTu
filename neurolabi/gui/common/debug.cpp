#include "debug.h"

#include "zstring.h"

HighlightDebug::HighlightDebug()
{
}

std::string HighlightDebug::getIcon(const std::string &topic) const
{
  if (!topicDisabled(topic)) {
    std::lock_guard<std::mutex> guard(m_mutex);

    if (m_iconMap.count(topic) == 0) {
      m_iconMap[topic] = m_iconList[m_iconMap.size() % m_iconList.size()];
    }

    return m_iconMap.at(topic);
  }

  return "";
}

void HighlightDebug::setTopicFilter(const std::string &filter)
{
  m_enabledTopicSet.clear();
  m_disabledTopicSet.clear();
  m_iconMap.clear();
  if (!filter.empty()) {
    ZString str(filter);
    str.trim();
    bool disabled = false;
    if (str.startsWith("~")) {
      disabled = true;
      str = str.substr(1);
    }
    auto topicArray = str.toWordArray(";");
    if (disabled) {
      for (const std::string &topic : topicArray) {
        m_disabledTopicSet.insert(ZString(topic).trimmed());
      }
    } else {
      for (const std::string &topic : topicArray) {
        m_enabledTopicSet.insert(ZString(topic).trimmed());
      }
    }
  }
}

void HighlightDebug::activate(bool on)
{
  m_isActive = on;
}

bool HighlightDebug::topicDisabled(const std::string &key) const
{
  if (key.empty()) {
    return true;
  }

  if (!m_enabledTopicSet.empty()) {
    return m_enabledTopicSet.count(key) == 0;
  }

  return (m_disabledTopicSet.count(key) > 0);
}

HighlightDebug& HLDebug(const std::string &key)
{
  HighlightDebug& hd = HighlightDebug::GetInstance();
  std::string icon = hd.getIcon(key);
  if (!key.empty()) {
    hd.activate(!icon.empty());
  }
  if (!icon.empty()) {
    hd << hd.getIcon(key);
  }

  return hd;
}

// Handle endl
// https://www.cplusplus.com/forum/general/49590/
HighlightDebug& HighlightDebug::operator <<(std::ostream& (*os)(std::ostream&))
{
  if (m_isActive) {
    std::cout << os;
  }
  return *this;
}

