#include "zdvidenv.h"

const char* ZDvidEnv::KEY_GRAYSCALE = "@grayscale";
const char* ZDvidEnv::KEY_SEGMENTATION = "@segmentation";

ZDvidEnv::ZDvidEnv()
{
  enableRole(ERole::GRAYSCALE);
  enableRole(ERole::SEGMENTATION);
}

void ZDvidEnv::enableRole(ERole role)
{
  m_targetMap[role] = std::vector<ZDvidTarget>();
}

bool ZDvidEnv::isValid() const
{
  if (!m_mainTarget.isValid()) {
    for (auto &v : m_targetMap) {
      if (!v.second.empty()) {
        return true;
      }
      return false;
    }
  }

  return true;
}

void ZDvidEnv::clear()
{
  m_mainTarget.clear();
  for (auto &v : m_targetMap) {
    v.second.clear();
  }
}

ZDvidTarget &ZDvidEnv::getMainTarget()
{
  return m_mainTarget;
}

std::vector<ZDvidTarget>& ZDvidEnv::getTargetList(ERole role)
{
  return m_targetMap[role];
}

const ZDvidTarget &ZDvidEnv::getMainTarget() const
{
  return m_mainTarget;
}

const std::vector<ZDvidTarget>& ZDvidEnv::getTargetList(ERole role) const
{
  return m_targetMap.at(role);
}

void ZDvidEnv::set(const ZDvidTarget &target)
{
  clear();

  m_mainTarget = target;
//  m_mainTarget.clearGrayScale();
  m_targetMap[ERole::GRAYSCALE] = target.getGrayScaleTargetList();
}

void ZDvidEnv::appendDvidTarget(
    std::vector<ZDvidTarget> &targetList, const ZJsonArray &arrayObj)
{
  for (size_t i = 0; i < arrayObj.size(); ++i) {
    ZDvidTarget target;
    target.loadJsonObject(ZJsonObject(arrayObj.value(i)));
    if (target.isValid()) {
      targetList.push_back(target);
    }
  }
}

void ZDvidEnv::setMainTarget(const ZDvidTarget &target)
{
  m_mainTarget = target;
}

void ZDvidEnv::loadJsonObject(const ZJsonObject &obj)
{
  clear();

  m_mainTarget.loadJsonObject(obj);

  std::vector<ZDvidTarget>& grayscaleTargetList = m_targetMap[ERole::GRAYSCALE];

  if (m_mainTarget.hasGrayScaleData()) {
    grayscaleTargetList.push_back(m_mainTarget.getGrayScaleTarget());
//    m_mainTarget.clearGrayScale(); //Grayscale moved to target map
  }

  if (obj.hasKey(KEY_GRAYSCALE)) {
    ZJsonArray arrayObj(obj.value(KEY_GRAYSCALE));
    appendDvidTarget(grayscaleTargetList, arrayObj);
  }

  if (obj.hasKey(KEY_SEGMENTATION)) {
    std::vector<ZDvidTarget>& segTargetList = m_targetMap[ERole::GRAYSCALE];
    ZJsonArray arrayObj(obj.value(KEY_SEGMENTATION));
    appendDvidTarget(segTargetList, arrayObj);
  }
}

ZJsonObject ZDvidEnv::toJsonObject() const
{
  ZJsonObject obj = m_mainTarget.toJsonObject();

  {
    auto targetList = getTargetList(ERole::GRAYSCALE);
    if (!targetList.empty()) {
      ZJsonArray arrayObj;
      for (const ZDvidTarget &target : targetList) {
        arrayObj.append(target.toJsonObject());
      }
      obj.addEntry(KEY_GRAYSCALE, arrayObj);
    }
  }

  {
    auto targetList = getTargetList(ERole::SEGMENTATION);
    if (!targetList.empty()) {
      ZJsonArray arrayObj;
      for (const ZDvidTarget &target : targetList) {
        arrayObj.append(target.toJsonObject());
      }
      obj.addEntry(KEY_SEGMENTATION, arrayObj);
    }
  }

  return obj;
}
