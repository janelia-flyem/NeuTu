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
