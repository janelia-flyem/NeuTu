#ifndef ZFLYEMBODYANNOTATION_H
#define ZFLYEMBODYANNOTATION_H

#include <string>

class ZJsonObject;

/*!
 * \brief The class of body annotation
 */
class ZFlyEmBodyAnnotation
{
public:
  ZFlyEmBodyAnnotation();

  inline int getBodyId() const { return m_bodyId; }
  inline const std::string& getStatus() const { return m_status; }
  inline const std::string& getComment() const { return m_comment; }
  inline const std::string& getName() const { return m_name; }
  inline const std::string& getType() const { return m_type; }

  inline void setBodyId(int bodyId) { m_bodyId = bodyId; }
  inline void setStatus(const std::string &status) {
    m_status = status;
  }
  inline void setName(const std::string &name) { m_name = name; }
  inline void setType(const std::string &c) { m_type = c; }
  inline void setComment(const std::string &comment) { m_comment = comment; }

  /*!
   * \brief Load the data from a json string
   *
   * {
   *   "status": string,
   *   "comment": string,
   *   "body ID": int,
   *   "name": string
   *   "class": string
   * }
   *
   * A property is cleared if the corresponding field does not exist.
   */
  void loadJsonString(const std::string &str);

  void loadJsonObject(const ZJsonObject &obj);

  void clear();
  void print() const;

private:
  int m_bodyId;
  std::string m_status;
  std::string m_comment;
  std::string m_name;
  std::string m_type;

  static const char *m_bodyIdKey;
  static const char *m_statusKey;
  static const char *m_nameKey;
  static const char *m_typeKey;
  static const char *m_commentKey;
};

#endif // ZFLYEMBODYANNOTATION_H
