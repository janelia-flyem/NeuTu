#include "zdvidtile.h"

#include <QTransform>
#include "zstack.hxx"
#include "zstackfactory.h"
#include "zdvidreader.h"
#include "zimage.h"
#include "zpainter.h"

ZDvidTile::ZDvidTile()
{
}

ZDvidTile::~ZDvidTile()
{
  clear();
}

ZSTACKOBJECT_DEFINE_CLASS_NAME(ZDvidTile)

void ZDvidTile::clear()
{
  m_dvidTarget.clear();
}

void ZDvidTile::loadDvidPng(const QByteArray &buffer)
{
  m_image.loadFromData(buffer);
}

void ZDvidTile::display(
    ZPainter &painter, int slice, EDisplayStyle /*option*/) const
{
#if 0
  if (m_stack != NULL) {
    int z = painter.getOffset().z() + slice;
    update(z);

    ZImage image = getImage();
    ZPoint pt = getFinalOffset() - painter.getOffset();

    painter.save();

    QTransform transform;
    transform.scale(m_res.getScale(), m_res.getScale());
    //transform.translate(pt.x(), pt.y());
    painter.setTransform(transform);
    painter.drawImage(pt.x(), pt.y(), image);

    painter.restore();
  }
#endif
}

void ZDvidTile::update(int x, int y, int z, int width, int height)
{
#if 0
  bool updating = false;
  if (m_stack == NULL) {
    m_stack = ZStackFactory::makeZeroStack(GREY, width, height, 1);
    m_stack->setOffset(x, y, z);
    updating = true;
  } else if (m_stack->getOffset().getZ() != z ||
             m_stack->getOffset().getX() != x ||
             m_stack->getOffset().getZ() != z ||
             m_stack->width() != width || m_stack->height() != height) {
    updating = true;
  }

  if (updating) {
    ZDvidReader reader;
    if (reader.open(m_dvidTarget)) {
      Stack *stack = reader.readTile(x, y, z, width, heigth, m_res.getLevel());
    }
  }
#endif
}

void ZDvidTile::setTileOffset(int x, int y, int z)
{
  m_offset.set(x, y, z);
}

void ZDvidTile::printInfo() const
{
  std::cout << "Dvid tile: " << std::endl;
  m_res.print();
  std::cout << "Offset: " << m_offset.toString() << std::endl;
  std::cout << "Size: " << m_image.width() << " x " << m_image.height()
            << std::endl;

}

void ZDvidTile::setResolutionLevel(int level)
{
  m_res.setLevel(level);
}
