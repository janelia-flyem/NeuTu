#include "flyemmvcdialogmanager.h"

#include "common/utilities.h"
#include "logging/zlog.h"

#include "zdialogfactory.h"

#include "zflyemproofmvc.h"

#include "dialogs/flyemtododialog.h"
#include "dialogs/zdvidtargetproviderdialog.h"
#include "dialogs/zflyemtodoannotationdialog.h"
#include "dialogs/neuprintquerydialog.h"
#include "dialogs/neuprintsetupdialog.h"
#include "dialogs/zcontrastprotocaldialog.h"
#include "dialogs/zspinboxdialog.h"
#include "dialogs/flyembodyinfodialog.h"
#include "dialogs/zflyemsplitcommitdialog.h"
#include "dialogs/zflyemsplituploadoptiondialog.h"
#include "dialogs/zflyembodychopdialog.h"
#include "dialogs/zinfodialog.h"
#include "dialogs/flyemdialogfactory.h"
#include "dialogs/zflyemskeletonupdatedialog.h"
#include "dialogs/zflyemroitooldialog.h"
#include "dialogs/zflyemgrayscaledialog.h"

FlyEmMvcDialogManager::FlyEmMvcDialogManager(ZFlyEmProofMvc *parent) :
  m_parent(parent)
{
}

/*
bool FlyEmMvcDialogManager::creationRequired(void *dlg) const
{
  return isNull(dlg) && m_parent->hasWidgetRole();
}
*/

ZDvidTargetProviderDialog* FlyEmMvcDialogManager::getDvidDlg()
{
  if (isNull(m_dvidDlg)) {
    KINFO << "Creating ZDvidTargetProviderDialog";
    m_dvidDlg = ZDialogFactory::makeDvidDialog(m_parent);
  }

  return m_dvidDlg;
}

void FlyEmMvcDialogManager::setDvidDlg(ZDvidTargetProviderDialog *dlg)
{
  m_dvidDlg = dlg;
}

bool FlyEmMvcDialogManager::isDvidDlgReady() const
{
  return !isNull(m_dvidDlg);
}

bool FlyEmMvcDialogManager::isRoiDlgReady() const
{
  return !isNull(m_roiDlg);
}

bool FlyEmMvcDialogManager::isBodyInfoDlgReady() const
{
  return !isNull(m_bodyInfoDlg);
}

bool FlyEmMvcDialogManager::isSplitUploadDlgReady() const
{
  return !isNull(m_splitUploadDlg);
}

ZContrastProtocalDialog* FlyEmMvcDialogManager::getContrastDlg()
{
  if (isNull(m_contrastDlg)) {
    KINFO << "Creating ZContrastProtocalDialog";
    m_contrastDlg = new ZContrastProtocalDialog(m_parent);
    m_parent->configureContrastDlg(m_contrastDlg);
    //    m_contrastDlg->setContrastProtocol(getPresenter()->getHighContrastProtocal());
    QObject::connect(m_contrastDlg, &ZContrastProtocalDialog::protocolChanged,
                     m_parent, &ZFlyEmProofMvc::updateTmpContrast);
    QObject::connect(m_contrastDlg, &ZContrastProtocalDialog::canceled,
                     m_parent, &ZFlyEmProofMvc::resetContrast);
    QObject::connect(m_contrastDlg, &ZContrastProtocalDialog::committing,
                     m_parent, &ZFlyEmProofMvc::saveTmpContrast);
  }

  return m_contrastDlg;
}

ZFlyEmSplitCommitDialog* FlyEmMvcDialogManager::getSplitCommitDlg()
{
  if (isNull(m_splitCommitDlg)) {
    KINFO << "Creating ZFlyEmSplitCommitDialog";
    m_splitCommitDlg = new ZFlyEmSplitCommitDialog(m_parent);
  }

  return m_splitCommitDlg;
}

FlyEmTodoDialog* FlyEmMvcDialogManager::getTodoDlg()
{
  if (isNull(m_todoDlg)) {
    KINFO << "Creating FlyEmTodoDialog";
    m_todoDlg = new FlyEmTodoDialog(m_parent);
    m_parent->configureTodoDlg(m_todoDlg);
  }

  return m_todoDlg;
}

ZFlyEmRoiToolDialog* FlyEmMvcDialogManager::getRoiDlg()
{
  if (isNull(m_roiDlg)) {
    KINFO << "Creating ZFlyEmRoiToolDialog";
    m_roiDlg = new ZFlyEmRoiToolDialog(m_parent);

    QObject::connect(m_roiDlg, SIGNAL(projectActivited()),
                     m_parent, SLOT(loadRoiProject()));
    QObject::connect(m_roiDlg, SIGNAL(projectClosed()),
                     m_parent, SLOT(closeRoiProject()));
    QObject::connect(m_roiDlg, SIGNAL(showing3DRoiCurve()),
                     m_parent, SLOT(showRoi3dWindow()));
    QObject::connect(m_roiDlg, SIGNAL(goingToSlice(int)),
                     m_parent, SLOT(goToSlice(int)));
    QObject::connect(m_roiDlg, SIGNAL(steppingSlice(int)),
                     m_parent, SLOT(stepSlice(int)));
    QObject::connect(m_roiDlg, SIGNAL(goingToNearestRoi()),
                     m_parent, SLOT(goToNearestRoi()));
    QObject::connect(m_roiDlg, SIGNAL(estimatingRoi()),
                     m_parent, SLOT(estimateRoi()));
    QObject::connect(m_roiDlg, SIGNAL(movingPlane(double,double)),
                     m_parent, SLOT(movePlaneRoi(double, double)));
    QObject::connect(m_roiDlg, SIGNAL(rotatingPlane(double)),
                     m_parent, SLOT(rotatePlaneRoi(double)));
    QObject::connect(m_roiDlg, SIGNAL(scalingPlane(double,double)),
                     m_parent, SLOT(scalePlaneRoi(double,double)));

    m_roiDlg->updateDvidTarget();
    m_roiDlg->downloadAllProject();
  }

  return m_roiDlg;
}

ZFlyEmSplitUploadOptionDialog* FlyEmMvcDialogManager::getSplitUploadDlg()
{
  if (isNull(m_splitUploadDlg)) {
    KINFO << "Creating ZFlyEmSplitUploadOptionDialog";
    m_splitUploadDlg = new ZFlyEmSplitUploadOptionDialog(m_parent);
    m_parent->configureSplitUploadDlg(m_splitUploadDlg);
  }

  return m_splitUploadDlg;
}

ZFlyEmBodyChopDialog* FlyEmMvcDialogManager::getBodyChopDlg()
{
  if (isNull(m_bodyChopDlg)) {
    KINFO << "Creating ZFlyEmBodyChopDialog";
    m_bodyChopDlg = new ZFlyEmBodyChopDialog(m_parent);
  }

  return m_bodyChopDlg;
}

ZInfoDialog* FlyEmMvcDialogManager::getInfoDlg()
{
  if (isNull(m_bodyInfoDlg)) {
    KINFO << "Creating ZInfoDialog";
    m_infoDlg = new ZInfoDialog(m_parent);
  }

  return m_infoDlg;
}

ZFlyEmSkeletonUpdateDialog* FlyEmMvcDialogManager::getSkeletonUpdateDlg()
{
  if (isNull(m_skeletonUpdateDlg)) {
     KINFO << "Creating ZFlyEmSkeletonUpdateDialog";
    m_skeletonUpdateDlg = new ZFlyEmSkeletonUpdateDialog(m_parent);
  }

  return m_skeletonUpdateDlg;
}

ZFlyEmGrayscaleDialog* FlyEmMvcDialogManager::getGrayscaleDlg()
{
  if (isNull(m_grayscaleDlg)) {
    m_grayscaleDlg = new ZFlyEmGrayscaleDialog(m_parent);
  }

  return m_grayscaleDlg;
}

template<typename T>
FlyEmBodyInfoDialog* FlyEmMvcDialogManager::makeBodyInfoDlg(const T &flag)
{
//  KINFO << "Creating FlyEmBodyInfoDialog";
  FlyEmBodyInfoDialog *dlg = new FlyEmBodyInfoDialog(flag, m_parent);
  dlg->dvidTargetChanged(m_parent->getDvidTarget());
  QObject::connect(m_parent, SIGNAL(dvidTargetChanged(ZDvidTarget)),
                   dlg, SLOT(dvidTargetChanged(ZDvidTarget)));
  QObject::connect(dlg, SIGNAL(bodyActivated(uint64_t)),
                   m_parent, SLOT(locateBody(uint64_t)));
  QObject::connect(dlg, SIGNAL(addBodyActivated(uint64_t)),
                   m_parent, SLOT(addLocateBody(uint64_t)));
  QObject::connect(dlg, SIGNAL(bodiesActivated(QList<uint64_t>)),
                   m_parent, SLOT(selectBody(QList<uint64_t>)));
  QObject::connect(dlg, SIGNAL(pointDisplayRequested(int,int,int)),
                   m_parent, SLOT(zoomTo(int,int,int)));

  QObject::connect(dlg, SIGNAL(colorMapChanged(ZFlyEmSequencerColorScheme)),
                   m_parent, SLOT(updateSequencerBodyMap(ZFlyEmSequencerColorScheme)));

  return dlg;
}

FlyEmBodyInfoDialog* FlyEmMvcDialogManager::getBodyInfoDlg()
{
  if (isNull(m_bodyInfoDlg)) {
    KINFO << "Creating sequencer dialog";
    m_bodyInfoDlg = new FlyEmBodyInfoDialog(
          FlyEmBodyInfoDialog::EMode::SEQUENCER, m_parent);

    QObject::connect(m_bodyInfoDlg, SIGNAL(bodyActivated(uint64_t)),
                     m_parent, SLOT(locateBody(uint64_t)));
    QObject::connect(m_bodyInfoDlg, SIGNAL(addBodyActivated(uint64_t)),
                     m_parent, SLOT(addLocateBody(uint64_t)));
    QObject::connect(m_bodyInfoDlg, SIGNAL(bodiesActivated(QList<uint64_t>)),
                     m_parent, SLOT(selectBody(QList<uint64_t>)));
    QObject::connect(m_parent, SIGNAL(dvidTargetChanged(ZDvidTarget)),
                     m_bodyInfoDlg, SLOT(dvidTargetChanged(ZDvidTarget)));
    QObject::connect(m_bodyInfoDlg, SIGNAL(namedBodyChanged(ZJsonValue)),
                     m_parent, SLOT(prepareBodyMap(ZJsonValue)));
    QObject::connect(m_bodyInfoDlg, SIGNAL(colorMapChanged(ZFlyEmSequencerColorScheme)),
                     m_parent, SLOT(updateSequencerBodyMap(ZFlyEmSequencerColorScheme)));
    QObject::connect(m_bodyInfoDlg, SIGNAL(pointDisplayRequested(int,int,int)),
                     m_parent, SLOT(zoomTo(int,int,int)));
  }

  return m_bodyInfoDlg;
}

FlyEmBodyInfoDialog* FlyEmMvcDialogManager::getBodyQueryDlg()
{
  if (isNull(m_bodyQueryDlg)) {
    KINFO << "Creating body query dialog";
    m_bodyQueryDlg = makeBodyInfoDlg(FlyEmBodyInfoDialog::EMode::QUERY);
    QObject::connect(m_bodyQueryDlg, SIGNAL(refreshing()),
            m_parent, SLOT(showBodyConnection()));
  }

  return m_bodyQueryDlg;
}

FlyEmBodyInfoDialog* FlyEmMvcDialogManager::getNeuprintBodyDlg()
{
  if (isNull(m_neuprintBodyDlg)) {
    neutu::EServerStatus status = m_parent->getNeuPrintStatus();
    switch (status) {
    case neutu::EServerStatus::NOSUPPORT:
      ZDialogFactory::Error(
            "NeuPrint Not Supported",
            "Cannot use NeuPrint because this dataset is not supported by the server.",
            m_parent);
      break;
    default:
      if (status != neutu::EServerStatus::NORMAL) {
        getNeuprintSetupDlg()->exec();
      }

      if (m_parent->getNeuPrintStatus() == neutu::EServerStatus::NORMAL) {
        KINFO << "Creating NeuPrint dialog";
        m_neuprintBodyDlg = makeBodyInfoDlg(FlyEmBodyInfoDialog::EMode::NEUPRINT);
      }
      break;
    }
  }

  return m_neuprintBodyDlg;
}

NeuprintSetupDialog* FlyEmMvcDialogManager::getNeuprintSetupDlg()
{
  if (isNull(m_neuprintSetupDlg)) {
    KINFO << "Creating NeuprintSetupDialog";
    m_neuprintSetupDlg = new NeuprintSetupDialog(m_parent);
  }
  m_neuprintSetupDlg->setUuid(m_parent->getDvidTargetUuid().c_str());

  return m_neuprintSetupDlg;
}
