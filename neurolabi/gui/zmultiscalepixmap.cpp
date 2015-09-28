#include "zmultiscalepixmap.h"
#include "zpixmap.h"

ZMultiscalePixmap::ZMultiscalePixmap()
{
  init();
}

ZMultiscalePixmap::~ZMultiscalePixmap()
{
  clear();
}

void ZMultiscalePixmap::init()
{

}

void ZMultiscalePixmap::clear()
{
  for (std::vector<ZPixmap*>::iterator iter = m_pixmapArray.begin();
       iter != m_pixmapArray.end(); ++iter) {
    delete *iter;
  }

  m_pixmapArray.clear();
}


void ZMultiscalePixmap::setSize(const QSize &size)
{
  if (m_originalSize != size) {
    clear();
  }

  m_originalSize = size;
}

void ZMultiscalePixmap::setOffset(const QPoint &offset)
{
  m_offset = offset;
}

int ZMultiscalePixmap::getScale(int level) const
{
  return level + 1;
}

ZPixmap* ZMultiscalePixmap::getPixmap(int level)
{
  if (level > (int) m_pixmapArray.size()) {
    m_pixmapArray.resize(level + 1, 0x0);
  }

  if (m_pixmapArray[level] == NULL) {
    int scale = getScale(level);

    ZPixmap *pixmap = new ZPixmap(m_originalSize / scale);
    pixmap->setOffset(m_offset.x(), m_offset.y());
    if (scale > 1) {
      pixmap->setScale(1.0 / scale, 1.0 / scale);
    }
    m_pixmapArray[level] = pixmap;
  }

  return m_pixmapArray[level];
}
