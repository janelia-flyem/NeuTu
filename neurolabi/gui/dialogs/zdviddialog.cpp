#include "zdviddialog.h"

#include <iostream>
#include <QInputDialog>
#include "neutubeconfig.h"
#include "neutube.h"
#include "ui_zdviddialog.h"
#include "zdvidtargetproviderdialog.h"
#include "zjsonarray.h"
#include "zjsonobject.h"
#include "zdialogfactory.h"
#include "stringlistdialog.h"
#include "dialogs/zdvidadvanceddialog.h"
#include "zdialogfactory.h"
#include "dvid/zdvidreader.h"
#include "dvid/zdvidtarget.h"
#include "dvid/zdvidenv.h"

const char* ZDvidDialog::m_dvidRepoKey = "dvid repo";

ZDvidDialog::ZDvidDialog(QWidget *parent) :
  ZDvidTargetProviderDialog(parent),
  ui(new Ui::ZDvidDialog)
{
  ui->setupUi(this);
  ZDvidTarget customTarget("emdata1.int.janelia.org", "", -1);
  customTarget.setName("Custom");
  m_dvidRepo.push_back(customTarget);

  QRegExp rx("[^\\s]*");
  QValidator *validator = new QRegExpValidator(rx, this);
  ui->labelBlockLineEdit->setValidator(validator);
  ui->grayScalelineEdit->setValidator(validator);

#if defined(_FLYEM_)
  const std::vector<ZDvidTarget> dvidRepo = GET_FLYEM_CONFIG.getDvidRepo();

  std::string userName = neutu::GetCurrentUserName();

  for (std::vector<ZDvidTarget>::const_iterator iter = dvidRepo.begin();
           iter != dvidRepo.end(); ++iter) {
    const ZDvidTarget &target = *iter;
    bool access = true;
    if (!target.getUserNameSet().empty()) {
      if (target.getUserNameSet().count(userName) == 0) {
        access = false;
      }
    }
    if (access) {
      m_dvidRepo.push_back(target);
    }
  }

  //Load locally saved targets
  QSettings &settings = NeutubeConfig::getInstance().getSettings();
  if (settings.contains("DVID")) {
    QString localDvidJsonStr = settings.value("DVID").toString();
    if (!localDvidJsonStr.isEmpty()) {
      ZJsonArray localDvidJson;
      localDvidJson.decode(localDvidJsonStr.toStdString());
      for (size_t i = 0; i < localDvidJson.size(); ++i) {
        ZJsonObject dvidTargetJson(
              localDvidJson.at(i), ZJsonValue::SET_INCREASE_REF_COUNT);
        ZDvidTarget target;
        target.loadJsonObject(dvidTargetJson);
        m_dvidRepo.push_back(target);
      }
    }
  }

//  m_dvidRepo.insert(m_dvidRepo.end(), dvidRepo.begin(), dvidRepo.end());
#endif

  m_roiDlg = new StringListDialog(this);

  for (QList<ZDvidTarget>::const_iterator iter = m_dvidRepo.begin();
       iter != m_dvidRepo.end(); ++iter) {
    const ZDvidTarget &target = *iter;
    if (!target.getName().empty()) {
      ui->serverComboBox->addItem(target.getName().c_str());
    } else {
      ui->serverComboBox->addItem(target.getSourceString(false).c_str());
    }
  }

  m_advancedDlg = new ZDvidAdvancedDialog(this);
  m_advancedDlg->setDvidServer(getDvidTarget().getSourceString(false).c_str());
  connect(ui->advancedPushButton, SIGNAL(clicked()),
          this, SLOT(setAdvanced()));

  setServer(0);
  connect(ui->serverComboBox, SIGNAL(currentIndexChanged(int)),
          this, SLOT(setServer(int)));

  connect(ui->saveButton, SIGNAL(clicked()), this, SLOT(saveCurrentTarget()));
  connect(ui->saveAsButton, SIGNAL(clicked()), this, SLOT(saveCurrentTargetAs()));
  connect(ui->deleteButton, SIGNAL(clicked()), this, SLOT(deleteCurrentTarget()));
  connect(ui->roiPushButton, SIGNAL(clicked()), this, SLOT(editRoiList()));
  connect(ui->settingCheckBox, SIGNAL(toggled(bool)),
          this, SLOT(updateWidgetForDefaultSetting()));
  connect(ui->loadPushButton, SIGNAL(clicked()), this, SLOT(load()));

//  setFixedSize(size());


  ui->roiLabel->hide();
  ui->roiPushButton->hide();
}

ZDvidDialog::~ZDvidDialog()
{
  delete ui;
}

int ZDvidDialog::getPort() const
{
  return ui->dvidSourceWidget->getPort();
}

QString ZDvidDialog::getAddress() const
{
  return ui->dvidSourceWidget->getAddress();
}

QString ZDvidDialog::getUuid() const
{
  return ui->dvidSourceWidget->getUuid();
}

std::string ZDvidDialog::getBodyLabelName() const
{
  return m_advancedDlg->getBodyLabelName();
}

std::string ZDvidDialog::getGrayscaleName() const
{
  return ui->grayScalelineEdit->text().trimmed().toStdString();
}

std::string ZDvidDialog::getTileName() const
{
  return ui->tileLineEdit->text().trimmed().toStdString();
}

std::string ZDvidDialog::getSegmentationName() const
{
  return ui->labelBlockLineEdit->text().trimmed().toStdString();
}

std::string ZDvidDialog::getSynapseName() const
{
  return ui->synapseLineEdit->text().trimmed().toStdString();
}

std::string ZDvidDialog::getRoiName() const
{
  return ui->roiLineEdit->text().trimmed().toStdString();
}

ZDvidTarget &ZDvidDialog::getDvidTarget()
{
  ZDvidTarget &target = m_dvidRepo[ui->serverComboBox->currentIndex()];
  if (target.isEditable()) {
    target.setServer(getAddress().toStdString());
    target.setUuid(getUuid().toStdString());
    target.setPort(getPort());
    target.setBodyLabelName(getBodyLabelName());
    if (getSegmentationName().empty()) {
      target.setNullSegmentationName();
    } else {
      target.setSegmentationName(getSegmentationName());
    }
    target.setGrayScaleName(getGrayscaleName());

    std::string tileName = getTileName();
    target.setMultiscale2dName(tileName);
    target.configTile(tileName, ui->lowQualityCheckBox->isChecked());

    target.setSynapseName(getSynapseName());

    target.setRoiName(getRoiName());
    target.setReadOnly(ui->readOnlyCheckBox->isChecked());
    target.useDefaultDataSetting(usingDefaultSetting());

    m_advancedDlg->configure(&target);
//    target.setLabelszName(ui->labelszLineEdit->text().toStdString());
//    target.setSupervisorServer(ui->liblineEdit->text().toStdString());
  }

  return target;
}

void ZDvidDialog::setServer(const ZDvidTarget &dvidTarget, int index)
{
  ui->readOnlyCheckBox->setChecked(dvidTarget.readOnly());
  ui->dvidSourceWidget->setAddress(dvidTarget.getAddress());
  ui->dvidSourceWidget->setPort(dvidTarget.getPort());
  ui->dvidSourceWidget->setUuid(dvidTarget.getUuid());
  ui->infoLabel->setText(dvidTarget.getComment().c_str());
  ui->grayScalelineEdit->setText(dvidTarget.getGrayScaleName().c_str());
  ui->labelBlockLineEdit->setText(dvidTarget.getSegmentationName().c_str());
  //ui->maxZoomSpinBox->setValue(dvidTarget.getMaxLabelZoom());
//  ui->labelszLineEdit->setText(dvidTarget.getLabelszName().c_str());
  ui->tileLineEdit->setText(dvidTarget.getMultiscale2dName().c_str());
  if (index == 0) {
    ui->lowQualityCheckBox->setChecked(false);
  } else {
    ui->lowQualityCheckBox->setChecked(
          dvidTarget.isLowQualityTile(dvidTarget.getMultiscale2dName()));
  }
  ui->synapseLineEdit->setText(dvidTarget.getSynapseName().c_str());

//  resetAdvancedDlg(dvidTarget);
#if 0
  ui->librarianCheckBox->setChecked(dvidTarget.isSupervised());
#if defined(_FLYEM_)
  ui->librarianLineEdit->setText(
        dvidTarget.getSupervisor().empty() ?
        GET_FLYEM_CONFIG.getDefaultLibrarian().c_str() :
        dvidTarget.getSupervisor().c_str());
#endif
#endif

  ui->roiLineEdit->setText(dvidTarget.getRoiName().c_str());
  ui->settingCheckBox->setChecked(dvidTarget.usingDefaultDataSetting());

  ui->dvidSourceWidget->setReadOnly(!dvidTarget.isEditable());
//  ui->bodyLineEdit->setReadOnly(!dvidTarget.isEditable());
  ui->labelBlockLineEdit->setReadOnly(!dvidTarget.isEditable());
  ui->grayScalelineEdit->setReadOnly(!dvidTarget.isEditable());
  ui->tileLineEdit->setReadOnly(!dvidTarget.isEditable());
  ui->synapseLineEdit->setReadOnly(!dvidTarget.isEditable());
  ui->settingCheckBox->setEnabled(dvidTarget.isEditable());
//  ui->librarianCheckBox->setEnabled(dvidTarget.isEditable());
//  ui->librarianLineEdit->setReadOnly(!dvidTarget.isEditable());
  //ui->maxZoomSpinBox->setReadOnly(!dvidTarget.isEditable());
  ui->roiLineEdit->setReadOnly(!dvidTarget.isEditable());
  ui->readOnlyCheckBox->setEnabled(dvidTarget.isEditable());
//  ui->labelszLineEdit->setReadOnly(!dvidTarget.isEditable());

  ui->saveButton->setEnabled(dvidTarget.isEditable());
  ui->deleteButton->setEnabled(dvidTarget.isEditable() &&
                               (dvidTarget.getName() != "Custom"));
  ui->roiLabel->setText(QString("%1 ROI").arg(dvidTarget.getRoiList().size()));

  resetAdvancedDlg(dvidTarget);
}

void ZDvidDialog::setServer(int index)
{
  ZDvidTarget dvidTarget = m_dvidRepo[index];

  ui->loadPushButton->setEnabled(index == 0);
  setServer(dvidTarget, index);
}

const ZDvidTarget& ZDvidDialog::getDvidTarget(const std::string &name) const
{
  for (QList<ZDvidTarget>::const_iterator iter = m_dvidRepo.begin();
       iter != m_dvidRepo.end(); ++iter) {
    const ZDvidTarget &target = *iter;
    if (name == target.getName()) {
      return target;
    }
  }

  return m_emptyTarget;
}

bool ZDvidDialog::hasNameConflict(const std::string &name) const
{
  for (QList<ZDvidTarget>::const_iterator iter = m_dvidRepo.begin();
       iter != m_dvidRepo.end(); ++iter) {
    const ZDvidTarget &target = *iter;
    if (name == target.getName()) {
      return true;
    }
  }

  return false;
}

void ZDvidDialog::addDvidTarget(ZDvidTarget &target)
{
  if (hasNameConflict(target.getName()) || target.getName().empty()) {
    if (ZDialogFactory::Ask(
          "Invalid Name",
          QString("Empty name or %1 alread exists. "
                  "Use a different name?").arg(target.getName().c_str()),
          this)) {
      QString targetName =
          QInputDialog::getText(this, "Database Name", "Database Name");
      target.setName(targetName.toStdString());
      addDvidTarget(target);
    }
  } else {
    m_dvidRepo.push_back(target);
    ui->serverComboBox->addItem(target.getName().c_str());
  }
}

void ZDvidDialog::saveCurrentTarget()
{
  saveCurrentTarget(false);
}

void ZDvidDialog::saveCurrentTargetAs()
{
  saveCurrentTarget(true);
}

void ZDvidDialog::saveCurrentTarget(bool cloning)
{
  ZDvidTarget target = getDvidTarget();
  if (target.getName() == "Custom") {
    cloning = true;
  }
  bool cloned = false;
  if (cloning) {
    QString targetName =
        QInputDialog::getText(this, "Database Name", "Database Name");
    if (!targetName.isEmpty()) {
      target.setName(targetName.toStdString());
      target.setEditable(true);
      addDvidTarget(target);
      ui->serverComboBox->setCurrentIndex(ui->serverComboBox->count() - 1);
//      setServer(ui->serverComboBox->count() - 1);
      cloned = true;
    }
  }

  if (!cloning || cloned) {
    QSettings &settings = NeutubeConfig::getInstance().getSettings();

    ZJsonArray dvidJson;
    dvidJson.decode(settings.value("DVID").toString().toStdString());
    if (cloned) {
      dvidJson.append(target.toJsonObject());
    } else {
      for (size_t i = 0; i < dvidJson.size(); ++i) {
        ZJsonObject dvidTargetJson(dvidJson.at(i), ZJsonValue::SET_INCREASE_REF_COUNT);
        ZDvidTarget tmpTarget;
        tmpTarget.loadJsonObject(dvidTargetJson);
        if (tmpTarget.getName() == target.getName()) {
          dvidJson.setValue(i, target.toJsonObject());
          break;
        }
      }
      m_dvidRepo[ui->serverComboBox->currentIndex()] = target;
    }

#ifdef _DEBUG_
    std::cout << "Saving DVID target:" << std::endl;
    std::cout << target.toJsonObject().dumpString(0) << std::endl;
#endif
    settings.setValue("DVID", QString(dvidJson.dumpString(0).c_str()));
  }
}

bool ZDvidDialog::usingDefaultSetting() const
{
  return ui->settingCheckBox->isChecked();
}

void ZDvidDialog::resetAdvancedDlg(const ZDvidTarget &dvidTarget)
{
  m_advancedDlg->update(dvidTarget);

#if 0
  m_advancedDlg->setSupervised(dvidTarget.isSupervised());
#if defined(_FLYEM_)
  m_advancedDlg->setSupervisorServer(
        dvidTarget.getSupervisor().empty() ?
          GET_FLYEM_CONFIG.getDefaultLibrarian().c_str() :
          dvidTarget.getSupervisor().c_str());
#endif

  m_advancedDlg->setTodoName(dvidTarget.getTodoListName());
  m_advancedDlg->setDvidServer(dvidTarget.getAddressWithPort().c_str());

  ZDvidNode node = dvidTarget.getGrayScaleSource();
  m_advancedDlg->setGrayscaleSource(node, node == dvidTarget.getNode());

  node = dvidTarget.getTileSource();
  m_advancedDlg->setTileSource(
        dvidTarget.getTileSource(), node == dvidTarget.getNode());

  m_advancedDlg->updateWidgetForEdit(dvidTarget.isEditable());
#endif
}


void ZDvidDialog::setAdvanced()
{
  m_advancedDlg->backup();

  if (!m_advancedDlg->exec()) {
    m_advancedDlg->recover();
  }
}


void ZDvidDialog::updateWidgetForDefaultSetting()
{
  ui->grayScalelineEdit->setVisible(true);
//  ui->bodyLineEdit->setVisible(true);
  ui->labelBlockLineEdit->setVisible(true);
  ui->synapseLineEdit->setVisible(true);


  ZJsonObject obj;

  if (usingDefaultSetting()) {
    ZDvidReader reader;
    if (reader.open(getDvidTarget())) {
      obj = reader.readDefaultDataSetting();
    }
  }

  ZDvidAdvancedDialog::UpdateWidget(
        ui->grayscaleLabel, ui->grayScalelineEdit, "Gray Scale", obj,
        "grayscale");
//  ZDvidAdvancedDialog::UpdateWidget(
//        ui->bodyLabelLabel, ui->bodyLineEdit, "Body Label",
//        obj, "bodies");
  ZDvidAdvancedDialog::UpdateWidget(
        ui->labelBlockLabel, ui->labelBlockLineEdit, "Label Block",
        obj, "segmentation");
  ZDvidAdvancedDialog::UpdateWidget(
        ui->synapseLabel, ui->synapseLineEdit, "Synapse",
        obj, "synapses");

  m_advancedDlg->updateWidgetForDefaultSetting(obj);
}

void ZDvidDialog::deleteCurrentTarget()
{
  ZDvidTarget target = getDvidTarget();
  if (target.isEditable() && target.getName() != "Custom") {
    QSettings &settings = NeutubeConfig::getInstance().getSettings();

    ZJsonArray dvidJson;
    dvidJson.decode(settings.value("DVID").toString().toStdString());
    for (size_t i = 0; i < dvidJson.size(); ++i) {
      ZJsonObject dvidTargetJson(dvidJson.at(i), ZJsonValue::SET_INCREASE_REF_COUNT);
      ZDvidTarget tmpTarget;
      tmpTarget.loadJsonObject(dvidTargetJson);
      if (tmpTarget.getName() == target.getName()) {
        dvidJson.remove(i);
        break;
      }
    }

    settings.setValue("DVID", QString(dvidJson.dumpString(0).c_str()));

    m_dvidRepo.removeAt(ui->serverComboBox->currentIndex());
    ui->serverComboBox->removeItem(ui->serverComboBox->currentIndex());
  }
}

void ZDvidDialog::editRoiList()
{
  m_roiDlg->setStringList(getDvidTarget().getRoiList());
  if (m_roiDlg->exec()) {
    QStringList strList = m_roiDlg->getStringList();
    std::vector<std::string> roiList;
    foreach(const QString &str, strList) {
      roiList.push_back(str.toStdString());
    }

    getDvidTarget().setRoiList(roiList);
    ui->roiLabel->setText(QString("%1 ROI").arg(roiList.size()));
  }
}

void ZDvidDialog::load()
{
  QString fileName =
      ZDialogFactory::GetOpenFileName("Load DVID Settings", "", this);
  if (!fileName.isEmpty()) {
    ZJsonObject dvidJson;
    dvidJson.load(fileName.toStdString());

    ZDvidEnv env;
    env.loadJsonObject(dvidJson);
    ZDvidTarget target = env.getFullMainTarget();
//    ZDvidTarget target;
//    target.loadJsonObject(dvidJson);
    if (target.isValid()) {
      setServer(target, 1);
    }
  }
}
