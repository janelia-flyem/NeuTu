#ifndef ZCOLORSCHEME_H
#define ZCOLORSCHEME_H

#include <QColor>
#include <QVector>

#include "tz_stdint.h"

class ZColorScheme
{
public:
  ZColorScheme();
  virtual ~ZColorScheme() {}

  enum EColorScheme {
    VAA3D_TYPE_COLOR, BIOCYTIN_TYPE_COLOR, JIN_TYPE_COLOR, UNIQUE_COLOR,
    PUNCTUM_TYPE_COLOR, RANDOM_COLOR, CONV_RANDOM_COLOR, GMU_TYPE_COLOR
  };

  QColor getColor(int index) const;
  QColor getColor(uint64_t index) const;
  int getColorNumber() const { return m_colorTable.size(); }

  virtual void setColorScheme(EColorScheme scheme);

  const QVector<QColor>& getColorTable() const {
    return m_colorTable;
  }

protected:
  void buildRandomColorTable(int n);
  void buildConvRandomColorTable(int n);
  void buildPunctumColorTable();
  void buildUniqueColorTable();

protected:
  EColorScheme m_colorScheme;
  QVector<QColor> m_colorTable;
};

#endif // ZCOLORSCHEME_H
