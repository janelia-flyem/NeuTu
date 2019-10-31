#include "zflyemgrayscaledialog.h"
#include "ui_zflyemgrayscaledialog.h"
#include "flyem/zproofreadwindow.h"
#include "flyem/zflyemproofmvc.h"
#include "mvc/zstackview.h"
#include "geometry/zintpoint.h"
#include "geometry/zintcuboid.h"

ZFlyEmGrayscaleDialog::ZFlyEmGrayscaleDialog(QWidget *parent) :
  QDialog(parent),
  ui(new Ui::ZFlyEmGrayscaleDialog)
{
  ui->setupUi(this);

  connectSignalSlot();
}

ZFlyEmGrayscaleDialog::~ZFlyEmGrayscaleDialog()
{
  delete ui;
}

void ZFlyEmGrayscaleDialog::connectSignalSlot()
{
  connect(ui->offsetPushButton, SIGNAL(clicked()), SLOT(useCurrentOffset()));
  connect(ui->centerPushButton, SIGNAL(clicked()), SLOT(useViewCenter()));
  connect(ui->viewPortPushButton, SIGNAL(clicked()), SLOT(useViewPort()));
  connect(ui->fullRangeCheckBox, SIGNAL(toggled(bool)), SLOT(updateWidget()));
}

ZProofreadWindow* ZFlyEmGrayscaleDialog::getMainWindow() const
{
  return qobject_cast<ZProofreadWindow*>(parentWidget());
}

ZFlyEmProofMvc* ZFlyEmGrayscaleDialog::getFlyEmProofMvc() const
{
  return qobject_cast<ZFlyEmProofMvc*>(parentWidget());
}

ZIntPoint ZFlyEmGrayscaleDialog::getOffset() const
{
  return ZIntPoint(getOffsetX(), getOffsetY(), getOffsetZ());
}

ZIntPoint ZFlyEmGrayscaleDialog::getSize() const
{
  return ZIntPoint(getWidth(), getHeight(), getDepth());
}

ZIntPoint ZFlyEmGrayscaleDialog::getFirstCorner() const
{
  return getOffset();
}

ZIntPoint ZFlyEmGrayscaleDialog::getLastCorner() const
{
  return getOffset() + getSize() - 1;
}

ZIntCuboid ZFlyEmGrayscaleDialog::getBoundBox() const
{
  ZIntCuboid box;

  box.setFirstCorner(getFirstCorner());
  box.setLastCorner(getLastCorner());

  return box;
}

ZIntCuboid ZFlyEmGrayscaleDialog::getRange() const
{
  ZIntCuboid box;
  if (!isFullRange()) {
    box = getBoundBox();
  }

  return box;
}

int ZFlyEmGrayscaleDialog::getOffsetX() const
{
  return ui->xSpinBox->value();
}

int ZFlyEmGrayscaleDialog::getOffsetY() const
{
  return ui->ySpinBox->value();
}

int ZFlyEmGrayscaleDialog::getOffsetZ() const
{
  return ui->zSpinBox->value();
}

int ZFlyEmGrayscaleDialog::getWidth() const
{
  return ui->widthSpinBox->value();
}

int ZFlyEmGrayscaleDialog::getHeight() const
{
  return ui->heightSpinBox->value();
}

int ZFlyEmGrayscaleDialog::getDepth() const
{
  return ui->depthSpinBox->value();
}

int ZFlyEmGrayscaleDialog::getDsIntv() const
{
  return ui->downsampleSpinBox->value() - 1;
}

void ZFlyEmGrayscaleDialog::setOffset(int x, int y, int z)
{
  ui->xSpinBox->setValue(x);
  ui->ySpinBox->setValue(y);
  ui->zSpinBox->setValue(z);
}

void ZFlyEmGrayscaleDialog::setWidth(int width)
{
  ui->widthSpinBox->setValue(width);
}

void ZFlyEmGrayscaleDialog::setHeight(int height)
{
  ui->heightSpinBox->setValue(height);
}

void ZFlyEmGrayscaleDialog::setDepth(int depth)
{
  ui->depthSpinBox->setValue(depth);
}

ZStackViewParam ZFlyEmGrayscaleDialog::getViewParam() const
{
  ZFlyEmProofMvc *mvc = getFlyEmProofMvc();
  ZStackViewParam viewParam;
  if (mvc != NULL) {
    viewParam = mvc->getView()->getViewParameter();
  }

  return viewParam;
}

void ZFlyEmGrayscaleDialog::useViewCenter()
{
  ZStackViewParam viewParam = getViewParam();
  QRect viewPort = viewParam.getViewPort();
  if (viewPort.isValid()) {
    QPoint center = viewPort.center();
    setOffset(center.x() - getWidth() / 2, center.y() - getHeight() / 2,
              viewParam.getZ() - getDepth() / 2);
  }
}

void ZFlyEmGrayscaleDialog::useViewPort()
{
  ZStackViewParam viewParam = getViewParam();
  QRect viewPort = viewParam.getViewPort();
  if (viewPort.isValid()) {
    setOffset(viewPort.left(), viewPort.top(), viewParam.getZ());
    setWidth(viewPort.width());
    setHeight(viewPort.height());
    setDepth(1);
  }
}

void ZFlyEmGrayscaleDialog::useCurrentOffset()
{
  ZStackViewParam viewParam = getViewParam();
  QRect viewPort = viewParam.getViewPort();
  if (viewPort.isValid()) {
    setOffset(viewPort.left(), viewPort.top(), viewParam.getZ());
  }
}

bool ZFlyEmGrayscaleDialog::isFullRange() const
{
  return ui->fullRangeCheckBox->isChecked();
}

bool ZFlyEmGrayscaleDialog::isSparse() const
{
  return ui->sparseCheckBox->isChecked();
}

void ZFlyEmGrayscaleDialog::updateWidget()
{
  bool rangeAjustable = !(ui->fullRangeCheckBox->isChecked());
  ui->xSpinBox->setEnabled(rangeAjustable);
  ui->ySpinBox->setEnabled(rangeAjustable);
  ui->zSpinBox->setEnabled(rangeAjustable);
  ui->widthSpinBox->setEnabled(rangeAjustable);
  ui->heightSpinBox->setEnabled(rangeAjustable);
  ui->depthSpinBox->setEnabled(rangeAjustable);
  ui->centerPushButton->setEnabled(rangeAjustable);
  ui->viewPortPushButton->setEnabled(rangeAjustable);
  ui->offsetPushButton->setEnabled(rangeAjustable);
}

void ZFlyEmGrayscaleDialog::makeBodyExportAppearance()
{
  ui->downsampleSpinBox->setVisible(false);
  ui->downsampleLabel->setVisible(false);
  ui->sparseCheckBox->setVisible(true);
  ui->fullRangeCheckBox->setVisible(true);
}

void ZFlyEmGrayscaleDialog::makeGrayscaleExportAppearance()
{
  ui->downsampleSpinBox->setVisible(true);
  ui->downsampleLabel->setVisible(true);
  ui->sparseCheckBox->setVisible(false);
  ui->fullRangeCheckBox->setVisible(false);
}

void ZFlyEmGrayscaleDialog::makeBodyFieldExportAppearance()
{
  ui->downsampleSpinBox->setVisible(false);
  ui->downsampleLabel->setVisible(false);
  ui->sparseCheckBox->setVisible(false);
  ui->fullRangeCheckBox->setVisible(true);
}


