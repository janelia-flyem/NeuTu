#include "zflyembodyannotationdialog.h"
#include "ui_zflyembodyannotationdialog.h"
#include "zflyembodyannotation.h"

ZFlyEmBodyAnnotationDialog::ZFlyEmBodyAnnotationDialog(QWidget *parent) :
  QDialog(parent),
  ui(new Ui::ZFlyEmBodyAnnotationDialog)
{
  ui->setupUi(this);

  setNameEdit(ui->nameComboBox->currentText());

  connect(ui->nameComboBox, SIGNAL(currentIndexChanged(QString)),
          this, SLOT(setNameEdit(QString)));
}

ZFlyEmBodyAnnotationDialog::~ZFlyEmBodyAnnotationDialog()
{
  delete ui;
}

void ZFlyEmBodyAnnotationDialog::setBodyId(uint64_t bodyId)
{
  m_bodyId = bodyId;
  ui->bodyIdLabel->setText(QString("%1").arg(bodyId));
}

QString ZFlyEmBodyAnnotationDialog::getComment() const
{
  if (ui->orphanCheckBox->isChecked()) {
    return "Orphan";
  }

  return "";
}

void ZFlyEmBodyAnnotationDialog::setNameEdit(const QString &name)
{
  if (name != "---") {
    ui->nameLineEdit->setText(name);
  } else {
    ui->nameLineEdit->clear();
  }
}

QString ZFlyEmBodyAnnotationDialog::getStatus() const
{
  if (ui->skipCheckBox->isChecked()) {
    return "Skip";
  }

  return "";
}

QString ZFlyEmBodyAnnotationDialog::getName() const
{
  return ui->nameLineEdit->text();
}

QString ZFlyEmBodyAnnotationDialog::getType() const
{
  return "";
}

uint64_t ZFlyEmBodyAnnotationDialog::getBodyId() const
{
  return m_bodyId;
}

ZFlyEmBodyAnnotation ZFlyEmBodyAnnotationDialog::getBodyAnnotation() const
{
  ZFlyEmBodyAnnotation annotation;
  annotation.setBodyId(getBodyId());
  annotation.setComment(getComment().toStdString());
  annotation.setStatus(getStatus().toStdString());
  annotation.setName(getName().toStdString());
  annotation.setType(getType().toStdString());

  return annotation;
}

void ZFlyEmBodyAnnotationDialog::setComment(const std::string &comment)
{
  ui->orphanCheckBox->setChecked(comment == "Orphan");
}

void ZFlyEmBodyAnnotationDialog::setStatus(const std::string &status)
{
  ui->skipCheckBox->setChecked(status == "Skip");
}

void ZFlyEmBodyAnnotationDialog::setName(const std::string &name)
{
  ui->nameLineEdit->setText(name.c_str());
}

void ZFlyEmBodyAnnotationDialog::setType(const std::string &/*type*/)
{
}

void ZFlyEmBodyAnnotationDialog::loadBodyAnnotation(
    const ZFlyEmBodyAnnotation &annotation)
{
  setBodyId(annotation.getBodyId());

  setComment(annotation.getComment());
  setStatus(annotation.getStatus());
  setName(annotation.getName());
  setType(annotation.getType());
}
