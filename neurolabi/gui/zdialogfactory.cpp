#include "zdialogfactory.h"
#include <QVBoxLayout>
#include <QPushButton>
#include <QMessageBox>
#include <QFileDialog>
#include <QFileInfo>

#include "zwidgetfactory.h"
#include "dialogs/zdviddialog.h"
#include "dialogs/dvidbranchdialog.h"
#include "dialogs/zdvidtargetproviderdialog.h"
#include "neutubeconfig.h"
#include "zstring.h"
#include "widgets/zparameterarray.h"
#include "zstackmvc.h"
#include "zstackdoc.h"
#include "zflyemcontrolform.h"
#include "dialogs/flyembodymergeprojectdialog.h"
#include "dvid/zdvidreader.h"
#include "zstackfactory.h"
#include "dvid/zdvidtile.h"
#include "zstackview.h"
#include "dvid/zdvidtileensemble.h"
#include "flyem/zflyembodymergedoc.h"
#include "dvid/zdvidlabelslice.h"
#include "zstackpresenter.h"
#include "flyem/flyemproofcontrolform.h"
#include "flyem/zflyemproofmvc.h"
#include "flyem/zflyemproofdoc.h"
#include "zsysteminfo.h"
#include "zwidgetmessage.h"

#ifdef _WIN32
#undef GetOpenFileName
#undef GetSaveFileName
#endif

QString ZDialogFactory::m_currentDirectory = "";
QString ZDialogFactory::m_currentOpenFileName = "";
QString ZDialogFactory::m_currentSaveFileName = "";

ZDialogFactory::ZDialogFactory(QWidget *parentWidget)
{
  m_parentWidget = parentWidget;
}

ZDialogFactory::~ZDialogFactory()
{

}

ZDvidTargetProviderDialog* ZDialogFactory::makeDvidDialog(QWidget *parent, ZDvidDialogType dialogType)
{
  ZDvidTargetProviderDialog *dlg;

  // default = whatever is specified in config file
  if (dialogType == DEFAULT) {
    if (NeutubeConfig::getInstance().usingDvidBrowseDialog()) {
        dialogType = BRANCH_BROWSER;
    } else {
        dialogType = ORIGINAL;
    }
  }

  if (dialogType == ORIGINAL) {
    dlg = new ZDvidDialog(parent);
  } else if (dialogType == BRANCH_BROWSER) {
    dlg = new DvidBranchDialog(parent);
  } else {
      // should never happen, but make the compiler happy:
      dlg = new ZDvidDialog(parent);
  }

  return dlg;
}

DvidImageDialog* ZDialogFactory::makeDvidImageDialog(
    ZDvidTargetProviderDialog *dvidDlg, QWidget *parent)
{
  DvidImageDialog *dlg = new DvidImageDialog(parent);
  if (dvidDlg != NULL) {
    dlg->setDvidDialog(dvidDlg);
  }

  return dlg;
}

ZSpinBoxDialog* ZDialogFactory::makeSpinBoxDialog(QWidget *parent)
{
  ZSpinBoxDialog *dlg = new ZSpinBoxDialog(parent);
  return dlg;
}

QDialog* ZDialogFactory::makeStackDialog(QWidget *parent)
{
  QDialog *dlg = new QDialog(parent);
  QHBoxLayout *layout = new QHBoxLayout(dlg);
  layout->setMargin(1);
  dlg->setLayout(layout);

  ZDvidTarget target;

//  target.set("http://emrecon100.janelia.priv", "2a3", -1);
//  target.setLabelBlockName("bodies");
//  target.setMultiscale2dName("graytiles");

//  target.set("http://emdata2.int.janelia.org", "628", -1);
  target.set("http://emdata1.int.janelia.org", "9db", 8500);
#if 0
  ZDvidReader reader;
  reader.open(target);
  ZDvidInfo dvidInfo = reader.readGrayScaleInfo();
  ZFlyEmProofDoc *doc = new ZFlyEmProofDoc(NULL, NULL);
  ZStack *stack = ZStackFactory::makeVirtualStack(
        ZIntCuboid(dvidInfo.getStartCoordinates(),
                   dvidInfo.getEndCoordinates()));
  doc->loadStack(stack);
  //doc->loadFile(GET_TEST_DATA_DIR + "/benchmark/ball.tif");

  ZFlyEmProofMvc *stackWidget =
      ZFlyEmProofMvc::Make(NULL, ZSharedPointer<ZFlyEmProofDoc>(doc));
  stackWidget->getPresenter()->setObjectStyle(ZStackObject::SOLID);

  ZDvidTileEnsemble *ensemble = new ZDvidTileEnsemble;
  ensemble->setDvidTarget(target);
  ensemble->attachView(stackWidget->getView());
  doc->addObject(ensemble);

//  target.setBodyLabelName("labels");

  ZDvidLabelSlice *labelSlice = new ZDvidLabelSlice;
  labelSlice->setRole(ZStackObjectRole::ROLE_ACTIVE_VIEW);
  labelSlice->setDvidTarget(target);
  doc->addObject(labelSlice);
#endif

  ZFlyEmProofMvc *stackWidget = ZFlyEmProofMvc::Make(target);


   /*
  ZDvidReader reader;
  reader.open(target);

  ZDvidTile *tile = reader.readTile(3, 0, 0, 6000);
  tile->setDvidTarget(target);
  tile->printInfo();

  tile->attachView(stackWidget->getView());
  doc->addObject(tile);

  tile = reader.readTile(3, 0, 1, 6000);
  tile->attachView(stackWidget->getView());
  doc->addObject(tile);

  tile = reader.readTile(3, 1, 0, 6000);
  tile->attachView(stackWidget->getView());
  doc->addObject(tile);

  tile = reader.readTile(3, 1, 1, 6000);
  tile->attachView(stackWidget->getView());
  doc->addObject(tile);
  */


  layout->addWidget(stackWidget);
  FlyEmProofControlForm *controlForm = new FlyEmProofControlForm;
  layout->addWidget(controlForm);

  stackWidget->connectControlPanel(controlForm);

  return dlg;
}


QDialog* ZDialogFactory::makeTestDialog(QWidget *parent)
{
  QDialog *dlg = new QDialog(parent);
  QVBoxLayout *layout = new QVBoxLayout(dlg);
  dlg->setLayout(layout);

  layout->addWidget(ZWidgetFactory::MakeLabledEditWidget(
                      "test", ZWidgetFactory::ESpacerOption::NONE, dlg));


  QHBoxLayout *buttonLayout = new QHBoxLayout(dlg);
  buttonLayout->addSpacerItem(ZWidgetFactory::MakeHSpacerItem());
  QPushButton *okButton = new QPushButton(dlg);
  buttonLayout->addWidget(okButton);
  okButton->setText("OK");

  layout->addLayout(buttonLayout);

  dlg->connect(okButton, SIGNAL(clicked()), dlg, SLOT(accept()));

  return dlg;
}

QDialog* ZDialogFactory::makeParameterDialog(
    const ZParameterArray &parameterArray, QWidget *parent)
{
  QDialog *dlg = new QDialog(parent);
  QVBoxLayout *layout = new QVBoxLayout(dlg);
  foreach (QPointer<ZParameter> parameter, parameterArray.getData()) {
    if(parameter) {
      QHBoxLayout *wlayout = new QHBoxLayout(dlg);
      wlayout->addWidget(parameter->createNameLabel(dlg));
      wlayout->addWidget(parameter->createWidget(dlg));
      layout->addLayout(wlayout);
    }
  }

  return dlg;
}

bool ZDialogFactory::Ask(
    const QString &title, const QString &msg, QWidget *parent)
{
  return QMessageBox::question(
        parent, title, msg, QMessageBox::No | QMessageBox::Yes) ==
      QMessageBox::Yes;
}

void ZDialogFactory::Warn(
    const QString &title, const QString &msg, QWidget *parent)
{
  QMessageBox::warning(parent, title, msg);
}

void ZDialogFactory::Error(
    const QString &title, const QString &msg, QWidget *parent)
{
  QMessageBox::critical(parent, title, msg);
}

ZSpinBoxGroupDialog* ZDialogFactory::makeDownsampleDialog(QWidget *parent)
{
  ZSpinBoxGroupDialog *dlg = new ZSpinBoxGroupDialog(parent);
  dlg->setWindowTitle("Set Downsample Interval");
  dlg->addSpinBox("X", 0, 100, 0, 0);
  dlg->addSpinBox("Y", 0, 100, 0, 0);
  dlg->addSpinBox("Z", 0, 100, 0, 0);
  return dlg;
}

QString ZDialogFactory::GetDirectory(
    const QString &caption, const QString &filePath, QWidget *parent)
{
  QString fileName;

  QString currentDirectory = filePath;
  if (currentDirectory.isEmpty()) {
    currentDirectory = m_currentDirectory;
  }

  fileName = QFileDialog::getExistingDirectory(
        parent, caption, currentDirectory,
        QFileDialog::ShowDirsOnly);
  m_currentDirectory = fileName;

  return fileName;
}

QString ZDialogFactory::GetOpenFileName(
    const QString &caption, const QString &filePath, QWidget *parent)
{
  QString fileName;

  QString currentPath = filePath;
  if (currentPath.isEmpty()) {
    currentPath = m_currentOpenFileName;
  }
  fileName = QFileDialog::getOpenFileName(parent, caption, currentPath);
  m_currentOpenFileName = fileName;

  return fileName;
}

QString ZDialogFactory::GetSaveFileName(
    const QString &caption, const QString &filePath, QWidget *parent)
{
  QString fileName;

  QString currentPath = filePath;
  if (currentPath.isEmpty()) {
    currentPath = m_currentSaveFileName;
  }
  fileName = QFileDialog::getSaveFileName(parent, caption, currentPath);
  m_currentSaveFileName = fileName;

  return fileName;
}

void ZDialogFactory::Notify3DDisabled(QWidget *parent)
{
  QMessageBox::critical(parent, "3D functions are disabled",
                        ZSystemInfo::instance().errorMessage());
}

void ZDialogFactory::About(QWidget *parent)
{
  QString title = QString("<h2>%1</h2>").arg(GET_SOFTWARE_NAME.c_str());
#if defined(_CURRENT_COMMIT_)
  if (!NeutubeConfig::getInstance().getApplication().empty()) {
    title += QString("<p>") +
        NeutubeConfig::getInstance().getApplication().c_str() + " Edition" +
        " (" + _CURRENT_COMMIT_ + ")</p>";
  }
#endif

  QString version;
#if defined(_PKG_VERSION)
  version = ""
      NT_XSTR(_PKG_VERSION);
#endif

  if (!version.isEmpty()) {
    title += QString("<p>Version: %1</p>").arg(version);
  }

  QString thirdPartyLib = QString(
        "<p><a href=\"file:///%1/doc/ThirdPartyLibraries.txt\">Third-Party Credits</a></p>")
      .arg(GET_APPLICATION_DIR.c_str());
  QMessageBox::about(parent, QString("About %1").arg(GET_SOFTWARE_NAME.c_str()),
                     title +
                     "<p>" + GET_SOFTWARE_NAME.c_str() +" is software "
                     "for neuron reconstruction and visualization. "
#if !defined(_FLYEM_)
                     "It was originally developed by Ting Zhao "
                     "in Myers Lab "
                     "at Howard Hughes Medical Institute.</p>"
                     "<p>Current developers: </p>"
                     "<ul>"
                     "<li>Ting Zhao</li>"
                     "<p>Howard Hughes Medical Institute, Janelia Farm Research Campus, "
                     "Ashburn, VA 20147</p>"
                     "<li>Linqing Feng</li>"
                     "<p>Jinny Kim's Lab, Center for Functional Connectomics, KIST, Korea</p>"
                     "</ul>"
                     "<p>Website: <a href=\"www.neutracing.com\">www.neutracing.com</a></p>"
#endif
                     "<p>The Software is provided \"as is\" without warranty of any kind, "
                     "either express or implied, including without limitation any implied "
                     "warranties of condition, uniterrupted use, merchantability, fitness "
                     "for a particular purpose, or non-infringement.</p>"
                     "<p>For any regarded question or feedback, please mail to "
                     "<a href=mailto:tingzhao@gmail.com>tingzhao@gmail.com</a></p>"
                     "<p>Source code: "
                     "<a href=\"https://github.com/janelia-flyem/NeuTu\">"
                     "https://github.com/janelia-flyem/NeuTu</a></p>" + thirdPartyLib

                     );
}

void ZDialogFactory::PromptMessage(const ZWidgetMessage &msg, QWidget *parent)
{
  if (msg.hasTarget(ZWidgetMessage::TARGET_DIALOG)) {
      switch (msg.getType()) {
      case neutube::EMessageType::INFORMATION:
        QMessageBox::information(parent, msg.getTitle(), msg.toHtmlString());
        break;
      case neutube::EMessageType::WARNING:
      case neutube::EMessageType::ERROR:
        QMessageBox::warning(parent, msg.getTitle(), msg.toHtmlString());
        break;
      default:
        break;
      }
    }
}
