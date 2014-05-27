#ifndef ZDVIDTARGET_H
#define ZDVIDTARGET_H

#include <string>

/*!
 * \brief The class of representing the location of a dvid server
 */
class ZDvidTarget
{
public:
  ZDvidTarget();
  ZDvidTarget(const std::string &address, const std::string &uuid, int port = -1);

  void set(const std::string &address, const std::string &uuid, int port = -1);

  /*!
   * \brief Set dvid target from source string
   *
   * The old settings will be cleared no matter what.
   *
   * \param sourceString Must start with "http:".
   */
  void set(const std::string &sourceString);

  inline const std::string& getAddress() const {
    return m_address;
  }

  std::string getAddressWithPort() const;

  inline const std::string& getUuid() const {
    return m_uuid;
  }

  inline int getPort() const {
    return m_port;
  }

  std::string getUrl() const;
  std::string getUrl(const std::string &dataName) const;

  /*!
   * \brief Get a single string to represent the target
   *
   * \return "http:address:port:uuid"
   */
  std::string getSourceString() const;

  /*!
   * \brief getBodyPath
   *
   * The functions does not check if a body exists.
   *
   * \return The path of a certain body.
   */
  std::string getBodyPath(int bodyId) const;

  /*!
   * \brief Test if the target is valid
   *
   * \return true iff the target is valid.
   */
  bool isValid() const;

  void print() const;

private:
  std::string m_address;
  std::string m_uuid;
  int m_port;
};

#endif // ZDVIDTARGET_H
