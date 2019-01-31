#include "zswccolorparam.h"

ZSwcColorParam::ZSwcColorParam()
{

}

void ZSwcColorParam::setColorScheme(
    const ZColorScheme::EColorScheme colorScheme)
{
  m_scheme.setColorScheme(colorScheme);
}

void ZSwcColorParam::setPrefix(const QString &prefix)
{
  m_prefix = prefix;
}

void ZSwcColorParam::remove(void *obj)
{
  auto iter = m_mapper.find(obj);
  if (iter != m_mapper.end()) {
    m_paramTaken[iter->second] = false;
    m_mapper.erase(iter);
  }
}

void ZSwcColorParam::makeColorPamater(void *obj)
{
  int index = m_paramList.size();

  QColor color = m_scheme.getColor(index);
  QString name = m_prefix + QString(" %1 Color").arg(index + 1);
  std::shared_ptr<ZVec4Parameter> param = std::make_shared<ZVec4Parameter>(
        name, glm::vec4(color.redF(), color.greenF(), color.blueF(), 1.f));
  param->setStyle("COLOR");
//  param->setWidgetSyncMutex(m_syncMutex);

  m_paramList.push_back(param);
  m_mapper[obj] = index;
  m_paramTaken.push_back(true);
}

ZVec4Parameter& ZSwcColorParam::getColorParameter(void *obj)
{
  return *getColorParameterPtr(obj);
}

ZVec4Parameter* ZSwcColorParam::getColorParameterPtr(void *obj)
{
  int colorIndex = -1;

  if (m_mapper.count(obj) == 0) {
    for (size_t i = 0; i < m_paramTaken.size(); ++i) {
      if (!m_paramTaken[i]) {
        m_mapper[obj] = i;
        colorIndex = i;
        m_paramTaken[i] = true;
        break;
      }
    }

    if (colorIndex < 0) {
      makeColorPamater(obj);
      colorIndex = m_paramList.size() - 1;
    }

//    if (m_host) {
//      QObject::connect(
//            m_paramList[colorIndex].get(), &ZVec4Parameter::valueChanged,
//            m_host, &Z3DSwcFilter::prepareColor);
//    }
  } else {
    colorIndex = m_mapper.at(obj);
  }

  return m_paramList[colorIndex].get();
}

std::map<void*, ZVec4Parameter*> ZSwcColorParam::getColorParamMap() const
{
  std::map<void*, ZVec4Parameter*> paramMap;

  for (auto m : m_mapper) {
    paramMap[m.first] = m_paramList[m.second].get();
  }

  return paramMap;
}

bool ZSwcColorParam::contains(void *obj) const
{
  return m_mapper.count(obj) > 0;
}
