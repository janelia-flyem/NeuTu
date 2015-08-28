#ifndef ZFLYEMBODYCOLORSCHEME_H
#define ZFLYEMBODYCOLORSCHEME_H

#include <QHash>

#include "zobjectcolorscheme.h"

class ZFlyEmBodyAnnotation;

class ZFlyEmBodyColorScheme : public ZObjectColorScheme
{
public:
  ZFlyEmBodyColorScheme();

  QColor getColor(const ZFlyEmBodyAnnotation &annotation);

  void importColorMap(const QString &filePath);

private:
  QHash<QString, QColor> m_colorMap;
};

#endif // ZFLYEMBODYCOLORSCHEME_H
