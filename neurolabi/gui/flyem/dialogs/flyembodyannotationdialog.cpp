#include "flyembodyannotationdialog.h"
#include "ui_flyembodyannotationdialog.h"

#include "neutube.h"
//#include "qt/gui/utilities.h"

#include "../zflyembodyannotation.h"

const QString FlyEmBodyAnnotationDialog::FINALIZED_TEXT = "Finalized";

const QString FlyEmBodyAnnotationDialog::KEY_TYPE = "type";
const QString FlyEmBodyAnnotationDialog::KEY_INSTANCE = "instance";
const QString FlyEmBodyAnnotationDialog::KEY_COMMENT = "comment";
const QString FlyEmBodyAnnotationDialog::KEY_MAJOR_INPUT = "majorInput";
const QString FlyEmBodyAnnotationDialog::KEY_MAJOR_OUTPUT = "majorOutput";
const QString FlyEmBodyAnnotationDialog::KEY_PRIMARY_NEURITE = "primaryNeurite";
const QString FlyEmBodyAnnotationDialog::KEY_LOCATION = "location";
const QString FlyEmBodyAnnotationDialog::KEY_OUT_OF_BOUNDS = "outOfBounds";
const QString FlyEmBodyAnnotationDialog::KEY_CROSS_MIDLINE = "crossMidline";
const QString FlyEmBodyAnnotationDialog::KEY_NEUROTRANSMITTER = "neurotransmitter";
const QString FlyEmBodyAnnotationDialog::KEY_SYNONYM = "synonym";
const QString FlyEmBodyAnnotationDialog::KEY_CLONAL_UNIT = "clonalUnit";
const QString FlyEmBodyAnnotationDialog::KEY_HEMILINEAGE = "hemilineage";
const QString FlyEmBodyAnnotationDialog::KEY_AUTO_TYPE = "autoType";
const QString FlyEmBodyAnnotationDialog::KEY_PROPERTY = "property";
const QString FlyEmBodyAnnotationDialog::KEY_STATUS = "status";

FlyEmBodyAnnotationDialog::FlyEmBodyAnnotationDialog(bool admin, QWidget *parent) :
  QDialog(parent),
  ui(new Ui::FlyEmBodyAnnotationDialog)
{
  ui->setupUi(this);

  initNullStatusItem();
  initWidgetMap();

  connect(ui->generatePushButton, &QPushButton::clicked,
          this, &FlyEmBodyAnnotationDialog::fillType);

  m_isAdmin = admin;
  if (!m_isAdmin) {
    hideWidget(KEY_TYPE);
//    ui->typeLineEdit->hide();
    ui->generatePushButton->hide();
    disableWidget(KEY_PRIMARY_NEURITE);
    disableWidget(KEY_CLONAL_UNIT);
    disableWidget(KEY_HEMILINEAGE);
//    ui->primaryNeuriteLineEdit->setEnabled(false);
//    ui->clonalUnitLineEdit->setEnabled(false);
//    neutu::HideLayout(ui->typeLayout, false);
  }
}

FlyEmBodyAnnotationDialog::~FlyEmBodyAnnotationDialog()
{
  delete ui;
}

void FlyEmBodyAnnotationDialog::disableWidget(const QString &key)
{
  if (m_widgetMap.contains(key)) {
    m_widgetMap[key]->setEnabled(false);
  }
}

void FlyEmBodyAnnotationDialog::hideWidget(const QString &key)
{
  if (m_widgetMap.contains(key)) {
    m_widgetMap[key]->hide();
  }
}

void FlyEmBodyAnnotationDialog::initWidgetMap()
{
  m_widgetMap[KEY_TYPE] = ui->typeLineEdit;
  m_widgetMap[KEY_INSTANCE] = ui->instanceLineEdit;
  m_widgetMap[KEY_COMMENT] = ui->commentLineEdit;
  m_widgetMap[KEY_MAJOR_INPUT] = ui->majorInputLineEdit;
  m_widgetMap[KEY_MAJOR_OUTPUT] = ui->majorOutputLineEdit;
  m_widgetMap[KEY_PRIMARY_NEURITE] = ui->primaryNeuriteLineEdit;
  m_widgetMap[KEY_OUT_OF_BOUNDS] = ui->outOfBoundsCheckBox;
  m_widgetMap[KEY_CROSS_MIDLINE] = ui->crossMidlineCheckBox;
  m_widgetMap[KEY_NEUROTRANSMITTER] = ui->neurotransmitterLineEdit;
  m_widgetMap[KEY_SYNONYM] = ui->SynonymLineEdit;
  m_widgetMap[KEY_CLONAL_UNIT] = ui->clonalUnitLineEdit;
  m_widgetMap[KEY_HEMILINEAGE] = ui->hemilineageEdit;
  m_widgetMap[KEY_AUTO_TYPE] = ui->autoTypeLineEdit;
  m_widgetMap[KEY_PROPERTY] = ui->propertyComboBox;
  m_widgetMap[KEY_STATUS] = ui->statusComboBox;
}

void FlyEmBodyAnnotationDialog::initNullStatusItem()
{
  ui->statusComboBox->clear();
  ui->statusComboBox->addItem("---");
}

void FlyEmBodyAnnotationDialog::setType(const std::string &type)
{
  ui->typeLineEdit->setText(QString::fromStdString(type));

  if (ui->typeLineEdit->isHidden()) {
    ui->typeLabel->setText("Type:" + QString::fromStdString(type));
  }
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

bool FlyEmBodyAnnotationDialog::isStatusChanged() const
{
  return getStatus() != m_oldStatus;
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

void FlyEmBodyAnnotationDialog::setClonalUnit(const std::string &v)
{
  ui->clonalUnitLineEdit->setText(QString::fromStdString(v));
}

void FlyEmBodyAnnotationDialog::setHemilineage(const std::string &v)
{
  ui->hemilineageEdit->setText(QString::fromStdString(v));
}

void FlyEmBodyAnnotationDialog::setAutoType(const std::string &v)
{
  ui->autoTypeLineEdit->setText(QString::fromStdString(v));
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
  if (ui->statusComboBox->currentIndex() > 0) {
    return ui->statusComboBox->currentText().toStdString();
  }

  return "";
}

std::string FlyEmBodyAnnotationDialog::getProperty() const
{
  if (ui->propertyComboBox->currentIndex() > 0) {
    return ui->propertyComboBox->currentText().toStdString();
  }

  return "";
}


std::string FlyEmBodyAnnotationDialog::getAutoType() const
{
  return ui->autoTypeLineEdit->text().toStdString();
}

std::string FlyEmBodyAnnotationDialog::getClonalUnit() const
{
  return ui->clonalUnitLineEdit->text().toStdString();
}

std::string FlyEmBodyAnnotationDialog::getHemilineage() const
{
  return ui->hemilineageEdit->text().toStdString();
}

void FlyEmBodyAnnotationDialog::loadBodyAnnotation(
    const ZFlyEmBodyAnnotation &annotation)
{
  setBodyId(annotation.getBodyId());
  setPrevUser(annotation.getUser());
  setPrevNamingUser(annotation.getNamingUser());
  setPrevStatusUser(annotation.getStatusUser());

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
  setHemilineage(annotation.getHemilineage());
  setSynonym(annotation.getSynonym());
  setClonalUnit(annotation.getClonalUnit());
  setAutoType(annotation.getAutoType());
  setProperty(annotation.getProperty());
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
  } else {
    annotation.setNamingUser(m_prevNamingUser);
  }
  annotation.setStatusUser(isStatusChanged() ? user : m_prevStatusUser);
  annotation.setMajorInput(getMajorInput());
  annotation.setMajorOutput(getMajorOutput());
  annotation.setPrimaryNeurite(getPrimaryNeurite());
  annotation.setLocation(getLocation());
  annotation.setOutOfBounds(getOutOfBounds());
  annotation.setCrossMidline(getCrossMidline());
  annotation.setNeurotransmitter(getNeurotransmitter());
  annotation.setHemilineage(getHemilineage());
  annotation.setSynonym(getSynonym());
  annotation.setClonalUnit(getClonalUnit());
  annotation.setAutoType(getAutoType());
  annotation.setProperty(getProperty());

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
  m_prevNamingUser = name;
}

void FlyEmBodyAnnotationDialog::setPrevStatusUser(const std::string &name)
{
  m_prevStatusUser = name;
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
  m_oldStatus = status;

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


void FlyEmBodyAnnotationDialog::setProperty(const std::string &v)
{
  int index = 0;
  if (!v.empty()) {
    index = ui->propertyComboBox->findText(v.c_str(), Qt::MatchExactly);
  }

  if (index >= 0) {
    ui->propertyComboBox->setCurrentIndex(index);
  } else {
    processUnknownProperty(v);
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
  initNullStatusItem();
  ui->statusComboBox->addItems(m_defaultStatusList);
}

void FlyEmBodyAnnotationDialog::updatePropertyBox()
{
  ui->propertyComboBox->clear();
  ui->propertyComboBox->addItem("---");
  ui->propertyComboBox->addItem("Distinct");
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
      ui->statusComboBox->setEnabled(m_isAdmin);
    }
  }
}

void FlyEmBodyAnnotationDialog::processUnknownProperty(
    const std::string &property)
{
  ui->propertyComboBox->addItem(property.c_str());
  ui->propertyComboBox->setCurrentIndex(ui->propertyComboBox->count() - 1);
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
  ui->typeLineEdit->setText(QString::fromStdString(annot.getInferredType()));
}
