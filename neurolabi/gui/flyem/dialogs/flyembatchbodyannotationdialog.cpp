#include "flyembatchbodyannotationdialog.h"

#include "zjsonobject.h"

FlyEmBatchBodyAnnotationDialog::FlyEmBatchBodyAnnotationDialog(QWidget *parent) :
  ZGenericBodyAnnotationDialog(parent)
{
  m_ignoringEmptyField = new QCheckBox(this);
  m_ignoringEmptyField->setChecked(true);
  m_ignoringEmptyField->setText("Ignore empty fields");
  addAuxWidget(m_ignoringEmptyField);
}

void FlyEmBatchBodyAnnotationDialog::postProcess(ZJsonObject &obj) const
{
  if (m_ignoringEmptyField->isChecked()) {
    RemoveEmptyStringValues(obj);
  }
}
