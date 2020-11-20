#include "zmenuconfig.h"

#include <iostream>

ZMenuConfig::ZMenuConfig() : m_groupHelper(this)
{
}

void ZMenuConfig::append(const QString &groupName, ZActionFactory::EAction action)
{
  m_actionGroup.emplace_back(groupName, action);
}

void ZMenuConfig::append(ZActionFactory::EAction action)
{
  append("", action);
}

ZActionFactory::EAction ZMenuConfig::getLastAction() const
{
  ZActionFactory::EAction action = ZActionFactory::ACTION_NULL;
  if (!m_actionGroup.empty()) {
    action = m_actionGroup.back().second;
  }

  return action;
}

void ZMenuConfig::appendSeparator(const QString &groupName)
{
  if (!m_actionGroup.empty()) {
    const auto &action =  m_actionGroup.back();
    bool appending = true;
    if (action.first == groupName) {
      if (action.second == ZActionFactory::ACTION_SEPARATOR) {
        appending = false;
      }
    }

    if (appending) {
      append(groupName, ZActionFactory::ACTION_SEPARATOR);
    }
  }
}

ZMenuConfig::const_iterator ZMenuConfig::begin() const
{
  return m_actionGroup.begin();
}

ZMenuConfig::const_iterator ZMenuConfig::end() const
{
  return m_actionGroup.end();
}

ZMenuConfig& ZMenuConfig::operator <<(ZActionFactory::EAction action)
{
  append(action);

  return *this;
}

ZMenuConfig::GroupHelper& ZMenuConfig::operator <<(const QString &name)
{
  m_groupHelper.setGroupName(name);
  return m_groupHelper;
}

QString ZMenuConfig::GroupHelper::getGroupName() const
{
  return m_groupName;
}

void ZMenuConfig::GroupHelper::setGroupName(const QString &groupName)
{
  m_groupName = groupName;
}

ZMenuConfig::GroupHelper& ZMenuConfig::GroupHelper::operator <<(
    ZActionFactory::EAction action)
{
  m_config->append(getGroupName(), action);

  return *this;
}

std::ostream& operator <<(
    std::ostream &stream, const ZMenuConfig &config)
{
  stream << "Menu config: ";
  for (const ZMenuConfig::TElement &item : config) {
    if (item.first.isEmpty()) {
      stream << item.second << ";";
    } else {
      stream << item.first.toStdString() << ":" << item.second << ";";
    }
  }

  return stream;
}

/*
void ZMenuConfig::startGroup(const QString &groupName)
{
  m_currentGroupName = groupName;
}

void ZMenuConfig::endGroup()
{
  m_currentGroupName = "";
}
*/
