#ifndef ZFLYEMBOOKMARK_H
#define ZFLYEMBOOKMARK_H

#include <QString>
#include "zintpoint.h"
#include "tz_stdint.h"

class ZFlyEmBookmark
{
public:
  ZFlyEmBookmark();

  inline uint64_t getBodyId() const { return m_bodyId; }
  inline const QString& getTime() const { return m_time; }
  inline const QString& getUserName() const { return m_userName; }
  inline const QString& getStatus() const { return m_status; }
  inline const ZIntPoint& getLocation() const { return m_location; }

  inline void setBodyId(uint64_t bodyId) { m_bodyId = bodyId; }
  inline void setLocation(int x, int y, int z) {
    m_location.set(x, y, z);
  }

  bool isChecked() const {
    return m_isChecked;
  }

  void setChecked(bool checked) {
    m_isChecked = checked;
  }

  void print() const;

private:
  uint64_t m_bodyId;
  QString m_userName;
  QString m_time;
  QString m_status;
  ZIntPoint m_location;
  bool m_isChecked;
};

#endif // ZFLYEMBOOKMARK_H
