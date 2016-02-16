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
  int level = 0;
  for (std::vector<ZPixmap*>::iterator iter = m_pixmapArray.begin();
       iter != m_pixmapArray.end(); ++iter, ++level) {
    ZPixmap *pixmap = *iter;
    if (pixmap != NULL) {
      int scale = getScale(level);
      pixmap->setOffset(m_offset.x() / scale, m_offset.y() / scale);
    }
  }
}

int ZMultiscalePixmap::getScale(int level) const
{
  return level + 1;
}

ZPixmap* ZMultiscalePixmap::getPixmap(int level)
{
  if (m_originalSize.isEmpty()) {
    return NULL;
  }

  if (level >= (int) m_pixmapArray.size()) {
    m_pixmapArray.resize(level + 1, 0x0);
  }

  if (m_pixmapArray[level] == NULL) {
    int scale = getScale(level);

    ZPixmap *pixmap = new ZPixmap(m_originalSize / scale);
    pixmap->setVisible(m_visible);
    pixmap->setOffset(m_offset.x() / scale, m_offset.y() / scale);
    if (scale > 1) {
      pixmap->setScale(1.0 / scale, 1.0 / scale);
    }
    m_pixmapArray[level] = pixmap;
  }

  return m_pixmapArray[level];
}

int ZMultiscalePixmap::getWidth() const
{
  return m_originalSize.width();
}

int ZMultiscalePixmap::getHeight() const
{
  return m_originalSize.height();
}

int ZMultiscalePixmap::getTx() const
{
  return m_offset.x();
}

int ZMultiscalePixmap::getTy() const
{
  return m_offset.y();
}

void ZMultiscalePixmap::setVisible(bool visible)
{
  m_visible = visible;
  for (std::vector<ZPixmap*>::iterator iter = m_pixmapArray.begin();
       iter != m_pixmapArray.end(); ++iter) {
    ZPixmap *pixmap = *iter;
    if (pixmap != NULL) {
      pixmap->setVisible(visible);
    }
  }
}

bool ZMultiscalePixmap::isEmpty() const
{
  return m_originalSize.isEmpty();
}
