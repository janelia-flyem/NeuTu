#ifndef ZFLYEMNEURONINFO_H
#define ZFLYEMNEURONINFO_H

#include <map>
#include <string>
#include <set>

namespace flyem {
class ZFlyEmTypeSet : public std::set<std::string>
{
public:
  ZFlyEmTypeSet();

};

class ZTypeClassMap : public std::map<std::string, std::string>
{
public:
  ZTypeClassMap();
};

class ZClassSuperMap : public std::map<std::string, std::string>
{
public:
  ZClassSuperMap();
};

}

class ZFlyEmNeuronInfo
{
public:
  ZFlyEmNeuronInfo();

  static std::string GetClassFromType(const std::string &type);
  static std::string GetSuperclassFromType(const std::string &type);
  static std::string GuessTypeFromName(const std::string &name);

private:
  static const flyem::ZTypeClassMap m_typeClassMap;
  static const flyem::ZClassSuperMap m_classSuperMap;
  static const flyem::ZFlyEmTypeSet m_typeSet;
};

#endif // ZFLYEMNEURONINFO_H
