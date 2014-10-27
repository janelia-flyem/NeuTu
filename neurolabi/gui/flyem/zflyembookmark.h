#ifndef ZFLYEMBOOKMARK_H
#define ZFLYEMBOOKMARK_H

#include <QString>
#include "zintpoint.h"

class ZFlyEmBookmark
{
public:
  ZFlyEmBookmark();

  inline int getBodyId() const { return m_bodyId; }
  inline const QString& getTime() const { return m_time; }
  inline const QString& getUserName() const { return m_userName; }
  inline const QString& getStatus() const { return m_status; }
  inline const ZIntPoint& getLocation() const { return m_location; }

  inline void setBodyId(int bodyId) { m_bodyId = bodyId; }
  inline void setLocation(int x, int y, int z) {
    m_location.set(x, y, z);
  }

  void print() const;

private:
  int m_bodyId;
  QString m_userName;
  QString m_time;
  QString m_status;
  ZIntPoint m_location;
};

#endif // ZFLYEMBOOKMARK_H
