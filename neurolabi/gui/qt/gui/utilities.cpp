#include "utilities.h"

#include <QTextDocument>
#include <QPixmap>
#include <QPainter>
#include <QLayout>

QString neutu::GetKeyString(int key, const Qt::KeyboardModifiers &modifier)
{
  QString str;

  if (modifier & Qt::ShiftModifier) {
    str = "Shift ";
  }

  if (modifier & Qt::ControlModifier) {
    str = "Control ";
  }

  if (modifier & Qt::AltModifier) {
    str = "Alt ";
  }

  if (modifier == Qt::NoModifier) {
    str += QKeySequence(key).toString();
  }

  return str;
}

void neutu::SetHtmlIcon(QPushButton *button, const QString &text)
{
  QTextDocument doc;
  doc.setHtml(text);

  QPixmap pixmap(doc.size().toSize());
  pixmap.fill(Qt::transparent);
  QPainter painter(&pixmap);
  doc.drawContents(&painter, pixmap.rect());

  button->setIcon(QIcon(pixmap));
}

namespace {

QSize get_canvas_size(const QStringList &text)
{
  int maxLength = 0;
  QString compText;
  foreach (const QString &str, text) {
    if (str.length() > maxLength) {
      maxLength = str.length();
    }
    compText += str + "\n";
  }

  maxLength = std::min(maxLength, 80);

  int width = maxLength * 8;
  int height = text.length() * 18;

  return QSize(width, height);
}

}

/************
 * 0 -- 3
 * |    |
 * 1 -- 2
 */
void neutu::DrawText(
    QPainter &painter, const QSize &windowSize, int cornerIndex,
    const QStringList &text)
{
  QSize canvasSize = get_canvas_size(text);

  QPoint pos(10, 1);
  if (cornerIndex == 1 || cornerIndex == 2) {
    pos.setY(std::max(0, windowSize.height() - canvasSize.height()));
  }

  if (cornerIndex == 2 || cornerIndex == 3) {
    pos.setX(std::max(0, windowSize.width() - canvasSize.width()));
  }

  DrawText(painter, pos, text);
}

void neutu::DrawText(QPainter &painter, const QPoint &pos, const QString &text)
{
  if (!text.isEmpty()) {
    QStringList textList;
    textList.append(text);
    DrawText(painter, pos, text);
  }
}

void neutu::DrawText(QPainter &painter, const QPoint &pos, const QStringList &text)
{
  if (!text.empty()) {
    int maxLength = 0;
    QString compText;
    foreach (const QString &str, text) {
      if (str.length() > maxLength) {
        maxLength = str.length();
      }
      compText += str + "\n";
    }

    maxLength = std::min(maxLength, 80);

    int width = maxLength * 8;
    int height = text.length() * 18;

    if (width > 0 && height > 0) {
      QPixmap pixmap(width, height);
      pixmap.fill(QColor(0, 0, 0, 128));

      QPainter bufferPainter(&pixmap);
      bufferPainter.setPen(QColor(255, 255, 255));

      //    bufferPainter.fillRect(pixmap.rect(), QColor(0, 0, 0, 0));
      bufferPainter.drawText(QRectF(pos.x(), pos.y(), width, height), compText);
      painter.save();
      painter.setCompositionMode(QPainter::CompositionMode_SourceOver);
      painter.drawPixmap(0, 0, pixmap);
      painter.restore();
    }
  }
}

namespace {

/*
inline void pixel_centered_adjust(const QPainter &painter, double &x, double &y)
{
  if (painter.pen().isCosmetic()) { //may have no effect due to subpixel limitation of qt painting
    x += 0.5;
    y += 0.5;
    x -= 0.5 / painter.transform().m11();
    y -= 0.5 / painter.transform().m22();
  }
}

template<typename T, typename... Args>
inline void pixel_centered_adjust(const QPainter &painter, T &x, T&y, Args&... args)
{
  pixel_centered_adjust(painter, x, y);
  pixel_centered_adjust(painter, args...);
}
*/

double get_center_adjustment(double s, bool isCosmetic)
{
  double c = 0.0;
  if (s > 1.0) {
    c = s * 0.5;
    if (isCosmetic) {
      c -= 0.5;
    }
  }
  return c;
}

struct AdjustPixelCenter {
  AdjustPixelCenter(QPainter *painter, bool adjusting) {
    m_adjusting = adjusting;
    m_painter = painter;
    /*
    if (m_painter) {
      if (!m_painter->pen().isCosmetic()) {
        m_adjusting = false;
      }
    } else {
      m_adjusting = false;
    }
    */
    if (m_adjusting && m_painter) {
      m_oldTransform = m_painter->transform();
      QTransform t = m_oldTransform;
      t.setMatrix(
            t.m11(), t.m12(), t.m13(), t.m21(), t.m22(), t.m23(),
            get_center_adjustment(
              t.m11(), m_painter->pen().isCosmetic()) + t.m31(),
            get_center_adjustment(
              t.m22(), m_painter->pen().isCosmetic()) + t.m32(),
            t.m33());
      m_painter->setTransform(t);
    }
  }

  ~AdjustPixelCenter() {
    if (m_painter && m_adjusting) {
      m_painter->setTransform(m_oldTransform);
    }
  }

  bool m_adjusting = true;
  QPainter *m_painter = nullptr;
  QTransform m_oldTransform;
};

}

void neutu::ReviseBrush(
    QPainter *painter, std::function<void(QBrush &brush)> revise)
{
  if (painter) {
    QBrush brush = painter->brush();
    revise(brush);
    painter->setBrush(brush);
  }
}

void neutu::RevisePen(QPainter *painter, std::function<void(QPen &pen)> revise)
{
  if (painter) {
    QPen pen = painter->pen();
    revise(pen);
    painter->setPen(pen);
  }
}

void neutu::SetPenColor(QPainter *painter, QColor color)
{
  if (painter) {
    QPen pen = painter->pen();
    pen.setColor(color);
    painter->setPen(pen);
  }
}

void neutu::ScalePenAlpha(QPainter *painter, double s)
{
  RevisePen(painter, [&](QPen &pen) {
    QColor color = pen.color();
    color.setAlphaF(color.alphaF() * s);
    pen.setColor(color);
  });
}

void neutu::DrawPoint(
    QPainter &painter, double x, double y, const PixelCentered p)
{
  AdjustPixelCenter adjustOnce(&painter, p);
  /*
  if (p) {
    pixel_centered_adjust(painter, x, y);
  }
  */

  painter.drawPoint(QPointF(x, y));
}

void neutu::DrawPoints(
    QPainter &painter, const std::vector<QPointF> &pts, const PixelCentered p)
{
  AdjustPixelCenter adjustOnce(&painter, p);
  painter.drawPoints(pts.data(), pts.size());
}

void neutu::DrawPoints(
    QPainter &painter, const std::vector<QPoint> &pts, const PixelCentered p)
{
  AdjustPixelCenter adjustOnce(&painter, p);
  painter.drawPoints(pts.data(), pts.size());
}

void neutu::DrawPolyline(
    QPainter &painter, const std::vector<QPointF> &pts, const PixelCentered p)
{
  AdjustPixelCenter adjustOnce(&painter, p);
  neutu::RevisePen(&painter, [](QPen &pen) {
    pen.setCapStyle(Qt::RoundCap);
  });
  painter.drawPolyline(pts.data(), pts.size());
}

void neutu::DrawPolyline(
    QPainter &painter, const std::vector<QPoint> &pts, const PixelCentered p)
{
  AdjustPixelCenter adjustOnce(&painter, p);
  neutu::RevisePen(&painter, [](QPen &pen) {
    pen.setCapStyle(Qt::RoundCap);
  });
  painter.drawPolyline(pts.data(), pts.size());
}


void neutu::DrawCircle(
    QPainter &painter, double cx, double cy, double r, const PixelCentered &p)
{
  AdjustPixelCenter adjustOnce(&painter, p);

  painter.drawEllipse(QRectF(cx - r, cy - r, r + r, r + r));
}

void neutu::DrawLine(
    QPainter &painter, const QPoint &v0, const QPoint &v1,
    const PixelCentered &p)
{
  AdjustPixelCenter adjustOnce(&painter, p);

  painter.drawLine(v0, v1);
}

void neutu::DrawLine(
    QPainter &painter, double x0, double y0, double x1, double y1,
    const PixelCentered &p)
{
  AdjustPixelCenter adjustOnce(&painter, p);

  painter.drawLine(QPointF(x0, y0), QPointF(x1, y1));
}

void neutu::DrawLines(
    QPainter &painter, const std::vector<QLineF> &lines, const PixelCentered &p)
{
  AdjustPixelCenter adjustOnce(&painter, p);
  painter.drawLines(lines.data(), lines.size());
}

void neutu::DrawLines(
    QPainter &painter, const std::vector<QLine> &lines, const PixelCentered &p)
{
  AdjustPixelCenter adjustOnce(&painter, p);
  painter.drawLines(lines.data(), lines.size());
}

void neutu::DrawRect(
    QPainter &painter, double x0, double y0, double x1, double y1,
    const PixelCentered &p)
{
  AdjustPixelCenter adjustOnce(&painter, p);

  painter.drawRect(QRectF(QPointF(x0, y0), QPointF(x1, y1)));
}

void neutu::DrawIntRect(
    QPainter &painter, int x0, int y0, int x1, int y1, const PixelCentered &p)
{
  AdjustPixelCenter adjustOnce(&painter, p);

  painter.drawRect(QRect(QPoint(x0, y0), QPoint(x1, y1)));
}

void neutu::DrawStar(
    QPainter &painter, double cx, double cy, double r, const PixelCentered &p)
{
  std::vector<QPointF> pts = MakeStar(QPointF(cx, cy), r);
  DrawPolyline(painter, pts, p);
}

void neutu::DrawCross(
    QPainter &painter, double cx, double cy, double r, const PixelCentered &p)
{
  std::vector<QLineF> lines(2);

  lines[0] = QLineF({cx - r, cy}, {cx + r, cy});
  lines[1] = QLineF({cx, cy - r}, {cx, cy + r});

  DrawLines(painter, lines, p);
}

std::vector<QPointF> neutu::MakeStar(
    const QPointF &center, double radius, double shapeFactor)
{
  std::vector<QPointF> ptArray(9);
  MakeStar(center, radius, ptArray.data(), shapeFactor);

  return ptArray;
}

void neutu::MakeStar(
    const QPointF &center, double radius, QPointF *ptArray, double shapeFactor)
{
  double sw = radius * shapeFactor;
  double left = center.x() - radius;
  double right = center.x() + radius;
  double top = center.y() - radius;
  double bottom = center.y() + radius;

  ptArray[0] = QPointF(center.x(), top);
  ptArray[1] = QPointF(center.x() + sw, center.y() - sw);
  ptArray[2] = QPointF(right, center.y());
  ptArray[3] = QPointF(center.x() + sw, center.y() + sw);
  ptArray[4] = QPointF(center.x(), bottom);
  ptArray[5] = QPointF(center.x() - sw, center.y() + sw);
  ptArray[6] = QPointF(left, center.y());
  ptArray[7] = QPointF(center.x() - sw, center.y() - sw);
  ptArray[8] = ptArray[0];
}

void neutu::HideLayout(QLayout *layout, bool removing)
{
  if (layout) {
    for (int i = 0; i < layout->count(); ++i) {
      QLayoutItem *item = layout->itemAt(i);
      QWidget *widget = item->widget();
      if (widget) {
        widget->hide();
      }
//      if (removing) {
//        layout->removeItem(item);
//      }
    }

    if (removing) {
      ClearLayout(layout);
    }

    layout->setSizeConstraint(QLayout::SetNoConstraint);
  }
}

void neutu::ClearLayout(QLayout *layout, bool deletingWidget)
{
  while (auto item = layout->takeAt(0)) {
    if (deletingWidget) {
      delete item->widget();
    }
    delete item;
  }
}
