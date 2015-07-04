#ifndef ZFLYEMBOOKMARK_H
#define ZFLYEMBOOKMARK_H

#include <QString>
#include "zintpoint.h"
#include "tz_stdint.h"

class ZJsonObject;

class ZFlyEmBookmark
{
public:
  ZFlyEmBookmark();

  enum EType {
    TYPE_FALSE_MERGE, TYPE_FALSE_SPLIT, TYPE_LOCATION
  };

  inline uint64_t getBodyId() const { return m_bodyId; }
  inline const QString& getTime() const { return m_time; }
  inline const QString& getUserName() const { return m_userName; }
  inline const QString& getStatus() const { return m_status; }
  inline const ZIntPoint& getLocation() const { return m_location; }
  inline EType getType() const { return m_type; }
  inline void setType(EType type) { m_type = type; }

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

  QString getDvidKey() const;

  ZJsonObject toJsonObject() const;


  void print() const;

private:
  uint64_t m_bodyId;
  QString m_userName;
  QString m_time;
  QString m_status;
  ZIntPoint m_location;
  EType m_type;
  bool m_isChecked;
};

#endif // ZFLYEMBOOKMARK_H
