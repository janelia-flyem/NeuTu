#include "zflyembodyideditor.h"

#include <QRegularExpressionValidator>

ZFlyEmBodyIdEditor::ZFlyEmBodyIdEditor(QWidget *parent) : QLineEdit(parent)
{
#if 0 //Validitor sliences the action of setting model data for unknown reason
  QRegularExpression re("[1-9][0-9]*");
  QRegularExpressionValidator *validator =
      new QRegularExpressionValidator(re, this);
  setValidator(validator);
#endif

  setPlaceholderText("Input Body ID");
}
