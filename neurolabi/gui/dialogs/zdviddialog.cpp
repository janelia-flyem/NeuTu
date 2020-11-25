#include "zdviddialog.h"

#include <iostream>
#include <algorithm>

#include <QInputDialog>

#include "qfonticon.h"
#include "neutubeconfig.h"
#include "neutube.h"
#include "zglobal.h"
#include "ui_zdviddialog.h"
#include "zdvidtargetproviderdialog.h"
#include "zjsonarray.h"
#include "zjsonobject.h"
#include "zjsonobjectparser.h"
#include "zdialogfactory.h"
#include "stringlistdialog.h"
#include "dialogs/zdvidadvanceddialog.h"
#include "zdialogfactory.h"
#include "dvid/zdvidreader.h"
#include "dvid/zdvidtarget.h"
#include "dvid/zdvidenv.h"

const char* ZDvidDialog::DVID_REPO_KEY = "dvid repo";
const char* ZDvidDialog::UNTITTLED_NAME = "<Untitled>";

ZDvidDialog::ZDvidDialog(QWidget *parent) :
  ZDvidTargetProviderDialog(parent),
  ui(new Ui::ZDvidDialog)
{
  ui->setupUi(this);

  m_defaultSegmentationLabel = ui->labelBlockLabel->text();
  m_defaultGrayscaleLabel = ui->grayscaleLabel->text();
  m_defaultSynapseLabel = ui->synapseLabel->text();

  QRegExp rx("[^\\s]*");
  QValidator *validator = new QRegExpValidator(rx, this);
  ui->labelBlockLineEdit->setValidator(validator);
  ui->grayScalelineEdit->setValidator(validator);

  initDvidRepo();

  m_roiDlg = new StringListDialog(this);

  for (QList<ZDvidTarget>::const_iterator iter = m_dvidRepo.begin();
       iter != m_dvidRepo.end(); ++iter) {
    const ZDvidTarget &target = *iter;
    addTargetItem(target);
  }

  m_advancedDlg = new ZDvidAdvancedDialog(this);
//  m_advancedDlg->setDvidServer(getDvidTarget().getSourceString(false).c_str());
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
  connect(ui->exportPushButton, SIGNAL(clicked()), this, SLOT(exportTarget()));

//  setFixedSize(size());


  ui->roiLabel->hide();
  ui->roiPushButton->hide();
}

ZDvidDialog::~ZDvidDialog()
{
  delete ui;
}

void ZDvidDialog::updateLastItemIcon(const ZDvidTarget &target)
{
  if (target.getName() != UNTITTLED_NAME) {
    ui->serverComboBox->setItemIcon(
          ui->serverComboBox->count() - 1,
          QFontIcon::icon(target.isEditable() ? 0xf044 : 0xf013, Qt::gray));
  }
}

void ZDvidDialog::addTargetItem(const ZDvidTarget &target)
{
  if (!target.getName().empty()) {
    ui->serverComboBox->addItem(target.getName().c_str());
  } else {
    ui->serverComboBox->addItem(target.getSourceString(false).c_str());
  }
  updateLastItemIcon(target);
}

void ZDvidDialog::initDvidRepo()
{
  ZDvidTarget customTarget("emdata1.int.janelia.org", "", -1);
  customTarget.setName(UNTITTLED_NAME);
  m_dvidRepo.push_back(customTarget);

  const std::vector<ZDvidTarget> dvidRepo = GET_FLYEM_CONFIG.getDvidRepo();
  std::string userName = neutu::GetCurrentUserName();

  for ( const ZDvidTarget &target : dvidRepo) {
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
}

void ZDvidDialog::forEachTarget(
    std::function<void (const ZDvidTarget &)> f) const
{
  for (const ZDvidTarget &target : m_dvidRepo) {
    f(target);
  }
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

ZDvidTarget ZDvidDialog::getDvidTargetWithOriginalData()
{
  ZDvidTarget target = m_dvidRepo[ui->serverComboBox->currentIndex()];
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
  }

  return target;
}

ZDvidTarget &ZDvidDialog::getDvidTarget()
{
  m_currentTarget = getDvidTargetWithOriginalData();

  m_currentTarget.useDefaultDataSetting(false);
  m_currentTarget.loadDvidDataSetting(m_currentDefaultSettings);

  return m_currentTarget;
}

void ZDvidDialog::updateAdvancedInfo()
{
  if (m_advancedDlg->isSupervised()) {
    ui->advancedInfoLabel->setText(
          QString::fromStdString(
            "Librarian: " + m_advancedDlg->getSupervisorServer()));
  } else {
    ui->advancedInfoLabel->setText("No librarian.");
  }
}

void ZDvidDialog::setServer(const ZDvidTarget &dvidTarget)
{
  m_currentDefaultSettings.clear();

  updateWidgetValue(dvidTarget);

  if (ui->settingCheckBox->isChecked() != dvidTarget.usingDefaultDataSetting()) {
    ui->settingCheckBox->setChecked(dvidTarget.usingDefaultDataSetting());
  } else if (ui->settingCheckBox->isChecked()) {
    updateWidgetForDefaultSetting(dvidTarget);
  }

  updateWidgetState(dvidTarget);

  resetAdvancedDlg(dvidTarget);
  updateAdvancedInfo();
}

void ZDvidDialog::setServer(int index)
{
  ZDvidTarget dvidTarget = m_dvidRepo[index];

  setServer(dvidTarget);
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

bool ZDvidDialog::inputTargetName(ZDvidTarget &target)
{
  bool ok = false;
  QString targetName =
      QInputDialog::getText(
        this, "Database Name", "Database Name", QLineEdit::Normal, "", &ok)
      .trimmed();

  if (ok) {
    target.setName(targetName.toStdString());
  }

  return ok;
}

bool ZDvidDialog::addDvidTarget(ZDvidTarget &target)
{
  if (hasNameConflict(target.getName()) || target.getName().empty()) {
    if (ZDialogFactory::Ask(
          "Invalid Name",
          (target.getName().empty() ? "The name is empty." :
          QString::fromStdString('"' + target.getName() + "\" already exists.")) +
          " Use a different name? "
          "([Yes] to input the name again; [No] to abort saving.)", this)) {
      if (inputTargetName(target)) {
        return addDvidTarget(target);
      }
    }
  } else {
    m_dvidRepo.push_back(target);
    addTargetItem(target);
    return true;
  }

  return false;
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
  ZDvidTarget target = getDvidTargetWithOriginalData();
  if (target.getName() == UNTITTLED_NAME) {
    cloning = true;
  }
  bool cloned = false;
  if (cloning) {
    if (inputTargetName(target)) {
      target.setEditable(true);
      if (addDvidTarget(target)) {
        ui->serverComboBox->setCurrentIndex(ui->serverComboBox->count() - 1);
        //      setServer(ui->serverComboBox->count() - 1);
        cloned = true;
      }
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
        ZJsonObject dvidTargetJson(
              dvidJson.at(i), ZJsonValue::SET_INCREASE_REF_COUNT);
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

  if (m_advancedDlg->exec()) {
    updateAdvancedInfo();
  } else {
    m_advancedDlg->recover();
  }
}

void ZDvidDialog::updateWidgetValue(const ZDvidTarget &dvidTarget)
{
  ui->readOnlyCheckBox->setChecked(dvidTarget.readOnly());
  ui->dvidSourceWidget->setNode(dvidTarget.getNode());
  ui->infoLabel->setText(dvidTarget.getComment().c_str());
  ui->grayScalelineEdit->setText(dvidTarget.getGrayScaleName().c_str());
  ui->labelBlockLineEdit->setText(dvidTarget.getSegmentationName().c_str());
  //ui->maxZoomSpinBox->setValue(dvidTarget.getMaxLabelZoom());
//  ui->labelszLineEdit->setText(dvidTarget.getLabelszName().c_str());
  ui->tileLineEdit->setText(dvidTarget.getMultiscale2dName().c_str());
  if (dvidTarget.getName() == UNTITTLED_NAME) {
    ui->lowQualityCheckBox->setChecked(false);
  } else {
    ui->lowQualityCheckBox->setChecked(
          dvidTarget.isLowQualityTile(dvidTarget.getMultiscale2dName()));
  }
  ui->synapseLineEdit->setText(dvidTarget.getSynapseName().c_str());

  ui->roiLineEdit->setText(dvidTarget.getRoiName().c_str());
  ui->roiLabel->setText(QString("%1 ROI").arg(dvidTarget.getRoiList().size()));

}

void ZDvidDialog::updateWidgetState(const ZDvidTarget &target)
{
  ui->loadPushButton->setEnabled(target.getName() == UNTITTLED_NAME);
  ui->dvidSourceWidget->setReadOnly(!target.isEditable());
//  ui->bodyLineEdit->setReadOnly(!dvidTarget.isEditable());
  ui->labelBlockLineEdit->setReadOnly(!target.isEditable());
  ui->grayScalelineEdit->setReadOnly(!target.isEditable());
  ui->tileLineEdit->setReadOnly(!target.isEditable());
  ui->synapseLineEdit->setReadOnly(!target.isEditable());
  ui->settingCheckBox->setEnabled(target.isEditable());
//  ui->librarianCheckBox->setEnabled(dvidTarget.isEditable());
//  ui->librarianLineEdit->setReadOnly(!dvidTarget.isEditable());
  //ui->maxZoomSpinBox->setReadOnly(!dvidTarget.isEditable());
  ui->roiLineEdit->setReadOnly(!target.isEditable());
  ui->readOnlyCheckBox->setEnabled(target.isEditable());
//  ui->labelszLineEdit->setReadOnly(!dvidTarget.isEditable());

  ui->saveButton->setEnabled(target.isEditable());
  ui->deleteButton->setEnabled(target.isEditable() &&
                               (target.getName() != UNTITTLED_NAME));
}

void ZDvidDialog::updateWidgetForDefaultSetting()
{
  updateWidgetForDefaultSetting(getDvidTargetWithOriginalData());
}

void ZDvidDialog::updateWidgetForDefaultSetting(const ZDvidTarget &target)
{
#ifdef _DEBUG_
  std::cout << "updateWidgetForDefaultSetting: "
            << usingDefaultSetting() << std::endl;
#endif
  ui->grayScalelineEdit->setVisible(true);
  ui->labelBlockLineEdit->setVisible(true);
  ui->synapseLineEdit->setVisible(true);

  m_currentDefaultSettings.clear();

  if (usingDefaultSetting()) {
    ZDvidReader *reader = ZGlobal::GetDvidReader(target);
    if (reader && reader->good()) {
      m_currentDefaultSettings = reader->readDefaultDataSetting();
    }
  }

  ZDvidAdvancedDialog::UpdateWidget(
        ui->grayscaleLabel, ui->grayScalelineEdit, m_defaultGrayscaleLabel,
        m_currentDefaultSettings, "grayscale");
//  ZDvidAdvancedDialog::UpdateWidget(
//        ui->bodyLabelLabel, ui->bodyLineEdit, "Body Label",
//        obj, "bodies");
  ZDvidAdvancedDialog::UpdateWidget(
        ui->labelBlockLabel, ui->labelBlockLineEdit, m_defaultSegmentationLabel,
        m_currentDefaultSettings, "segmentation");
  ZDvidAdvancedDialog::UpdateWidget(
        ui->synapseLabel, ui->synapseLineEdit, m_defaultSynapseLabel,
        m_currentDefaultSettings, "synapses");

  m_advancedDlg->updateWidgetForDefaultSetting(m_currentDefaultSettings);
}

void ZDvidDialog::deleteCurrentTarget()
{
  ZDvidTarget target = getDvidTarget();
  if (target.isEditable() && target.getName() != UNTITTLED_NAME) {
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

    int indexToDelete = ui->serverComboBox->currentIndex();
    ui->serverComboBox->setCurrentIndex(0);
    m_dvidRepo.removeAt(indexToDelete);
    ui->serverComboBox->removeItem(indexToDelete);
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
      setServer(target);
    }
  }
}

void ZDvidDialog::exportTarget()
{
  QString fileName =
      ZDialogFactory::GetSaveFileName("Export DVID Settings", "", this);
  if (!fileName.isEmpty()) {
    const ZDvidTarget &target = getDvidTargetWithOriginalData();
    target.toJsonObject().dump(fileName.toStdString());
  }
}
