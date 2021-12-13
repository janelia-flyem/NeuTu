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
    auto keys = obj.getAllKey();
    for (const std::string &key : keys) {
      ZJsonValue value = obj.value(key);
      if (value.isString() && value.toString().empty()) {
        obj.removeKey(key.c_str());
      }
    }
  }
}
