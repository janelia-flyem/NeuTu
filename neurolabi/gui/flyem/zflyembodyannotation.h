#ifndef ZFLYEMBODYANNOTATION_H
#define ZFLYEMBODYANNOTATION_H

#include <string>
#include <functional>

#include "tz_stdint.h"

class ZJsonObject;

/*!
 * \brief The class of body annotation
 */
class ZFlyEmBodyAnnotation
{
public:
  ZFlyEmBodyAnnotation();

  inline uint64_t getBodyId() const { return m_bodyId; }
  inline const std::string& getStatus() const { return m_status; }
  inline const std::string& getComment() const { return m_comment; }
//  inline const std::string& getName() const { return m_name; }
//  inline const std::string& getType() const { return m_type; }
  inline const std::string& getUser() const { return m_userName; }
  inline const std::string& getNamingUser() const { return m_namingUser; }

  std::string getMajorInput() const;
  std::string getMajorOutput() const;
  std::string getPrimaryNeurite() const;
  std::string getLocation() const;
  bool getOutOfBounds() const;
  bool getCrossMidline() const;
  std::string getNeurotransmitter() const;
  std::string getSynonym() const;
  std::string getClonalUnit() const;
  std::string getProperty() const;

  std::string getName() const;

  std::string getType() const;
  std::string getInferredType() const;
  std::string getAutoType() const;
//  std::string getAutoName() const;

  inline void setBodyId(uint64_t bodyId) { m_bodyId = bodyId; }
  void setBodyId(int bodyId);
  void setBodyId(int64_t bodyId);
  inline void setStatus(const std::string &status) {
    m_status = status;
  }
  inline void setName(const std::string &name) { m_name = name; }
  inline void setType(const std::string &c) { m_type = c; }
  inline void setComment(const std::string &comment) { m_comment = comment; }
  inline void setUser(const std::string &user) { m_userName = user; }
  inline void setNamingUser(const std::string &user) { m_namingUser = user; }
  inline void setInstance(const std::string &instance) { m_instance = instance; }
  void setMajorInput(const std::string &v);
  void setMajorOutput(const std::string &v);
  void setPrimaryNeurite(const std::string &v);
  void setLocation(const std::string &v);
  void setOutOfBounds(bool v);
  void setCrossMidline(bool v);
  void setNeurotransmitter(const std::string &v);
  void setSynonym(const std::string &v);
  void setClonalUnit(const std::string &v);
  void setAutoType(const std::string &v);
  void setProperty(const std::string &v);

  /*!
   * \brief Load the data from a json string
   *
   * {
   *   "status": string,
   *   "comment": string,
   *   "body ID": int,
   *   "name": string,
   *   "class": string,
   *   "user": string,
   *   "naming user": string
   * }
   *
   * A property is cleared if the corresponding field does not exist.
   */
  void loadJsonString(const std::string &str);

  void loadJsonObject(const ZJsonObject &obj);

  ZJsonObject toJsonObject() const;

  void clear();
  void print() const;

  bool isEmpty() const;

  void mergeAnnotation(
      const ZFlyEmBodyAnnotation &annotation,
      const std::function<int(const std::string&)> &getStatusRank);

  std::string toString() const;

  bool isFinalized() const;

  bool hasSameUserStatus(const ZFlyEmBodyAnnotation &annot) const;
  bool operator == (const ZFlyEmBodyAnnotation &annot) const;

public:
  static int GetStatusRank(const std::string &status);

  static const char *KEY_BODY_ID;
  static const char *KEY_STATUS;
  static const char *KEY_NAME;
  static const char *KEY_TYPE;
  static const char *KEY_COMMENT;
  static const char *KEY_USER;
  static const char *KEY_NAMING_USER;
  static const char *KEY_INSTANCE;
  static const char *KEY_MAJOR_INPUT;
  static const char *KEY_MAJOR_OUTPUT;
  static const char *KEY_PRIMARY_NEURITE;
  static const char *KEY_LOCATION;
  static const char *KEY_OUT_OF_BOUNDS;
  static const char *KEY_CROSS_MIDLINE;
  static const char *KEY_NEURONTRANSMITTER;
  static const char *KEY_SYNONYM;
  static const char *KEY_CLONAL_UNIT;
  static const char *KEY_AUTO_TYPE;
  static const char *KEY_PROPERTY;

private:
  static std::string GetOldFormatKey(const ZJsonObject &obj);

private:
  uint64_t m_bodyId = 0;
  std::string m_status;
  std::string m_comment;
  std::string m_name;
  std::string m_type;
  std::string m_userName;
  std::string m_namingUser;

  std::string m_instance;
  std::string m_property;
  std::string m_majorInput;
  std::string m_majorOutput;
  std::string m_primaryNeurite;
  std::string m_location;
  bool m_outOfBounds = false;
  bool m_crossMidline = false;
  std::string m_neurotransmitter;
  std::string m_synonym;
  std::string m_clonalUnit;
  std::string m_autoType;
};

#endif // ZFLYEMBODYANNOTATION_H
