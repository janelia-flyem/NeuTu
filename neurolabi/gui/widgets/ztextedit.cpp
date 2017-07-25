#include "ztextedit.h"
#include <QPainter>
ZTextEdit::ZTextEdit(QWidget *parent) : QTextEdit(parent)
{
  setPlaceholderText("test");
}
