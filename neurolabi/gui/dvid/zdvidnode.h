#ifndef ZDVIDNODE_H
#define ZDVIDNODE_H

#include <string>

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

  std::string getUrl() const;

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
  void init();

private:
  std::string m_address;
  std::string m_uuid;
  int m_port;

  const static char* m_addressKey;
  const static char* m_portKey;
  const static char* m_uuidKey;
};

#endif // ZDVIDNODE_H
