#include "zflyemroidialog.h"
#include <QtConcurrentRun>
#include "neutubeconfig.h"
#include "ui_zflyemroidialog.h"
#include "dvid/zdvidreader.h"
#include "dvid/zdvidinfo.h"
#include "dvid/zdvidwriter.h"
#include "mainwindow.h"
#include "zstackframe.h"

ZFlyEmRoiDialog::ZFlyEmRoiDialog(QWidget *parent) :
  QDialog(parent), ZQtBarProgressReporter(),
  ui(new Ui::ZFlyEmRoiDialog)
{
  ui->setupUi(this);

  m_dvidDlg = ZDialogFactory::makeDvidDialog(this);
  m_zDlg = ZDialogFactory::makeSpinBoxDialog(this);

  connect(ui->loadGrayScalePushButton, SIGNAL(clicked()),
          this, SLOT(loadGrayscale()));
  connect(ui->dvidServerPushButton, SIGNAL(clicked()),
          this, SLOT(setDvidTarget()));
  connect(ui->estimateRoiPushButton,
          SIGNAL(clicked()), this, SLOT(estimateRoi()));
  connect(ui->saveResultPushButton, SIGNAL(clicked()), this, SLOT(addRoi()));
  connect(ui->zSpinBox, SIGNAL(valueChanged(int)), this, SLOT(setZ(int)));
  connect(ui->fullRoiPreviewPushButton, SIGNAL(clicked()),
          this, SLOT(previewFullRoi()));
  connect(ui->uploadResultPushButton, SIGNAL(clicked()),
          this, SLOT(uploadRoi()));

  connect(this, SIGNAL(newDocReady()), this, SLOT(newDataFrame()));
  connect(this, SIGNAL(progressFailed()), ui->progressBar, SLOT(reset()));
  connect(this, SIGNAL(progressAdvanced(double)),
          this, SLOT(advanceProgressSlot(double)));

  m_project.setZ(ui->zSpinBox->value());
  //m_project.setDvidTarget(m_dvidDlg->getDvidTarget());

  updateWidget();
  setProgressBar(ui->progressBar);
  ui->progressBar->hide();

#ifndef _DEBUG_
  ui->testPushButton->hide();
#endif
}

ZFlyEmRoiDialog::~ZFlyEmRoiDialog()
{
  delete ui;
}

void ZFlyEmRoiDialog::clear()
{
  m_project.clear();
  updateWidget();
}

void ZFlyEmRoiDialog::updateWidget()
{
  QString text = QString("<p>DVID: %1</p>"
                         "<p>Z Range: [%2, %3]"
                         "<p>Opened Slice: Z = %4; ROI: %5</p>").
      arg(m_project.getDvidTarget().getName().c_str()).
      arg(m_project.getDvidInfo().getMinZ()).
      arg(m_project.getDvidInfo().getMaxZ()).
      arg(m_project.getDataZ()).arg(m_project.hasOpenedRoi());
  ui->infoWidget->setText(text);

  ui->loadGrayScalePushButton->setEnabled(m_project.getDvidTarget().isValid());
}

void ZFlyEmRoiDialog::setDvidTarget()
{
  if (m_dvidDlg->exec()) {
    m_project.setDvidTarget(m_dvidDlg->getDvidTarget());
    updateWidget();
  }
}

void ZFlyEmRoiDialog::loadGrayscaleFunc(int z)
{
  //advance(0.1);
  emit progressAdvanced(0.1);
  ZDvidReader reader;
  if (z >= 0 && reader.open(m_project.getDvidTarget())) {
    if (m_project.getRoi(z) == NULL) {
      m_project.downloadRoi(z);
    }

    QString infoString = reader.readInfo("grayscale");

    qDebug() << infoString;

    ZDvidInfo dvidInfo;
    dvidInfo.setFromJsonString(infoString.toStdString());

    //int z = m_zDlg->getValue();
    //m_project.setZ(z);

    ZIntCuboid boundBox = reader.readBoundBox(z);

    ZStack *stack = NULL;
    if (!boundBox.isEmpty()) {
      stack = reader.readGrayScale(boundBox.getFirstCorner().getX(),
                                   boundBox.getFirstCorner().getY(),
                                   z, boundBox.getWidth(),
                                   boundBox.getHeight(), 1);
    } else {
      stack = reader.readGrayScale(
            dvidInfo.getStartCoordinates().getX(),
            dvidInfo.getStartCoordinates().getY(),
            z, dvidInfo.getStackSize()[0],
          dvidInfo.getStackSize()[1], 1);
      if (stack != NULL) {
        boundBox = ZFlyEmRoiProject::estimateBoundBox(*stack);
        if (!boundBox.isEmpty()) {
          stack->crop(boundBox);
        }
        ZDvidWriter writer;
        if (writer.open(m_project.getDvidTarget())) {
          writer.writeBoundBox(boundBox, z);
        }
      }
    }

    if (stack != NULL) {
      //advance(0.5);
      emit progressAdvanced(0.5);
      m_docReader.clear();
      m_docReader.setStack(stack);

      ZSwcTree *tree = m_project.getRoiSwc(z);
      if (tree != NULL) {
        m_docReader.addObject(
              tree, NeuTube::Documentable_SWC, ZDocPlayer::ROLE_ROI);
      }
      emit newDocReady();
    } else {
      emit progressFailed();
    }
  } else {
    emit progressFailed();
  }
}

void ZFlyEmRoiDialog::newDataFrame()
{
  ZStackFrame *frame = getMainWindow()->createStackFrame(
        m_docReader, NeuTube::Document::FLYEM_ROI);
  frame->document()->setStackBackground(NeuTube::IMAGE_BACKGROUND_BRIGHT);
  setDataFrame(frame);

  getMainWindow()->addStackFrame(frame);
  getMainWindow()->presentStackFrame(frame);
  updateWidget();
  end();
}

void ZFlyEmRoiDialog::loadGrayscale()
{
  start();
  int z = m_project.getCurrentZ();
  QtConcurrent::run(
        this, &ZFlyEmRoiDialog::loadGrayscaleFunc, z);
}

void ZFlyEmRoiDialog::setDataFrame(ZStackFrame *frame)
{
  connect(frame, SIGNAL(destroyed()), this, SLOT(shallowClearDataFrame()));
  m_project.setDataFrame(frame);
}

MainWindow* ZFlyEmRoiDialog::getMainWindow()
{
  return dynamic_cast<MainWindow*>(this->parentWidget());
}

void ZFlyEmRoiDialog::shallowClearDataFrame()
{
  m_project.shallowClearDataFrame();
  updateWidget();
}

void ZFlyEmRoiDialog::addRoi()
{
  if (!m_project.addRoi()) {
    dump("The result cannot be saved because the ROI is invalid.");
  } else {
    dump("ROI saved successfully.");
  }
}

void ZFlyEmRoiDialog::dump(const QString &str)
{
  ui->outputWidget->setText(str);
}

void ZFlyEmRoiDialog::setZ(int z)
{
  m_project.setZ(z);
  //updateWidget();
}

void ZFlyEmRoiDialog::previewFullRoi()
{
  ZSwcTree *tree = m_project.getAllRoiSwc();

  if (tree != NULL) {
    ZStackFrame *frame = new ZStackFrame();
    frame->document()->addSwcTree(tree);

    frame->open3DWindow(this);
    delete frame;
  } else {
    dump("No ROI saved");
  }
}

void ZFlyEmRoiDialog::uploadRoi()
{
  int count = m_project.uploadRoi();
  dump(QString("%1 ROI curves uploaded").arg(count));
}

void ZFlyEmRoiDialog::estimateRoi()
{
  m_project.estimateRoi();
}

void ZFlyEmRoiDialog::on_searchPushButton_clicked()
{
  int z = m_project.findSliceToCreateRoi(ui->zSpinBox->value());
  if (z >= 0) {
    start();
    m_project.setZ(z);
    ui->zSpinBox->setValue(z);
    QtConcurrent::run(
          this, &ZFlyEmRoiDialog::loadGrayscaleFunc, z);
  }
}


void ZFlyEmRoiDialog::on_testPushButton_clicked()
{
  ZObject3dScan obj = m_project.getRoiObject();

  obj.save(GET_DATA_DIR + "/test.sobj");

  dump(QString("%1 saved.").arg((GET_DATA_DIR + "/test.sobj").c_str()));
}

#define ROI_SCALE 1.01

void ZFlyEmRoiDialog::on_xIncPushButton_clicked()
{
  m_project.scaleRoiSwc(ROI_SCALE, 1.0);
}

void ZFlyEmRoiDialog::on_xDecPushButton_clicked()
{
  m_project.scaleRoiSwc(1. / ROI_SCALE, 1.0);
}


void ZFlyEmRoiDialog::on_yDecPushButton_clicked()
{
  m_project.scaleRoiSwc(1.0, 1. / ROI_SCALE);
}

void ZFlyEmRoiDialog::on_yIncPushButton_clicked()
{
  m_project.scaleRoiSwc(1.0, ROI_SCALE);
}

void ZFlyEmRoiDialog::on_rotateLeftPushButton_clicked()
{
  m_project.rotateRoiSwc(-0.1);
}

void ZFlyEmRoiDialog::on_rotateRightPushButton_clicked()
{
  m_project.rotateRoiSwc(0.1);
}

void ZFlyEmRoiDialog::on_xyDecPushButton_clicked()
{
  m_project.scaleRoiSwc(1. / ROI_SCALE, 1. / ROI_SCALE);
}

void ZFlyEmRoiDialog::on_xyIncPushButton_clicked()
{
  m_project.scaleRoiSwc(ROI_SCALE, ROI_SCALE);
}

#define MOVE_STEP 5.0

void ZFlyEmRoiDialog::on_movexyDecPushButton_clicked()
{
  m_project.translateRoiSwc(-MOVE_STEP, -MOVE_STEP);
}

void ZFlyEmRoiDialog::on_movexyIncPushButton_clicked()
{
  m_project.translateRoiSwc(MOVE_STEP, MOVE_STEP);
}

void ZFlyEmRoiDialog::on_movexDecPushButton_clicked()
{
  m_project.translateRoiSwc(-MOVE_STEP, 0.0);
}

void ZFlyEmRoiDialog::on_movexIncPushButton_clicked()
{
  m_project.translateRoiSwc(MOVE_STEP, 0.0);
}

void ZFlyEmRoiDialog::on_moveyDecPushButton_clicked()
{
  m_project.translateRoiSwc(0.0, -MOVE_STEP);
}

void ZFlyEmRoiDialog::on_moveyIncPushButton_clicked()
{
  m_project.translateRoiSwc(0.0, MOVE_STEP);
}

void ZFlyEmRoiDialog::advanceProgressSlot(double p)
{
  advance(p);
}
