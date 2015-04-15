#ifndef ZDVIDVERSIONNODE_H
#define ZDVIDVERSIONNODE_H

#include <string>

class ZDvidVersionNode
{
public:
  ZDvidVersionNode();

  inline bool isLocked() const { return m_isLocked; }
  inline const std::string& getUuid() const { return m_uuid; }

  inline void setUuid(const std::string &uuid) { m_uuid = uuid; }
  inline void lock() { m_isLocked = true; }
  inline void unlock() { m_isLocked = false; }

  inline void activate() { m_isActive = true; }
  inline void deactivate() { m_isActive = false; }
  inline bool isActive() const { return m_isActive; }

private:
  bool m_isLocked;
  bool m_isActive;
  std::string m_uuid;
};

#endif // ZDVIDVERSIONNODE_H
