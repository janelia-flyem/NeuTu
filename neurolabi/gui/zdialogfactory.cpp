#include "zdialogfactory.h"
#include <QVBoxLayout>
#include <QPushButton>
#include <QMessageBox>
#include <QFileDialog>
#include <QFileInfo>

#include "zwidgetfactory.h"
#include "neutubeconfig.h"
#include "zstring.h"
#include "zparameterarray.h"
#include "zstackmvc.h"
#include "zstackdoc.h"

#include "zstackfactory.h"
#include "zstackview.h"
#include "zstackpresenter.h"
#include "z3dapplication.h"
#if defined (_FLYEM_)
#include "flyem/flyemproofcontrolform.h"
#include "flyem/zflyemproofmvc.h"
#include "flyem/zflyemproofdoc.h"
#include "dvid/zdvidtileensemble.h"
#include "flyem/zflyembodymergedoc.h"
#include "zflyemcontrolform.h"
#include "dialogs/flyembodymergeprojectdialog.h"
#include "dvid/zdvidreader.h"
#include "dvid/zdvidtile.h"
#include "dvid/zdvidlabelslice.h"
#include "dialogs/zdviddialog.h"
#endif

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
#if defined (_FLYEM_)
ZDvidDialog* ZDialogFactory::makeDvidDialog(QWidget *parent)
{
  ZDvidDialog *dlg = new ZDvidDialog(parent);
  /*
  dlg->loadConfig(ZString::fullPath(GET_APPLICATION_DIR,
                                    "json", "", "flyem_config.json"));
                                    */

  return dlg;
}

DvidImageDialog* ZDialogFactory::makeDvidImageDialog(
    ZDvidDialog *dvidDlg, QWidget *parent)
{
  DvidImageDialog *dlg = new DvidImageDialog(parent);
  if (dvidDlg != NULL) {
    dlg->setDvidDialog(dvidDlg);
  }

  return dlg;
}
#endif
ZSpinBoxDialog* ZDialogFactory::makeSpinBoxDialog(QWidget *parent)
{
  ZSpinBoxDialog *dlg = new ZSpinBoxDialog(parent);
  return dlg;
}
#if defined (_FLYEM_)
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
#endif

QDialog* ZDialogFactory::makeTestDialog(QWidget *parent)
{
  QDialog *dlg = new QDialog(parent);
  QVBoxLayout *layout = new QVBoxLayout(dlg);
  dlg->setLayout(layout);

  layout->addWidget(ZWidgetFactory::makeLabledEditWidget(
                      "test", ZWidgetFactory::SPACER_NONE, dlg));


  QHBoxLayout *buttonLayout = new QHBoxLayout(dlg);
  buttonLayout->addSpacerItem(ZWidgetFactory::makeHSpacerItem());
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
  QMessageBox::information(
        parent, "3D Unavailable", "The 3D visualization is unavailable in this"
        "plug-in because of some technical problems. To obtain a "
        "fully-functioing version of neuTube, because visit "
        "<a href=www.neutracing.com>www.neutracing.com</a>");
}
