#include "utilities.h"

#include <QTextDocument>
#include <QPixmap>
#include <QPainter>

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
