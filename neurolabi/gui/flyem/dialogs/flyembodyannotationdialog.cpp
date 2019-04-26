#include "flyembodyannotationdialog.h"
#include "ui_flyembodyannotationdialog.h"

#include "neutube.h"

#include "../zflyembodyannotation.h"

const QString FlyEmBodyAnnotationDialog::FINALIZED_TEXT = "Finalized";

FlyEmBodyAnnotationDialog::FlyEmBodyAnnotationDialog(QWidget *parent) :
  QDialog(parent),
  ui(new Ui::FlyEmBodyAnnotationDialog)
{
  ui->setupUi(this);

  connect(ui->generatePushButton, &QPushButton::clicked,
          this, &FlyEmBodyAnnotationDialog::fillType);
}

FlyEmBodyAnnotationDialog::~FlyEmBodyAnnotationDialog()
{
  delete ui;
}

void FlyEmBodyAnnotationDialog::setType(const std::string &type)
{
  ui->typeLineEdit->setText(QString::fromStdString(type));
}

void FlyEmBodyAnnotationDialog::setInstance(const std::string &instance)
{
  m_oldInstance = instance;
  ui->instanceLineEdit->setText(QString::fromStdString(instance));
}

bool FlyEmBodyAnnotationDialog::isInstanceChanged() const
{
  return ui->instanceLineEdit->text().toStdString() != m_oldInstance;
}

void FlyEmBodyAnnotationDialog::setComment(const std::string &comment)
{
  ui->commentLineEdit->setText(QString::fromStdString(comment));
}

void FlyEmBodyAnnotationDialog::setMajorInput(const std::string &v)
{
  ui->majorInputLineEdit->setText(QString::fromStdString(v));
}

void FlyEmBodyAnnotationDialog::setMajorOutput(const std::string &v)
{
  ui->majorOutputLineEdit->setText(QString::fromStdString(v));
}

void FlyEmBodyAnnotationDialog::setPrimaryNeurite(const std::string &v)
{
  ui->primaryNeuriteLineEdit->setText(QString::fromStdString(v));
}

void FlyEmBodyAnnotationDialog::setLocation(const std::string &v)
{
  if (v == "L") {
    ui->leftRadioButton->setChecked(true);
  } else if (v == "M") {
    ui->middleRadioButton->setChecked(true);
  } else if (v == "R") {
    ui->rightRadioButton->setChecked(true);
  } else {
    ui->unknownRadioButton->setChecked(true);
  }
}

void FlyEmBodyAnnotationDialog::setOutOfBounds(bool v)
{
  ui->outOfBoundsCheckBox->setChecked(v);
}

void FlyEmBodyAnnotationDialog::setCrossMidline(bool v)
{
  ui->crossMidlineCheckBox->setChecked(v);
}

void FlyEmBodyAnnotationDialog::setNeurotransmitter(const std::string &v)
{
  ui->neurotransmitterLineEdit->setText(QString::fromStdString(v));
}

void FlyEmBodyAnnotationDialog::setSynonym(const std::string &v)
{
  ui->SynonymLineEdit->setText(QString::fromStdString(v));
}

uint64_t FlyEmBodyAnnotationDialog::getBodyId() const
{
  return m_bodyId;
}

std::string FlyEmBodyAnnotationDialog::getType() const
{
  return ui->typeLineEdit->text().toStdString();
}

std::string FlyEmBodyAnnotationDialog::getInstance() const
{
  return ui->instanceLineEdit->text().toStdString();
}

std::string FlyEmBodyAnnotationDialog::getComment() const
{
  return ui->commentLineEdit->text().toStdString();
}

std::string FlyEmBodyAnnotationDialog::getMajorInput() const
{
  return ui->majorInputLineEdit->text().toStdString();
}

std::string FlyEmBodyAnnotationDialog::getMajorOutput() const
{
  return ui->majorOutputLineEdit->text().toStdString();
}

std::string FlyEmBodyAnnotationDialog::getPrimaryNeurite() const
{
  return ui->primaryNeuriteLineEdit->text().toStdString();
}

std::string FlyEmBodyAnnotationDialog::getLocation() const
{
  if (ui->leftRadioButton->isChecked()) {
    return "L";
  } else if (ui->rightRadioButton->isChecked()) {
    return "R";
  } else if (ui->middleRadioButton->isChecked()) {
    return "M";
  }

  return "";
}

bool FlyEmBodyAnnotationDialog::getOutOfBounds() const
{
  return ui->outOfBoundsCheckBox->isChecked();
}

bool FlyEmBodyAnnotationDialog::getCrossMidline() const
{
  return ui->crossMidlineCheckBox->isChecked();
}

std::string FlyEmBodyAnnotationDialog::getNeurotransmitter() const
{
  return ui->neurotransmitterLineEdit->text().toStdString();
}

std::string FlyEmBodyAnnotationDialog::getSynonym() const
{
  return ui->SynonymLineEdit->text().toStdString();
}

std::string FlyEmBodyAnnotationDialog::getStatus() const
{
  return ui->statusComboBox->currentText().toStdString();
}

void FlyEmBodyAnnotationDialog::loadBodyAnnotation(
    const ZFlyEmBodyAnnotation &annotation)
{
  setBodyId(annotation.getBodyId());
  setPrevUser(annotation.getUser());
  setPrevNamingUser(annotation.getNamingUser());

  setComment(annotation.getComment());
  setStatus(annotation.getStatus());
  setInstance(annotation.getName());
  setType(annotation.getType());
  setComment(annotation.getComment());

  setMajorInput(annotation.getMajorInput());
  setMajorOutput(annotation.getMajorOutput());
  setPrimaryNeurite(annotation.getPrimaryNeurite());
  setLocation(annotation.getLocation());
  setOutOfBounds(annotation.getOutOfBounds());
  setCrossMidline(annotation.getCrossMidline());
  setNeurotransmitter(annotation.getNeurotransmitter());
  setSynonym(annotation.getSynonym());
//  setInstance(annotation.get);
}

ZFlyEmBodyAnnotation FlyEmBodyAnnotationDialog::getBodyAnnotation() const
{
  ZFlyEmBodyAnnotation annotation;
  annotation.setBodyId(getBodyId());
  annotation.setStatus(getStatus());
  annotation.setType(getType());
  annotation.setInstance(getInstance());
  annotation.setComment(getComment());
  std::string user = neutu::GetCurrentUserName();
  annotation.setUser(user);
  if (isInstanceChanged()) {
    annotation.setNamingUser(user);
  }
  annotation.setMajorInput(getMajorInput());
  annotation.setMajorOutput(getMajorOutput());
  annotation.setPrimaryNeurite(getPrimaryNeurite());
  annotation.setLocation(getLocation());
  annotation.setOutOfBounds(getOutOfBounds());
  annotation.setCrossMidline(getCrossMidline());
  annotation.setNeurotransmitter(getNeurotransmitter());
  annotation.setSynonym(getSynonym());

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

void FlyEmBodyAnnotationDialog::hideFinalizedStatus()
{
  int index = ui->statusComboBox->findText(FINALIZED_TEXT);
  if (index >= 0) {
    ui->statusComboBox->removeItem(index);
  }
}

void FlyEmBodyAnnotationDialog::setStatus(const std::string &status)
{
  int index = 0;
  if (!status.empty()) {
    index = ui->statusComboBox->findText(status.c_str(), Qt::MatchExactly);
  }

//  ui->statusLabel->setText("Status");
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
}

void FlyEmBodyAnnotationDialog::showFinalizedStatus()
{
  int index = ui->statusComboBox->findText(FINALIZED_TEXT);
  if (index < 0) {
    ui->statusComboBox->addItem(FINALIZED_TEXT);
  }
}

void FlyEmBodyAnnotationDialog::setDefaultStatusList(
    const QList<QString> statusList)
{
  m_defaultStatusList = statusList;
}

void FlyEmBodyAnnotationDialog::addAdminStatus(const QString &status)
{
  m_adminSatutsList.insert(status);
}

void FlyEmBodyAnnotationDialog::updateStatusBox()
{
  ui->statusComboBox->clear();
  ui->statusComboBox->addItem("---");
  ui->statusComboBox->addItems(m_defaultStatusList);
}

void FlyEmBodyAnnotationDialog::freezeFinalizedStatus()
{
  showFinalizedStatus();
  ui->statusComboBox->setCurrentIndex(ui->statusComboBox->count() - 1);
  ui->statusComboBox->setEnabled(false);
}

void FlyEmBodyAnnotationDialog::processUnknownStatus(const std::string &status)
{
  int index = ui->statusComboBox->findText(status.c_str());
  if (index < 0) {
    ui->statusComboBox->addItem(status.c_str());
    ui->statusComboBox->setCurrentIndex(ui->statusComboBox->count() - 1);

    if (m_adminSatutsList.contains(status.c_str())) {
      ui->statusComboBox->setEnabled(neutu::IsAdminUser());
    }
  }
}

void FlyEmBodyAnnotationDialog::freezeUnknownStatus(const std::string &status)
{
  int index = ui->statusComboBox->findText(status.c_str());
  if (index < 0) {
    ui->statusComboBox->addItem(status.c_str());
    ui->statusComboBox->setCurrentIndex(ui->statusComboBox->count() - 1);
    ui->statusComboBox->setEnabled(false);
  }
}

void FlyEmBodyAnnotationDialog::fillType()
{
  ZFlyEmBodyAnnotation annot = getBodyAnnotation();
  ui->typeLineEdit->setText(QString::fromStdString(annot.getAutoType()));
}
