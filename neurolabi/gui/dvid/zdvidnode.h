#ifndef ZDVIDNODE_H
#define ZDVIDNODE_H

#include <string>
#include <vector>

class ZJsonObject;

class ZDvidNode
{
public:
  ZDvidNode();
  ZDvidNode(const std::string &address, const std::string &uuid, int port = -1);

  void clear();

  void set(const std::string &address, const std::string &uuid, int port = -1);
  void setServer(const std::string &address);
  void setUuid(const std::string &uuid);
  void setPort(int port);

  void setInferredUuid(const std::string &uuid);
  void setMappedUuid(const std::string &original, const std::string &mapped);

  void setMock(bool on);
  bool isMock() const;

  bool hasDvidUuid() const;

  /*!
   * \brief Set dvid target from source string
   *
   * The old settings will be cleared no matter what. The source string is
   * http:address:port:uuid, with tokens separated by colons.
   *
   * \param sourceString Must start with "http:".
   */
  void setFromSourceString(const std::string &sourceString);
  bool setFromSourceToken(const std::vector<std::string> &tokens);

  void setFromUrl(const std::string &url);

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

  inline const std::string& getOriginalUuid() const {
    return m_originalUuid;
  }

  std::string getUrl() const;

  /*!
   * \brief Get a single string to represent the target
   *
   * \a withHttpPrefix specifies whether the source string contains the "http:"
   * prefix or not. \a uuidBrief specifies the max number of characters of the
   * uuid used in the source string, except when its no greater than 0, the
   * intrinsic uuid will be used.
   *
   * \return "[http:]address:port:uuid". Return empty if the address is empty.
   */
  std::string getSourceString(
      bool withHttpPrefix = true, int uuidBrief = 0) const;


  /*!
   * \brief Test if the target is valid
   *
   * \return true iff the target is valid.
   */
  bool isValid() const;

  void print() const;

  /*!
   * \brief Load json object
   */
  void loadJsonObject(const ZJsonObject &obj);
  ZJsonObject toJsonObject() const;

  bool operator == (const ZDvidNode &node) const;
  bool operator != (const ZDvidNode &node) const;

private:
  std::string m_address;
  std::string m_uuid;
  std::string m_originalUuid;
  int m_port = -1;
  bool m_isMocked = false; //Mocked node if true

  const static char* m_addressKey;
  const static char* m_portKey;
  const static char* m_uuidKey;
};

#endif // ZDVIDNODE_H
