#include "zflyembodyideditor.h"

#include <QRegularExpressionValidator>

ZFlyEmBodyIdEditor::ZFlyEmBodyIdEditor(QWidget *parent) : QLineEdit(parent)
{
  QRegularExpression re("[1-9][0-9]*");
  QRegularExpressionValidator *validator =
      new QRegularExpressionValidator(re, this);
  setValidator(validator);
}
