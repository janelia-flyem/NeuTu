#ifndef ZCOLORSCHEME_H
#define ZCOLORSCHEME_H

#include <QColor>
#include <QVector>

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
  int getColorNumber() const { return m_colorTable.size(); }

  virtual void setColorScheme(EColorScheme scheme);

protected:
  void buildRandomColorTable(int n);
  void buildConvRandomColorTable(int n);
  void buildPunctumColorTable();

protected:
  EColorScheme m_colorScheme;
  QVector<QColor> m_colorTable;
};

#endif // ZCOLORSCHEME_H
