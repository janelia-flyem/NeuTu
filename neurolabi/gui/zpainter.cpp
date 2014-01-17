#include "zpainter.h"

void ZPainter::drawLine(int x1, int y1, int x2, int y2)
{
#ifdef _QT_GUI_USED_
  x1 += m_offset.x();
  y1 += m_offset.y();
  x2 += m_offset.x();
  y2 += m_offset.z();

  QPainter::drawLine(x1, y1, x2, y2);
#endif
}
