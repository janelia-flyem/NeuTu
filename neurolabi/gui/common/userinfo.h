#ifndef USERINFO_H
#define USERINFO_H

#include <string>

namespace neutu {

/*!
 * \brief The class of origanizing user information.
 *
 * It is mainly designed based on data from config.int.janelia.org.
 */
class UserInfo
{
public:
  UserInfo();

  void setUserName(const std::string &name);
  void setOrganization(const std::string &org);
  void setLocation(const std::string &location);

  std::string getUserName() const;
  std::string getOrganization() const;
  std::string getLocation() const;

private:
  std::string m_name;
  std::string m_organization;
  std::string m_location;
};

}

#endif // USERINFO_H
