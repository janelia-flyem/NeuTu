#include "zstackview.h"

#include <iostream>
#include <QElapsedTimer>
#include <QMdiArea>
#include <QImageWriter>
#include <QJsonObject>

#include "neulib/math/utilities.h"
#include "common/debug.h"
//#include "common/math.h"
#include "logging/zlog.h"
#include "logging/zbenchtimer.h"
#include "qt/core/qthelper.h"
#include "data3d/displayconfig.h"
#include "geometry/zgeometry.h"
#include "widgets/zimagewidget.h"
//#include "z3dwindow.h"
#include "zimage.h"
#include "zstackdoc.h"
#include "zstackframe.h"
#include "zstackpresenter.h"
#include "zslider.h"
#include "zinteractivecontext.h"
#include "zstack.hxx"
#include "zclickablelabel.h"
#include "zstackball.h"
#include "swctreenode.h"
#include "zstroke2d.h"
#include "tz_rastergeom.h"
#include "neutubeconfig.h"
#include "zsparsestack.h"
#include "zstackviewparam.h"
#include "zstackfactory.h"
#include "zstackpatch.h"
#include "zstackobjectsourcefactory.h"
#include "zmessagemanager.h"
#include "zmessage.h"
#include "zmessagefactory.h"
#include "zbodysplitbutton.h"
#include "zstackmvc.h"
#include "zpixmap.h"
#include "zlabeledspinboxwidget.h"
#include "zstackobjectpainter.h"
//#include "dvid/zdvidlabelslice.h"
#include "zstackviewlocator.h"
#include "zscrollslicestrategy.h"
#include "zarbsliceviewparam.h"
#include "zstackdocutil.h"
#include "data3d/utilities.h"
#include "dialogs/zstackviewrecorddialog.h"
#include "zdialogfactory.h"
#include "ztask.h"
#include "vis2d/zslicecanvas.h"
#include "vis2d/zimageslicefactory.h"
#include "widgets/zcheckboxgroup.h"
#include "widgets/zh3widget.h"
#include "widgets/zhwidget.h"

#include "zstackviewrecorder.h"
#include "zpositionmapper.h"
#include "utilities.h"

std::atomic<int> ZStackView::m_nextViewId{1};

using namespace std;

static const char* VIEW_TOPIC = "view2d";

ZStackView::ZStackView(ZStackFrame *parent) : QWidget(parent)
{
  init();
}

ZStackView::ZStackView(QWidget *parent) : QWidget(parent)
{
  init();
}

ZStackView::~ZStackView()
{
  endWorkThread();

  if (m_image != NULL) {
    delete m_image;
  }

//  delete m_dynamicObjectCanvas;
//  delete m_activeDecorationCanvas;


  if (m_ctrlLayout != NULL) {
    if (m_ctrlLayout->parent() == NULL) {
      delete m_ctrlLayout;
    }
  }
  delete m_imageMask;

  delete m_sliceStrategy;
//  delete m_tileCanvas;
//  delete m_objectCanvas;
}

void ZStackView::init()
{
  m_viewId = m_nextViewId++;

  if (m_viewId < 0) {
    ZDialogFactory::Warn(
          "Failed to initialize GUI",
          "Invalid view ID detected. Please restart NeuTu to reset it back to normal",
          nullptr);
  }

  setFocusPolicy(Qt::ClickFocus);
  m_depthControl = new ZSlider(true, this);
  m_depthControl->setFocusPolicy(Qt::NoFocus);

  m_depthSpinBox = new ZLabeledSpinBoxWidget(this);
  m_depthSpinBox->setLabel("Depth:");
  m_depthSpinBox->setFocusPolicy(Qt::ClickFocus);

  m_imageWidget = new ZImageWidget(this);
  m_imageWidget->setSizePolicy(QSizePolicy::Expanding,
                               QSizePolicy::Expanding);
  m_imageWidget->setFocusPolicy(Qt::ClickFocus);
//  m_imageWidget->setPaintBundle(&m_paintBundle);

  connect(m_imageWidget, SIGNAL(transformChanged()),
          this, SLOT(processTransformChange()));
  connect(m_imageWidget, SIGNAL(sceneUpdated()),
          this, SLOT(processSceneUpdate()));
//  connect(m_imageWidget, SIGNAL(transformChanged()),
//          this, SLOT(notifyViewChanged()));
//  connect(m_imageWidget, SIGNAL(transformChanged()),
//          this, SLOT(updateDataInfo()));
  connect(m_imageWidget, SIGNAL(transformControlSyncNeeded()),
          this, SLOT(syncTransformControl()));
  connect(m_imageWidget, SIGNAL(sliceAxisChanged()),
          this, SIGNAL(sliceAxisChanged()));

  setSliceAxis(neutu::EAxis::Z);

  m_infoLabel = new QLabel(this);
  m_infoLabel->setText(tr("Stack Information"));
  m_infoLabel->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
  m_infoLabel->setFocusPolicy(Qt::NoFocus);

  m_stackLabel = new QLabel(this);
  m_stackLabel->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
  m_stackLabel->setFocusPolicy(Qt::NoFocus);

  m_activeLabel = new QLabel(this);
  m_activeLabel->setWindowFlags(Qt::FramelessWindowHint);
  m_activeLabel->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Fixed);
  m_activeLabel->setText("<font color='green'>*Active*</font>");
//  m_activeLabel->hide();
  m_activeLabel->setFocusPolicy(Qt::NoFocus);

  m_progress = new QProgressBar(this);
  m_progress->setVisible(false);
  m_progress->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Fixed);
  m_progress->setFocusPolicy(Qt::NoFocus);

  m_topWidget = new ZH3Widget(this);
  m_topWidget->addWidget(m_infoLabel, neutu::EH3Layout::LEFT);
  m_topWidget->addWidget(m_activeLabel, neutu::EH3Layout::LEFT);
  m_topWidget->assumeVisible(m_activeLabel, false);
  m_topWidget->updateVisibility();
//  m_topLayout = new QHBoxLayout;
//  m_topLayout->addWidget(m_infoLabel);
//  m_topLayout->addWidget(m_msgLabel);
//  m_topLayout->addWidget(m_activeLabel);

  //m_topLayout->addWidget(m_progress);


  m_secondTopWidget = new ZH3Widget;
//  m_secondTopLayout = new QHBoxLayout;

  m_channelControlLayout = new QHBoxLayout;
  m_toolLayout = new QHBoxLayout;
#ifdef _DEBUG_0
  m_toolLayout->addWidget(new QPushButton("test"));
  enableCustomCheckBox(0, "test", nullptr, nullptr);
  enableCustomCheckBox(1, "test2", nullptr, nullptr);
#endif
  m_secondTopWidget->addLayout(m_channelControlLayout, neutu::EH3Layout::LEFT);
  m_secondTopWidget->addLayout(m_toolLayout, neutu::EH3Layout::LEFT);
//  m_secondTopLayout->addStretch();
  m_secondTopWidget->addWidget(m_stackLabel, neutu::EH3Layout::CENTER);
//  m_msgLabel->setText("test");
  m_secondTopWidget->addWidget(m_progress, neutu::EH3Layout::CENTER);

  m_checkBoxControl = new ZCheckBoxGroup;
  m_secondTopWidget->addWidget(m_checkBoxControl, neutu::EH3Layout::RIGHT);
  m_secondTopWidget->updateVisibility();

  m_layout = new QVBoxLayout;
  m_layout->setSpacing(0);
  //m_layout->addWidget(m_infoLabel);
  m_layout->addWidget(m_topWidget);
//  m_layout->addLayout(m_secondTopLayout);
  m_layout->addWidget(m_secondTopWidget);
  m_layout->addSpacing(6);
  m_layout->addWidget(m_imageWidget);

  m_bottomWidget = new ZHWidget(1, this);

  m_bottomWidget->addWidget(m_depthControl, 0);
  m_bottomWidget->addWidget(m_depthSpinBox, 0);
//  m_zControlLayout = new QHBoxLayout;
//  m_zControlLayout->addWidget(m_depthControl);
//  m_zControlLayout->addWidget(m_depthSpinBox);

  m_layout->addWidget(m_bottomWidget);
  m_bottomWidget->updateVisibility();

//  m_layout->addWidget(m_depthControl);

#if defined(_ADVANCED_) && !defined(_FLYEM_)
  m_thresholdSlider = new ZSlider(false, this);
  m_thresholdSlider->setFocusPolicy(Qt::NoFocus);

  m_autoThreButton = new QPushButton("Auto", this);
  m_autoThreButton->setFocusPolicy(Qt::NoFocus);

  m_ctrlLayout = new QHBoxLayout;
  m_ctrlLayout->addWidget(m_autoThreButton);  
  m_ctrlLayout->addWidget(m_thresholdSlider);
  m_layout->addLayout(m_ctrlLayout);

  if (!NeutubeConfig::getInstance().getMainWindowConfig().isThresholdControlOn()) {
    hideThresholdControl();
  }
#else
  m_ctrlLayout = NULL;
  m_thresholdSlider = NULL;
  m_autoThreButton = NULL;
#endif

  setLayout(m_layout);


//  m_dynamicObjectOpacity = 0.5;

  connectSignalSlot();

  setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
  m_sizeHintOption = neutu::ESizeHintOption::DEFAULT;

  m_isRedrawBlocked = false;

  m_messageManager = NULL;
  m_splitButton = NULL;

  setDepthFrozen(false);
  setViewPortFrozen(false);
  blockViewChangeEvent(false);

  m_sliceStrategy = new ZScrollSliceStrategy(this);

  m_recorder = std::shared_ptr<ZStackViewRecorder>(
        new ZStackViewRecorder);
}

int ZStackView::getViewId() const
{
  return m_viewId;
}

void ZStackView::addToolButton(QPushButton *button)
{
  if (button) {
    m_toolLayout->addWidget(button);
  }
}

void ZStackView::removeToolButton(QPushButton *button)
{
  if (button) {
    m_toolLayout->removeWidget(button);
  }
}

void ZStackView::setWidgetReady(bool ready)
{
  if (imageWidget()->isReady() != ready) {
    imageWidget()->setReady(ready);
    if (ready && isVisible()) {
      updateViewData();
      redraw();
    }
  }
}

void ZStackView::toggleAllControls()
{
  m_topWidget->toggleDefaultVisible();
  m_topWidget->updateVisibility();
  m_secondTopWidget->toggleDefaultVisible();
  m_secondTopWidget->updateVisibility();
  m_bottomWidget->toggleDefaultVisible();
  m_bottomWidget->updateVisibility();
}

void ZStackView::enableMessageManager()
{
  if (m_messageManager == NULL) {
    m_messageManager = ZMessageManager::Make<MessageProcessor>(this);
  }
  if (m_splitButton != NULL) {
    m_splitButton->enableMessageManager(m_messageManager);
  }
}

ZStackViewRecorder* ZStackView::getRecorder()
{
  return m_recorder.get();
}

void ZStackView::hideThresholdControl()
{
  m_layout->removeItem(m_ctrlLayout);
  if (m_thresholdSlider != NULL) {
    m_thresholdSlider->hide();
  }
  if (m_autoThreButton != NULL) {
    m_autoThreButton->hide();
  }
}

void ZStackView::hideLayout(QLayout *layout, bool removing)
{
  for (int i = 0; i < layout->count(); ++i) {
    QWidget *widget = layout->itemAt(i)->widget();
    if (widget != NULL) {
      widget->hide();
    }
  }

  if (removing) {
    m_layout->removeItem(layout);
  }
}

/*
void ZStackView::setDynamicObjectAlpha(int alpha)
{
  if (alpha < 0) {
    m_dynamicObjectOpacity = 0;
  } else if (alpha > 255) {
    m_dynamicObjectOpacity = 1;
  } else {
    m_dynamicObjectOpacity = double(alpha) / 255.0;
  }
}
*/

void ZStackView::resetViewProj()
{
//  ZIntCuboid box = getViewBoundBox();
  /*
  m_imageWidget->resetViewProj(
        box.getMinCorner().getX(), box.getMinCorner().getY(),
        box.getWidth(), box.getHeight(), m_defaultViewPort);
        */
}

bool ZStackView::viewingInfo(neutu::mvc::ViewInfoFlags f) const
{
  return (m_viewFlags & f) == f;
}

neutu::mvc::ViewInfoFlags ZStackView::getViewInfoFlag() const
{
  return m_viewFlags;
}

void ZStackView::setViewInfoFlag(neutu::mvc::ViewInfoFlags f)
{
  m_viewFlags = f;
}

void ZStackView::setInfo(const QString &info)
{
  if (m_infoLabel != NULL) {
    m_infoLabel->setText(info);
//    m_infoLabel->update();
  }
}

void ZStackView::setInfo()
{
  if (m_infoLabel != NULL) {
    ZIntCuboid box = getCurrentStackRange();
    setInfo(QString("%1 x %2 => %3 x %4").arg(box.getWidth()).
            arg(box.getHeight()).
            arg(m_imageWidget->screenSize().width()).
            arg(m_imageWidget->screenSize().height()));
  }
}

void ZStackView::setStackInfo(const QString &info)
{
  if (m_stackLabel != NULL) {
    m_stackLabel->setText(info);
  }
}

/*
void ZStackView::setCentralView(int width, int height)
{
  ZIntCuboid box = getViewBoundBox();

}
*/

void ZStackView::updateDataInfo()
{
  setInfo(neutu::mvc::ComposeViewInfo(this, m_lastModelPosForDataInfo));
}

void ZStackView::updateDataInfo(const QPoint &widgetPos)
{
  ZPoint pt = getSliceViewTransform().inverseTransform(
        widgetPos.x(), widgetPos.y(), 0);
  setInfo(neutu::mvc::ComposeViewInfo(this, pt));
  m_lastModelPosForDataInfo = pt;
#ifdef _DEBUG_2
  qDebug() << "Last widget pos: " << m_lastModelPosForDataInfo;
#endif
#if 0
  int z = sliceIndex();
  if (buddyPresenter()->interactiveContext().isProjectView()) {
    z = -1;
  }

#ifdef _DEBUG_2
  std::cout << "Slice index in " << __FUNCTION__ << ": " << z << std::endl;
#endif

  //    QPointF pos = imageWidget()->canvasCoordinate(widgetPos);
  ZPoint pos(widgetPos.x(), widgetPos.y(), z);

  QString info;

  if (getViewProj().getZoom() > 0.00001) {
    if (buddyDocument()->getResolution().getUnit() ==
        ZResolution::UNIT_NANOMETER) {
      double s = iround(
            m_imageWidget->screenSize().width() / getViewProj().getZoom() *
            buddyDocument()->getResolution().voxelSizeX());
      QString unit = "nm";
      if (s > 1000.0) {
        s /= 1000.0;
        unit = "um";
      }
      if (unit == "nm" || s > 10.0) {
        info += QString(" Screen Width: ~%1").arg(iround(s)) + unit;
      } else {
        info += QString(" Screen Width: ~") + QString::number(s, 'g', 2) + unit;
      }
    } else {
      info += QString(" Screen Width: ~%1 pixels").arg(
            iround(m_imageWidget->screenSize().width() / getViewProj().getZoom()));
    }
    info += "  ";
  }

  if (buddyDocument()->hasStackData()) {
    ZPoint pt = ZPositionMapper::WidgetToRawStack(pos, getViewProj());
    info += buddyDocument()->rawDataInfo(
              pt.x(), pt.y(), pt.z(), getSliceAxis());
  } else {
    ZIntCuboid box = ZStackDocHelper::GetStackSpaceRange(
          *buddyDocument(), getSliceAxis());
#ifdef _DEBUG_2
    std::cout << "Stack range: " << box.toString() << std::endl;
#endif
    QPointF stackPos = ZPositionMapper::WidgetToStack(
          widgetPos.x(), widgetPos.y(), getViewProj());
    ZPoint dataPos;
    if (getSliceAxis() == neutu::EAxis::ARB) {
      dataPos = ZPositionMapper::StackToData(stackPos, getAffinePlane());
      info += QString("(%1, %2, %3)").
              arg(iround(dataPos.getX())).arg(iround(dataPos.getY())).
              arg(iround(dataPos.getZ()));
    } else {
      dataPos = ZPositionMapper::StackToData(
            ZPositionMapper::WidgetToStack(
              pos, getViewProj(), box.getFirstCorner().getZ()), getSliceAxis());
      info += QString("(%1, %2, %3); (%4, %5, %6)").
              arg(pos.x()).arg(pos.y()).arg(z).
              arg(iround(dataPos.getX())).arg(iround(dataPos.getY())).
              arg(iround(dataPos.getZ()));
    }
  }



  setInfo(info);
#endif
}

bool ZStackView::event(QEvent *event)
{
  if (event->type() == QEvent::KeyPress) {
    QKeyEvent *ke = (QKeyEvent*)(event);
    if (ke != NULL) {
      if (ke->key() == Qt::Key_Tab) {
        event->ignore();
        return false;
      }
    }
  }

  return QWidget::event(event);
}


void ZStackView::connectSignalSlot()
{
  /*
  connect(m_depthControl, SIGNAL(valueChanged(int)),
          this, SIGNAL(currentSliceChanged(int)));
  */

  connect(this, &ZStackView::canvasUpdated,
          this, &ZStackView::processCanvasUpdate);

  connect(m_depthControl, SIGNAL(valueChanged(int)),
          this, SLOT(processDepthSliderValueChange(int)));
  connect(m_depthControl, &ZSlider::sliderPressed,
          this, &ZStackView::notifySliceSliderPressed);
  connect(m_depthControl, &ZSlider::sliderReleased,
          this, &ZStackView::notifySliceSliderReleased);
//  connect(m_depthControl, SIGNAL(sliderReleased()),
//          this, SIGNAL(sliceSliderReleased()));
  connect(m_depthSpinBox, SIGNAL(valueChanged(int)),
          this, SLOT(setDepth(int)));
  connect(m_depthControl, SIGNAL(valueChanged(int)),
          this, SLOT(updateZSpinBoxValue()));

//  connect(this, SIGNAL(currentSliceChanged(int)), this, SLOT(redraw()));

  connect(m_imageWidget, SIGNAL(mouseReleased(QMouseEvent*)),
    this, SLOT(mouseReleasedInImageWidget(QMouseEvent*)));
  connect(m_imageWidget, SIGNAL(mouseMoved(QMouseEvent*)),
    this, SLOT(mouseMovedInImageWidget(QMouseEvent*)));
  connect(m_imageWidget, SIGNAL(mousePressed(QMouseEvent*)),
    this, SLOT(mousePressedInImageWidget(QMouseEvent*)));
  connect(m_imageWidget, SIGNAL(mouseDoubleClicked(QMouseEvent*)),
          this, SLOT(mouseDoubleClickedInImageWidget(QMouseEvent*)));
  connect(m_imageWidget, SIGNAL(mouseWheelRolled(QWheelEvent*)),
          this, SLOT(mouseRolledInImageWidget(QWheelEvent*)));

  if (m_thresholdSlider != NULL) {
    connect(m_thresholdSlider, SIGNAL(valueChanged(int)),
            this, SLOT(paintStack()));
  }

  if (m_autoThreButton != NULL) {
    connect(m_autoThreButton, SIGNAL(clicked()),
            this, SLOT(autoThreshold()));
  }

//  connect(this, SIGNAL(viewPortChanged()), this, SLOT(paintActiveTile()));
}

void ZStackView::notifySliceSliderPressed()
{
  emit sliceSliderPressed(this);
}

void ZStackView::notifySliceSliderReleased()
{
  emit sliceSliderReleased(this);
}

void ZStackView::updateZSpinBoxValue()
{
#if 0
  int z0 = buddyDocument()->getStackOffset(m_sliceAxis);
  int prevIndex = m_zSpinBox->getValue() - z0;
  int currentIndex = getCurrentZ() - z0;
  int newPos = m_sliceStrategy->scroll(prevIndex, currentIndex - prevIndex);
#ifdef _DEBUG_
  std::cout << "Scrolling: " << currentIndex << " " << prevIndex << " "
            << newPos << std::endl;
#endif
#endif

  m_depthSpinBox->setValue(getCurrentDepth());
}

/*
double ZStackView::getCanvasWidthZoomRatio() const
{
  return (double) m_imageWidget->canvasSize().width() /
      m_imageWidget->viewPort().width();
}

double ZStackView::getCanvasHeightZoomRatio() const
{
  return (double) m_imageWidget->canvasSize().height() /
      m_imageWidget->viewPort().height();
}

double ZStackView::getProjZoomRatio() const
{
  return (double) m_imageWidget->projectSize().width() /
      m_imageWidget->viewPort().width();
}
*/

ZIntCuboid ZStackView::getCurrentStackRange() const
{
  return imageWidget()->getModelRange();
}

#if 0
ZIntCuboid ZStackView::getViewBoundBox() const
{
  return ZStackDocUtil::GetStackSpaceRange(*buddyDocument(), getSliceAxis());
  /*
  ZStack *stack = stackData();
  ZIntCuboid box;
  if (stack != NULL) {
    box = stack->getBoundBox();
    box.shiftSliceAxis(m_sliceAxis);
  }

  return box;
  */
}
#endif

int ZStackView::getDepth() const
{
  return getCurrentStackRange().getDepth();
  /*
  ZStack *stack = stackData();
  if (stack != NULL) {
    return stack->getBoundBox().getDim(m_sliceAxis);
  }

  return 0;
  */
}

neutu::EAxis ZStackView::getSliceAxis() const
{
  return m_imageWidget->getSliceAxis();
}

ZPlane ZStackView::getCutOrientation() const
{
  return m_imageWidget->getCutOrientation();
}

void ZStackView::setSliceAxis(neutu::EAxis axis)
{
//  m_sliceAxis = axis;
  m_imageWidget->setSliceAxis(axis, neutu::ESignalControl::BROADCASTING);
//  m_paintBundle.setViewParam(getViewParameter());
//  m_paintBundle.setSliceAxis(axis);
}

void ZStackView::setRightHanded(bool r)
{
  m_imageWidget->setRightHanded(r);
}

void ZStackView::setCutPlane(neutu::EAxis axis)
{
  imageWidget()->setCutPlane(axis, neutu::ESignalControl::BROADCASTING);
}

void ZStackView::setCutPlane(const ZPoint &v1, const ZPoint &v2)
{
  imageWidget()->setCutPlane(v1, v2, neutu::ESignalControl::BROADCASTING);
}

void ZStackView::setCutPlane(const ZAffinePlane &plane)
{
  imageWidget()->setCutPlane(plane, neutu::ESignalControl::BROADCASTING);
//  imageWidget()->setCutPlane(plane.getV1(), plane.getV2(), neutu::ESignalControl::SLIENT);
//  imageWidget()->setCutCenter(plane.getOffset(), neutu::ESignalControl::BOARDCASTING);
}

void ZStackView::setZoomScale(double s)
{
  imageWidget()->setZoomRatio(s, neutu::ESignalControl::BROADCASTING);
}

void ZStackView::setInitialScale(double s)
{
  imageWidget()->setInitialScale(s);
}

/*
ZAffinePlane ZStackView::getAffinePlane() const
{
  return m_sliceViewParam.getAffinePlane();
}
*/

void ZStackView::setSliceRange(int minSlice, int maxSlice)
{
  m_depthControl->setRangeQuietly(minSlice, maxSlice);
  m_depthSpinBox->setRange(minSlice, maxSlice);
  m_sliceStrategy->setRange(minSlice, maxSlice);
}

void ZStackView::enableOffsetAdjustment(bool on)
{
  m_imageWidget->enableOffsetAdjustment(on);
}

void ZStackView::processTransformChange()
{
  if (isVisible()) {
    updateDataInfo();
    buddyPresenter()->setSliceViewTransform(getViewId(), getSliceViewTransform());
    //  m_paintSorter.clear();
    //  m_paintBundle.setSliceViewTransform(getSliceViewTransform());
    QElapsedTimer timer;
    timer.start();
    updateViewData();
    qint64 paintTime = timer.elapsed();

    ZOUT(KLog(VIEW_TOPIC), 5) << ZLog::Profile()
                    << ZLog::Description("view data update time")
                    << ZLog::Duration(paintTime);

    notifyViewChanged();
    redraw();
  }
}

void ZStackView::setDepthRangeQuietly(int minZ, int maxZ)
{
  m_depthControl->setRangeQuietly(minZ, maxZ);
  m_depthSpinBox->setRangeQuietly(minZ, maxZ);
}

void ZStackView::syncTransformControl()
{
  int depth = neulib::iround(imageWidget()->getCutDepth());

  setDepthRangeQuietly(
        imageWidget()->getMinCutDepth(), imageWidget()->getMaxCutDepth());
  m_depthControl->setValueQuietly(depth);
  m_depthSpinBox->setValueQuietly(depth);

#ifdef _DEBUG_
  std::cout << "Transform control synced: depth=" << depth << " within ["
            << imageWidget()->getMinCutDepth() << ", "
            << imageWidget()->getMaxCutDepth() << "]" << std::endl;
#endif
}

#if 0
void ZStackView::resetDepthControl()
{
  ZStack *stack = stackData();
  if (stack != NULL) {
    updateSlider();
//    setSliceRange(0, getDepth() - 1);
//    m_depthControl->setRange(0, getDepth() - 1);
    m_depthControl->setValue(getDepth() / 2);
  }
}
#endif

#if 0
void ZStackView::reset(bool updatingScreen)
{ 
  LDEBUG() << "Resetting view";



  ZStack *stack = stackData();
  updateChannelControl();
  if (stack != NULL) {
    updateSlider();
    m_depthControl->initValue(getDepth() / 2);
//    resetDepthControl();
//    m_imageWidget->reset();

    if (updatingScreen) {
      redraw(EUpdateOption::DIRECT);
    }

    if (stack->isThresholdable()) {
      if (m_thresholdSlider != NULL) {
        m_thresholdSlider->setRange(stack->min(), stack->max());
        m_thresholdSlider->setValue(stack->max());
      }
    } else {
      hideThresholdControl();
    }
  }
  updateStackInfo();
  setInfo();

}
#endif

void ZStackView::configure(EMode mode)
{
  switch (mode) {
  case EMode::IMAGE_ONLY:
//    hideLayout(m_topLayout, true);
    m_topWidget->hide();
    m_secondTopWidget->hide();
    m_bottomWidget->hide();
//    hideLayout(m_secondTopLayout, true);
//    hideLayout(m_zControlLayout, true);
    break;
  case EMode::PLAIN_IMAGE:
#ifndef _DEBUG_
//    hideLayout(m_topLayout);
#endif
    m_secondTopWidget->removeWidget(m_stackLabel);
    m_secondTopWidget->removeWidget(m_progress);
    m_bottomWidget->hide();
//    hideLayout(m_secondTopLayout, true);
//    hideLayout(m_zControlLayout, true);
    m_imageWidget->hideZoomHint();
    break;
  default:
    break;
  }


}

void ZStackView::updateThresholdSlider()
{
  if (stackData() != NULL && m_thresholdSlider != NULL) {
    m_thresholdSlider->setRangeQuietly(stackData()->min(), stackData()->max());
    m_thresholdSlider->setValueQuietly(stackData()->max());
  }
}

void ZStackView::updateSlider()
{
  if (stackData() != NULL) {
    ZIntCuboid box = getCurrentStackRange();

//    int value = m_depthControl->value();
    m_depthControl->setRangeQuietly(0, box.getDepth() - 1);

    int z = getZ(neutu::ECoordinateSystem::STACK);
    if (z < box.getMinZ()) {
      m_depthControl->setValueQuietly(0);
    } else if (z > box.getMaxZ()) {
      m_depthControl->setValueQuietly(box.getDepth() - 1);
    }

    m_sliceStrategy->setRange(
          m_depthControl->minimum(), m_depthControl->maximum());

    m_depthSpinBox->setRange(box.getMinCorner().getZ(),
                         box.getMaxCorner().getZ());
  }
}

void ZStackView::updateViewBox()
{
  resetViewProj();
  updateSlider();
//  updateImageCanvas();
//  updateObjectCanvas();
//  updateTileCanvas();
//  updateActiveDecorationCanvas();

  setSliceIndexQuietly(m_depthControl->maximum() / 2);
  processViewChange(true, true);
}

void ZStackView::updateStackWidget()
{
  LDEBUG() << "Updating stack widget";
  updateChannelControl();
  updateThresholdSlider();
  updateSlider();
  updateStackInfo();
}

void ZStackView::updateStackInfo()
{
  ZStack *stack = stackData();
  if (stack != NULL) {
    setStackInfo(stack->getBoundBox().toString().c_str());
  } else {
    setStackInfo("");
  }
}


void ZStackView::updateChannelControl()
{  
  QLayoutItem *child;
  while ((child = m_channelControlLayout->takeAt(0)) != 0) {
    if (child->widget()) {
      m_channelControlLayout->removeWidget(child->widget());
      delete child->widget();
    }
    delete child;
  }
  m_chVisibleState.clear();
  m_depthSpinBox->setVisible(false);
  ZStack *stack = stackData();
  if (stack != NULL) {
    if (getDepth() > 1 && getSliceAxis() != neutu::EAxis::ARB) {
      m_depthSpinBox->setVisible(true);
    }

    if (!stack->isVirtual()) {
      std::vector<ZVec3Parameter*>& channelColors = stack->channelColors();
      for (int i=0; i<stack->channelNumber(); ++i) {
        m_chVisibleState.push_back(new ZBoolParameter("", true, this));
      }

      for (int ch=0; ch<stack->channelNumber(); ++ch) {
        QWidget *checkWidget = m_chVisibleState[ch]->createWidget();
        checkWidget->setFocusPolicy(Qt::NoFocus);
        if (buddyDocument()->getTag() != neutu::Document::ETag::FLYEM_ORTHO) {
          m_channelControlLayout->addWidget(checkWidget, 0, Qt::AlignLeft);
          m_channelControlLayout->addWidget(
                channelColors[ch]->createNameLabel(),0,Qt::AlignLeft);
        }
        ZClickableColorLabel *colorWidget = qobject_cast<ZClickableColorLabel*>
            (channelColors[ch]->createWidget());
        colorWidget->setMinimumHeight(20);
        colorWidget->setMinimumWidth(30);
        colorWidget->setMaximumHeight(20);
        colorWidget->setMaximumWidth(30);
        colorWidget->setFocusPolicy(Qt::NoFocus);
        if (buddyDocument()->getTag() != neutu::Document::ETag::FLYEM_ORTHO) {
          m_channelControlLayout->addWidget(colorWidget,0,Qt::AlignLeft);
          m_channelControlLayout->addSpacing(20);
        }

        connect(channelColors[ch], SIGNAL(valueChanged()),
                this, SLOT(redraw()));
        connect(m_chVisibleState[ch], SIGNAL(valueChanged()),
                this, SLOT(redraw()));
      }
      if (!m_channelControlLayout->isEmpty()) {
        m_channelControlLayout->addStretch(1);
      }
    }
  }
}

void ZStackView::autoThreshold()
{
#ifdef _ADVANCED_
  buddyPresenter()->autoThreshold();
#endif
}

#define MULTI_THREAD_VIEW_SIZE_THRESHOLD 65536

QImage::Format ZStackView::stackKindToImageFormat(int kind)
{
  switch (kind) {
  case GREY:
    return QImage::Format_Indexed8;
  case GREY16:
    return QImage::Format_RGB16;
  case COLOR:
    return QImage::Format_RGB888;
  default:
    return QImage::Format_Invalid;
  }
}

ZStack* ZStackView::stackData() const
{
  return (buddyDocument()) ? buddyDocument()->getStack() : NULL;
}

int ZStackView::maxSliceIndex()
{
  return m_depthControl->maximum();
}

int ZStackView::sliceIndex() const
{
  return m_depthControl->value();
}

int ZStackView::getZ0() const
{
  return getStackOffset().getZ();
}

ZIntPoint ZStackView::getStackOffset() const
{
  return ZStackDocUtil::GetStackSpaceRange(
        *buddyDocument(), getSliceAxis()).getMinCorner();
}

int ZStackView::getCurrentDepth() const
{
  return sliceIndex();
//  return sliceIndex() + getZ0();
}

void ZStackView::setDepth(int z)
{
  setSliceIndex(z);
  /*
  if (z != getZ(neutu::ECoordinateSystem::STACK)) {
    setSliceIndex(z - getZ0());
  }
  */
}

/*
void ZStackView::setZQuitely(int z)
{
  setSliceIndexQuietly(z - getZ0());
}
*/

void ZStackView::setSliceIndex(int slice)
{
  if (!isDepthFronzen()) {
//    KINFO(neutu::TOPIC_NULL) << QString("Set slice index: %1").arg(slice);

    recordViewParam();
//    setDepthFrozen(true);
    m_depthControl->setValue(slice);
//    updateSliceViewParam();
  }

  //emit viewChanged(getViewParameter(NeuTube::COORD_STACK));
}

void ZStackView::setSliceIndexQuietly(int slice)
{
  if (!isDepthFronzen()) {
//    KINFO(neutu::TOPIC_NULL) << QString("Set slice index: %1").arg(slice);

    recordViewParam();
    m_depthControl->setValueQuietly(slice);
    m_depthSpinBox->setValueQuietly(slice);
//    updateSliceViewParam();
  }
}

void ZStackView::stepSlice(int step)
{
  setSliceIndex(sliceIndex() + step);
  /*
  int newIndex = sliceIndex() + step;
  if (newIndex < minSliceIndex()) {
    newIndex = minSliceIndex();
  } else if (newIndex > maxSliceIndex()) {
    newIndex = maxSliceIndex();
  }

  if (newIndex != sliceIndex()) {
    setSliceIndex(newIndex);
  }
  */
}

int ZStackView::getIntensityThreshold()
{
  int threshold = -1;
  if (m_thresholdSlider != NULL) {
    if (m_thresholdSlider->value() < m_thresholdSlider->maximum()) {
      threshold = m_thresholdSlider->value();
    }
  }

  return threshold;
}

/*
void ZStackView::updatePaintBundle(bool requestingWidgetCanvasUpdate)
{
//  m_paintBundle.unsetSwcNodeList();
  m_paintBundle.clearAllDrawableLists();
  if (buddyDocument()) {
    m_paintBundle.setStackOffset(getStackOffset());
  }

  int slice = m_depthControl->value();
  if (buddyPresenter()->interactiveContext().isObjectProjectView()) {
    slice = -slice - 1;
  }
  m_paintBundle.setSliceViewTransform(getSliceViewTransform());
//  m_paintBundle.setViewParam(getViewParameter());
//  m_paintBundle.setSliceIndex(slice);
  m_paintBundle.setDisplayStyle(buddyPresenter()->objectStyle());
  m_paintBundle.setDisplaySliceMode(buddyPresenter()->getSliceMode());
//  m_paintBundle.setDisplaySliceMode(buddyPresenter()->getS)

  m_paintBundle.clearAllDrawableLists();
  // obj
  if (buddyPresenter()->isObjectVisible()) {
    if (buddyDocument()->hasDrawable()) {
      m_paintBundle.addDrawableList(*buddyDocument()->drawableList());
    }

    if (buddyPresenter()->hasObjectToShow()) {
      m_paintBundle.addDrawableList(*buddyPresenter()->decorations());
    }
  }

  if (buddyPresenter()->hightlightOn()) {
    m_paintBundle.addDrawableList(buddyPresenter()->getHighlightDecorationList());
  }

  // active deco
  m_paintBundle.addDrawableList(buddyPresenter()->getActiveDecorationList());
  if (requestingWidgetCanvasUpdate) {
    requestWidgetCanvasUpdate();
  }
}
*/

bool ZStackView::restoreFromBadView()
{
  return imageWidget()->restoreFromBadView(buddyDocument()->getDataRange());
}

void ZStackView::refreshScreen(const RefreshConfig &config)
{
  if (config.updateOption != EUpdateOption::NONE) {
//    updatePaintBundle(config.widgetCanvasUpdateRequired);
    ++m_frameCount;
#ifdef _DEBUG_0
    HLDEBUG_FUNC("2D View") << neutu::EnumValue(config.updateOption) << " v"
                            << getViewId() << " " << getFrameCount() << std::endl;
#endif

    bool blockingPaint = m_isRedrawBlocked || !buddyDocument()->isReadyForPaint();


    m_imageWidget->blockPaint(blockingPaint);

    ZOUT(LTRACE(), 5) << "Updating image widget" << m_imageWidget->screenSize();

    switch (config.updateOption) {
    case EUpdateOption::QUEUED:
      m_imageWidget->update();
      break;
    case EUpdateOption::DIRECT:
      m_imageWidget->repaint();
      break;
    default:
      break;
    }

//    tryAutoRecord();
  }
}

void ZStackView::processSceneUpdate()
{
#ifdef _DEBUG_0
  HLDEBUG_FUNC_LN("2D View");
#endif
  tryAutoRecord();
}

void ZStackView::updateImageScreen(EUpdateOption option)
{
//  HLDEBUG_FUNC_LN("2D View");

  RefreshConfig config;
  config.updateOption = option;
  refreshScreen(config);
}

QSize ZStackView::sizeHint() const
{
  QSize viewSize = QWidget::sizeHint();

  switch (m_sizeHintOption) {
  case neutu::ESizeHintOption::CURRENT_BEST:
    //m_imageWidget->updateGeometry();
    viewSize = QWidget::sizeHint();
    break;
  case neutu::ESizeHintOption::TAKING_SPACE:
    viewSize = QWidget::sizeHint();
    break;
  default:
    break;
  }

  if (!viewSize.isValid()) {
    viewSize = minimumSizeHint();
  }

  return viewSize;
  //return m_layout->minimumSize();
  /*
  return m_layout->sizeHint() + QSize(
        m_layout->contentsMargins().right() - m_layout->contentsMargins().left(),
        m_layout->contentsMargins().bottom() - m_layout->contentsMargins().top());*/
}

void ZStackView::mouseReleasedInImageWidget(QMouseEvent *event)
{
  buddyPresenter()->processMouseReleaseEvent(event, getViewId());
}

void ZStackView::mousePressedInImageWidget(QMouseEvent *event)
{
  buddyPresenter()->processMousePressEvent(event, getViewId());
}

void ZStackView::mouseMovedInImageWidget(QMouseEvent *event)
{
  buddyPresenter()->processMouseMoveEvent(event, getViewId());
}

void ZStackView::mouseDoubleClickedInImageWidget(QMouseEvent *event)
{
  buddyPresenter()->processMouseDoubleClickEvent(event, getViewId());
}

bool ZStackView::isDepthScrollable()
{
  return m_depthControl->isEnabled();
}

void ZStackView::ignoreScroll(bool on)
{
  m_ignoringScroll = on;
}

void ZStackView::pauseScroll()
{
  m_ignoringScroll = true;
  m_scrollPausedTime = neutu::GetTimeStamp();
}

void ZStackView::mouseRolledInImageWidget(QWheelEvent *event)
{
  if (m_scrollPausedTime > 0) {
    if (neutu::GetTimeStamp() - m_scrollPausedTime >
        NeutubeConfig::GetScrollCooldown()) {
      m_ignoringScroll = false;
      m_scrollPausedTime = 0;
    } else {
#ifdef _DEBUG_
      std::cout << "Mouse scroll ignored: " << neutu::GetTimeStamp() << " - " << m_scrollPausedTime << std::endl;
#endif
    }
  }

  if (m_ignoringScroll) {
    return;
  }

  int numSteps = -event->delta();

#ifdef _DEBUG_0
  std::cout << "Event time: " << event->timestamp() << std::endl;
  std::cout << "Time to event: " << QDateTime::currentMSecsSinceEpoch() << std::endl;
#endif

#if defined(__APPLE__)
  switch (QSysInfo::MacintoshVersion) {
  case QSysInfo::MV_10_5:
  case QSysInfo::MV_10_6:
#if (QT_VERSION >= QT_VERSION_CHECK(4, 8, 0))
  case QSysInfo::MV_10_7:
#endif
#if (QT_VERSION > QT_VERSION_CHECK(4, 8, 1))
  case QSysInfo::MV_10_8:
#endif
    numSteps = -numSteps;
    break;
  default:
    break;
  }
#endif

//  numSteps = -numSteps;

  if ((abs(numSteps) > 0) && (abs(numSteps) < 120)) {
    if (numSteps > 0) {
      numSteps = 1;
    } else {
      numSteps = -1;
    }
  } else {
    numSteps /= 120;
  }

  QElapsedTimer timer;
  timer.start();
  if (event->modifiers() == Qt::NoModifier ||
      event->modifiers() == Qt::ShiftModifier) {
    if (isDepthScrollable()) {
      setAttribute(Qt::WA_TransparentForMouseEvents);
      if (numSteps != 0) {
        int ratio = 1;
        if (event->modifiers() == Qt::ShiftModifier) {
          ratio = 10;
        }

        int step = numSteps * ratio;

        int zoomLevel = getViewParameter().getZoomLevel();

        imageWidget()->moveCutDepth(
              pow(2, zoomLevel) * step, neutu::ESignalControl::BROADCASTING);
        updateDataInfo(event->pos());
      }
      setAttribute(Qt::WA_TransparentForMouseEvents, false);
    }
  } else if (event->modifiers() == Qt::ControlModifier) {
    if (numSteps < 0) {
      increaseZoomRatio(event->pos().x(), event->pos().y());
    } else if (numSteps > 0) {
      decreaseZoomRatio(event->pos().x(), event->pos().y());
    }
  }

  auto delay = timer.elapsed();
  if (!NeutubeConfig::AdatpiveScrollCooldown() || delay > 100) {
    pauseScroll();
  }
}

#if 0
void ZStackView::resizeEvent(QResizeEvent *event)
{
#if 0
  LDEBUG() << "ZStackView::resizeEvent:" << size() << isVisible();
  setInfo();
  event->accept();

  if (isVisible()) {
    imageWidget()->adjustTransformWithResize();
//    m_mainTransform.canvasAdjust(imageWidget()->width(), imageWidget()->height(), m_viewA)

#if 0
    if (getSliceAxis() == neutu::EAxis::ARB) {
      //Adjust center
      QPointF stackPos = ZPositionMapper::WidgetToStack(
            imageWidget()->width() / 2, imageWidget()->height() / 2,
            getViewProj());
      LDEBUG() << "Stack center:" << stackPos.x() << stackPos.y();
      //    ZPoint dataPos = ZPositionMapper::StackToData(stackPos, getAffinePlane());
      //    m_sliceViewParam.setCenter(dataPos.toIntPoint());

#if 0
      ZStackViewParam param = getViewParameter(m_sliceViewParam);
      if (m_imageWidget->getViewProj() != param.getViewProj()) {
        m_imageWidget->setViewProj(param.getViewProj());
      }

      recordViewParam();
#endif


      m_depthControl->setValueQuietly(param.getZ() - getZ0());
      m_zSpinBox->setValueQuietly(m_depthControl->value());

//      updateViewParam(m_sliceViewParam);

      LDEBUG() << "Slice center:" << m_sliceViewParam.getCenter().toString();
    }
#endif
    updatePaintBundle();

    processViewChange(true, false);

  }
#endif
//  updateActiveDecorationCanvas();
//  updateTileCanvas();
  //buddyPresenter()->updateInteractiveContext();
}
#endif

void ZStackView::showEvent(QShowEvent */*event*/)
{
  LDEBUG() << "ZStackView::showEvent:" << size() << isVisible();

  if (isVisible()) {
    updateViewData();
    redraw();
  }
//  resetViewProj();
}

void ZStackView::updateStackRange()
{
  LDEBUG() << "Updating stack range";

  ZIntCuboid modelRange = buddyDocument()->getDataRange();
  if (modelRange != getCurrentStackRange()) {
    bool isOriginallyEmpty = getCurrentStackRange().isEmpty();
    imageWidget()->setModelRange(modelRange);
    if (isOriginallyEmpty) {
      imageWidget()->resetView();
    } else {
      if (modelRange.contains(imageWidget()->getCutCenter().roundToIntPoint())) {
        syncTransformControl();
      } else {
        imageWidget()->setCutCenter(
              modelRange.getCenter(), neutu::ESignalControl::BROADCASTING);
      }
    }
  }

  /*
  ZIntCuboid stackRange = getViewBoundBox();
  if (stackRange != getCurrentStackRange()) {
//    if (stackRange.getWidth() != getCurrentStackRange().getWidth() ||
//        stackRange.getHeight() != getCurrentStackRange().getHeight() ||
//        stackRange.getDepth() != getCurrentStackRange().getDepth()) {
      resetViewProj();
      updateSlider();
      setSliceIndexQuietly(m_depthControl->maximum() / 2);
//    }


//    setSliceIndexQuietly(m_depthControl->maximum() / 2);
    m_currentStackRange = stackRange;
    imageWidget()->setModelRange(stackRange);

//    updateObjectCanvas();
//    updateTileCanvas();
//    updateActiveDecorationCanvas();
//    updateImageCanvas();
  }
  */
}

void ZStackView::processStackChange(bool rangeChanged)
{
  LDEBUG() << "Processing stack change";

  if (rangeChanged) {
    updateStackRange();
    updateChannelControl();
  } else {
    paintStackBuffer();
    updateImageScreen(EUpdateOption::QUEUED);
  }
//  processViewChange(true, true);
}

void ZStackView::redrawObject()
{
  updateObjectBuffer(neutu::data3d::GetTargetSettled2dObjectCanvasList());
//  paintObjectBuffer();
//  paintDynamicObjectBuffer();
  updateImageScreen(EUpdateOption::QUEUED);
}

void ZStackView::redraw(EUpdateOption option)
{
//  tic();
  QElapsedTimer timer;
//  ZBenchTimer timer;
  timer.start();

  paintStackBuffer();
//  qint64 stackPaintTime = timer.elapsed();
//  ZOUT(LTRACE(), 5) << "paint stack per frame: " << stackPaintTime;
  paintMaskBuffer();

  updateObjectSorter();
  m_paintSorter.forEachVisibleTarget(
        neutu::data3d::GetTarget2dObjectCanvasList(),
        [this](neutu::data3d::ETarget target, const QList<ZStackObject*> &objList) {
          updateObjectBuffer(target, objList); },
        ZStackObjectPaintSorter::SortingZ(true));

  updateImageScreen(option);

//  timer.stop();
//  std::cout << "Paint time per frame: " << timer.time() * 1000 << " ms" << std::endl;
//  std::cout << "paint time per frame: " << toc() << std::endl;

  qint64 paintTime = timer.elapsed();

  ZOUT(KLog(VIEW_TOPIC), 5) << ZLog::Profile()
                  << ZLog::Description("paint time per frame")
                  << ZLog::Duration(paintTime);
}


void ZStackView::prepareDocument()
{
  startWorkThread();
  updateStackRange();
//  updateSlider();
//  m_objectUpdater.setDocument(buddyDocument());
}

QMenu* ZStackView::leftMenu()
{
  return m_imageWidget->leftMenu();
}

QMenu* ZStackView::rightMenu()
{
  return m_imageWidget->rightMenu();
}

bool ZStackView::popLeftMenu(const QPoint &pos)
{
  return m_imageWidget->popLeftMenu(pos);
}

bool ZStackView::popRightMenu(const QPoint &pos)
{
  return m_imageWidget->popRightMenu(pos);
}

bool ZStackView::showContextMenu(QMenu *menu, const QPoint &pos)
{
  return m_imageWidget->showContextMenu(menu, pos);
}

QStringList ZStackView::toStringList() const
{
  QStringList list;

  list.append(QString("screen size: %1 x %2")
              .arg(imageWidget()->screenSize().width())
              .arg(imageWidget()->screenSize().height()));

  list.append(QString::fromStdString(
                neutu::ToString(imageWidget()->getSliceViewTransform())));
  /*
  list.append(QString("Offset: %1, %2")
              .arg(imageWidget()->viewPort().left())
              .arg(imageWidget()->viewPort().top()));
              */

  return list;
}

/*
void ZStackView::setImageWidgetCursor(const QCursor & cursor)
{
  imageWidget()->setCursor(cursor);
}
*/
void ZStackView::setScreenCursor(const QCursor &cursor)
{
  imageWidget()->setCursor(cursor);
}

void ZStackView::setScrollStrategy(ZScrollSliceStrategy *strategy)
{
  delete m_sliceStrategy;

  m_sliceStrategy = strategy;
  m_sliceStrategy->setRange(
        m_depthControl->minimum(), m_depthControl->maximum());
}

/*
void ZStackView::resetScreenCursor()
{
  imageWidget()->setCursor(Qt::CrossCursor);
}
*/
void ZStackView::setThreshold(int thre)
{
  if (m_thresholdSlider != NULL) {
    m_thresholdSlider->setValue(thre);
  }
}

void ZStackView::takeScreenshot()
{
  if (getRecorder()->getPrefix().isEmpty()) {
    configureRecorder();
  }

  getRecorder()->takeShot(this);
}

void ZStackView::takeScreenshot(const QString &filename)
{
  m_imageWidget->takeScreenshot(filename);
}

//void ZStackView::updateView()
//{
//  redraw(UPDATE_QUEUED);
//}

void ZStackView::displayActiveDecoration(bool display)
{
  m_activeLabel->setVisible(display);
}

void ZStackView::paintSingleChannelStackSlice(ZStack *stack, int slice)
{
  switch (getSliceAxis()) {
  case neutu::EAxis::Z:
  case neutu::EAxis::ARB:
  {
    void *dataArray = stack->getDataPointer(0, slice);

    switch (stack->kind()) {
    case GREY:
      if (stack->isBinary()) {
        m_image->setBinaryData(static_cast<uint8_t*>(dataArray),
                               (uint8_t) (stack->min()), getIntensityThreshold());
      } else {
        ZImage::DataSource<uint8_t> stackData(static_cast<uint8_t*>(dataArray),
                                              buddyPresenter()->greyScale(0),
                                              buddyPresenter()->greyOffset(0),
                                              stack->getChannelColor(0));
        m_image->useContrastProtocal(buddyPresenter()->usingHighContrastProtocal());
        m_image->setData(stackData, getIntensityThreshold());
      }
      break;
    case GREY16:
      if (stack->isBinary()) {
        m_image->setBinaryData(static_cast<uint16_t*>(dataArray),
                               (uint16) (stack->min()), getIntensityThreshold());
      } else {
        ZImage::DataSource<uint16_t> stackData(static_cast<uint16_t*>(dataArray),
                                               buddyPresenter()->greyScale(0),
                                               buddyPresenter()->greyOffset(0),
                                               stack->getChannelColor(0));
        m_image->setData(stackData, getIntensityThreshold());
      }
      break;
    default:
      break;
    }
  }
    break;
  case neutu::EAxis::X:
  case neutu::EAxis::Y:
    switch (stack->kind()) {
    case GREY:
      m_image->setData(
            stack->array8(), stack->width(), stack->height(), stack->depth(),
            slice, buddyPresenter()->getGrayScale(),
            buddyPresenter()->getGrayOffset(), getSliceAxis());
      m_image->enhanceContrast(buddyPresenter()->usingHighContrastProtocal());
      break;
    default:
      break;
    }
    break;
  }
}

void ZStackView::paintMultipleChannelStackSlice(ZStack *stack, int slice)
{
  bool usingMt = false;

  if (stack->width() * stack->height() > MULTI_THREAD_VIEW_SIZE_THRESHOLD) {
    usingMt = true;
  }

  switch (stack->kind()) {
  case GREY: {
    std::vector<ZImage::DataSource<uint8_t> > stackData8;
    for (size_t i=0; i<m_chVisibleState.size(); ++i) {
      if (m_chVisibleState[i]->get()) {
        stackData8.push_back(
              ZImage::DataSource<uint8_t>(
                static_cast<uint8_t*>(stack->getDataPointer(i, slice)),
                buddyPresenter()->greyScale(i),
                buddyPresenter()->greyOffset(i),
                stack->getChannelColor(i)));
#ifdef _DEBUG_
        std::cout <<  stack->getChannelColor(i) << std::endl;
#endif
      }
    }
    m_image->setData(stackData8, 255, usingMt);
  }
    break;
  case GREY16: {
    std::vector<ZImage::DataSource<uint16_t> > stackData16;
    for (size_t i=0; i<m_chVisibleState.size(); ++i) {
      if (m_chVisibleState[i]->get()) {
        stackData16.push_back(
              ZImage::DataSource<uint16_t>(
                static_cast<uint16_t*>(stack->getDataPointer(i, slice)),
                buddyPresenter()->greyScale(i),
                buddyPresenter()->greyOffset(i),
                stack->getChannelColor(i)));
      }
    }
    m_image->setData(stackData16, 255, usingMt);
  }
    break;
  default:
    break;
  }
}

void ZStackView::paintSingleChannelStackMip(ZStack *stack)
{
  Image_Array ima;
  ima.array = (uint8*) stack->projection(
        buddyDocument()->getStackBackground(), ZSingleChannelStack::Z_AXIS);

  switch (stack->kind()) {
  case GREY:
    if (stack->isBinary()) {
      m_image->setBinaryData(ima.array8, (uint8) (stack->min()),
                             getIntensityThreshold());
    } else {
      ZImage::DataSource<uint8_t> stackData(ima.array8,
                                            buddyPresenter()->greyScale(0),
                                            buddyPresenter()->greyOffset(0),
                                            stack->getChannelColor(0));
      m_image->setData(stackData, getIntensityThreshold());
    }
    break;
  case GREY16:
    if (stack->isBinary()) {
      m_image->setBinaryData(ima.array16, (uint16) (stack->min()),
                             getIntensityThreshold());
    } else {
      ZImage::DataSource<uint16_t> stackData(ima.array16,
                                             buddyPresenter()->greyScale(0),
                                             buddyPresenter()->greyOffset(0),
                                             stack->getChannelColor(0));
      m_image->setData(stackData, getIntensityThreshold());
    }
    break;
  }
}

void ZStackView::paintMultipleChannelStackMip(ZStack *stack)
{
  bool usingMt = false;

  if (stack->width() * stack->height() > MULTI_THREAD_VIEW_SIZE_THRESHOLD) {
    usingMt = true;
  }
  switch (stack->kind()) {
  case GREY: {
    std::vector<ZImage::DataSource<uint8_t> > stackData8;
    for (size_t i=0; i<m_chVisibleState.size(); ++i) {
      if (m_chVisibleState[i]->get()) {
        Image_Array ima;
        ima.array8 = (uint8*) stack->projection(
              buddyDocument()->getStackBackground(), ZSingleChannelStack::Z_AXIS, i);
        stackData8.push_back(
              ZImage::DataSource<uint8_t>(ima.array8,
                                          buddyPresenter()->greyScale(i),
                                          buddyPresenter()->greyOffset(i),
                                          stack->getChannelColor(i)));
      }
    }
    m_image->setData(stackData8, 255, usingMt);
  }
    break;
  case GREY16: {
    std::vector<ZImage::DataSource<uint16_t> > stackData16;
    for (size_t i=0; i<m_chVisibleState.size(); ++i) {
      if (m_chVisibleState[i]->get()) {
        Image_Array ima;
        ima.array16 = (uint16*) stack->projection(
              buddyDocument()->getStackBackground(), ZSingleChannelStack::Z_AXIS, i);
        stackData16.push_back(ZImage::DataSource<uint16_t>(ima.array16,
                                                           buddyPresenter()->greyScale(i),
                                                           buddyPresenter()->greyOffset(i),
                                                           stack->getChannelColor(i)));
      }
    }
    m_image->setData(stackData16, 255, usingMt);
  }
    break;
  }
}

void ZStackView::clearCanvas()
{
//  m_imagePainter.end();
  delete m_image;
  m_image = NULL;

  m_imageWidget->setImage(NULL);
}

/*
template<typename T>
void ZStackView::resetCanvasWithStack(T &canvas, ZPainter *painter)
{
  if (canvas != NULL) {
    ZIntCuboid box = getViewBoundBox();
    if (canvas->width() != box.getWidth() ||
        canvas->height() != box.getHeight() ||
        neutu::iround(canvas->getTransform().getTx()) !=
        -box.getMinCorner().getX() ||
        neutu::iround(canvas->getTransform().getTy()) !=
        -box.getMinCorner().getY()) {
      if (painter != NULL) {
        painter->end();
      }
      m_imageWidget->removeCanvas(canvas);
      delete canvas;
      canvas = NULL;
    }
  }
}
*/

void ZStackView::updateContrastProtocal()
{
  if (m_image != NULL) {
    if (buddyPresenter()->hasHighContrastProtocal()) {
      m_image->loadContrastProtocal(
            buddyPresenter()->getHighContrastProtocal());
    } else {
      m_image->setDefaultContrastProtocal();
    }
  }
}

ZPoint ZStackView::getAnchorPoint(neutu::data3d::ESpace space) const
{
  return imageWidget()->getAnchorPoint(space);
}

#if 0
void ZStackView::updateImageCanvas()
{
  resetCanvasWithStack(m_image, NULL);
  if (buddyDocument()->hasStackPaint()) {
    ZIntCuboid box = getViewBoundBox();
    if (m_image != NULL) {
      m_image->setOffset(-box.getMinCorner().getX(),
                         -box.getMinCorner().getY());
      if ((m_image->width() != box.getWidth()) ||
          (m_image->height() != box.getHeight())) {
        clearCanvas();
      }
    }

    if (m_image == NULL) {
//      double scale = 0.5;
      if (buddyDocument()->hasStackData() &&
          buddyDocument()->getStack()->kind() == GREY &&
          buddyDocument()->getStack()->channelNumber() == 1) {
        m_image = new ZImage(
              box.getWidth(), box.getHeight(), QImage::Format_Indexed8);
      } else {
        m_image = new ZImage(box.getWidth(), box.getHeight());
      }

      updateContrastProtocal();

      m_image->setOffset(-box.getMinCorner().getX(),
                         -box.getMinCorner().getY());
//      m_image->setScale(scale, scale);
//      m_imagePainter.begin(m_image);
//      m_imagePainter.setZOffset(box.getFirstCorner().getZ());
      m_imageWidget->setImage(m_image);
    }
  }
}

void ZStackView::updateMaskCanvas()
{
  ZStack *stackMask = buddyDocument()->stackMask();

  if (stackMask == NULL) {
    return;
  }

  if (m_imageMask != NULL) {
    if (m_imageMask->width() != m_image->width() ||
        m_imageMask->height() != m_image->height()) {
      delete m_imageMask;
      m_imageMask = NULL;
    }
  }
  if (m_imageMask == NULL) {
    m_imageMask = m_image->createMask();
    m_imageWidget->setMask(m_imageMask, 0);
  }
}

void ZStackView::clearObjectCanvas()
{
//  m_objectCanvasPainter.end();

//  m_objectCanvas.clear();
  delete m_objectCanvas;
  m_objectCanvas = NULL;
  m_imageWidget->setObjectCanvas(NULL);

#if 0
  m_objectCanvasPainter.end();
  delete m_objectCanvas;
  m_objectCanvas = NULL;

  m_imageWidget->setObjectCanvas(NULL);
#endif
}

void ZStackView::clearTileCanvas()
{
  delete m_tileCanvas;
  m_tileCanvas = NULL;

//  m_tileCanvasPainter.end();

//  m_tileCanvas.clear();
  m_imageWidget->setTileCanvas(NULL);
}
#endif

/*
QSize ZStackView::getCanvasSize() const
{
  QSize size(0, 0);
  if (buddyDocument()->hasStack()) {
    ZIntCuboid box = getViewBoundBox();
    size.setWidth(box.getWidth());
    size.setHeight(box.getHeight());
  }

  return size;
}
*/

QSize ZStackView::getScreenSize() const
{
  return m_imageWidget->size();
}
#if 0
void ZStackView::resetCanvasWithStack(
    ZMultiscalePixmap &canvas, ZPainter *painter)
{
  QSize canvasSize = getCanvasSize();

  ZIntCuboid box = getViewBoundBox();

  int tx = -box.getMinCorner().getX();
  int ty = -box.getMinCorner().getY();

  if (canvas.getWidth() != canvasSize.width() ||
      canvas.getHeight() != canvasSize.height() ||
      canvas.getTx() != tx || canvas.getTy() != ty) {
    clearTileCanvas();
    clearObjectCanvas();
    canvas.setSize(canvasSize);
    canvas.setOffset(QPoint(tx, ty));
    if (painter != NULL) {
      painter->setZOffset(box.getMinCorner().getZ());
    }
  }
}

bool ZStackView::reloadObjectCanvas(bool repaint)
{
  bool reloaded = false;

  QSize canvasSize = getCanvasSize();

  if (!canvasSize.isEmpty() &&
      (buddyDocument()->hasDrawable(neutu::data3d::ETarget::TARGET_OBJECT_CANVAS) ||
      buddyPresenter()->hasDrawable(neutu::data3d::ETarget::TARGET_OBJECT_CANVAS))) {
    double zoomRatio = getProjZoomRatio();
    int level = 0;
    if (zoomRatio < 0.5 && zoomRatio > 0) {
      level = (int) std::floor(1.0 / zoomRatio - 1);
    }
//    level  = 0;
    ZPixmap *pixmap = m_objectCanvas.getPixmap(level);
    m_imageWidget->setObjectCanvas(pixmap);

//    pixmap->cleanUp();
    if (static_cast<QPaintDevice*>(pixmap) != m_objectCanvasPainter.device()) {
      m_objectCanvasPainter.end();

      reloaded = true;

      m_objectCanvasPainter.begin(pixmap);
      m_objectCanvasPainter.setCompositionMode(
            QPainter::CompositionMode_SourceOver);
    }

    TZ_ASSERT(pixmap == m_imageWidget->getObjectCanvas(), "Invalid pixmap");

    if (reloaded && repaint) {
      pixmap->cleanUp();
      paintObjectBuffer();
    }
  } else {
    m_objectCanvas.setVisible(false);
  }

  return reloaded;
}
#endif

#if 0
void ZStackView::reloadTileCanvas()
{
#ifdef _DEBUG_
  std::cout << "Before: Tile painter active? " << m_tileCanvasPainter.isActive() << std::endl;
#endif

  QSize canvasSize = getCanvasSize();

  if (!canvasSize.isEmpty() &&
      buddyDocument()->hasDrawable(neutu::data3d::ETarget::TARGET_TILE_CANVAS)) {
    double zoomRatio = getProjZoomRatio();
    int level = 1;
    if (zoomRatio > 0) {
      level = (int) std::floor(1.0 / zoomRatio);
    }
    ZPixmap *pixmap = m_tileCanvas.getPixmap(level);

    if (static_cast<QPaintDevice*>(pixmap) != m_tileCanvasPainter.device()) {
      m_tileCanvasPainter.end();
      m_tileCanvasPainter.begin(pixmap);
      m_imageWidget->setTileCanvas(pixmap);
    }
  }

#ifdef _DEBUG_
  std::cout << "After: Tile painter active? " << m_tileCanvasPainter.isActive() << std::endl;
#endif
}
#endif

#if 0
void ZStackView::updateObjectCanvas()
{
#ifdef _DEBUG_2
  std::cout << "Updating object canvas." << std::endl;
#endif

  updateCanvas(m_objectCanvas);
//  m_objectCanvas = updateProjCanvas(m_objectCanvas, &m_objectCanvasPainter);
//  m_imageWidget->setObjectCanvas(m_objectCanvas);

#if 0
  resetCanvasWithStack(m_objectCanvas, &m_objectCanvasPainter);
  reloadObjectCanvas();
//#if 0
  ZPixmap *canvas = getCanvas(neutu::data3d::ETarget::TARGET_OBJECT_CANVAS);
  if (canvas != NULL) {
    m_objectCanvasPainter.end();
    canvas->cleanUp();
    m_objectCanvasPainter.begin(canvas);
  }
//#endif
#endif
}


void ZStackView::updateTileCanvas()
{
  resetCanvasWithStack(m_tileCanvas, &m_tileCanvasPainter);
  reloadTileCanvas();
}
#endif

/*
ZPixmap *ZStackView::updateViewPortCanvas(ZPixmap *canvas)
{
  ZStTransform transform = getViewTransform();

  QRect viewPort = getViewPort(neutu::ECoordinateSystem::STACK);
  QSize viewPortSize = viewPort.size();
  QSize newSize = viewPortSize;

  if (canvas != NULL) {
    if (canvas->size() != newSize) {
      delete canvas;
      canvas = NULL;
    }
  }

  if (canvas == NULL) {
    canvas = new ZPixmap(newSize);
  }

  canvas->getProjTransform().estimate(
        QRectF(QPointF(0, 0), QSizeF(viewPortSize)), getProjRegion());
  transform.setScale(1.0, 1.0);
  transform.setOffset(-viewPort.left(), -viewPort.top());

  canvas->setTransform(transform);

  if (canvas != NULL) {
    if (canvas->isVisible()){
      canvas->cleanUp();
    }
  }

  return canvas;
}
*/

/*
ZSliceCanvas* ZStackView::updateCanvas(ZSliceCanvas *canvas)
{
  int width = imageWidget()->width();
  int height = imageWidget()->height();

  if (canvas) {
    canvas->resetCanvas(width, height, getSliceViewTransform());
  } else {
    canvas = new ZSliceCanvas(width, height);
    canvas->setTransform(getSliceViewTransform());
  }

  return canvas;
}
*/

void ZStackView::invalidateObjectSorter()
{
  m_paintSorter.clear();
}

void ZStackView::updateObjectSorter()
{
  if (m_paintSorter.isEmpty()) {
    if (buddyDocument()->hasDrawable()) {
      QList<ZStackObject*> *objs = buddyDocument()->drawableList();
      m_paintSorter.add(*objs);
    }
    m_paintSorter.add(*buddyPresenter()->decorations());
    m_paintSorter.add(buddyPresenter()->getActiveDecorationList());
  }
}

void ZStackView::updateObjectCanvasVisbility(bool visible)
{
  if (visible) {
    updateViewData();
  }

  std::vector<neutu::data3d::ETarget> targetList =
      neutu::data3d::GetTargetSettled2dObjectCanvasList();
  updateObjectBuffer(targetList);

  updateImageScreen(EUpdateOption::QUEUED);
}

ZPoint ZStackView::getCurrentMousePosition(neutu::data3d::ESpace space)
{
  return imageWidget()->getCurrentMousePosition(space);
}


#if 0
ZPixmap *ZStackView::updateProjCanvas(ZPixmap *canvas, ZPainter *painter)
{
  QRectF projRect = getProjRegion();
  QSize newCanvasSize = projRect.size().toSize();

  bool usingProjSize = true;

  QRect viewPort = getViewPort(neutu::ECoordinateSystem::STACK);

  //Get transform from viewport to projection region
  ZStTransform transform = getViewTransform();

  //When the projection region is not much smaller or even bigger than viewport,
  //use viewport instead for precise painting. It means that the canvas size
  //will be the same as the viewport size, and as a result, the scale is 1 for
  //painting world objects to the canvas.
  if (transform.getSx() > 1.1) {
    newCanvasSize = viewPort.size();
    usingProjSize = false;
  }

//  qDebug() << "  Canvas size" << newSize;

  if (canvas != NULL) {
    if (canvas->size() != newCanvasSize) {
      if (painter != NULL) {
        painter->end();
      }
      delete canvas;
      canvas = NULL;
    }
  }

  if (canvas == NULL) {
    canvas = new ZPixmap(newCanvasSize);
  }

  if (usingProjSize) {
    canvas->getProjTransform().setScale(1.0, 1.0);
    canvas->getProjTransform().setOffset(projRect.left(), projRect.top());
  } else {
    canvas->getProjTransform().estimate(
          QRectF(QPointF(0, 0), QSizeF(newCanvasSize)), getProjRegion());
    transform.setScale(1.0, 1.0);
    transform.setOffset(-viewPort.left(), -viewPort.top());
  }

  canvas->setTransform(transform);
  if (painter != NULL) {
    painter->updateTransform(canvas);
  }

  if (canvas->isVisible()){
    canvas->cleanUp();
  }

  return canvas;
}
#endif

#if 0
ZPixmap *ZStackView::updateProjCanvas(ZPixmap *canvas)
{
  ZStTransform transform = getViewTransform();

  QSize newSize = getProjRegion().size().toSize();

//  qDebug() << "  Canvas size" << newSize;

  if (canvas != NULL) {
    if (canvas->size() != newSize) {
      delete canvas;
      canvas = NULL;
    }
  }

  if (canvas == NULL) {
    canvas = new ZPixmap(newSize);
  }

  if (transform.getSx() > 1.1) {
    QRect viewPort = getViewPort(NeuTube::COORD_STACK);
    newSize = viewPort.size();
    canvas->getProjTransform().estimate(
          QRectF(QPointF(0, 0), QSizeF(newSize)), getProjRegion());
    transform.setScale(1.0, 1.0);
    transform.setOffset(-viewPort.left(), -viewPort.top());
  } else {
    canvas->getProjTransform().setScale(1.0, 1.0);
  }

  canvas->setTransform(transform);

  if (canvas != NULL) {
    if (canvas->isVisible()){
      canvas->cleanUp();
    }
  }

  return canvas;
}
#endif

#if 0
void ZStackView::updateDynamicObjectCanvas()
{
  updateCanvas(m_dynamicObjectCanvas);
#if 0
//  ZPixmap *newCanvas = updateViewPortCanvas(m_dynamicObjectCanvas);
  ZPixmap *newCanvas = updateProjCanvas(m_dynamicObjectCanvas, NULL);
  m_imageWidget->setDynamicObjectCanvas(newCanvas);

  if (m_dynamicObjectCanvas != newCanvas) {
    m_dynamicObjectCanvas = newCanvas;
    /* //doesn't work here. not sure why
    m_dynamicObjectCanvasPainter.end();
    if (m_dynamicObjectCanvas != NULL) {
      m_dynamicObjectCanvasPainter.begin(m_dynamicObjectCanvas);
    }
    */
  }
#endif
}

void ZStackView::updateActiveDecorationCanvas()
{
  updateCanvas(m_activeDecorationCanvas);
  /*
  m_activeDecorationCanvas = updateProjCanvas(m_activeDecorationCanvas, NULL);
  m_imageWidget->setActiveDecorationCanvas(m_activeDecorationCanvas);
  */
}

void ZStackView::updateTileCanvas()
{
#ifdef _DEBUG_2
  std::cout << "Updating tile canvas." << std::endl;
#endif

//  m_tileCanvasPainter.end();
  m_tileCanvas = updateProjCanvas(m_tileCanvas, &m_tileCanvasPainter);
  m_imageWidget->setTileCanvas(m_tileCanvas);
}

void ZStackView::prepareCanvasPainter(ZPixmap *canvas, ZPainter &canvasPainter)
{
  if (canvas == NULL) {
    canvasPainter.end();
  } else {
    if (!canvasPainter.isActive()) {
      canvasPainter.begin(canvas);
    } else {
      if (static_cast<QPaintDevice*>(canvas) != canvasPainter.device()) {
        canvasPainter.restart(canvas);
      }
    }
    canvasPainter.setZOffset(getZ0());
  }
}

ZPainter* ZStackView::getTileCanvasPainter()
{
  prepareCanvasPainter(m_tileCanvas, m_tileCanvasPainter);

  return &m_tileCanvasPainter;
}
#endif

/*
ZPainter* ZStackView::getObjectCanvasPainter()
{
  prepareCanvasPainter(m_objectCanvas, m_objectCanvasPainter);
  m_objectCanvasPainter.setRenderHint(QPainter::Antialiasing, false);

  return &m_objectCanvasPainter;
}
*/

void ZStackView::paintMultiresImageTest(int resLevel)
{
  ZStackPatch *patch = new ZStackPatch(
        ZStackFactory::makeSlice(*(buddyDocument()->getStack()), getCurrentDepth()));
  patch->setSource(ZStackObjectSourceFactory::MakeCurrentMsTileSource(resLevel));
  if (resLevel > 0) {
    patch->getStack()->downsampleMax(resLevel, resLevel, 0);
    patch->setXScale(resLevel + 1);
    patch->setYScale(resLevel + 1);
  }
  buddyDocument()->blockSignals(true);
  buddyDocument()->addObject(patch);
  buddyDocument()->blockSignals(false);
  //paintObjectBuffer();

  if (resLevel > 0) {
    paintMultiresImageTest(resLevel - 1);
    //QtConcurrent::run(this, &ZStackView::paintMultiresImageTest, resLevel - 1);
  }
}

void ZStackView::paintStackBuffer()
{
  ZStack *stack = stackData();
  if (stack) {
    if (stack->hasData()) {
      QSizeF viewportSize = imageWidget()->getViewportSize();
      if (viewportSize.width() > 0 && viewportSize.height() > 0) {
        auto canvas = getClearCanvas(neutu::data3d::ETarget::STACK_CANVAS);
//            imageWidget()->getValidCanvas(ZImageWidget::CANVAS_ROLE_IMAGE);
        ZImageSliceFactory::Make(
              *stack,
              imageWidget()->getSliceViewTransform().getModelViewTransform(),
              viewportSize.width(), viewportSize.height(), canvas);
#ifdef _DEBUG_2
        std::cout << "Saving image canvas ..." << std::endl;
        canvas->save(GET_TEST_DATA_DIR + "/_test.png");
#endif
      }
    }
  }

  if (buddyDocument()->getTag() == neutu::Document::ETag::NORMAL) {
    ZSparseStack *spStack = buddyDocument()->getSparseStack();
    if (spStack) {
      QSizeF viewportSize = imageWidget()->getViewportSize();
      if (viewportSize.width() > 0 && viewportSize.height() > 0) {
        auto canvas = getClearCanvas(neutu::data3d::ETarget::STACK_CANVAS);
        ZImageSliceFactory::Make(
              *spStack,
              imageWidget()->getSliceViewTransform().getModelViewTransform(),
              viewportSize.width(), viewportSize.height(), canvas);
      }
    }
  }
}

void ZStackView::paintStack()
{
  paintStackBuffer();
  updateImageScreen(EUpdateOption::QUEUED);
}

void ZStackView::paintMaskBuffer()
{
  ZStack *stackMask = buddyDocument()->stackMask();
  if (stackMask == NULL) {
    return;
  }

//  updateMaskCanvas();

  int slice = sliceIndex();
  if (stackMask->kind() == GREY) {

    if (stackMask->channelNumber() >= 3) {
      m_imageMask->set3ChannelData(
            static_cast<uint8_t*>(stackMask->getDataPointer(0, slice)), 1, 0,
            static_cast<uint8_t*>(stackMask->getDataPointer(1, slice)), 1, 0,
            static_cast<uint8_t*>(stackMask->getDataPointer(2, slice)), 1, 0, 100);
    } else if (stackMask->channelNumber() == 2) {
      m_imageMask->set2ChannelData(
            static_cast<uint8_t*>(stackMask->getDataPointer(0, slice)), 1, 0,
            static_cast<uint8_t*>(stackMask->getDataPointer(1, slice)), 1, 0,
            100);
    } else {
      m_imageMask->setCData(static_cast<uint8_t*>(
                              stackMask->getDataPointer(0, slice)), 100);
    }
  } else if (stackMask->kind() == GREY16) {
    m_imageMask->setCData(static_cast<uint8_t*>(
                            stackMask->getDataPointer(0, slice)), 100);
  }
  if (buddyPresenter()->objectStyle() == ZStackObject::EDisplayStyle::BOUNDARY) {
    m_imageMask->enhanceEdge();
  }
}

void ZStackView::paintMask()
{
  paintMaskBuffer();
  updateImageScreen(EUpdateOption::QUEUED);
}

#if 0
void ZStackView::paintObjectBuffer(
    ZSliceCanvas &canvas, neutu::data3d::ETarget target)
{
  if (canvas.isVisible()) {
//    ZSliceCanvasPaintHelper p(canvas);
//    QPainter *painter = p.getPainter();
//    if (painter->isActive()) {
      updateObjectSorter();
      auto objList = m_paintSorter.getVisibleObjectList(
            target, ZStackObjectPaintSorter::SortingZ(true));

      updateObjectBuffer(&canvas, target, objList);

      /*
      neutu::data3d::DisplayConfig config;
      config.setStyle(buddyPresenter()->objectStyle());
      config.setTransform(canvas.getTransform());
      for (ZStackObject *obj : objList) {
        if (obj->display(painter, config)) {
          canvas.setPainted(true);
        }
      }
      */
//    }
  }
}
#endif

void ZStackView::updateObjectBuffer(neutu::data3d::ETarget target)
{
  std::shared_ptr<ZSliceCanvas> canvas = getClearCanvas(target);

  if (canvas) {
    updateObjectBuffer(canvas, target);
  }
}

void ZStackView::updateObjectBuffer(
    std::shared_ptr<ZSliceCanvas> canvas, neutu::data3d::ETarget target)
{
  if (canvas  && canvas->updateNeeded()) {
    updateObjectSorter();
    auto objList = m_paintSorter.getVisibleObjectList(
          target, ZStackObjectPaintSorter::SortingZ(true));

    updateObjectBuffer(canvas, target, objList);
  }
}



void ZStackView::updateObjectBuffer(
    std::shared_ptr<ZSliceCanvas> canvas, neutu::data3d::ETarget target,
    const QList<ZStackObject*> &objList)
{
  if (neutu::data3d::IsNonblocking(target)) {
    addNonblockCanvasTask(canvas, target, objList);
  } else {
    updateObjectBuffer(canvas, objList);
  }
}

void ZStackView::updateObjectBuffer(
    std::shared_ptr<ZSliceCanvas> canvas, const QList<ZStackObject*> &objList)
{
  if (canvas && canvas->updateNeeded()) {
    ZSliceCanvasPaintHelper p(*canvas);
    QPainter *painter = p.getPainter();
    updateObjectBuffer(canvas, painter, objList);
  }
}

void ZStackView::updateObjectBuffer(
    std::shared_ptr<ZSliceCanvas> canvas, QPainter *painter,
    const QList<ZStackObject*> &objList)
{
  if (canvas  && canvas->updateNeeded() && painter->isActive()) {
    neutu::data3d::DisplayConfig config;
    config.setStyle(buddyPresenter()->objectStyle());
    config.setTransform(canvas->getTransform());
    config.setViewId(getViewId());

#ifdef _DEBUG_0
      if (m_viewId == 2) {
        std::cout << "debug here" << std::endl;
      }
#endif

    for (ZStackObject *obj : objList) {
#ifdef _DEBUG_0
      if (m_viewId == 2) {
        std::cout << "debug here" << std::endl;
      }
#endif

      if (obj->display(painter, config)) {
        canvas->setPainted(true);
      }
    }
  }
}

void ZStackView::updateObjectBuffer(
    neutu::data3d::ETarget target, const QList<ZStackObject *> &objList)
{
  if (neutu::data3d::IsNonblocking(target)) {
    addNonblockCanvasTask(nullptr, target, objList);
  } else {
    std::shared_ptr<ZSliceCanvas> canvas = getClearCanvas(target);
    if (canvas && canvas->updateNeeded()) {
      updateObjectBuffer(canvas, objList);
    }
  }
  /*
  std::shared_ptr<ZSliceCanvas> canvas = getClearCanvas(target);
  if (canvas && canvas->updateNeeded()) {
    if (neutu::data3d::IsNonblocking(target)) {
      addNonblockCanvasTask(canvas, target, objList);
    } else {
      updateObjectBuffer(canvas, objList);
    }
  }
  */
}

template<template<class...> class Container>
void ZStackView::updateObjectBuffer(
    const Container<neutu::data3d::ETarget> &targetList)
{
  for (auto target : targetList) {
    updateObjectBuffer(target);
  }
}

void ZStackView::paintObject(neutu::data3d::ETarget target)
{
  updateObjectBuffer(target);
  updateImageScreen(EUpdateOption::QUEUED);
}

void ZStackView::paintObject(const std::set<neutu::data3d::ETarget> &targetSet)
{
//  m_paintSorter.clear();
  for (neutu::data3d::ETarget target : targetSet) {
    updateObjectBuffer(target);
  }

  updateImageScreen(EUpdateOption::QUEUED);
}


/*
void ZStackView::addNonblockCanvasTask(
    neutu::data3d::ETarget target, const QList<ZStackObject *> &objList)
{
  addNonblockCanvasTask(nullptr, target, objList);
}
*/

void ZStackView::addNonblockCanvasTask(
    std::shared_ptr<ZSliceCanvas> canvas, neutu::data3d::ETarget target,
    const QList<ZStackObject *> &objList)
{
  if (imageWidget()->hasCanvas(canvas, target)) {
    //Ignore the managed canvas, but stil try to update the screen
    updateImageScreen(EUpdateOption::QUEUED);
  } else {
    if (!objList.isEmpty()) {
      ZTask *task = new ZFunctionTask([=]() {
        std::shared_ptr<ZSliceCanvas> bufferCanvas = canvas;
        if (!bufferCanvas) {
          bufferCanvas = imageWidget()->makeClearCanvas();
        }
        if (bufferCanvas->updateNeeded()) {
          updateObjectBuffer(bufferCanvas, objList);
        }
        notifyCanvasUpdate(target, bufferCanvas);
      });

      task->setName("ZStackView::addNonblockCanvasTask");

      addTask(task);
    }
  }
}

void ZStackView::configureRecorder()
{
  if (m_recordDlg == nullptr) {
    m_recordDlg = new ZStackViewRecordDialog(this);
  }

  if (m_recordDlg->exec()) {
    m_recordDlg->configureRecorder(getRecorder());
  }
}

#if 0
bool ZStackView::paintTileCanvasBuffer()
{
#ifdef _DEBUG_2
  std::cout << "ZStackView::paintTileCanvasBuffer" << std::endl;
#endif
  bool painted = false;

//  QElapsedTimer timer;
//  timer.start();
  if (buddyDocument()->hasObject(neutu::data3d::ETarget::TILE_CANVAS)) {
    auto canvas = getClearCanvas(neutu::data3d::ETarget::TILE_CANVAS);
//    auto canvas = imageWidget()->getValidCanvas(
//          ZImageWidget::ECanvasRole::CANVAS_ROLE_TILE);
    if (canvas && canvas->isVisible()) {
      paintObjectBuffer(*canvas, neutu::data3d::ETarget::TILE_CANVAS);
    }

#if 0
    //std::cout << "update time canvas time: " << timer.elapsed() << std::endl;
    if (m_tileCanvasPainter.isActive()) {
#ifdef _DEBUG_
      std::cout << "Painting tile buffer ..." << std::endl;
#endif
      paintObjectBuffer(m_tileCanvasPainter, neutu::data3d::ETarget::TARGET_TILE_CANVAS);
      painted = true;
    }
#endif
    //std::cout << "paint tile time: " << timer.elapsed() << std::endl;
  }

//  setDepthFrozen(false);
  setViewPortFrozen(false);

  return painted;
}
#endif

#if 0
void ZStackView::paintObject()
{
#ifdef _DEBUG_
  std::cout << "ZStackView::paintObject()" << std::endl;
#endif

  if (buddyPresenter()->isObjectVisible()) {
    updateObjectSorter();
  }

  updateObjectBuffer(neutu::data3d::GetTargetSettled2dObjectCanvasList());

  /*
  m_paintSorter.clear();
  paintObjectBuffer();
  paintDynamicObjectBuffer();
  paintObjectBuffer(neutu::data3d::ETarget::HD_OBJECT_CANVAS);
  &*/
  updateImageScreen(EUpdateOption::QUEUED);
}
#endif

/*
void ZStackView::paintActiveTile()
{
  if (paintTileCanvasBuffer()) {
    updateImageScreen(EUpdateOption::QUEUED);
  }
}
*/

void ZStackView::paintObject(const ZStackObjectInfoSet &selected,
                             const ZStackObjectInfoSet &deselected)
{
#ifdef _DEBUG_0
  HLDEBUG_FUNC_LN("2D View");
#endif
  updateObjectBuffer(
        neutu::setunion(selected.getTarget(), deselected.getTarget()));
  /*
  if (selected.getTarget().count(neutu::data3d::ETarget::PIXEL_OBJECT_CANVAS) > 0 ||
      deselected.getTarget().count(neutu::data3d::ETarget::PIXEL_OBJECT_CANVAS) > 0) {
    paintObjectBuffer();
  } else if (selected.getTarget().count(neutu::data3d::ETarget::STACK_CANVAS) > 0 ||
             deselected.getTarget().count(neutu::data3d::ETarget::STACK_CANVAS) > 0) {
    paintStackBuffer();
  }
  */

  RefreshConfig config;
  config.widgetCanvasUpdateRequired = false;
  refreshScreen(config);
//  updateImageScreen(EUpdateOption::QUEUED);
}

size_t ZStackView::getFrameCount() const
{
  return m_frameCount;
}

/*
void ZStackView::paintObject(
    QList<ZStackObject *> selected,
    QList<ZStackObject *> deselected)
{
  bool updatingObjectCanvas = false;
  bool updatingImageCanvas = false;
  foreach (ZStackObject *obj, selected) {
    if (obj->getTarget() == neutu::data3d::ETarget::PIXEL_OBJECT_CANVAS) {
      updatingObjectCanvas = true;
    } else if (obj->getTarget() == neutu::data3d::ETarget::STACK_CANVAS) {
      updatingImageCanvas = true;
    }
  }

  foreach (ZStackObject *obj, deselected) {
    if (obj->getTarget() == neutu::data3d::ETarget::PIXEL_OBJECT_CANVAS) {
      updatingObjectCanvas = true;
    } else if (obj->getTarget() == neutu::data3d::ETarget::STACK_CANVAS) {
      updatingImageCanvas = true;
    }
  }

  if (updatingObjectCanvas) {
    paintObjectBuffer();
  }

  if (updatingImageCanvas) {
    paintStackBuffer();
  }

  updateImageScreen(EUpdateOption::QUEUED);
}
*/

/*
void ZStackView::paintDynamicObjectBuffer()
{
  updateObjectBuffer(neutu::data3d::ETarget::DYNAMIC_OBJECT_CANVAS);
}

void ZStackView::paintActiveDecorationBuffer()
{
  updateObjectBuffer(neutu::data3d::ETarget::ROAMING_OBJECT_CANVAS);
}
*/


void ZStackView::paintActiveDecoration()
{
  paintObject(neutu::data3d::ETarget::ROAMING_OBJECT_CANVAS);
  /*
  paintActiveDecorationBuffer();
  RefreshConfig config;
  config.widgetCanvasUpdateRequired = false;
  refreshScreen(config);
  */
//  updateImageScreen(EUpdateOption::QUEUED);
}


ZStack* ZStackView::getStrokeMask(neutu::EColor color)
{
  std::vector<ZStroke2d*> strokeArray;
  QList<ZStroke2d*> strokeList = buddyDocument()->getStrokeList();
  foreach (ZStroke2d *stroke, strokeList) {
    bool isMask = true;
    if (!buddyPresenter()->interactiveContext().isObjectProjectView()) {
      if (!stroke->isPenetrating()) {
        if (stroke->getZ() != getCurrentDepth()) {
          isMask = false;
        }
      }
    }
    if (isMask) {
      if (!stroke->isEraser()) {
        switch (color) {
        case neutu::EColor::RED:
          isMask = (stroke->getColor().red() > 0 && stroke->getColor().green() == 0 &&
                    stroke->getColor().blue() == 0);
          break;
        case neutu::EColor::GREEN:
          isMask = (stroke->getColor().red() == 0 && stroke->getColor().green() > 0 &&
                    stroke->getColor().blue() == 0);
          break;
        case neutu::EColor::BLUE:
          isMask = (stroke->getColor().red() == 0 && stroke->getColor().green() == 0 &&
                    stroke->getColor().blue() > 0);
          break;
        case neutu::EColor::ALL:
          isMask = true;
          break;
        }
      }
    }
    if (isMask) {
      strokeArray.push_back(stroke);
    }
  }

  ZStack *stack = ZStackFactory::MakeStrokeProjMask(strokeArray);

  return stack;
}

ZStack* ZStackView::getStrokeMask(uint8_t maskValue)
{
  std::vector<ZStroke2d*> strokeArray;
  QList<ZStroke2d*> strokeList = buddyDocument()->getStrokeList();
  foreach (ZStroke2d *stroke, strokeList) {
    bool isMask = true;
    if (!buddyPresenter()->interactiveContext().isObjectProjectView()) {
      if (!stroke->isPenetrating()) {
        if (stroke->getZ() != getCurrentDepth()) {
          isMask = false;
        }
      }
    }

    if (isMask) {
      strokeArray.push_back(stroke);
    }
  }

  ZStack *stack = ZStackFactory::MakeStrokeProjMask(strokeArray, maskValue);

  return stack;
}

#if 0
ZStack* ZStackView::getObjectMask(uint8_t maskValue)
{
  ZStack *stack = NULL;

  if (m_objectCanvas != NULL) {
//    QPixmap *pixmap = m_objectCanvas.getPixmap(0);

    QImage image;
    image = m_objectCanvas->toImage();

    stack = new ZStack(GREY, image.width(), image.height(), 1, 1);
    size_t offset = 0;
    uint8_t *array = stack->array8();
    for (int y = 0; y < image.height(); ++y) {
      for (int x = 0; x < image.width(); ++x) {
        QRgb rgb = image.pixel(x, y);
        if (qRed(rgb) > 0 || qGreen(rgb) > 0 || qBlue(rgb) > 0) {
          array[offset] = maskValue;
        } else {
          array[offset] = 0;
        }
        ++offset;
      }
    }
  }

  return stack;
}
#endif

#if 0
ZStack* ZStackView::getObjectMask(neutu::EColor color, uint8_t maskValue)
{
  ZStack *stack = NULL;

  if (m_objectCanvas != NULL) {
//    QPixmap *pixmap = m_objectCanvas.getPixmap(0);
    stack = new ZStack(GREY, m_objectCanvas->getWidth(),
                       m_objectCanvas->getHeight(), 1, 1);
    size_t offset = 0;
    uint8_t *array = stack->array8();
    QImage image = m_objectCanvas->toImage();
    for (int y = 0; y < m_objectCanvas->getHeight(); ++y) {
      for (int x = 0; x < m_objectCanvas->getWidth(); ++x) {
        QRgb rgb = image.pixel(x, y);
        bool isForeground = false;
        switch (color) {
        case neutu::EColor::RED:
          if ((qRed(rgb) > qGreen(rgb)) && (qRed(rgb) > qBlue(rgb))) {
            isForeground = true;
          }
          break;
        case neutu::EColor::GREEN:
          if ((qGreen(rgb) > qRed(rgb)) && (qGreen(rgb) > qBlue(rgb))) {
            isForeground = true;
          }
          break;
        case neutu::EColor::BLUE:
          if ((qBlue(rgb) > qRed(rgb)) && (qBlue(rgb) > qGreen(rgb))) {
            isForeground = true;
          }
          break;
        default:
          break;
        }

        if (isForeground) {
          array[offset] = maskValue;
        } else {
          array[offset] = 0;
        }
        ++offset;
      }
    }
  }

  return stack;
}
#endif

#if 0
void ZStackView::exportObjectMask(const string &filePath)
{
  if (m_objectCanvas != NULL) {
    //m_objectCanvas->save(filePath.c_str());
    ZStack *stack = getObjectMask(255);
    if (stack != NULL) {
      stack->save(filePath);
      delete stack;
    }
  }
}

void ZStackView::exportObjectMask(
    neutu::EColor color, const string &filePath)
{
  if (m_objectCanvas != NULL) {
    //m_objectCanvas->save(filePath.c_str());
    ZStack *stack = getObjectMask(color, 255);
    if (stack != NULL) {
      stack->save(filePath);
      delete stack;
    }
  }
}
#endif

/*
void ZStackView::requestWidgetCanvasUpdate()
{
//  processWidgetCanvasUpdate(nullptr);
  if (m_paintBundle.hasWidgetObject()) {
    imageWidget()->paintWidgetObject();
//    addWidgetCanvasTask();
  } else {
    processWidgetCanvasUpdate(nullptr);
  }
}
*/
/*
void ZStackView::processWidgetCanvasUpdate(ZPixmap *canvas)
{
  m_imageWidget->updateWidgetCanvas(canvas);
}
*/

void ZStackView::processCanvasUpdate(
    neutu::data3d::ETarget target, std::shared_ptr<ZSliceCanvas> canvas)
{
  m_imageWidget->updateSliceCanvas(target, canvas);
}

void ZStackView::notifyCanvasUpdate(
    neutu::data3d::ETarget target, std::shared_ptr<ZSliceCanvas> canvas)
{
  emit canvasUpdated(target, canvas);
}

/*
void ZStackView::notifyWidgetCanvasUpdate(ZPixmap *canvas)
{
  emit widgetCanvasUpdated(canvas);
}
*/

/*
void ZStackView::notifyWidgetCanvasUpdate(ZImage *canvas)
{
  emit widgetCanvasUpdated(canvas);
}
*/

#if 0
void ZStackView::addWidgetCanvasTask()
{
  ZTask *task = new ZFunctionTask([this]() {
    ZImage *canvas = this->m_imageWidget->makeWidgetCanvas();
    canvas->fill(Qt::transparent);
    this->m_imageWidget->paintWidgetCanvas(canvas);
    ZPixmap *pixmap = new ZPixmap;
    pixmap->convertFromImage(*canvas);
    pixmap->setTransform(canvas->getWorldTransform());
    pixmap->setProjTransform(canvas->getProjectionTransform());
    delete canvas;
    this->notifyWidgetCanvasUpdate(pixmap);
  });
  task->setName("ZStackView::addWidgetCanvasTask");

  addTask(task);
}
#endif

void ZStackView::configurePainter(ZStackObjectPainter &/*painter*/)
{
//  painter.setRestoringPainter(true);
//  painter.setSliceAxis(getSliceAxis());
//  painter.setDisplayStyle(buddyPresenter()->objectStyle());
}

void ZStackView::printViewParam() const
{
#ifdef _DEBUG_
    std::cout << "Axis: " << neutu::EnumValue(getSliceAxis()) << std::endl;
    getViewProj().print();
#endif
}

void ZStackView::zoomTo(const ZIntPoint &pt)
{
  zoomTo(pt.getX(), pt.getY(), pt.getZ());
}

void ZStackView::zoomTo(int x, int y, int z, int w)
{
  if (w <= 0) {
    imageWidget()->setCutCenter(x, y, z, neutu::ESignalControl::BROADCASTING);
  } else {
    imageWidget()->zoomTo(
          ZPoint(x, y, z), w, w, neutu::data3d::ESpace::MODEL,
          neutu::ESignalControl::BROADCASTING);
  }
  buddyDocument()->activateLocationHint(x, y, z);
}

void ZStackView::zoomTo(const ZIntPoint &pt, int w)
{
  zoomTo(pt.getX(), pt.getY(), pt.getZ(), w);
}

void ZStackView::zoomTo(const ZPoint &pt, int w, bool showingHint)
{
  imageWidget()->zoomTo(pt, w, w, neutu::data3d::ESpace::MODEL, neutu::ESignalControl::BROADCASTING);
  if (showingHint) {
    buddyDocument()->activateLocationHint(pt);
  }
}

void ZStackView::zoomTo(int x, int y, int z)
{
  imageWidget()->zoomTo(ZPoint(x, y, z), neutu::ESignalControl::BROADCASTING);
  buddyDocument()->activateLocationHint(x, y, z);
//  zoomTo(x, y, z, 800);
}

void ZStackView::increaseZoomRatio()
{
  increaseZoomRatio(0, 0, false);
}

void ZStackView::decreaseZoomRatio()
{
  decreaseZoomRatio(0, 0, false);
}


void ZStackView::increaseZoomRatio(int x, int y, bool usingRef)
{
  if (!isViewPortFronzen()) {
    imageWidget()->increaseZoomRatio(x, y, usingRef, neutu::ESignalControl::BROADCASTING);
  }
}

void ZStackView::decreaseZoomRatio(int x, int y, bool usingRef)
{
  if (m_maxViewPort > 0) {
    ZAffineRect rect = imageWidget()->getViewPort();
    if (rect.getWidth() * rect.getHeight() >= m_maxViewPort) {
      return;
    }
  }

  if (!isViewPortFronzen()) {
    imageWidget()->decreaseZoomRatio(x, y, usingRef, neutu::ESignalControl::BROADCASTING);
//    reloadCanvas();
#if 0
//    setViewPortFrozen(true);
    recordViewParam();
    imageWidget()->blockPaint(true);
    imageWidget()->decreaseZoomRatio(x, y, usingRef);
    updateSliceViewParam();
//    reloadCanvas();
    if (buddyPresenter()->interactiveContext().exploreMode() !=
        ZInteractiveContext::EXPLORE_ZOOM_OUT_IMAGE) {
//      reloadTileCanvas();
//      reloadObjectCanvas(true);

//      updateNewTileCanvas();

      processViewChange(true);
    }

//    notifyViewChanged(NeuTube::View::EXPLORE_ZOOM);
    imageWidget()->blockPaint(false);
//    imageWidget()->update();
    updateImageScreen(EUpdateOption::QUEUED);

//    notifyViewPortChanged();
#endif
  }
}

void ZStackView::rotateView(double da, double db)
{
  imageWidget()->rotate(da, db, neutu::ESignalControl::BROADCASTING);
}

ZPoint ZStackView::getCutCenter() const
{
  return getSliceViewTransform().getCutCenter();
}

ZAffineRect ZStackView::getCutRect() const
{
  return getSliceViewTransform().getCutRect(
        imageWidget()->width(), imageWidget()->height());
}

/*
ZIntPoint ZStackView::getCenter(neutu::ECoordinateSystem coordSys) const
{
  ZIntPoint center;
  center.setZ(getZ(coordSys));

  QRect rect = getViewPort(coordSys);
  center.setX(rect.center().x());
  center.setY(rect.center().y());

  return center;
}
*/

int ZStackView::getZ(neutu::ECoordinateSystem coordSys) const
{
  int z = sliceIndex();
  if (coordSys == neutu::ECoordinateSystem::STACK) {
    z += getZ0();
  }

  return z;
}

ZAffineRect ZStackView::getViewPort() const
{
  return m_imageWidget->getViewPort();
}

/*
QRectF ZStackView::getProjRegion() const
{
  return m_imageWidget->projectRegion();
}
*/

ZViewProj ZStackView::getViewProj() const
{
//  return m_imageWidget->getViewProj();
  return ZViewProj();
}

/*
void ZStackView::setViewProj(int x0, int y0, double zoom)
{
  m_imageWidget->setViewProj(x0, y0, zoom);
  processViewChange(true, false);
}

void ZStackView::setViewProj(const QPoint &pt, double zoom)
{
  setViewProj(pt.x(), pt.y(), zoom);
}

void ZStackView::setViewProj(const ZViewProj &vp)
{
  if (m_imageWidget->getViewProj() != vp) {
    recordViewParam();
    m_imageWidget->setViewProj(vp);
    processViewChange(true);
  }
}
*/

void ZStackView::tryAutoRecord()
{
  if (getRecorder()->isAuto()) {
    // This can cause 'QWidget::repaint: Recursive repaint detected' warning.
    // Just ignore it as false alarm.
    getRecorder()->takeShot(this);
  }
}

void ZStackView::resetViewParam(const ZArbSliceViewParam &param)
{
  ZSliceViewTransform transform;
  transform.setCutPlane(
        param.getCenter().toPoint(), param.getPlaneV1(), param.getPlaneV2());
  transform.setAnchor(getSliceViewTransform().getAnchor());
  transform.zoomToViewRect(
        param.getWidth(), param.getHeight(),
        imageWidget()->width(), imageWidget()->height());
  imageWidget()->setSliceViewTransform(
        transform, neutu::ESignalControl::BROADCASTING);
}

ZStackViewParam ZStackView::getViewParameter() const
{
  ZStackViewParam param;
  param.setTransform(getSliceViewTransform());
  param.setSize(
        imageWidget()->width(), imageWidget()->height(),
        neutu::data3d::ESpace::CANVAS);
  param.setViewId(getViewId());
//  param.setExploreAction(action);

  return param;
  /*
  ZStackViewParam param;
  param.setZ(getZ(coordSys));
  param.setViewProj(getViewProj());
//  param.setViewPort(getViewPort(coordSys));
//  param.setProjRect(getProjRegion());
  param.setExploreAction(action);
  param.setSliceAxis(getSliceAxis());
  param.setZOffset(getZ0());

  if (getSliceAxis() == neutu::EAxis::ARB) {
    ZArbSliceViewParam viewParam = m_sliceViewParam;
    viewParam.setSize(0, 0);
    param.setArbSliceView(viewParam);
  }
  //param.setViewPort(imageWidget()->viewPort());

  return param;
  */
}

/*
ZStackViewParam ZStackView::getViewParameter(const ZArbSliceViewParam &param) const
{
  ZStackViewParam viewParam = getViewParameter(neutu::ECoordinateSystem::STACK);
  viewParam.setZ(param.getZ());
  viewParam.setViewPort(param.getViewPort());
  viewParam.setSliceAxis(neutu::EAxis::ARB);
  viewParam.setArbSliceView(param);

  return viewParam;
}
*/

/*
ZArbSliceViewParam ZStackView::getSliceViewParam() const
{
  return m_sliceViewParam;
}
*/

ZSliceViewTransform ZStackView::getSliceViewTransform() const
{
  return imageWidget()->getSliceViewTransform();
}

void ZStackView::setSliceViewTransform(const ZSliceViewTransform &t)
{
  imageWidget()->setSliceViewTransform(t, neutu::ESignalControl::BROADCASTING);
}

/*
void ZStackView::setSliceViewParam(const ZArbSliceViewParam &param)
{
  m_sliceViewParam = param;
}


void ZStackView::showArbSliceViewPort()
{
  setViewPort(m_sliceViewParam.getViewPort());
  updateViewData();
}
*/

/*
ZStTransform ZStackView::getViewTransform() const
{
  ZStTransform transform;
  QRectF projRegion = getProjRegion();
  projRegion.moveTopLeft(QPointF(0, 0));

  transform.estimate(getViewPort(neutu::ECoordinateSystem::STACK), projRegion);

  return transform;
}
*/

/*
void ZStackView::setViewPortOffset(int x, int y)
{
  recordViewParam();

  imageWidget()->blockPaint(true);
  imageWidget()->setViewPortOffset(x, y);
  imageWidget()->blockPaint(false);
  processViewChange(false, false);
  redraw(EUpdateOption::DIRECT);
//  notifyViewChanged(NeuTube::View::EXPLORE_MOVE);
}
*/

#if 0
void ZStackView::syncArbViewCenter()
{
  m_imageWidget->setViewPortCenterQuitely(
        m_sliceViewParam.getX(), m_sliceViewParam.getY());
  int z = m_sliceViewParam.getZ() - getZ0();
  m_depthControl->setValueQuietly(z);
  m_zSpinBox->setValueQuietly(z);
}
#endif

/*
void ZStackView::updateSliceViewParam()
{
  if (getSliceAxis() == neutu::EAxis::ARB) {
    if (m_oldViewParam.getSliceViewParam().hasSamePlaneCenter(m_sliceViewParam)) {
      ZStackViewParam viewParam = getViewParameter();
      if (viewParam.isValid() && m_oldViewParam.isValid()) {
        QPoint center = viewParam.getViewPort().center();
        QPoint oldCenter = m_oldViewParam.getViewPort().center();
        int dx = center.x() - oldCenter.x();
        int dy = center.y() - oldCenter.y();
        int dz = viewParam.getZ() - m_oldViewParam.getZ();

        if (dx != 0 || dy != 0 || dz != 0) {
          m_sliceViewParam.move(dx, dy, dz);
        }
      }
    }
  }
}
*/

void ZStackView::moveViewPort(const QPoint &src, const QPointF &dst)
{
  imageWidget()->moveViewPort(src, dst, neutu::ESignalControl::BROADCASTING);
#if 0
  recordViewParam();

#ifdef _DEBUG_2
  qDebug() << "=====> Viewport center before moving:"
           << imageWidget()->viewPort().center();
#endif

  imageWidget()->blockPaint(true);
  imageWidget()->moveViewPort(src, dst);
  imageWidget()->blockPaint(false);

#ifdef _DEBUG_2
  qDebug() << "=====> Viewport center after moving:"
           << imageWidget()->viewPort().center();
#endif

  updateSliceViewParam();

  processViewChange(false, false);
  redraw(EUpdateOption::DIRECT);
#endif
}

void ZStackView::moveViewPort(const ZPoint &src, int a, int b)
{
  imageWidget()->moveViewPort(src, QPointF(a, b), neutu::ESignalControl::BROADCASTING);
}

void ZStackView::moveViewPort(int dx, int dy)
{
  recordViewParam();

  imageWidget()->moveViewPort(dx, dy, neutu::ESignalControl::BROADCASTING);
//  processViewChange(false, false);
//  redraw(EUpdateOption::DIRECT);
}

void ZStackView::setViewPortCenter(
    const ZIntPoint &center, neutu::EAxisSystem system)
{
  setViewPortCenter(center.getX(), center.getY(), center.getZ(), system);
}

void ZStackView::recordViewParam()
{
  if (!m_viewParamRecordOnce || !m_viewParamRecorded) {
//    m_oldViewParam = getViewParameter();
    imageWidget()->recordTransform();
//    m_prevMainTransform = m_mainTransform;
    m_viewParamRecorded = true;
  }
}

void ZStackView::updateSliceFromZ(int z)
{
  bool depthChanged = false;
  int slice = z - getZ0();
  if (slice != m_depthControl->value()) {
    setSliceIndexQuietly(slice);
    depthChanged = true;
  }
//    setSliceIndex(
//          z - buddyDocument()->getStackOffset().getSliceCoord(getSliceAxis()));
  updateImageScreen(ZStackView::EUpdateOption::QUEUED);
  processViewChange(true, depthChanged);
}

ZPoint ZStackView::transform(
    const ZPoint &pt, neutu::data3d::ESpace src, neutu::data3d::ESpace dst) const
{
  return imageWidget()->transform(pt, src, dst);
}

void ZStackView::setViewPortCenter(
    int /*x*/, int /*y*/, int /*z*/, neutu::EAxisSystem /*system*/)
{
#if 0
  switch (system) {
  case neutu::EAxisSystem::NORMAL:
    zgeom::shiftSliceAxis(x, y, z, getSliceAxis());
    setViewPortCenter(x, y, z, neutu::EAxisSystem::SHIFTED);
    break;
  case neutu::EAxisSystem::SHIFTED:
  {
    /* Note that cx=x_0+floor((w-1)/2) */
    imageWidget()->setViewPortOffset(
          x - (imageWidget()->viewPort().width() - 1) / 2,
          y - (imageWidget()->viewPort().height() - 1) / 2);

    updateSliceFromZ(z);
  }
    break;
  }
#endif

}

ZIntPoint ZStackView::getViewCenter() const
{
  return imageWidget()->getSliceViewTransform().getCutCenter().roundToIntPoint();
  /*
  ZIntPoint center;

  QRect viewPort = getViewPort(neutu::ECoordinateSystem::STACK);
  QPoint viewPortCenter = viewPort.center();
  center.set(viewPortCenter.x(), viewPortCenter.y(),
             getZ(neutu::ECoordinateSystem::STACK));

  center.shiftSliceAxisInverse(getSliceAxis());

  return center;
  */
}

void ZStackView::reloadCanvas()
{
//  reloadObjectCanvas();
//  updateTileCanvas();
//  reloadTileCanvas();
}

/*
void ZStackView::setViewPort(const QRect &rect)
{
  imageWidget()->setViewPort(rect);

  reloadCanvas();
  processViewChange(false, false);
  redraw();
}
*/

void ZStackView::setViewPort(const ZAffineRect &rect)
{
  imageWidget()->setViewPort(rect, neutu::ESignalControl::BROADCASTING);
}

void ZStackView::maximizeViewPort()
{
  imageWidget()->maximizeViewPort(
        buddyDocument()->getDataRange(), neutu::ESignalControl::BROADCASTING);

//  reloadCanvas();
//  processViewChange(false, false);
//  redraw();
}

void ZStackView::showAxes(bool on)
{
  imageWidget()->showAxes(on);
}

void ZStackView::processDepthSliderValueChange()
{
  processTransformChange();
//  processDepthSliderValueChange(m_depthControl->value());
}

void ZStackView::processDepthSliderValueChange(int sliceIndex)
{
  ZOUT(LTRACE(), 5)<< "ZStackView::processDepthSliderValueChange" << sliceIndex;

//  if (getSliceAxis() != neutu::EAxis::ARB) {
    double prevDepth = imageWidget()->getCutDepth();
//        imageWidget()->getCutCenter().getValue(getSliceAxis());
    imageWidget()->moveCutDepth(sliceIndex - prevDepth, neutu::ESignalControl::BROADCASTING);
#ifdef _DEBUG_0
    std::cout << "🦋 " << "slice index: " << sliceIndex
              << "; prev depth: " << prevDepth
              << "; new depth: " << imageWidget()->getCutDepth() << std::endl;
#endif
//  }
//  imageWidget()->setCutCenter(0, 0, sliceIndex);
  /*
  processViewChange(false, true);

  redraw(EUpdateOption::DIRECT);
  */
}

std::set<neutu::data3d::ETarget> ZStackView::updateViewData(
    const ZStackViewParam &param)
{
  ZStackDoc::ActiveViewObjectUpdater updater(buddyDocument());
  if (buddyPresenter()->isObjectVisible()) {
    switch (buddyPresenter()->interactiveContext().exploreMode()) {
    case ZInteractiveContext::EXPLORE_ZOOM_IN_IMAGE:
    case ZInteractiveContext::EXPLORE_ZOOM_OUT_IMAGE:
      updater.exclude(ZStackObject::EType::DVID_LABEL_SLICE);
      updater.exclude(ZStackObject::EType::DVID_GRAY_SLICE_ENSEMBLE);
      break;
    case ZInteractiveContext::EXPLORE_MOVE_IMAGE:
      updater.exclude(ZStackObject::EType::DVID_LABEL_SLICE);
      break;
    default:
      break;
    }
    if (m_blinking) {
      updater.exclude(ZStackObject::EType::DVID_LABEL_SLICE);
    }
  } else {
    updater.exclude(neutu::data3d::ETarget::PIXEL_OBJECT_CANVAS);
    updater.exclude(neutu::data3d::ETarget::DYNAMIC_OBJECT_CANVAS);
  }

  //#Review-TZ: Consider adding sync level to param as well as passing doc into
  //            the update function.
  updater.update(param);

  return updater.getUpdatedTargetSet();
}

void ZStackView::updateSceneWithViewData()
{
  auto targetSet = updateViewData();
  updateObjectBuffer(targetSet);
  updateImageScreen(EUpdateOption::QUEUED);
}

std::set<neutu::data3d::ETarget> ZStackView::updateViewData()
{
  return updateViewData(getViewParameter());
#if 0
  ZStackDoc::ActiveViewObjectUpdater updater(buddyDocument());
  if (buddyPresenter()->isObjectVisible()) {
//  QSet<neutu::data3d::ETarget> targetSet =
    if (buddyPresenter()->interactiveContext().exploreMode() ==
        ZInteractiveContext::EXPLORE_ZOOM_IN_IMAGE ||
        buddyPresenter()->interactiveContext().exploreMode() ==
        ZInteractiveContext::EXPLORE_ZOOM_OUT_IMAGE) {
      updater.exclude(ZStackObject::EType::DVID_LABEL_SLICE);
    }
  } else {
    updater.exclude(neutu::data3d::ETarget::PIXEL_OBJECT_CANVAS);
    updater.exclude(neutu::data3d::ETarget::DYNAMIC_OBJECT_CANVAS);
  }

  ZStackViewParam param = getViewParameter();
  updater.update(param);

  return updater.getUpdatedTargetSet();
#endif
}

bool ZStackView::isViewChanged(const ZSliceViewTransform &t)
{
  return getSliceViewTransform() != t;
}

/*
bool ZStackView::isViewChanged(const ZStackViewParam &param) const
{
  ZStackViewParam currentParam = getViewParameter(neutu::ECoordinateSystem::STACK);

  return (currentParam != param);
}
*/

/*
void ZStackView::processViewChange(bool redrawing)
{
  ZStackViewParam param = getViewParameter();

  if (param != m_oldViewParam) {
    processViewChange(redrawing, param.getZ() != m_oldViewParam.getZ());
  }
}
*/

void ZStackView::logViewParam()
{
//  ZStackViewParam param = getViewParameter();
//  std::ostringstream stream;
//  stream << this;

//  QJsonDocument jdoc = QJsonDocument::fromJson(
//        QString::fromStdString(param.toJsonObject().dumpString(0)).toUtf8());

  ZSliceViewTransform param = getSliceViewTransform();

  KLOG(VIEW_TOPIC) << ZLog::Info()
       << ZLog::Description("View changed to " + neutu::ToString(param))
       << ZLog::Handle(this)
       << ZLog::Object("ZStackView")
       << ZLog::Tag("parameter", neutu::ToQJsonValue(param.toJsonObject()))
       << ZLog::Level(2);
//       << ZLog::Tag("parameter", QJsonValue(jdoc.object()));
}

bool ZStackView::signalingViewChange() const
{
  return m_signalingViewChange;
}

void ZStackView::enableViewChangeSignal(bool on)
{
  m_signalingViewChange = on;
}

void ZStackView::processViewChange(bool redrawing, bool depthChanged)
{
  if (!isViewChangeEventBlocked() && isVisible()) {
    logViewParam();

//    requestWidgetCanvasUpdate();
//    ZStackViewParam param = getViewParameter(neutube::COORD_STACK);
    std::set<neutu::data3d::ETarget> targetSet = updateViewData();

                     /*
    updateActiveDecorationCanvas();
    for (auto iter = targetSet.begin();
         iter != targetSet.end(); ++iter) {
      updateCanvas(*iter);
    }
                 */
//    updateNewTileCanvas();

    if (redrawing) {
      if (depthChanged) {
        paintStackBuffer();
      }
      targetSet.insert(neutu::data3d::ETarget::PIXEL_OBJECT_CANVAS);
      targetSet.insert(neutu::data3d::ETarget::DYNAMIC_OBJECT_CANVAS);

      std::string painted;
      foreach (neutu::data3d::ETarget target, targetSet) {
        updateObjectBuffer(target);
        painted += neutu::ToString(target) + ",";
      }
      LDEBUG() << "Painting objects in" << painted;
    }

    notifyViewChanged();
//    notifyViewChanged(getViewParameter()); //?

#ifdef _DEBUG_2
    if (m_objectCanvas) {
      m_objectCanvas->save((GET_TEST_DATA_DIR + "/_test.tif").c_str());
    }
#endif
  }
}

void ZStackView::setHoverFocus(bool on)
{
  m_imageWidget->setHoverFocus(on);
}

void ZStackView::setSmoothDisplay(bool on)
{
  m_imageWidget->setSmoothDisplay(on);
  updateImageScreen(EUpdateOption::QUEUED);
}

/*
void ZStackView::notifyViewChanged(NeuTube::View::EExploreAction action)
{
  notifyViewChanged(getViewParameter(NeuTube::COORD_STACK, action));
}
*/

void ZStackView::notifyViewChanged()
{
  if (signalingViewChange()) {
    emit viewChanged(getViewId());
  }
//  notifyViewChanged(getViewParameter(neutu::ECoordinateSystem::STACK));
}

#if 0
void ZStackView::notifyViewChanged(const ZStackViewParam &param)
{
//  updateActiveDecorationCanvas();
//  updateNewTileCanvas();

#ifdef _DEBUG_2
  std::cout << "Signal: ZStackView::viewChanged" << std::endl;
#endif
  if (!isViewChangeEventBlocked()) {
#ifdef _DEBUG_2
    std::cout << "BEFORE emit ZStackView::viewChanged" << std::endl;
#endif
//    processViewChange(param);

    emit viewChanged(param);
  }
}
#endif

/*
void ZStackView::notifyViewPortChanged()
{
  emit viewPortChanged();
}
*/

bool ZStackView::isImageMovable() const
{
  if (imageWidget()->freeMoving()) {
    return true;
  }

  return true;
//  return (imageWidget()->viewPort() != imageWidget()->canvasRegion());
}

void ZStackView::customizeWidget()
{
  if (buddyDocument()->getTag() == neutu::Document::ETag::FLYEM_MERGE) {
    QPushButton *mergeButton = new QPushButton(this);
    mergeButton->setText("Merge");
    m_secondTopWidget->addWidget(mergeButton, neutu::EH3Layout::LEFT);
    connect(mergeButton, SIGNAL(clicked()), this, SLOT(requestMerge()));

    m_splitButton = new ZBodySplitButton(this);
    m_secondTopWidget->addWidget(m_splitButton, neutu::EH3Layout::LEFT);

    QPushButton *quickVis3dButton = new QPushButton(this);
    quickVis3dButton->setText("Low-res 3D");
    m_secondTopWidget->addWidget(quickVis3dButton, neutu::EH3Layout::LEFT);
    connect(quickVis3dButton, SIGNAL(clicked()),
            this, SLOT(requestQuick3DVis()));

    /*
    QPushButton *vis3dButton = new QPushButton(this);
    vis3dButton->setText("High-res 3D");
    m_secondTopLayout->addWidget(vis3dButton);
    connect(vis3dButton, SIGNAL(clicked()),
            this, SLOT(requestHighresQuick3DVis()));
            */
  } else {
//    m_secondTopLayout->addStretch();
    QPushButton *vis3dButton = new QPushButton(this);
    vis3dButton->setText("3D");
    vis3dButton->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);
    m_secondTopWidget->addWidget(vis3dButton, neutu::EH3Layout::RIGHT);
    connect(vis3dButton, SIGNAL(clicked()), this, SLOT(request3DVis()));

    if (buddyDocument()->getTag() == neutu::Document::ETag::NORMAL) {
      if (GET_APPLICATION_NAME == "General") {
        QPushButton *autoTraceButton = new QPushButton(this);
        autoTraceButton->setIcon(QIcon(":/images/autotrace.png"));
        autoTraceButton->setToolTip("Automatic Tracing");
        m_secondTopWidget->addWidget(autoTraceButton, neutu::EH3Layout::RIGHT);
        connect(autoTraceButton, SIGNAL(clicked()), this, SLOT(requestAutoTracing()));
      }

      QPushButton *settingButton = new QPushButton(this);
      settingButton->setText("Settings");
      settingButton->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);
      m_secondTopWidget->addWidget(settingButton, neutu::EH3Layout::RIGHT);
      connect(settingButton, SIGNAL(clicked()), this, SLOT(requestSetting()));
    }

    if (GET_APPLICATION_NAME == "Biocytin") {
      if (buddyDocument() != NULL) {
        if (buddyDocument()->getTag() == neutu::Document::ETag::BIOCYTIN_STACK) {
          QPushButton *closeChildFrameButton = new QPushButton(this);
          closeChildFrameButton->setText("Close Projection Windows");
          closeChildFrameButton->setSizePolicy(
                QSizePolicy::Maximum, QSizePolicy::Maximum);
          m_secondTopWidget->addWidget(closeChildFrameButton, neutu::EH3Layout::LEFT);
          connect(closeChildFrameButton, SIGNAL(clicked()),
                  this, SLOT(closeChildFrame()));
        }
      }
    }
  }
}

void ZStackView::enableCustomCheckBox(
    int index, const QString &text, bool defaultValue,
    QObject *receiver, const char *slot)
{
  m_checkBoxControl->setCheckBox(
        index,
        ZCheckBoxGroup::CheckBoxConfigBuilder(text)
          .checkedByDefault(defaultValue)
          .connectTo(receiver, slot));
#if 0
#ifdef _DEBUG_0
  std::cout << OUTPUT_HIGHTLIGHT_2 << __FUNCTION__ << std::endl;
  m_toolLayout->addWidget(new QPushButton("test"));
  m_secondTopLayout->addWidget(new QPushButton("test"));
#endif
  while (m_secondTopLayout->takeAt(0));

  if (m_customCheckBoxList.size() <= index) {
    m_customCheckBoxList.resize(index + 1);
  }
  if (m_customCheckBoxList[index] != NULL) {
    m_customCheckBoxList[index]->deleteLater();
  }
  QCheckBox *widget = new QCheckBox(text);
  m_customCheckBoxList[index] = widget;
  if (receiver && slot) {
    connect(widget, SIGNAL(toggled(bool)), receiver, slot);
  }
  widget->setChecked(defaultValue);

  foreach (auto widget, m_customCheckBoxList) {
    m_secondTopLayout->addWidget(widget);
  }
#endif
}

/*
void ZStackView::addHorizontalWidget(QWidget *widget)
{
  if (widget != NULL) {
    m_secondTopLayout->addWidget(widget);
  }
}

void ZStackView::addHorizontalWidget(QSpacerItem *spacer)
{
  if (spacer != NULL) {
    m_secondTopLayout->addSpacerItem(spacer);
  }
}
*/

void ZStackView::closeChildFrame()
{
  emit closingChildFrame();
}

void ZStackView::request3DVis()
{
  if (m_messageManager != NULL) {
    ZMessage message(this);
    if (buddyDocument()->getTag() == neutu::Document::ETag::FLYEM_SPLIT) {
      ZMessageFactory::MakeFlyEmSplit3DVisMessage(message);
    } else {
      ZMessageFactory::Make3DVisMessage(message);
    }
    //message.setBodyEntry("title", "3dvis");
    //message.setBodyEntry("body", "message test");

    m_messageManager->processMessage(&message, true);
  }
}

void ZStackView::setBlinking(bool on)
{
  m_blinking = on;
}

void ZStackView::requestSetting()
{
  emit changingSetting();
}

void ZStackView::requestAutoTracing()
{
  emit autoTracing();
}

void ZStackView::requestQuick3DVis()
{
  if (m_messageManager != NULL) {
    ZMessage message(this);
    if (buddyDocument()->getTag() == neutu::Document::ETag::FLYEM_MERGE) {
      ZMessageFactory::MakeQuick3DVisMessage(message, 1);
    }
    m_messageManager->processMessage(&message, true);
  }
}

/*
void ZStackView::requestHighresQuick3DVis()
{
  if (m_messageManager != NULL) {
    ZMessage message(this);
    if (buddyDocument()->getTag() == neutu::Document::ETag::FLYEM_MERGE) {
      ZMessageFactory::MakeQuick3DVisMessage(message, 0);
    }
    m_messageManager->processMessage(&message, true);
  }
}
*/

void ZStackView::requestMerge()
{
  if (m_messageManager != NULL) {
    /*
    ZMessage message;
    message.setType(ZMessage::TYPE_FLYEM_MERGE);
    message.setOriginalSource(this);
    message.setCurrentSource(this);
    */

    ZMessage message(this);
    message.setType(ZMessage::TYPE_FLYEM_MERGE);

    m_messageManager->processMessage(&message, true);
  }
}

void ZStackView::MessageProcessor::processMessage(
    ZMessage * /*message*/, QWidget * /*host*/) const
{
#ifdef _DEBUG_
  std::cout << "ZStackView::MessageProcessor::processMessage" << std::endl;
#endif
}

ZStackFrame* ZStackView::getParentFrame() const
{
  return qobject_cast<ZStackFrame*>(parent());
}

ZStackMvc* ZStackView::getParentMvc() const
{
  return qobject_cast<ZStackMvc*>(parent());
}

ZSharedPointer<ZStackDoc> ZStackView::buddyDocument() const
{
  if (getParentFrame() != NULL) {
    return getParentFrame()->document();
  } else if (getParentMvc() != NULL) {
    return getParentMvc()->getDocument();
  }
  return ZSharedPointer<ZStackDoc>();
}

ZStackPresenter* ZStackView::buddyPresenter() const
{
  if (getParentFrame() != NULL) {
    return getParentFrame()->presenter();
  } else if (getParentMvc() != NULL) {
    return getParentMvc()->getPresenter();
  }
  return NULL;
}

bool ZStackView::isViewPortFronzen() const
{
  return m_viewPortFrozen;
}

void ZStackView::setViewPortFrozen(bool state)
{
  m_viewPortFrozen = state;
}

bool ZStackView::isDepthFronzen() const
{
  return m_depthFrozen;
}

bool ZStackView::isViewChangeEventBlocked() const
{
  return m_viewChangeEventBlocked;
}

void ZStackView::setDepthFrozen(bool state)
{
  m_depthFrozen = state;
}

void ZStackView::blockViewChangeEvent(bool state)
{
  m_viewChangeEventBlocked = state;
}
/*
void ZStackView::setCanvasVisible(neutu::data3d::ETarget target, bool visible)
{
  switch (target) {
  case neutu::data3d::ETarget::OBJECT_CANVAS:
    if (m_objectCanvas != NULL) {
      m_objectCanvas->setVisible(visible);
    }
//    m_objectCanvas.setVisible(visible);
    break;
  case neutu::data3d::ETarget::TILE_CANVAS:
//    m_tileCanvas.setVisible(visible);
    if (m_tileCanvas != NULL) {
      m_tileCanvas->setVisible(visible);
    }
    break;
  case neutu::data3d::ETarget::DYNAMIC_OBJECT_CANVAS:
    m_dynamicObjectCanvas->setVisible(true);
    break;
  default:
    break;
  }
}
*/

/*
ZPixmap* ZStackView::getCanvas(neutu::data3d::ETarget target)
{
  switch (target) {
  case neutu::data3d::ETarget::OBJECT_CANVAS:
    return imageWidget()->getObjectCanvas();
  case neutu::data3d::ETarget::TILE_CANVAS:
    return imageWidget()->getTileCanvas();
  case neutu::data3d::ETarget::DYNAMIC_OBJECT_CANVAS:
    return imageWidget()->getDynamicObjectCanvas();
  default:
    break;
  }

  return NULL;
}
*/

#if 0
void ZStackView::updateCanvas(neutu::data3d::ETarget target)
{
  switch(target) {
  case neutu::data3d::ETarget::OBJECT_CANVAS:
    updateObjectCanvas();
    break;
  case neutu::data3d::ETarget::TILE_CANVAS:
    updateTileCanvas();
    break;
  case neutu::data3d::ETarget::STACK_CANVAS:
    break;
  case neutu::data3d::ETarget::DYNAMIC_OBJECT_CANVAS:
    updateDynamicObjectCanvas();
    break;
  default:
    break;
  }
}

ZPainter* ZStackView::getPainter(neutu::data3d::ETarget target)
{
  switch (target) {
  case neutu::data3d::ETarget::OBJECT_CANVAS:
    updateObjectCanvas();
    return getObjectCanvasPainter();
  case neutu::data3d::ETarget::TILE_CANVAS:
    updateTileCanvas();
    return getTileCanvasPainter();
#if 0
  case neutu::data3d::ETarget::TARGET_DYNAMIC_OBJECT_CANVAS:
    updateDynamicObjectCanvas();
    if (!m_dynamicObjectCanvasPainter.isActive()) {
      return NULL;
    }

    return &m_dynamicObjectCanvasPainter;
#endif
  default:
    break;
  }

  return NULL;
}
#endif

std::shared_ptr<ZSliceCanvas> ZStackView::getClearCanvas(
    neutu::data3d::ETarget target)
{
  bool visible = true;
  if (neutu::data3d::IsSettled2dObjectCanvas(target)
      && target != neutu::data3d::ETarget::TILE_CANVAS) {
    visible = buddyPresenter()->isObjectVisible();
  }

  auto canvas = std::shared_ptr<ZSliceCanvas>();

  if (neutu::data3d::IsNonblocking(target)) {
    if (visible) {
      canvas = imageWidget()->makeClearCanvas();
    }
  }

  if (canvas == nullptr) {
    canvas = imageWidget()->getClearCanvas(target);
  }

  if (canvas) {
    canvas->setVisible(visible);
  }

  return canvas;
}
/*
ZSliceCanvas* ZStackView::getClearVisibleCanvas(neutu::data3d::ETarget target)
{
  bool visible = true;
  if (neutu::data3d::IsSettled2dObjectCanvas(target)
      && target != neutu::data3d::ETarget::TILE_CANVAS) {
    visible = buddyPresenter()->isObjectVisible();
  }

  ZSliceCanvas *canvas = nullptr;

  if (visible) {
    if (neutu::data3d::IsNonblocking(target)) {
      canvas = imageWidget()->makeClearCanvas();
    } else {
      canvas = getClearCanvas(target).get();
    }

    if (canvas && !canvas->isVisible()) {
      canvas = nullptr;
    }
  }

  return canvas;
}
*/
void ZStackView::dump(const QString &msg)
{
  m_stackLabel->setText(msg);
}

void ZStackView::highlightPosition(const ZIntPoint &pt)
{
  highlightPosition(pt.getX(), pt.getY(), pt.getZ());
}

void ZStackView::highlightPosition(int x, int y, int z)
{
//  ZStackBall *ball = new ZStackBall(x, y, z, 5.0);
//  ball->setColor(255, 0, 0);
//  ball->addVisualEffect(neutu::display::Sphere::VE_GRADIENT_FILL);
//  ball->display(m_objectCanvasPainter, sliceIndex(), ZStackObject::SOLID);

//  buddyPresenter()->setHighlight(true);
//  buddyPresenter()->highlight(x, y, z);
  buddyPresenter()->flashHighlight(x, y, z);
//  buddyPresenter()->addDecoration(ball);

  updateImageScreen(EUpdateOption::QUEUED);
}
