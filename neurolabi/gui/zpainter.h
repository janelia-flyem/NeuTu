#ifndef ZPAINTER_H
#define ZPAINTER_H

#include "zqtheader.h"

#ifdef _QT_GUI_USED_
#include <QPainter>
#endif

#include "zpoint.h"

/*!
 * \brief The painter class using QPainter to draw objects with extended options
 */
class ZPainter : public QPainter
{
public:
  inline void setOffset(double x, double y, double z) {
    m_offset.set(x, y, z);
  }

  inline const ZPoint& getOffset() { return m_offset; }

  void drawLine(int x1, int y1, int x2, int y2);

private:
  ZPoint m_offset;

};

#endif // ZPAINTER_H
