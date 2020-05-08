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

inline void pixel_centered_adjust(const QPainter &painter, double &x, double &y)
{
  x += 0.5;
  y += 0.5;
  if (painter.pen().isCosmetic()) { //may have no effect due to subpixel limitation of qt painting
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
  if (p) {
    pixel_centered_adjust(painter, x, y);
  }

  painter.drawPoint(QPointF(x, y));
}

void neutu::DrawCircle(
    QPainter &painter, double cx, double cy, double r, const PixelCentered p)
{
  if (p) {
    pixel_centered_adjust(painter, cx, cy);
  }

  painter.drawEllipse(QRectF(cx - r, cy - r, r + r, r + r));
}

void neutu::DrawLine(
    QPainter &painter, double x0, double y0, double x1, double y1,
    const PixelCentered p)
{
  if (p) {
    pixel_centered_adjust(painter, x0, y0, x1, y1);
  }

  painter.drawLine(QPointF(x0, y0), QPointF(x1, y1));
}

void neutu::DrawRect(
    QPainter &painter, double x0, double y0, double x1, double y1,
    const PixelCentered p)
{
  if (p) {
    pixel_centered_adjust(painter, x0, y0, x1, y1);
  }

  painter.drawRect(QRectF(QPointF(x0, y0), QPointF(x1, y1)));
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
      if (removing) {
        layout->removeItem(item);
      }
    }

    layout->setSizeConstraint(QLayout::SetNoConstraint);
  }
}
