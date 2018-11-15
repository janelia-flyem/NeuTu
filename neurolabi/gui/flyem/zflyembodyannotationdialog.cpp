#include "zflyembodyannotationdialog.h"
#include "ui_zflyembodyannotationdialog.h"
#include "zflyembodyannotation.h"
#include "neutube.h"
#include "zflyemmisc.h"
#include "zflyembodystatus.h"

const QString ZFlyEmBodyAnnotationDialog::FINALIZED_TEXT = "Finalized";

ZFlyEmBodyAnnotationDialog::ZFlyEmBodyAnnotationDialog(QWidget *parent) :
  QDialog(parent),
  ui(new Ui::ZFlyEmBodyAnnotationDialog)
{
  ui->setupUi(this);

  ZFlyEmMisc::PrepareBodyStatus(ui->statusComboBox);

  if (neutube::IsAdminUser()) {
    showFinalizedStatus();
//    ui->statusComboBox->addItem("Finalized");
  }
  setNameEdit(ui->nameComboBox->currentText());

  setWhatsThis("Annotate the selected body. You can specify the name and status"
               "of the body as well as add any comment as you like. "
               "The annotation will be saved into the database after 'OK' "
               "is clicked.");

//  ui->commentLineEdit->setEnabled(false);

  connectSignalSlot();
}

ZFlyEmBodyAnnotationDialog::~ZFlyEmBodyAnnotationDialog()
{
  delete ui;
}

void ZFlyEmBodyAnnotationDialog::connectSignalSlot()
{
  connect(ui->nameComboBox, SIGNAL(currentIndexChanged(QString)),
          this, SLOT(setNameEdit(QString)));
}

bool ZFlyEmBodyAnnotationDialog::isNameChanged() const
{
  return ui->nameLineEdit->text().toStdString() != m_oldName;
}

void ZFlyEmBodyAnnotationDialog::setPrevUser(const std::string &name)
{
  if (!name.empty()) {
    ui->userLabel->setText(QString("Previously annotated by %1.").arg(name.c_str()));
  } else {
    ui->userLabel->setText("");
  }
}

void ZFlyEmBodyAnnotationDialog::setPrevNamingUser(const std::string &name)
{
  if (!name.empty()) {
    ui->namingUserLabel->setText(QString("Named by %1.").arg(name.c_str()));
  } else {
    ui->namingUserLabel->setText("");
  }
}

void ZFlyEmBodyAnnotationDialog::setBodyId(uint64_t bodyId)
{
  m_bodyId = bodyId;
  ui->bodyIdLabel->setText(QString("%1").arg(bodyId));
}

QString ZFlyEmBodyAnnotationDialog::getComment() const
{
  return ui->commentLineEdit->text();
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
  QString status;

  if (ui->statusComboBox->currentIndex() > 0) {
    status = ui->statusComboBox->currentText();
  }

  return status;
  /*
  if (ui->skipCheckBox->isChecked()) {
    return "Skip";
  }

  return "";
  */
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
  annotation.setUser(neutube::GetCurrentUserName());
  if (isNameChanged()) {
    annotation.setNamingUser(neutube::GetCurrentUserName());
  }

  return annotation;
}

void ZFlyEmBodyAnnotationDialog::setComment(const std::string &comment)
{
  ui->commentLineEdit->setText(comment.c_str());
//  ui->orphanCheckBox->setChecked(comment == "Orphan");
}

void ZFlyEmBodyAnnotationDialog::setStatus(const std::string &status)
{
  int index = 0;
  if (!status.empty()) {
    index = ui->statusComboBox->findText(status.c_str(), Qt::MatchExactly);
  }

  ui->statusLabel->setText("Status");
  ui->statusComboBox->setEnabled(true);
  if (index >= 0) {
    ui->statusComboBox->setCurrentIndex(index);
  } else {
    if (status == FINALIZED_TEXT.toStdString()) {
      freezeFinalizedStatus();
    } else {
      processUnknownStatus(status);
    }
  }
//  ui->skipCheckBox->setChecked(status == "Skip");
}

void ZFlyEmBodyAnnotationDialog::setName(const std::string &name)
{
  m_oldName = name;
  ui->nameLineEdit->setText(name.c_str());
}

void ZFlyEmBodyAnnotationDialog::setType(const std::string &/*type*/)
{
}

void ZFlyEmBodyAnnotationDialog::loadBodyAnnotation(
    const ZFlyEmBodyAnnotation &annotation)
{
  setBodyId(annotation.getBodyId());
  setPrevUser(annotation.getUser());
  setPrevNamingUser(annotation.getNamingUser());

  setComment(annotation.getComment());
  setStatus(annotation.getStatus());
  setName(annotation.getName());
  setType(annotation.getType());
}

void ZFlyEmBodyAnnotationDialog::hideFinalizedStatus()
{
  int index = ui->statusComboBox->findText(FINALIZED_TEXT);
  if (index >= 0) {
    ui->statusComboBox->removeItem(index);
  }
}

void ZFlyEmBodyAnnotationDialog::showFinalizedStatus()
{
  int index = ui->statusComboBox->findText(FINALIZED_TEXT);
  if (index < 0) {
    ui->statusComboBox->addItem(FINALIZED_TEXT);
  }
}

void ZFlyEmBodyAnnotationDialog::setDefaultStatusList(
    const QList<QString> statusList)
{
  m_defaultStatusList = statusList;
}

void ZFlyEmBodyAnnotationDialog::updateStatusBox()
{
  ui->statusComboBox->clear();
  ui->statusComboBox->addItem("---");
  ui->statusComboBox->addItems(m_defaultStatusList);
}

void ZFlyEmBodyAnnotationDialog::freezeFinalizedStatus()
{
  showFinalizedStatus();
  ui->statusComboBox->setCurrentIndex(ui->statusComboBox->count() - 1);
  ui->statusComboBox->setEnabled(false);
}

void ZFlyEmBodyAnnotationDialog::processUnknownStatus(const std::string &status)
{
  int index = ui->statusComboBox->findText(status.c_str());
  if (index < 0) {
    ui->statusComboBox->addItem(status.c_str());
    ui->statusComboBox->setCurrentIndex(ui->statusComboBox->count() - 1);

//    if (!neutube::IsAdminUser()) {
    if (!ZFlyEmBodyStatus::IsAccessible(status)) {
      ui->statusComboBox->setEnabled(false);
    }
  }
}

void ZFlyEmBodyAnnotationDialog::freezeUnknownStatus(const std::string &status)
{
  int index = ui->statusComboBox->findText(status.c_str());
  if (index < 0) {
    ui->statusComboBox->addItem(status.c_str());
    ui->statusComboBox->setCurrentIndex(ui->statusComboBox->count() - 1);
    ui->statusComboBox->setEnabled(false);
  }
}
