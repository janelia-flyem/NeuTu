#include "zflyemroidialog.h"
#include <QtConcurrentRun>
#include "ui_zflyemroidialog.h"
#include "dvid/zdvidreader.h"
#include "dvid/zdvidinfo.h"
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

  m_project.setZ(ui->zSpinBox->value());
  //m_project.setDvidTarget(m_dvidDlg->getDvidTarget());

  updateWidget();
  setProgressBar(ui->progressBar);
  ui->progressBar->hide();
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
  advance(0.1);
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

    ZStack *stack = reader.readGrayScale(
          dvidInfo.getStartCoordinates().getX(),
          dvidInfo.getStartCoordinates().getY(),
          z, dvidInfo.getStackSize()[0],
          dvidInfo.getStackSize()[1], 1);

    if (stack != NULL) {
      ZIntCuboid cuboid = estimateBoundBox(*stack);
      if (!cuboid.isEmpty()) {
        stack->crop(cuboid);
      }

      advance(0.5);

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
  int z = m_project.findSliceToCreateRoi();
  if (z >= 0) {
    start();
    m_project.setZ(z);
    ui->zSpinBox->setValue(z);
    QtConcurrent::run(
          this, &ZFlyEmRoiDialog::loadGrayscaleFunc, z);
  }
}

ZIntCuboid ZFlyEmRoiDialog::estimateBoundBox(const ZStack &stack)
{
  const static uint8_t fgValue = 255;
  int width = stack.width();
  int height = stack.height();
  const uint8_t *array = stack.array8();

  ZIntCuboid cuboid = stack.getBoundBox();
  //Left bound
  for (int y = 0; y < height; y += 100) {
    size_t offset = width * y;
    for (int x = 0; x < width; ++x) {
      if (array[offset++] != fgValue) {
        if (x > cuboid.getFirstCorner().getX()) {
          cuboid.setFirstX(x);
        }
        break;
      }
    }
  }

  //Right bound
  for (int y = 0; y < height; y += 100) {
    size_t offset = width * y + width - 1;
    for (int x = width - 1; x >= 0; --x) {
      if (array[offset--] != fgValue) {
        if (x < cuboid.getLastCorner().getX()) {
          cuboid.setLastX(x);
        }
        break;
      }
    }
  }

  //Up bound
  for (int x = 0; x < width; x += 100) {
    size_t offset =x;
    for (int y = 0; y < height; ++y) {
      if (array[offset] != fgValue) {
        if (y > cuboid.getFirstCorner().getY()) {
          cuboid.setFirstY(y);
        }
        break;
      }
      offset += width;
    }
  }

  //Down bound
  size_t area = width * height;
  for (int x = 0; x < width; x += 100) {
    size_t offset = area - 1 - x;
    for (int y = height - 1; y >= 0; --y) {
      if (array[offset] != fgValue) {
        if (y < cuboid.getLastCorner().getY()) {
          cuboid.setLastY(y);
        }
        break;
      }
      offset -= width;
    }
  }

  return cuboid;
}
