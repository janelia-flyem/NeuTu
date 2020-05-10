#ifndef ZCOLORSCHEME_H
#define ZCOLORSCHEME_H

#include <cstdint>

#include <QColor>
#include <QVector>

class ZColorScheme
{
public:
  ZColorScheme();
  virtual ~ZColorScheme() {}

  enum EColorScheme {
    VAA3D_TYPE_COLOR, BIOCYTIN_TYPE_COLOR, JIN_TYPE_COLOR, UNIQUE_COLOR,
    PUNCTUM_TYPE_COLOR, RANDOM_COLOR, CONV_RANDOM_COLOR, GMU_TYPE_COLOR,
    LABEL_COLOR
  };

  QColor getColor(int index) const;
  QColor getColor(uint64_t index) const;
  virtual int getColorNumber() const { return m_colorTable.size(); }

  virtual void setColorScheme(EColorScheme scheme);

  /*
  const QVector<QColor>& getColorTable() const {
    return m_colorTable;
  }
  */

  void setStartIndex(int startIndex);

  void printColorTable() const;

protected:
  void buildRandomColorTable(int n);
  void buildConvRandomColorTable(int n);
  void buildConvRandomColorTable(int n, int seed);
  void buildPunctumColorTable();
  void buildUniqueColorTable();
  void buildLabelColorTable();

protected:
  EColorScheme m_colorScheme;
  QVector<QColor> m_colorTable;
  int m_startIndex = 0;
};

#endif // ZCOLORSCHEME_H
