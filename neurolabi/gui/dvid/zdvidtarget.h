#ifndef ZDVIDTARGET_H
#define ZDVIDTARGET_H

#include <string>
#include <set>

#include "zjsonobject.h"
#include "zdviddata.h"

/*!
 * \brief The class of representing a dvid node.
 */
class ZDvidTarget
{
public:
  ZDvidTarget();
  ZDvidTarget(const std::string &address, const std::string &uuid, int port = -1);

  void clear();

  void set(const std::string &address, const std::string &uuid, int port = -1);
  void setServer(const std::string &address);
  void setUuid(const std::string &uuid);
  void setPort(int port);


  /*!
   * \brief Set dvid target from source string
   *
   * The old settings will be cleared no matter what. The source string is
   * http:address:port:uuid.
   *
   * \param sourceString Must start with "http:".
   */
  void setFromSourceString(const std::string &sourceString);

  void setFromUrl(const std::string &url);

  inline const std::string& getAddress() const {
    return m_address;
  }

  /*!
   * \brief Get the address with port
   *
   * \return "address[:port]" or empty if the address is empty.
   */
  std::string getAddressWithPort() const;

  inline const std::string& getUuid() const {
    return m_uuid;
  }

  inline const std::string& getComment() const {
    return m_comment;
  }

  inline const std::string& getName() const {
    return m_name;
  }

  inline int getPort() const {
    return m_port;
  }

  /*!
   * \brief Check if there is a port
   *
   * A valid port is any non-negative port number.
   *
   * \return true iff the port is available.
   */
  bool hasPort() const;

  inline void setName(const std::string &name) {
    m_name = name;
  }
  inline void setComment(const std::string &comment) {
    m_comment = comment;
  }

  std::string getUrl() const;
  std::string getUrl(const std::string &dataName) const;

  /*!
   * \brief Get a single string to represent the target
   *
   * \a withHttpPrefix specifies whether the source string contains the "http:"
   * prefix or not.
   *
   * \return "[http:]address:port:uuid". Return empty if the address is empty.
   */
  std::string getSourceString(bool withHttpPrefix = true) const;

  /*!
   * \brief getBodyPath
   *
   * The functions does not check if a body exists.
   *
   * \return The path of a certain body.
   */
  std::string getBodyPath(uint64_t bodyId) const;

  /*!
   * \brief Test if the target is valid
   *
   * \return true iff the target is valid.
   */
  bool isValid() const;

  /*!
   * \brief Load json object
   */
  void loadJsonObject(const ZJsonObject &obj);
  ZJsonObject toJsonObject() const;

  void print() const;

  //Special functions
  inline const std::string& getLocalFolder() const {
    return m_localFolder;
  }

  inline void setLocalFolder(const std::string &folder) {
    m_localFolder = folder;
  }

  std::string getLocalLowResGrayScalePath(
      int xintv, int yintv, int zintv) const;
  std::string getLocalLowResGrayScalePath(
      int xintv, int yintv, int zintv, int z) const;

  inline int getBgValue() const {
    return m_bgValue;
  }

  inline void setBgValue(int v) {
    m_bgValue = v;
  }

  std::string getName(ZDvidData::ERole role) const;

  std::string getBodyLabelName() const;
  void setBodyLabelName(const std::string &name);

  std::string getLabelBlockName() const;
  void setLabelBlockName(const std::string &name);

  std::string getMultiscale2dName() const;
  void setMultiscale2dName(const std::string &name);

  std::string getGrayScaleName() const;
  void setGrayScaleName(const std::string &name);

  std::string getRoiName(size_t index) const;
  void addRoiName(const std::string &name);

  const std::vector<std::string>& getRoiList() const {
    return m_roiList;
  }

  void setRoiList(const std::vector<std::string> &roiList) {
    m_roiList = roiList;
  }

  std::string getSynapseName() const;
  void setSynapseName(const std::string &name);

  std::string getBookmarkName() const;
  std::string getBookmarkKeyName() const;
  std::string getSkeletonName() const;
  std::string getThumbnailName() const;

  std::string getTodoListName() const;

  const std::set<std::string>& getUserNameSet() const;
  //void setUserName(const std::string &name);

  static bool isDvidTarget(const std::string &source);

  inline bool isSupervised() const { return m_isSupervised; }
  const std::string& getSupervisor() const { return m_supervisorServer; }

  inline bool isEditable() const { return m_isEditable; }
  void setEditable(bool on) { m_isEditable = on; }

private:
  std::string m_address;
  std::string m_uuid;
  int m_port;
  std::string m_name;
  std::string m_comment;
  std::string m_localFolder;
  std::string m_bodyLabelName;
  std::string m_labelBlockName;
  std::string m_multiscale2dName;
  std::string m_grayScaleName;
  std::vector<std::string> m_roiList;
  std::string m_synapseName;
  std::set<std::string> m_userList;
  bool m_isSupervised;
  std::string m_supervisorServer;
//  std::string m_userName;
//  std::string m_tileName;

  int m_bgValue; //grayscale background

  bool m_isEditable;

  const static char* m_addressKey;
  const static char* m_portKey;
  const static char* m_uuidKey;
  const static char* m_commentKey;
  const static char* m_nameKey;
  const static char* m_localKey;
  const static char* m_debugKey;
  const static char* m_bgValueKey;
  const static char* m_grayScaleNameKey;
  const static char* m_bodyLabelNameKey;
  const static char* m_labelBlockNameKey;
  const static char* m_multiscale2dNameKey;
  const static char* m_roiNameKey;
  const static char* m_synapseNameKey;
  const static char* m_userNameKey;
  const static char* m_supervisorKey;
  const static char* m_supervisorServerKey;
};

#endif // ZDVIDTARGET_H
