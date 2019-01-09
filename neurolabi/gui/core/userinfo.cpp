#include "userinfo.h"

neutu::UserInfo::UserInfo()
{

}

void neutu::UserInfo::setUserName(const std::string &name)
{
  m_name = name;
}

void neutu::UserInfo::setOrganization(const std::string &org)
{
  m_organization = org;
}

void neutu::UserInfo::setLocation(const std::string &location)
{
  m_location = location;
}

std::string neutu::UserInfo::getUserName() const
{
  return m_name;
}

std::string neutu::UserInfo::getOrganization() const
{
  return m_organization;
}

std::string neutu::UserInfo::getLocation() const
{
  return m_location;
}
