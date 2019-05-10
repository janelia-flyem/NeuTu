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

void neutu::HideLayout(QLayout *layout, bool removing)
{
  if (layout) {
    for (int i = 0; i < layout->count(); ++i) {
      QWidget *widget = layout->itemAt(i)->widget();
      if (widget != NULL) {
        widget->hide();
      }
    }

    if (removing) {
      layout->removeItem(layout);
    }
  }
}
