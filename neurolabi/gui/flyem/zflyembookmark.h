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

  bool display(QPainter *painter, const DisplayConfig &config) const override;

  /*
  void display(ZPainter &painter, int slice, EDisplayStyle option,
               neutu::EAxis sliceAxis) const override;
               */

  inline uint64_t getBodyId() const { return m_bodyId; }
//  inline const QString& getTime() const { return m_time; }
  inline const QString& getUserName() const { return m_userName; }
  inline const QString& getStatus() const { return m_status; }
  inline const QString& getComment() const { return m_comment; }
  inline const QStringList& getTags() const { return m_tags; }
  inline ZIntPoint getLocation() const { return getCenter().roundToIntPoint(); }
  inline EBookmarkType getBookmarkType() const { return m_bookmarkType; }
  QString getTypeString() const;

  QString getTime() const;

  inline void setBookmarkType(EBookmarkType type) { m_bookmarkType = type; }
  void setBookmarkType(const std::string &type);

  inline void setComment(const QString &comment) { m_comment = comment; }
  void setUser(const QString &user);
  inline void setStatus(const QString &status) { m_status = status; }

  /*!
   * \brief Update user.
   *
   * It's similar to setUser except that nothing will be done if \a user is
   * empty.
   */
  void updateUser(const QString &user);

  inline void setComment(const std::string &comment) {
    setComment(QString::fromStdString(comment)); }
  inline void setUser(const std::string &user) {
    setUser(QString::fromStdString(user)); }
  inline void setStatus(const std::string &status) {
    setStatus(QString::fromStdString(status)); }

  void setUser(const char *user);

  inline void setBodyId(uint64_t bodyId) { m_bodyId = bodyId; }
  inline void setLocation(int x, int y, int z) {
//    m_location.set(x, y, z);
    setCenter(x, y, z);
  }
  void setLocation(const ZIntPoint &pt);

  bool isChecked() const;
  void setChecked(bool checked);

  bool isCustom() const;

  /*
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
  */

  QString getDvidKey() const;

  bool loadJsonObject(const ZJsonObject &jsonObj);

  //For the new annotation API
  ZJsonObject toDvidAnnotationJson() const;
  bool loadDvidAnnotation(const ZJsonObject &jsonObj);

  void print() const;
  std::string toLogString() const;
  std::string toString(bool ignoringComment = false) const;

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

  ZJsonObject& getPropJson() {
    return m_propJson;
  }

  const ZJsonObject& getPropJson() const {
    return m_propJson;
  }

  std::string getProp(const std::string &key);

  std::string getPrevUser() const;


private:
  void init();
  bool loadClioAnnotation(const ZJsonObject &jsonObj);
  void setTimestampS(const std::string &t);
  void updatePrevUser(const std::string &user);
  void normalizePrevUser();
  ZJsonObject toJsonObject(bool ignoringComment = false) const;

private:
  uint64_t m_bodyId;
  QString m_userName;
//  QString m_time;
  QString m_status;
  QString m_comment;
  QStringList m_tags;
//  ZIntPoint m_location;
  EBookmarkType m_bookmarkType;
//  EBookmarkRole m_bookmarkRole;
//  bool m_isChecked;
//  bool m_isCustom;
  bool m_isInTable;
  ZJsonObject m_propJson;
//  QString m_decorationText;
};

#endif // ZFLYEMBOOKMARK_H
