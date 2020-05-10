#include "zflyembodycoloroption.h"

QMap<QString, ZFlyEmBodyColorOption::EColorOption>
ZFlyEmBodyColorOption::m_colorMap = ZFlyEmBodyColorOption::InitColorMap();

ZFlyEmBodyColorOption::ZFlyEmBodyColorOption()
{
}

QMap<QString, ZFlyEmBodyColorOption::EColorOption>
ZFlyEmBodyColorOption::InitColorMap()
{
  QMap<QString, ZFlyEmBodyColorOption::EColorOption> colorMap;
  colorMap["Normal"] = BODY_COLOR_NORMAL;
  colorMap["Random"] = BODY_COLOR_RANDOM;
  colorMap["Name"] = BODY_COLOR_NAME;
  colorMap["Sequencer"] = BODY_COLOR_SEQUENCER;
  colorMap["Protocol"] = BODY_COLOR_PROTOCOL;
  colorMap["Sequencer+Normal"] = BODY_COLOR_SEQUENCER_NORMAL;

  return colorMap;
}

QString ZFlyEmBodyColorOption::GetColorMapName(EColorOption option)
{
  QString name;

  for (QMap<QString, EColorOption>::const_iterator iter = m_colorMap.begin();
       iter != m_colorMap.end(); ++iter) {
    if (iter.value() == option) {
      name = iter.key();
      break;
    }
  }

  return name;
}

ZFlyEmBodyColorOption::EColorOption
ZFlyEmBodyColorOption::GetColorOption(const QString colorName)
{
  return m_colorMap[colorName];
}
