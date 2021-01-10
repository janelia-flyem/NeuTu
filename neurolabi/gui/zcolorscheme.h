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

  uint32_t getColorCode(int index) const;
  uint32_t getColorCode(uint64_t index) const;
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

  static constexpr uint32_t GetIntCode(int r, int g, int b, int a) {
    return (a << 24) + (r << 16) + (g << 8) + b;
  }

  static constexpr uint32_t GetIntCode(int r, int g, int b) {
    return GetIntCode(r, g, b, 255);
  }

  static uint32_t GetIntCode(const QColor &color) {
//    return color.rgba(); //might be faster, but not worth it in practice
    return GetIntCode(color.red(), color.green(), color.blue(), color.alpha());
  }

  static QColor GetColorFromCode(uint32_t code) {
    return QColor((code & 0x00FF0000) >> 16, (code & 0x0000FF00) >> 8,
                  code & 0x000000FF, code >> 24);
  }

protected:
  void buildRandomColorTable(int n);
  void buildConvRandomColorTable(int n);
  void buildConvRandomColorTable(int n, int seed);
  void buildPunctumColorTable();
  void buildUniqueColorTable();
  void buildLabelColorTable();

protected:
  EColorScheme m_colorScheme;
  QVector<uint32_t> m_colorTable;
  int m_startIndex = 0;
};

#endif // ZCOLORSCHEME_H
