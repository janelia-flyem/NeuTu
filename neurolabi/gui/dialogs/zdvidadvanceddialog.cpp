#include "zdvidadvanceddialog.h"
#include "ui_zdvidadvanceddialog.h"

#include "zjsonobject.h"
#include "zjsonparser.h"
#include "dvid/zdvidtarget.h"
#include "neutubeconfig.h"

ZDvidAdvancedDialog::ZDvidAdvancedDialog(QWidget *parent) :
  QDialog(parent),
  ui(new Ui::ZDvidAdvancedDialog)
{
  ui->setupUi(this);
  m_oldSupervised = true;

  if (ui->grayscaleMainCheckBox->isChecked()) {
    ui->grayscaleSourceWidget->setEnabled(false);
  }
  if (ui->tileMainCheckBox->isChecked()) {
    ui->tileSourceWidget->setEnabled(false);
  }

  connect(ui->grayscaleMainCheckBox, SIGNAL(toggled(bool)),
          ui->grayscaleSourceWidget, SLOT(setDisabled(bool)));
  connect(ui->tileMainCheckBox, SIGNAL(toggled(bool)),
          ui->tileSourceWidget, SLOT(setDisabled(bool)));
}

ZDvidAdvancedDialog::~ZDvidAdvancedDialog()
{
  delete ui;
}

void ZDvidAdvancedDialog::update(const ZDvidTarget &dvidTarget)
{
  setSupervised(dvidTarget.isSupervised());
#if defined(_FLYEM_)
  setSupervisorServer(
        dvidTarget.getSupervisor().empty() ?
          GET_FLYEM_CONFIG.getDefaultLibrarian().c_str() :
          dvidTarget.getSupervisor().c_str());
#endif

  if (dvidTarget.isDefaultTodoListName()) {
    setTodoName("");
  } else {
    setTodoName(dvidTarget.getTodoListName());
  }

  if (dvidTarget.isDefaultBodyLabelName()) {
    setBodyLabelName("");
  } else {
    setBodyLabelName(dvidTarget.getBodyLabelName());
  }

  setDvidServer(dvidTarget.getAddressWithPort().c_str());

  ZDvidNode node = dvidTarget.getGrayScaleSource();
  setGrayscaleSource(node, node == dvidTarget.getNode());

  node = dvidTarget.getTileSource();
  setTileSource(
        dvidTarget.getTileSource(), node == dvidTarget.getNode());

  updateWidgetForEdit(dvidTarget.isEditable());
//  updateWidgetForDefaultSetting(dvidTarget.usingDefaultDataSetting());
}

void ZDvidAdvancedDialog::configure(ZDvidTarget *target)
{
  if (target != NULL) {
    target->enableSupervisor(isSupervised());
    target->setSupervisorServer(getSupervisorServer());
    target->setTodoListName(getTodoName());
    target->setBodyLabelName(getBodyLabelName());
    target->setGrayScaleSource(getGrayscaleSource());
    target->setTileSource(getTileSource());
  }
}

void ZDvidAdvancedDialog::backup()
{
  m_oldSupervised = isSupervised();
  m_oldSupervisorServer = getSupervisorServer();
  m_oldTodoName = getTodoName();
  m_oldBodyLabelName = getBodyLabelName();
  m_oldGrayscaleSource = ui->grayscaleSourceWidget->getNode();
  m_oldTileSource = ui->tileSourceWidget->getNode();
  m_oldMainGrayscale = ui->grayscaleMainCheckBox->isChecked();
  m_oldMainTile = ui->tileMainCheckBox->isChecked();
//  m_oldDefaultTodo = ui->defaultTodoCheckBox->isChecked();
}

void ZDvidAdvancedDialog::recover()
{
  setSupervised(m_oldSupervised);
  setSupervisorServer(m_oldSupervisorServer);
  setTodoName(m_oldTodoName);
  setBodyLabelName(m_oldBodyLabelName);
  setGrayscaleSource(m_oldGrayscaleSource, m_oldMainGrayscale);
  setTileSource(m_oldTileSource, m_oldMainTile);
//  ui->defaultTodoCheckBox->setChecked(m_oldDefaultTodo);
}

void ZDvidAdvancedDialog::setDvidServer(const QString &str)
{
  ui->serverLabel->setText(str);
}

void ZDvidAdvancedDialog::updateWidgetForEdit(bool editable)
{
  ui->librarianCheckBox->setEnabled(editable);
  ui->librarianLineEdit->setEnabled(editable);
  ui->todoLineEdit->setEnabled(editable);
  ui->bodyLabelLineEdit->setEnabled(editable);
}

void ZDvidAdvancedDialog::UpdateWidget(QLabel *label, QLineEdit *lineEdit,
    const QString &labelText, const QString &dataText, QWidget *hintWidget)
{
  if (!dataText.isEmpty()) {
    lineEdit->setVisible(false);
    label->setText(labelText + ": " + dataText);
    if (hintWidget != NULL) {
      hintWidget->setVisible(false);
    }
  } else {
    lineEdit->setVisible(true);
    label->setText(labelText);
    if (hintWidget != NULL) {
      hintWidget->setVisible(true);
    }
  }
}

void ZDvidAdvancedDialog::UpdateWidget(
    QLabel *label, QLineEdit *lineEdit, const QString &labelText,
    const ZJsonObject &obj, const char *key, QWidget *hintWidget)
{
  if (obj.hasKey(key)) {
    UpdateWidget(
          label, lineEdit, labelText,
          ZJsonParser::stringValue(obj[key]).c_str(),
          hintWidget);
  } else {
    UpdateWidget(label, lineEdit, labelText, "", hintWidget);
  }
}

void ZDvidAdvancedDialog::updateWidgetForDefaultSetting(const ZJsonObject &obj)
{
  UpdateWidget(ui->todoLabel, ui->todoLineEdit, "Todo Name", obj, "todos",
               ui->todoHintLabel);
  UpdateWidget(ui->bodyLabel, ui->bodyLabelLineEdit, "Body Label", obj, "bodies",
               ui->bodyLabelHintLabel);
//  ui->todoHintLabel->setVisible(ui->todoLineEdit->isVisible());
}


void ZDvidAdvancedDialog::setGrayscaleSource(
    const ZDvidNode &node)
{
  ui->grayscaleSourceWidget->setNode(node);
}

void ZDvidAdvancedDialog::setTileSource(
    const ZDvidNode &node)
{
  ui->tileSourceWidget->setNode(node);
}


void ZDvidAdvancedDialog::setGrayscaleSource(
    const ZDvidNode &node, bool sameMainSource)
{
  ui->grayscaleSourceWidget->setNode(node);
  ui->grayscaleMainCheckBox->setChecked(sameMainSource);
}

void ZDvidAdvancedDialog::setTileSource(
    const ZDvidNode &node, bool sameMainSource)
{
  ui->tileSourceWidget->setNode(node);
  ui->tileMainCheckBox->setChecked(sameMainSource);
}

ZDvidNode ZDvidAdvancedDialog::getGrayscaleSource() const
{
  if (ui->grayscaleMainCheckBox->isChecked()) {
    return ZDvidNode();
  }

  return ui->grayscaleSourceWidget->getNode();
}

ZDvidNode ZDvidAdvancedDialog::getTileSource() const
{
  if (ui->tileMainCheckBox->isChecked()) {
    return ZDvidNode();
  }

  return ui->tileSourceWidget->getNode();
}

void ZDvidAdvancedDialog::setTodoName(const std::string &name)
{
  ui->todoLineEdit->setText(name.c_str());
}

std::string ZDvidAdvancedDialog::getTodoName() const
{
  return ui->todoLineEdit->text().trimmed().toStdString();
}

void ZDvidAdvancedDialog::setBodyLabelName(const std::string &name)
{
  ui->bodyLabelLineEdit->setText(name.c_str());
}

std::string ZDvidAdvancedDialog::getBodyLabelName() const
{
  return ui->bodyLabelLineEdit->text().trimmed().toStdString();
}

bool ZDvidAdvancedDialog::isSupervised() const
{
  return ui->librarianCheckBox->isChecked();
}

void ZDvidAdvancedDialog::setSupervised(bool supervised)
{
  ui->librarianCheckBox->setChecked(supervised);
}

std::string ZDvidAdvancedDialog::getSupervisorServer() const
{
  return ui->librarianLineEdit->text().trimmed().toStdString();
}

void ZDvidAdvancedDialog::setSupervisorServer(const std::string &server)
{
  ui->librarianLineEdit->setText(server.c_str());
}
