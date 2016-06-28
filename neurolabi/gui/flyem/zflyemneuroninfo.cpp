#include "zflyemneuroninfo.h"

const FlyEm::ZTypeClassMap ZFlyEmNeuronInfo::m_typeClassMap =
    FlyEm::ZTypeClassMap();

const FlyEm::ZClassSuperMap ZFlyEmNeuronInfo::m_classSuperMap =
    FlyEm::ZClassSuperMap();

const FlyEm::ZFlyEmTypeSet ZFlyEmNeuronInfo::m_typeSet =
    FlyEm::ZFlyEmTypeSet();

ZFlyEmNeuronInfo::ZFlyEmNeuronInfo()
{
}

std::string ZFlyEmNeuronInfo::GetClassFromType(const std::string &type)
{
  if (m_typeClassMap.count(type) == 0) {
    return "";
  }

  return m_typeClassMap.at(type);
}

std::string ZFlyEmNeuronInfo::GetSuperclassFromType(const std::string &type)
{
  if (type == "T1") {
    return "Lamina";
  }

  std::string className = GetClassFromType(type);

  if (m_classSuperMap.count(className) == 0) {
    return "";
  }

  return m_classSuperMap.at(className);
}

std::string ZFlyEmNeuronInfo::GuessTypeFromName(const std::string &name)
{
  size_t len = name.size();
  while (len-- > 0) {
    std::string substr = name.substr(0, len);
    if (m_typeSet.count(substr) > 0) {
      //digit check
      while (len < name.size()){
        if (name[len] != ' ' && name[len] != '-') {
//        if (isdigit(name[len])) {
          substr += name[len];
        } else {
          break;
        }
        len++;
      }

      //check '-like'
      if (len + 5 <= name.size()) {
        if (name.substr(len, 5) == "-like") {
          substr += "-like";
        }
      }

      return substr;
    }
  }

  return "";
}

FlyEm::ZFlyEmTypeSet::ZFlyEmTypeSet()
{
  insert("C");
  insert("C2");
  insert("C3");
  insert("Dm");
  insert("Dm1");
  insert("Dm2");
  insert("Dm3");
  insert("Dm4");
  insert("Dm5");
  insert("Dm6");
  insert("Dm7");
  insert("Dm8");
  insert("Dm9");
  insert("L");
  insert("L1");
  insert("L2");
  insert("L3");
  insert("L4");
  insert("L5");
  insert("Lawf1");
  insert("Lawf2");
  insert("Mi");
  insert("Mi1");
  insert("Mi10");
  insert("Mi2");
  insert("Mi1");
  insert("Mi3");
  insert("Mi4");
  insert("Mi9");
  insert("Pm");
  insert("Pm1");
  insert("R");
  insert("R7");
  insert("R8");
  insert("T");
  insert("T1");
  insert("T2");
  insert("T2a");
  insert("T3");
  insert("T4");
  insert("Tm");
  insert("Tm1");
  insert("Tm3");
  insert("Tm4");
  insert("Tm5a");
  insert("Tm5b");
  insert("Tm5c");
  insert("Tm5Y");
  insert("Tm6/14");
  insert("Tm8/22");
  insert("Tm9");
  insert("TmY");
  insert("TmY3");
  insert("Y");
}

FlyEm::ZTypeClassMap::ZTypeClassMap()
{
  (*this)["C"] = "C";
  (*this)["C2"] = "C";
  (*this)["C3"] = "C";
  (*this)["Dm"] = "Dm";
  (*this)["Dm1"] = "Dm";
  (*this)["Dm2"] = "Dm";
  (*this)["Dm3"] = "Dm";
  (*this)["Dm4"] = "Dm";
  (*this)["Dm5"] = "Dm";
  (*this)["Dm6"] = "Dm";
  (*this)["Dm7"] = "Dm";
  (*this)["Dm8"] = "Dm";
  (*this)["Dm9"] = "Dm";
  (*this)["L"] = "L";
  (*this)["L1"] = "L";
  (*this)["L2"] = "L";
  (*this)["L3"] = "L";
  (*this)["L4"] = "L";
  (*this)["L5"] = "L";
  (*this)["Lawf1"] = "L";
  (*this)["Lawf2"] = "L";
  (*this)["Mi"] = "Mi";
  (*this)["Mi1"] = "Mi";
  (*this)["Mi10"] = "Mi";
  (*this)["Mi15"] = "Mi";
  (*this)["Mi2"] = "Mi";
  (*this)["Mi1"] = "Mi";
  (*this)["Mi3"] = "Mi";
  (*this)["Mi4"] = "Mi";
  (*this)["Mi9"] = "Mi";
  (*this)["Pm"] = "Pm";
  (*this)["Pm1"] = "Pm";
  (*this)["R"] = "R";
  (*this)["R7"] = "R";
  (*this)["R8"] = "R";
  (*this)["T"] = "T";
  (*this)["T1"] = "T";
  (*this)["T2"] = "T";
  (*this)["T2a"] = "T";
  (*this)["T3"] = "T";
  (*this)["T4"] = "T";
  (*this)["Tm"] = "Tm";
  (*this)["Tm1"] = "Tm";
  (*this)["Tm3"] = "Tm";
  (*this)["Tm4"] = "Tm";
  (*this)["Tm5a"] = "Tm";
  (*this)["Tm5b"] = "Tm";
  (*this)["Tm5c"] = "Tm";
  (*this)["Tm5Y"] = "Tm";
  (*this)["Tm6/14"] = "Tm";
  (*this)["Tm8/22"] = "Tm";
  (*this)["Tm9"] = "Tm";
  (*this)["TmY"] = "TmY";
  (*this)["TmY3"] = "TmY";
  (*this)["Y"] = "Y";
}

FlyEm::ZClassSuperMap::ZClassSuperMap()
{
  (*this)["C"] = "Lamina";
  (*this)["Dm"] = "Intrinsic Medulla";
  (*this)["Dm/Mi"] = "Intrinsic Medulla";
  (*this)["L"] = "Lamina";
  (*this)["Mi"] = "Intrinsic Medulla";
  (*this)["R"] = "Retina";
  (*this)["Tm"] = "Projection Medulla";
  (*this)["TmY"] = "Projection Medulla";
  (*this)["Y"] = "T/Y";
}

