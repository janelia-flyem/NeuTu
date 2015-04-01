#include "zdialogfactory.h"
#include <QVBoxLayout>
#include <QPushButton>
#include <QMessageBox>
#include <QFileDialog>
#include <QFileInfo>

#include "zwidgetfactory.h"
#include "zdviddialog.h"
#include "neutubeconfig.h"
#include "zstring.h"
#include "zparameterarray.h"
#include "zstackmvc.h"
#include "zstackdoc.h"
#include "zflyemcontrolform.h"
#include "flyembodymergeprojectdialog.h"
#include "dvid/zdvidreader.h"
#include "zstackfactory.h"
#include "dvid/zdvidtile.h"
#include "zstackview.h"
#include "dvid/zdvidtileensemble.h"

ZDialogFactory::ZDialogFactory(QWidget *parentWidget)
{
  m_parentWidget = parentWidget;
}

ZDialogFactory::~ZDialogFactory()
{

}

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

  target.set("http://emrecon100.janelia.priv", "2a3", -1);

  //target.set("http://emdata2.int.janelia.org", "628", -1);

  ZDvidReader reader;
  reader.open(target);
  ZDvidInfo dvidInfo = reader.readGrayScaleInfo();
  ZStackDoc *doc = new ZStackDoc(NULL, NULL);
  ZStack *stack = ZStackFactory::makeVirtualStack(
        ZIntCuboid(dvidInfo.getStartCoordinates(),
                   dvidInfo.getEndCoordinates()));
  doc->loadStack(stack);
  //doc->loadFile(GET_TEST_DATA_DIR + "/benchmark/ball.tif");

  ZStackMvc *stackWidget =
      ZStackMvc::Make(NULL, ZSharedPointer<ZStackDoc>(doc));

  ZDvidTileEnsemble *ensemble = new ZDvidTileEnsemble;
  ensemble->setDvidTarget(target);
  ensemble->attachView(stackWidget->getView());
  doc->addObject(ensemble);

  stackWidget->getView()->reset(false);
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
  layout->addWidget(new FlyEmBodyMergeProjectDialog);

  return dlg;
}

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

bool ZDialogFactory::ask(
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
  fileName = QFileDialog::getExistingDirectory(
        parent, caption, filePath,
        QFileDialog::ShowDirsOnly);

  return fileName;
}

QString ZDialogFactory::GetFileName(
    const QString &caption, const QString &filePath, QWidget *parent)
{
  QString fileName;
  fileName = QFileDialog::getOpenFileName(parent, caption, filePath);

  return fileName;
}
