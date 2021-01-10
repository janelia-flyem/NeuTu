#ifndef ZFLYEMBODYCOLOROPTION_H
#define ZFLYEMBODYCOLOROPTION_H

#include <QMap>
#include <QString>

class ZFlyEmBodyColorOption
{
public:
  ZFlyEmBodyColorOption();

  enum EColorOption {
    BODY_COLOR_NORMAL, BODY_COLOR_NAME, BODY_COLOR_SEQUENCER, BODY_COLOR_PROTOCOL,
    BODY_COLOR_SEQUENCER_NORMAL, BODY_COLOR_RANDOM
  };

  static QString GetColorMapName(EColorOption option);
  static EColorOption GetColorOption(const QString colorName);

private:
  static QMap<QString, EColorOption> InitColorMap();
  static QList<QString> InitColorNameList();

private:
//  static QList<QString> m_colorNameList;
  static QMap<QString, EColorOption> m_colorMap;
};

#endif // ZFLYEMBODYCOLOROPTION_H
