#include "zimageframeshot.h"

#include <QPainter>

#include "common/math.h"
#include "geometry/zintcuboid.h"
#include "geometry/zintpoint.h"
#include "geometry/zgeometry.h"
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

int ZImageFrameShot::estimateZoom(int sourceWidth, int sourceHeight) const
{
  double zoomRatio = std::min(
        double(m_destWidth) / sourceWidth, double(m_destHeight) / sourceHeight);
  if (zoomRatio <= 0.5) {
    return std::min(m_stackSource->getMaxZoom(),
                    zgeom::GetZoomLevel(std::floor(1.0 / zoomRatio)));
  }

  return 0;
}

QRect ZImageFrameShot::getViewport(int cx, int cy, double zoomRatio) const
{
  return getViewport(cx, cy, neutu::iround(m_destWidth / zoomRatio),
                     neutu::iround(m_destHeight / zoomRatio));
}

QRect ZImageFrameShot::getViewport(int cx, int cy, int width, int height) const
{
  return QRect(cx - width / 2, cy - height / 2, width, height);
}

QImage ZImageFrameShot::takeShot(const QRect &viewport, int z)
{
  ZIntCuboid box;
  int zoom = estimateZoom(viewport.width(), viewport.height());
  int zoomRatio = zgeom::GetZoomScale(zoom);
  box.setMinCorner(viewport.x() / zoomRatio, viewport.y() / zoomRatio, z / zoomRatio);
  box.setSize(viewport.width() / zoomRatio, viewport.height() / zoomRatio, 1);

  auto stack = m_stackSource->getStack(box, zoom);
  auto format = QImage::Format_ARGB32_Premultiplied;
  if (stack) {
    if (stack->kind() != GREY) {
      format = QImage::Format_ARGB32_Premultiplied;
    }
  }
  QImage output(m_destWidth, m_destHeight, format);
//                QImage::Format_ARGB32_Premultiplied);
  output.fill(Qt::black);
  if (stack) {
    ZIntPoint stackOffset = stack->getOffset();
    stack->setOffset(0, 0, 0);
    ZImage image(stack->width(), stack->height(), format);
    image.setData(stack.get(), 0);

    QPainter painter(&output);
    int sx = stack->getDsIntv().getX() + 1;
    int sy = stack->getDsIntv().getY() + 1;
    int x0 = viewport.x() / sx  - stackOffset.getX();
    int y0 = viewport.y() / sy - stackOffset.getY();
    painter.drawImage(
          QRect(0, 0, m_destWidth, m_destHeight), image,
          QRect(x0, y0, viewport.width() / sx, viewport.height() / sy));
  }

  return output;
}
