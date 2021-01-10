#ifndef ZFLYEMBOOKMARK_H
#define ZFLYEMBOOKMARK_H

#include <string>
#include <cstdint>

#include <QString>
#include <QStringList>

#include "geometry/zintpoint.h"

#include "zstackball.h"
#include "zjsonobject.h"

class ZFlyEmBookmark : public ZStackBall
{
public:
  ZFlyEmBookmark();
  ~ZFlyEmBookmark();

  enum class EBookmarkType {
    FALSE_MERGE, FALSE_SPLIT, LOCATION
  };

  /*
  enum EBookmarkRole {
    ROLE_ASSIGNED, ROLE_USER, ROLE_REVIEW
  };
  */

  static ZStackObject::EType GetType() {
    return ZStackObject::EType::FLYEM_BOOKMARK;
  }

  ZFlyEmBookmark* clone() const override;

  void display(ZPainter &painter, int slice, EDisplayStyle option,
               neutu::EAxis sliceAxis) const override;

  inline uint64_t getBodyId() const { return m_bodyId; }
  inline const QString& getTime() const { return m_time; }
  inline const QString& getUserName() const { return m_userName; }
  inline const QString& getStatus() const { return m_status; }
  inline const QString& getComment() const { return m_comment; }
  inline const QStringList& getTags() const { return m_tags; }
  inline ZIntPoint getLocation() const { return getCenter().toIntPoint(); }
  inline EBookmarkType getBookmarkType() const { return m_bookmarkType; }
  QString getTypeString() const;

  inline void setBookmarkType(EBookmarkType type) { m_bookmarkType = type; }

  inline void setComment(const QString &comment) { m_comment = comment; }
  inline void setUser(const QString &user) { m_userName = user; }
  inline void setStatus(const QString &status) { m_status = status; }

  inline void setComment(const std::string &comment) {
    setComment(QString::fromStdString(comment)); }
  inline void setUser(const std::string &user) {
    setUser(QString::fromStdString(user)); }
  inline void setStatus(const std::string &status) {
    setStatus(QString::fromStdString(status)); }

  inline void setBodyId(uint64_t bodyId) { m_bodyId = bodyId; }
  inline void setLocation(int x, int y, int z) {
//    m_location.set(x, y, z);
    setCenter(x, y, z);
  }
  void setLocation(const ZIntPoint &pt);

  bool isChecked() const {
    return m_isChecked;
  }

  bool isCustom() const {
//    return (m_bookmarkRole == ROLE_USER);
    return m_isCustom;
  }

  void setChecked(bool checked) {
    m_isChecked = checked;
  }

  QString getDvidKey() const;

  ZJsonObject toJsonObject(bool ignoringComment = false) const;
  bool loadJsonObject(const ZJsonObject &jsonObj);

  //For the new annotation API
  ZJsonObject toDvidAnnotationJson() const;
  void loadDvidAnnotation(const ZJsonObject &jsonObj);

  void print() const;
  std::string toLogString() const;

  void setCustom(bool state);

//  virtual const std::string& className() const;

  void addTag(const char* tag);
  void addTag(const std::string &tag);
  void addTag(const QString &tag);

  /*!
   * \brief Add the tag associated with the user name.
   *
   * The format of the tag will be "user:<user_name>". If the bookmark has no
   * user, the tag will be "user:".
   */
  void addUserTag();

  void clear();

//  ZFlyEmBookmark* clone() const;

  ZJsonObject& getPropertyJson() {
    return m_propertyJson;
  }

  const ZJsonObject& getPropertyJson() const {
    return m_propertyJson;
  }

private:
  void init();

private:
  uint64_t m_bodyId;
  QString m_userName;
  QString m_time;
  QString m_status;
  QString m_comment;
  QStringList m_tags;
//  ZIntPoint m_location;
  EBookmarkType m_bookmarkType;
//  EBookmarkRole m_bookmarkRole;
  bool m_isChecked;
  bool m_isCustom;
  bool m_isInTable;
  ZJsonObject m_propertyJson;
//  QString m_decorationText;
};

#endif // ZFLYEMBOOKMARK_H
