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
  return getMainTarget().isValid();
}

void ZDvidEnv::clear()
{
  m_mainTarget = ZDvidTarget();
//  m_mainTarget.clear();
  for (auto &v : m_targetMap) {
    v.second.clear();
  }
}

ZDvidTarget &ZDvidEnv::getMainTarget()
{
  return const_cast<ZDvidTarget &>(
        static_cast<const ZDvidEnv&>(*this).getMainTarget());
}

const ZDvidTarget& ZDvidEnv::getMainTarget() const
{
  if (!m_mainTarget.isValid()) {
    const std::vector<ZDvidTarget> &targetList =
        ZDvidEnv::getTargetList(ERole::GRAYSCALE);
    if (!targetList.empty()) {
      return targetList.front();
    }
  }

  return m_mainTarget;
}

ZDvidTarget ZDvidEnv::getFullMainTarget() const
{
  ZDvidTarget target = m_mainTarget;

  if (!target.hasGrayScaleData()) {
    ZDvidTarget grayscaleTarget = getMainGrayscaleTarget();
    if (grayscaleTarget.hasGrayScaleData()) {
      target.setGrayScaleName(grayscaleTarget.getGrayScaleName());
      target.setGrayScaleSource(grayscaleTarget.getNode());
    }
  }

  return target;
}

void ZDvidEnv::setHost(const std::string &host)
{
  m_mainTarget.setServer(host);
}

void ZDvidEnv::setPort(int port)
{
  m_mainTarget.setPort(port);
}

void ZDvidEnv::setUuid(const std::string &uuid)
{
  m_mainTarget.setUuid(uuid);
}

void ZDvidEnv::setSegmentation(const std::string &name)
{
  m_mainTarget.setSegmentationName(name);
}

std::vector<ZDvidTarget>& ZDvidEnv::getTargetList(ERole role)
{
  return m_targetMap[role];
}

const ZDvidTarget& ZDvidEnv::getMainGrayscaleTarget() const
{
  if (m_mainTarget.hasGrayScaleData()) {
    return m_mainTarget;
  }

  const std::vector<ZDvidTarget> &targetList =
      ZDvidEnv::getTargetList(ERole::GRAYSCALE);
  if (!targetList.empty()) {
    return targetList.front();
  }

  return m_emptyTarget;
}

const std::vector<ZDvidTarget>& ZDvidEnv::getTargetList(ERole role) const
{
  return m_targetMap.at(role);
}

void ZDvidEnv::set(const ZDvidTarget &target)
{
  clear();

  m_mainTarget = target;
  m_mainTarget.clearGrayScale();
  m_targetMap[ERole::GRAYSCALE] = target.getGrayScaleTargetList();
}

namespace {

void append_dvid_target(
    std::vector<ZDvidTarget> &targetList, const ZDvidTarget &target)
{
  if (target.isValid()) {
    if (target.hasDvidUuid()) {
      targetList.push_back(target);
    } else {
      std::cout << "WARNING: Failed to add a target: DVID node with alias detected!";
    }
  }
}

}

void ZDvidEnv::appendValidDvidTarget(const ZDvidTarget &target, ERole role)
{
  append_dvid_target(getTargetList(role), target);
}

void ZDvidEnv::appendDvidTarget(
    std::vector<ZDvidTarget> &targetList, const ZJsonArray &arrayObj)
{
  for (size_t i = 0; i < arrayObj.size(); ++i) {
    ZDvidTarget target;
    target.loadJsonObject(ZJsonObject(arrayObj.value(i)));
    append_dvid_target(targetList, target);
  }
}

void ZDvidEnv::setReadOnly(bool on)
{
  getMainTarget().setReadOnly(on);
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
    m_mainTarget.clearGrayScale(); //Grayscale moved to target map
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
