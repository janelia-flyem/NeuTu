#include "zflyembodycoloroption.h"

//QMap<QString, ZFlyEmBodyColorOption::EColorOption>
//ZFlyEmBodyColorOption::m_colorNameList = ZFlyEmBodyColorOption::InitColorNameList();

QMap<QString, ZFlyEmBodyColorOption::EColorOption>
ZFlyEmBodyColorOption::m_colorMap = ZFlyEmBodyColorOption::InitColorMap();

ZFlyEmBodyColorOption::ZFlyEmBodyColorOption()
{
}

/*
QList<QString> ZFlyEmBodyColorOption::InitColorNameList()
{
  QList<QString> colorNameList;
  colorNameList << "Normal" << "Name" << "Sequencer" << "Focused";

  return colorNameList;
}
*/

QMap<QString, ZFlyEmBodyColorOption::EColorOption>
ZFlyEmBodyColorOption::InitColorMap()
{
  QMap<QString, ZFlyEmBodyColorOption::EColorOption> colorMap;
  colorMap["Normal"] = BODY_COLOR_NORMAL;
  colorMap["Name"] = BODY_COLOR_NAME;
  colorMap["Sequencer"] = BODY_COLOR_SEQUENCER;
  colorMap["Focused"] = BODY_COLOR_FOCUSED;

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
