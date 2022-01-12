#ifndef ZLABELCOLORTABLE_H
#define ZLABELCOLORTABLE_H

#include <QVector>
#include <QColor>

class ZLabelColorTable
{
public:
  ZLabelColorTable();

  const QColor& getColor(int label) const;
  static int GetColorCount();

public:
  const static int COLOR_COUNT;

private:
  static QVector<QColor> ConstructColorTable();

  const static QVector<QColor> m_colorTable;
  const static QColor m_blackColor;
  const static QColor m_transparentColor;
};

#endif // ZLABELCOLORTABLE_H
