#include "zimageframeshot.h"

#include <QPainter>

#include "geometry/zintcuboid.h"
#include "geometry/zintpoint.h"
#include "imgproc/zstacksource.h"
#include "zimage.h"
#include "zstack.hxx"

ZImageFrameShot::ZImageFrameShot(int w, int h)
{
  m_destWidth = w;
  m_destHeight = h;
}

void ZImageFrameShot::setStackSource(std::shared_ptr<ZStackSource> source)
{
  m_stackSource = source;
}

QImage ZImageFrameShot::takeShot(const QRect &viewport, int z)
{
  ZIntCuboid box;
  box.setMinCorner(viewport.x(), viewport.y(), z);
  box.setSize(viewport.width(), viewport.height(), 1);
  auto stack = m_stackSource->getStack(box, 0);
  ZIntPoint stackOffset = stack->getOffset();
  stack->setOffset(0, 0, 0);
  ZImage image(stack->width(), stack->height());
  image.setData(stack.get(), 0);

  QImage output(m_destWidth, m_destHeight,
                QImage::Format_ARGB32_Premultiplied);
  QPainter painter(&output);
  int sx = stack->getDsIntv().getX() + 1;
  int sy = stack->getDsIntv().getY() + 1;
  painter.drawImage(
        QRect(0, 0, m_destWidth, m_destHeight), image,
        QRect(viewport.x() / sx  - stackOffset.getX(),
              viewport.y() / sy - stackOffset.getY(),
              viewport.width() / sx, viewport.height() / sy));

  return output;
}
