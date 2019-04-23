#include "flyembodyannotationdialog.h"
#include "ui_flyembodyannotationdialog.h"

#include "../zflyembodyannotation.h"

FlyEmBodyAnnotationDialog::FlyEmBodyAnnotationDialog(QWidget *parent) :
  QDialog(parent),
  ui(new Ui::FlyEmBodyAnnotationDialog)
{
  ui->setupUi(this);
}

FlyEmBodyAnnotationDialog::~FlyEmBodyAnnotationDialog()
{
  delete ui;
}

ZFlyEmBodyAnnotation FlyEmBodyAnnotationDialog::getBodyAnnotation() const
{
  ZFlyEmBodyAnnotation annotation;
//  annotation.setType(getType());
//  annotation.setInstance(getInstance);

  return annotation;
}

void FlyEmBodyAnnotationDialog::setPrevUser(const std::string &name)
{
  if (!name.empty()) {
    ui->userLabel->setText(QString("Previously annotated by %1.").arg(name.c_str()));
  } else {
    ui->userLabel->setText("");
  }
}

void FlyEmBodyAnnotationDialog::setPrevNamingUser(const std::string &name)
{
  if (!name.empty()) {
    ui->namingUserLabel->setText(QString("Named by %1.").arg(name.c_str()));
  } else {
    ui->namingUserLabel->setText("");
  }
}

void FlyEmBodyAnnotationDialog::setBodyId(uint64_t bodyId)
{
  m_bodyId = bodyId;
  ui->bodyIdLabel->setText(QString("%1").arg(bodyId));
}
