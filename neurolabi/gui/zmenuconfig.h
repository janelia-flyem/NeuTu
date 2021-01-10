#ifndef ZMENUCONFIG_H
#define ZMENUCONFIG_H

#include <QString>
#include <vector>

#include "zactionfactory.h"

class ZMenuConfig
{
public:
  ZMenuConfig();

  using TElement = std::pair<QString, ZActionFactory::EAction>;
  using TActionGroup = std::vector<TElement>;
  using const_iterator =  TActionGroup::const_iterator;

  void append(const QString &groupName, ZActionFactory::EAction action);
  void append(ZActionFactory::EAction action);
  void appendSeparator(const QString &groupName = "");

  ZActionFactory::EAction getLastAction() const;

  const_iterator begin() const;
  const_iterator end() const;

  ZMenuConfig& operator << (ZActionFactory::EAction action);

  friend std::ostream& operator << (
      std::ostream &stream, const ZMenuConfig &config);

  struct GroupHelper {
    GroupHelper(ZMenuConfig *config) : m_config(config) {}
    GroupHelper& operator << (ZActionFactory::EAction action);
    void setGroupName(const QString &groupName);
    QString getGroupName() const;

  private:
    QString m_groupName;
    ZMenuConfig *m_config = NULL;
  };

  GroupHelper& operator << (const QString &name);

private:
  TActionGroup m_actionGroup;
  GroupHelper m_groupHelper;
};

#endif // ZMENUCONFIG_H
