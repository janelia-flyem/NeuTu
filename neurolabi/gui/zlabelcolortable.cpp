#include "zlabelcolortable.h"

const QVector<QColor> ZLabelColorTable::m_colorTable =
    ZLabelColorTable::constructColorTable();
const QColor ZLabelColorTable::m_blackColor = QColor(0, 0, 0, 255);
const QColor ZLabelColorTable::m_transparentColor = QColor(0, 0, 0, 0);

QVector<QColor> ZLabelColorTable::constructColorTable()
{
  QVector<QColor> colorTable(10);

  colorTable[0] = QColor(Qt::white);
  colorTable[1] = QColor(Qt::red);
  colorTable[2] = QColor(Qt::green);
  colorTable[3] = QColor(Qt::blue);
  colorTable[4] = QColor(Qt::cyan);
  colorTable[5] = QColor(Qt::magenta);
  colorTable[6] = QColor(Qt::yellow);
  colorTable[7] = QColor(Qt::darkCyan);
  colorTable[8] = QColor(165, 42, 42);
  colorTable[9] = QColor(255, 140, 0);

  for (QVector<QColor>::iterator iter = colorTable.begin();
       iter != colorTable.end(); ++iter) {
    iter->setAlpha(128);
  }

  return colorTable;
}

ZLabelColorTable::ZLabelColorTable()
{
}

const QColor& ZLabelColorTable::getColor(int label) const
{
  if (label < 0) {
    return m_transparentColor;
  }

  if (label == 255) {
    return m_blackColor;
  }

  int index = label % m_colorTable.size();

  return m_colorTable[index];
}
