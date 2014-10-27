#ifndef ZLABELCOLORTABLE_H
#define ZLABELCOLORTABLE_H

#include <QVector>
#include <QColor>

class ZLabelColorTable
{
public:
  ZLabelColorTable();

  const QColor& getColor(int label) const;

private:
  static QVector<QColor> constructColorTable();

  const static QVector<QColor> m_colorTable;
  const static QColor m_blackColor;
  const static QColor m_transparentColor;
};

#endif // ZLABELCOLORTABLE_H
