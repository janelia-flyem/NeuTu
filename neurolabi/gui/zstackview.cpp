#include <iostream>
#include <QElapsedTimer>

#include "zstackview.h"
#include "zimagewidget.h"
#include "z3dwindow.h"
#include "zimage.h"
#include "zstackdoc.h"
#include "zstackframe.h"
#include "zstackpresenter.h"
#include "zstackdrawable.h"
#include "zslider.h"
#include "zinteractivecontext.h"
#include "zstack.hxx"
#include "zstackdoc.h"
#include "zclickablelabel.h"
#include "tz_error.h"
#include "zstackball.h"
#include "swctreenode.h"
#include "QsLog.h"
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

#include <QtGui>
#ifdef _QT5_
#include <QtWidgets>
#endif

using namespace std;

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
  m_imagePainter.end();
  m_objectCanvasPainter.end();
  m_tileCanvasPainter.end();

  if (m_image != NULL) {
    delete m_image;
  }

  delete m_objectCanvas;
  delete m_activeDecorationCanvas;

  if (m_ctrlLayout != NULL) {
    if (m_ctrlLayout->parent() == NULL) {
      delete m_ctrlLayout;
    }
  }
  delete m_imageMask;

  delete m_tileCanvas;
}

void ZStackView::init()
{
  m_depthControl = new ZSlider(true, this);
  m_depthControl->setFocusPolicy(Qt::NoFocus);

  m_imageWidget = new ZImageWidget(this);
  m_imageWidget->setSizePolicy(QSizePolicy::Expanding,
                               QSizePolicy::Expanding);
  m_imageWidget->setFocusPolicy(Qt::StrongFocus);
  m_imageWidget->setPaintBundle(&m_paintBundle);

  m_infoLabel = new QLabel(this);
  m_infoLabel->setText(tr("Stack Information"));
  m_infoLabel->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
  m_infoLabel->setFocusPolicy(Qt::NoFocus);

  m_activeLabel = new QLabel(this);
  m_activeLabel->setWindowFlags(Qt::FramelessWindowHint);
  m_activeLabel->setText("<font color='green'>*Active*</font>");
  m_activeLabel->hide();
  m_activeLabel->setFocusPolicy(Qt::NoFocus);

  m_progress = new QProgressBar(this);
  m_progress->setVisible(false);
  m_progress->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
  m_progress->setFocusPolicy(Qt::NoFocus);

  m_topLayout = new QHBoxLayout;
  m_topLayout->addWidget(m_infoLabel);
  m_topLayout->addWidget(m_activeLabel);
  //m_topLayout->addWidget(m_progress);


  m_secondTopLayout = new QHBoxLayout;

  m_channelControlLayout = new QHBoxLayout;

  m_secondTopLayout->addLayout(m_channelControlLayout);
  m_secondTopLayout->addWidget(m_progress);
  m_secondTopLayout->addSpacerItem(new QSpacerItem(1, m_progress->height(),
                                               QSizePolicy::Expanding,
                                               QSizePolicy::Fixed));

  m_layout = new QVBoxLayout;
  m_layout->setSpacing(0);
  //m_layout->addWidget(m_infoLabel);
  m_layout->addLayout(m_topLayout);
  m_layout->addLayout(m_secondTopLayout);
  m_layout->addSpacing(6);
  m_layout->addWidget(m_imageWidget);
  m_layout->addWidget(m_depthControl);

#ifdef _ADVANCED_
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

  //m_parent = parent;
  m_image = NULL;
  m_imageMask = NULL;
  m_objectCanvas = NULL;
  m_activeDecorationCanvas = NULL;
  m_tileCanvas = NULL;
  //m_scrollEnabled = false;
  //updateScrollControl();

  //m_imageWidget->setFocus(); //Cause problem in creating subwindows

  connectSignalSlot();

  setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
  m_sizeHintOption = NeuTube::SIZE_HINT_DEFAULT;

  m_isRedrawBlocked = false;

  m_messageManager = NULL;
  m_splitButton = NULL;

  setDepthFrozen(false);
  setViewPortFrozen(false);
  //customizeWidget();
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


void ZStackView::hideThresholdControl()
{
#ifdef _ADVANCED_
  m_layout->removeItem(m_ctrlLayout);
  m_thresholdSlider->hide();
  m_autoThreButton->hide();
#endif
}

void ZStackView::setInfo(QString info)
{
  if (m_infoLabel != NULL) {
    m_infoLabel->setText(info);
//    m_infoLabel->update();
  }
}

void ZStackView::connectSignalSlot()
{
  /*
  connect(m_depthControl, SIGNAL(valueChanged(int)),
          this, SIGNAL(currentSliceChanged(int)));
  */

  connect(m_depthControl, SIGNAL(valueChanged(int)),
          this, SLOT(processDepthSliderValueChange(int)));

  connect(this, SIGNAL(currentSliceChanged(int)), this, SLOT(redraw()));

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

  connect(this, SIGNAL(viewPortChanged()), this, SLOT(paintActiveTile()));
}

void ZStackView::setInfo()
{
  if (m_infoLabel != NULL) {
    setInfo(QString("%1 x %2 => %3 x %4").
                         arg(buddyDocument()->getStackWidth()).
                         arg(buddyDocument()->getStackHeight()).
			 arg(m_imageWidget->screenSize().width()).
			 arg(m_imageWidget->screenSize().height()));
  }
}

double ZStackView::getZoomRatio() const
{
  return (double) m_imageWidget->projectSize().width() /
      m_imageWidget->viewPort().width();
}

void ZStackView::reset(bool updatingScreen)
{ 
  ZStack *stack = stackData();
  updateChannelControl();
  if (stack != NULL) {
    m_depthControl->setRange(0, stack->depth() - 1);
    m_depthControl->setValue(stack->depth() / 2);
//    m_imageWidget->reset();

    if (updatingScreen) {
      redraw();
    }

#ifdef _ADVANCED_
    if (stack->isThresholdable()) {
      m_thresholdSlider->setRange(stack->min(), stack->max());
      m_thresholdSlider->setValue(stack->max());
    } else {
      hideThresholdControl();
    }
#endif
  }
  setInfo();
}

#if 0
void ZStackView::updateScrollControl()
{
  m_depthControl->setEnabled(m_scrollEnabled);

  //m_spinBox->setEnabled(m_scrollEnabled);
#ifdef _ADVANCED_
  //m_thresholdSlider->clearFocus();
#endif
}
#endif

void ZStackView::updateThresholdSlider()
{
#ifdef _ADVANCED_
  if (stackData() != NULL) {
    m_thresholdSlider->setRangeQuietly(stackData()->min(), stackData()->max());
    m_thresholdSlider->setValueQuietly(stackData()->max());
  }
#endif
}

void ZStackView::updateSlider()
{
  if (stackData() != NULL) {
    int value = m_depthControl->value();
    m_depthControl->setRangeQuietly(0, stackData()->depth() - 1);
    if (value >= stackData()->depth()) {
      m_depthControl->setValueQuietly(stackData()->depth() - 1);
    }
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
  ZStack *stack = stackData();
  if (stack != NULL) {
    if (!stack->isVirtual()) {
      std::vector<ZVec3Parameter*>& channelColors = stack->channelColors();
      for (int i=0; i<stack->channelNumber(); ++i) {
        m_chVisibleState.push_back(new ZBoolParameter("", true, this));
      }

      for (int ch=0; ch<stack->channelNumber(); ++ch) {
        QWidget *checkWidget = m_chVisibleState[ch]->createWidget();
        checkWidget->setFocusPolicy(Qt::NoFocus);
        m_channelControlLayout->addWidget(checkWidget, 0, Qt::AlignLeft);
        m_channelControlLayout->addWidget(
              channelColors[ch]->createNameLabel(),0,Qt::AlignLeft);
        ZClickableColorLabel *colorWidget = dynamic_cast<ZClickableColorLabel*>
            (channelColors[ch]->createWidget());
        colorWidget->setMinimumHeight(20);
        colorWidget->setMinimumWidth(30);
        colorWidget->setMaximumHeight(20);
        colorWidget->setMaximumWidth(30);
        colorWidget->setFocusPolicy(Qt::NoFocus);
        m_channelControlLayout->addWidget(colorWidget,0,Qt::AlignLeft);
        m_channelControlLayout->addSpacing(20);
        connect(channelColors[ch], SIGNAL(valueChanged()), this, SLOT(updateView()));
        connect(m_chVisibleState[ch], SIGNAL(valueChanged()), this, SLOT(updateView()));
      }
      m_channelControlLayout->addStretch(1);
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

#if 0
void ZStackView::updateData(int nslice, int threshold)
{
  /* get stack data */
  ZStack *stack = stackData();

  if (stack == NULL) {
    return;
  }

  if (threshold >= stack->max()) {
    threshold = -1;
  }

  if (stack != NULL) {
    size_t area = stack->width() * stack->height();
    if (m_image != NULL) {
      if ((m_image->width() != stack->width()) ||
          (m_image->height() != stack->height())) {
        delete(m_image);
        m_image = NULL;
      }
    }

    if (m_image == NULL) {
      m_image = new ZImage(stack->width(), stack->height());
    }

    //Set mask
    if (buddyDocument()->hasStackMask() &&
        buddyPresenter()->isObjectVisible()) {
      ZStack *stackMask = buddyDocument()->stackMask();
      TZ_ASSERT(stackMask->width() == stack->width() &&
                stackMask->height() == stack->height() &&
                stackMask->depth() == stack->depth(),
                "Inconsistent stack size");
      if (m_imageMask != NULL) {
        if (m_imageMask->width() != m_image->width() ||
            m_imageMask->height() != m_image->height()) {
          delete m_imageMask;
          m_imageMask = NULL;
        }
      }
      if (m_imageMask == NULL) {
        m_imageMask = m_image->createMask();
      }
      if (stackMask->kind() == GREY) {
        //m_imageMask->setData(stack->arrayc() + area * nslice, 100);
        if (stackMask->channelNumber() >= 3) {
          m_imageMask->set3ChannelData(
                stackMask->array8(0) + area * nslice, 1, 0,
                stackMask->array8(1) + area * nslice, 1, 0,
                stackMask->array8(2) + area * nslice, 1, 0, 100);
        } else if (stackMask->channelNumber() == 2) {
          m_imageMask->set2ChannelData(
                stackMask->array8(0) + area * nslice, 1, 0,
                stackMask->array8(1) + area * nslice, 1, 0, 100);
        } else {
          m_imageMask->setCData(stackMask->array8(0) + area * nslice, 100);
        }
      } else if (stackMask->kind() == GREY16) {
        m_imageMask->setCData(stackMask->array16(0) + area * nslice, 100);
      }
      if (buddyPresenter()->objectStyle() == ZStackObject::BOUNDARY) {
        m_imageMask->enhanceEdge();
      }
    }

    bool showImage = false;
    for (size_t i=0; i<m_chVisibleState.size(); ++i) {
      if (m_chVisibleState[i]->get()) {
        showImage = true;
        break;
      }
    }

    if (buddyPresenter()->interactiveContext().isNormalView()) {
      if (!stack->isVirtual() && showImage) {
        if (stack->channelNumber() == 1) {   //grey
          switch (stack->kind()) {
          case GREY:
            if (stack->isBinary()) {
              m_image->setBinaryData(stack->array8() + area * nslice,
                                     (uint8) (stack->min()), threshold);
            } else {
//              m_image->setData(stack->array8() + area * nslice,
//                                 buddyPresenter()->greyScale(),
//                                 buddyPresenter()->greyOffset(), threshold);
              ZImage::DataSource<uint8_t> stackData(stack->array8(0) + area * nslice,
                                                    buddyPresenter()->greyScale(0),
                                                    buddyPresenter()->greyOffset(0),
                                                    stack->getChannelColor(0));
              m_image->setData(stackData, threshold);

            }
            break;
          case GREY16:
            if (stack->isBinary()) {
              m_image->setBinaryData(stack->array16() + area * nslice,
                                     (uint16) (stack->min()), threshold);
            } else {
//              m_image->setData(stack->array16() + area * nslice,
//                               buddyPresenter()->greyScale(),
//                               buddyPresenter()->greyOffset(), threshold);
              ZImage::DataSource<uint16_t> stackData(stack->array16(0) + area * nslice,
                                                     buddyPresenter()->greyScale(0),
                                                     buddyPresenter()->greyOffset(0),
                                                     stack->getChannelColor(0));
              m_image->setData(stackData, threshold);
            }
            break;
          }
        } else { // multi channel image
          switch (stack->kind()) {
          case GREY: {
            std::vector<ZImage::DataSource<uint8_t> > stackData8;
            for (size_t i=0; i<m_chVisibleState.size(); ++i) {
              if (m_chVisibleState[i]->get()) {
                stackData8.push_back(ZImage::DataSource<uint8_t>(stack->array8(i) + area * nslice,
                                                                 buddyPresenter()->greyScale(i),
                                                                 buddyPresenter()->greyOffset(i),
                                                                 stack->getChannelColor(i)));
              }
            }
            m_image->setData(stackData8);
          }
//            if (stack->channelNumber() == 2) {
//              m_image->set2ChannelData(stack->array8(0) + area * nslice, buddyPresenter()->greyScale(0), buddyPresenter()->greyOffset(0),
//                                       stack->array8(1) + area * nslice, buddyPresenter()->greyScale(1), buddyPresenter()->greyOffset(1));
//            } else {
//              m_image->set3ChannelData(stack->array8(0) + area * nslice, buddyPresenter()->greyScale(0), buddyPresenter()->greyOffset(0),
//                                       stack->array8(1) + area * nslice, buddyPresenter()->greyScale(1), buddyPresenter()->greyOffset(1),
//                                       stack->array8(2) + area * nslice, buddyPresenter()->greyScale(2), buddyPresenter()->greyOffset(2));
//            }
            break;
          case GREY16: {
            std::vector<ZImage::DataSource<uint16_t> > stackData16;
            for (size_t i=0; i<m_chVisibleState.size(); ++i) {
              if (m_chVisibleState[i]->get()) {
                stackData16.push_back(ZImage::DataSource<uint16_t>(stack->array16(i) + area * nslice,
                                                                   buddyPresenter()->greyScale(i),
                                                                   buddyPresenter()->greyOffset(i),
                                                                   stack->getChannelColor(i)));
              }
            }
            m_image->setData(stackData16);
          }
//            if (stack->channelNumber() == 2) {
//              m_image->set2ChannelData(stack->array16(0) + area * nslice, buddyPresenter()->greyScale(0), buddyPresenter()->greyOffset(0),
//                                       stack->array16(1) + area * nslice, buddyPresenter()->greyScale(1), buddyPresenter()->greyOffset(1));
//            } else {
//              m_image->set3ChannelData(stack->array16(0) + area * nslice, buddyPresenter()->greyScale(0), buddyPresenter()->greyOffset(0),
//                                       stack->array16(1) + area * nslice, buddyPresenter()->greyScale(1), buddyPresenter()->greyOffset(1),
//                                       stack->array16(2) + area * nslice, buddyPresenter()->greyScale(2), buddyPresenter()->greyOffset(2));
//            }
            break;
          }
        }
      } else {
        m_image->setBackground();
      }
      m_scrollEnabled = true;
    } else if (buddyPresenter()->interactiveContext().isProjectView()) {
      m_scrollEnabled = false;
      if (!stack->isVirtual() && showImage) {
        if (stack->channelNumber() == 1) {
          Image_Array ima;
          ima.array = (uint8*) stack->projection(ZSingleChannelStack::MAX_PROJ, ZSingleChannelStack::Z_AXIS);

          switch (stack->kind()) {
          case GREY:
            if (stack->isBinary()) {
              m_image->setBinaryData(ima.array8, (uint8) (stack->min()), threshold);
            } else {
//              m_image->setData(ima.array8,
//                                 buddyPresenter()->greyScale(),
//                                 buddyPresenter()->greyOffset(), threshold);
              ZImage::DataSource<uint8_t> stackData(ima.array8,
                                                    buddyPresenter()->greyScale(0),
                                                    buddyPresenter()->greyOffset(0),
                                                    stack->getChannelColor(0));
              m_image->setData(stackData, threshold);
            }
            break;
          case GREY16:
            if (stack->isBinary()) {
              m_image->setBinaryData(ima.array16, (uint16) (stack->min()),
                                     threshold);
            } else {
//              m_image->setData(ima.array16,
//                               buddyPresenter()->greyScale(),
//                               buddyPresenter()->greyOffset(), threshold);
              ZImage::DataSource<uint16_t> stackData(ima.array16,
                                                     buddyPresenter()->greyScale(0),
                                                     buddyPresenter()->greyOffset(0),
                                                     stack->getChannelColor(0));
              m_image->setData(stackData, threshold);
            }
            break;
          }
        } else {    // for color image
          switch (stack->kind()) {
          case GREY: {
            std::vector<ZImage::DataSource<uint8_t> > stackData8;
            for (size_t i=0; i<m_chVisibleState.size(); ++i) {
              if (m_chVisibleState[i]->get()) {
                Image_Array ima;
                ima.array8 = (uint8*) stack->projection(ZSingleChannelStack::MAX_PROJ, ZSingleChannelStack::Z_AXIS, i);
                stackData8.push_back(ZImage::DataSource<uint8_t>(ima.array8,
                                                                 buddyPresenter()->greyScale(i),
                                                                 buddyPresenter()->greyOffset(i),
                                                                 stack->getChannelColor(i)));
              }
            }
            m_image->setData(stackData8);
          }
            break;
          case GREY16: {
            std::vector<ZImage::DataSource<uint16_t> > stackData16;
            for (size_t i=0; i<m_chVisibleState.size(); ++i) {
              if (m_chVisibleState[i]->get()) {
                Image_Array ima;
                ima.array16 = (uint16*) stack->projection(ZSingleChannelStack::MAX_PROJ, ZSingleChannelStack::Z_AXIS, i);
                stackData16.push_back(ZImage::DataSource<uint16_t>(ima.array16,
                                                                   buddyPresenter()->greyScale(i),
                                                                   buddyPresenter()->greyOffset(i),
                                                                   stack->getChannelColor(i)));
              }
            }
            m_image->setData(stackData16);
          }
            break;
          }

//          Image_Array ima0, ima1, ima2;
//          ima0.array = (uint8*) stack->projection(ZSingleChannelStack::MAX_PROJ, ZSingleChannelStack::Z_AXIS, 0);
//          ima1.array = (uint8*) stack->projection(ZSingleChannelStack::MAX_PROJ, ZSingleChannelStack::Z_AXIS, 1);
//          switch (stack->kind()) {
//          case GREY:
//            if (stack->channelNumber() == 2) {
//              m_image->set2ChannelData(ima0.array8, buddyPresenter()->greyScale(0), buddyPresenter()->greyOffset(0),
//                                       ima1.array8, buddyPresenter()->greyScale(1), buddyPresenter()->greyOffset(1));
//            } else {
//              ima2.array = (uint8*) stack->projection(ZSingleChannelStack::MAX_PROJ, ZSingleChannelStack::Z_AXIS, 2);
//              m_image->set3ChannelData(ima0.array8, buddyPresenter()->greyScale(0), buddyPresenter()->greyOffset(0),
//                                       ima1.array8, buddyPresenter()->greyScale(1), buddyPresenter()->greyOffset(1),
//                                       ima2.array8, buddyPresenter()->greyScale(2), buddyPresenter()->greyOffset(2));
//            }
//            break;
//          case GREY16:
//            if (stack->channelNumber() == 2) {
//              m_image->set2ChannelData(ima0.array16, buddyPresenter()->greyScale(0), buddyPresenter()->greyOffset(0),
//                                       ima1.array16, buddyPresenter()->greyScale(1), buddyPresenter()->greyOffset(1));
//            } else {
//              ima2.array = (uint8*) stack->projection(ZSingleChannelStack::MAX_PROJ, ZSingleChannelStack::Z_AXIS, 2);
//              m_image->set3ChannelData(ima0.array16, buddyPresenter()->greyScale(0), buddyPresenter()->greyOffset(0),
//                                       ima1.array16, buddyPresenter()->greyScale(1), buddyPresenter()->greyOffset(1),
//                                       ima2.array16, buddyPresenter()->greyScale(2), buddyPresenter()->greyOffset(2));
//            }
//            break;
//          }
        }
      } else {
        m_image->setBackground();
      }
    }

    updateScrollControl();

    if (buddyPresenter()->isObjectVisible()) {
      if (buddyPresenter()->interactiveContext().isProjectView()) {
        nslice = -1;
      }

      if (buddyPresenter()->interactiveContext().isReconPreview()) {
        if (buddyDocument()->previewSwc() != NULL) {
          buddyDocument()->previewSwc()->display(m_image, nslice,
                                               buddyPresenter()->objectStyle());
        }
      } else {
        if (buddyDocument()->hasDrawable()) {
          QList<ZStackObject*> *objs = buddyDocument()->drawableList();
          for (QList<ZStackObject*>::const_iterator obj = objs->end() - 1;
          obj != objs->begin() - 1; --obj) {
            (*obj)->display(m_image, nslice, buddyPresenter()->objectStyle());
          }
        }

        if (buddyPresenter()->hasObjectToShow()) {
          if (buddyPresenter()->interactiveContext().isProjectView()) {
            nslice = -1;
          }
          QList<ZStackObject*> *objs = buddyPresenter()->decorations();
          for (QList<ZStackObject*>::const_iterator obj = objs->end() - 1;
          obj != objs->begin() - 1; --obj) {
            (*obj)->display(m_image, nslice, buddyPresenter()->objectStyle());
          }
        }

        //For special handling
        if (!buddyDocument()->selectedSwcTreeNodes()->empty()) {
          for (std::set<Swc_Tree_Node*>::const_iterator iter =
               buddyDocument()->selectedSwcTreeNodes()->begin();
               iter != buddyDocument()->selectedSwcTreeNodes()->end(); ++iter) {
            ZCircle circle;
            circle.setColor(255, 255, 0);
            circle.set(SwcTreeNode::x(*iter), SwcTreeNode::y(*iter),
                       SwcTreeNode::z(*iter), SwcTreeNode::radius(*iter));
            circle.display(m_image, nslice);
          }
        }
      }
    }

    m_imageWidget->setImage(m_image);
    if (buddyPresenter()->isObjectVisible()) {
      m_imageWidget->setMask(m_imageMask, 0);
      m_imageWidget->setMask(m_objectCanvas, 1);
      m_imageWidget->setMask(m_activeStrokeCanvas, 2);
    } else {
      m_imageWidget->setMask(NULL, 0);
      m_imageWidget->setMask(NULL, 1);
    }

    updateImageScreen();
  }
}

void ZStackView::updateSlice(int nslice)
{
#ifdef _ADVANCED_
  updateData(nslice, getIntensityThreshold());
#else
  updateData(nslice);
#endif
}
#endif

/*
void ZStackView::viewThreshold(int threshold)
{
  updateData(m_depthControl->value(), threshold);
}
*/

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

ZStack* ZStackView::stackData()
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

int ZStackView::getCurrentZ() const
{
  return sliceIndex() + buddyDocument()->getStackOffset().getZ();
}

void ZStackView::setSliceIndex(int slice)
{
  if (!isDepthFronzen()) {
//    setDepthFrozen(true);
    m_depthControl->setValue(slice);
  }

  //emit viewChanged(getViewParameter(NeuTube::COORD_STACK));
}

void ZStackView::stepSlice(int step)
{
  int newIndex = sliceIndex() + step;
  if (newIndex < 0) {
    newIndex = 0;
  } else if (newIndex > maxSliceIndex()) {
    newIndex = maxSliceIndex();
  }

  if (newIndex != sliceIndex()) {
    setSliceIndex(newIndex);
  }
}

int ZStackView::getIntensityThreshold()
{
  int threshold = -1;
#ifdef _ADVANCED_
  if (m_thresholdSlider->value() < m_thresholdSlider->maximum()) {
    threshold = m_thresholdSlider->value();
  }
#endif

  return threshold;
}

void ZStackView::updatePaintBundle()
{
  m_paintBundle.unsetSwcNodeList();
  m_paintBundle.clearAllDrawableLists();
  if (buddyDocument()) {
    m_paintBundle.setStackOffset(buddyDocument()->getStackOffset());
  }

  int slice = m_depthControl->value();
  if (buddyPresenter()->interactiveContext().isProjectView()) {
    slice = -slice - 1;
  }
  m_paintBundle.setSliceIndex(slice);
  m_paintBundle.setDisplayStyle(buddyPresenter()->objectStyle());

  m_paintBundle.clearAllDrawableLists();
  // obj
  if (buddyPresenter()->isObjectVisible()) {
    if (buddyDocument()->hasDrawable()) {
      m_paintBundle.addDrawableList(buddyDocument()->drawableList());
    }

    if (buddyPresenter()->hasObjectToShow()) {
      m_paintBundle.addDrawableList(buddyPresenter()->decorations());
    }
  }

  // active deco
  m_paintBundle.addDrawableList(&(buddyPresenter()->getActiveDecorationList()));

}

void ZStackView::updateImageScreen()
{
#ifdef _DEBUG_
  std::cout << "ZStackView::updateImageScreen" << std::endl;
#endif

  updatePaintBundle();

  m_imageWidget->blockPaint(m_isRedrawBlocked ||
                            !buddyDocument()->isReadyForPaint());

  qDebug() << m_imageWidget->screenSize();
  m_imageWidget->update(QRect(QPoint(0, 0), m_imageWidget->screenSize()));
}

QSize ZStackView::sizeHint() const
{
  QSize viewSize = QWidget::sizeHint();

  switch (m_sizeHintOption) {
  case NeuTube::SIZE_HINT_CURRENT_BEST:
    //m_imageWidget->updateGeometry();
    viewSize = QWidget::sizeHint();
    break;
  case NeuTube::SIZE_HINT_TAKING_SPACE:
  {
    ZStackFrame *frame = getParentFrame();
    if (frame != NULL) {
      QMdiArea *area = frame->mdiArea();
      QSize space = area->size();
      QSize margin = QSize(30, 50);
      QSize frameSize = m_imageWidget->canvasSize() + margin;
      if (frameSize.isValid()) {
        int nw, nh;
        Raster_Ratio_Scale(frameSize.width(), frameSize.height(),
                           space.width(), space.height(), &nw, &nh);
        viewSize.setWidth(nw);
        viewSize.setHeight(nh);
      }
      viewSize -= margin;
    } else {
      viewSize = QWidget::sizeHint();
    }
  }
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
  buddyPresenter()->processMouseReleaseEvent(event);
}

void ZStackView::mousePressedInImageWidget(QMouseEvent *event)
{
  buddyPresenter()->processMousePressEvent(event);
}

void ZStackView::mouseMovedInImageWidget(QMouseEvent *event)
{
  buddyPresenter()->processMouseMoveEvent(event);
}

void ZStackView::mouseDoubleClickedInImageWidget(QMouseEvent *event)
{
  buddyPresenter()->processMouseDoubleClickEvent(event);
}

bool ZStackView::isDepthChangable()
{
  return m_depthControl->isEnabled();
}

void ZStackView::mouseRolledInImageWidget(QWheelEvent *event)
{
  int numSteps = event->delta();

#if !defined(_NEUTUBE_MAC_)
  numSteps = -numSteps;
#endif


  if ((abs(numSteps) > 0) && (abs(numSteps) < 120)) {
    if (numSteps > 0) {
      numSteps = 1;
    } else {
      numSteps = -1;
    }
  } else {
    numSteps /= 120;
  }

  if (event->modifiers() == Qt::NoModifier ||
      event->modifiers() == Qt::ShiftModifier) {
    if (isDepthChangable()) {
      //for strange mighty mouse response in Qt 4.6.2
      if (numSteps != 0) {
        int ratio = 1;
        if (event->modifiers() == Qt::ShiftModifier) {
          ratio = 5;
        }

        int newPos = m_depthControl->value() + numSteps * ratio;
        if ((newPos >= m_depthControl->minimum()) &&
            (newPos <= m_depthControl->maximum())) {
          setSliceIndex(newPos);
          //m_depthControl->setValue(newPos);

          QPointF pos = imageWidget()->canvasCoordinate(event->pos());
          int z = sliceIndex();
          if (buddyPresenter()->interactiveContext().isProjectView()) {
            z = -1;
          }
          setInfo(buddyDocument()->dataInfo(pos.x(), pos.y(), z));
        }
      }
    }
  } else if (event->modifiers() == Qt::ControlModifier) {
    if (numSteps > 0) {
      increaseZoomRatio(event->pos().x(), event->pos().y());
    } else if (numSteps < 0) {
      decreaseZoomRatio(event->pos().x(), event->pos().y());
    }
  }
}

void ZStackView::resizeEvent(QResizeEvent *event)
{
  setInfo();
  event->accept();
  //buddyPresenter()->updateInteractiveContext();
}

void ZStackView::redrawObject()
{
  paintObjectBuffer();
  updateImageScreen();
}

void ZStackView::redraw()
{
//  tic();
  QElapsedTimer timer;
  timer.start();
  m_imageWidget->setCanvasRegion(
        buddyDocument()->getStackOffset().getX(),
        buddyDocument()->getStackOffset().getY(),
        buddyDocument()->getStackSize().getX(),
        buddyDocument()->getStackSize().getY());

  paintStackBuffer();
  std::cout << "paint stack per frame: " << timer.elapsed() << std::endl;
  paintMaskBuffer();
  paintTileCanvasBuffer();
  std::cout << "paint tile per frame: " << timer.elapsed() << std::endl;
  paintActiveDecorationBuffer();
  paintObjectBuffer();
  std::cout << "paint object per frame: " << timer.elapsed() << std::endl;

  updateImageScreen();
//  std::cout << "paint time per frame: " << toc() << std::endl;
  std::cout << "paint time per frame: " << timer.elapsed() << std::endl;
}

/*
void ZStackView::prepareDocument()
{
}
*/

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

  list.append(QString("Offset: %1, %2")
              .arg(imageWidget()->viewPort().left())
              .arg(imageWidget()->viewPort().top()));

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

/*
void ZStackView::resetScreenCursor()
{
  imageWidget()->setCursor(Qt::CrossCursor);
}
*/
void ZStackView::setThreshold(int thre)
{
#ifdef _ADVANCED_
  m_thresholdSlider->setValue(thre);
#else
  UNUSED_PARAMETER(thre);
#endif
}

void ZStackView::takeScreenshot(const QString &filename)
{
  QImageWriter writer(filename);
  writer.setCompression(1);

  QImage image(m_imageWidget->projectSize(), QImage::Format_ARGB32);

  m_imageWidget->setViewHintVisible(false);
  m_imageWidget->render(&image);
  m_imageWidget->setViewHintVisible(true);
  ZImage::writeImage(image, filename);

//  const QRect& viewPort = m_imageWidget->viewPort();
//  if(!writer.write(m_image->copy(viewPort))) {
//  if(!writer.write(image)) {
//    LERROR() << writer.errorString();
//  } else {
//    LINFO() << "wrote screenshot:" << filename;
//  }
}

void ZStackView::updateView()
{
  redraw();
}

void ZStackView::displayActiveDecoration(bool display)
{
  m_activeLabel->setVisible(display);
}

void ZStackView::paintSingleChannelStackSlice(ZStack *stack, int slice)
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
        ZSingleChannelStack::MAX_PROJ, ZSingleChannelStack::Z_AXIS);

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
              ZSingleChannelStack::MAX_PROJ, ZSingleChannelStack::Z_AXIS, i);
        stackData8.push_back(ZImage::DataSource<uint8_t>(ima.array8,
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
              ZSingleChannelStack::MAX_PROJ, ZSingleChannelStack::Z_AXIS, i);
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
  m_imagePainter.end();
  delete m_image;
  m_image = NULL;

  m_imageWidget->setImage(NULL);
}

template<typename T>
void ZStackView::resetCanvasWithStack(T &canvas, ZPainter *painter)
{
  if (canvas != NULL) {
    if (canvas->width() != buddyDocument()->getStackWidth() ||
        canvas->height() != buddyDocument()->getStackHeight() ||
        iround(canvas->getTransform().getTx()) !=
        -buddyDocument()->getStackOffset().getX() ||
        iround(canvas->getTransform().getTy()) !=
        -buddyDocument()->getStackOffset().getY()) {
      if (painter != NULL) {
        painter->end();
      }
      m_imageWidget->removeCanvas(canvas);
      delete canvas;
      canvas = NULL;
    }
  }
}

void ZStackView::updateImageCanvas()
{
  resetCanvasWithStack(m_image, &m_imagePainter);
  if (buddyDocument()->hasStackPaint()) {
    if (m_image != NULL) {
      if ((m_image->width() != buddyDocument()->getStackWidth()) ||
          (m_image->height() != buddyDocument()->getStackHeight())) {
        clearCanvas();
      }
    }

    if (m_image == NULL) {
//      double scale = 0.5;
      m_image = new ZImage(buddyDocument()->getStackWidth(),
                           buddyDocument()->getStackHeight());
      m_image->setOffset(-buddyDocument()->getStackOffset().getX(),
                         -buddyDocument()->getStackOffset().getY());
//      m_image->setScale(scale, scale);
      m_imagePainter.begin(m_image);
      m_imagePainter.setZOffset(buddyDocument()->getStackOffset().getZ());
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
  m_objectCanvasPainter.end();
  delete m_objectCanvas;
  m_objectCanvas = NULL;

  m_imageWidget->setObjectCanvas(NULL);
}

void ZStackView::clearTileCanvas()
{
  m_tileCanvasPainter.end();
  delete m_tileCanvas;
  m_tileCanvas = NULL;

  m_imageWidget->setTileCanvas(NULL);
}

QSize ZStackView::getCanvasSize() const
{
  QSize size(0, 0);
  if (buddyDocument()->hasStack()) {
    size.setWidth(buddyDocument()->getStackWidth());
    size.setHeight(buddyDocument()->getStackHeight());
  }

  return size;
}

void ZStackView::updateObjectCanvas()
{
  resetCanvasWithStack(m_objectCanvas, &m_objectCanvasPainter);

  QSize canvasSize = getCanvasSize();

  if (!canvasSize.isEmpty() &&
      buddyDocument()->hasDrawable(ZStackObject::OBJECT_CANVAS)) {
    if (m_objectCanvas != NULL) {
      if (m_objectCanvas->width() != canvasSize.width() ||
          m_objectCanvas->height() != canvasSize.height()) {
        clearObjectCanvas();
      }
    }
    if (m_objectCanvas == NULL) {
//      m_objectCanvas = ZImage::createMask(canvasSize);
      m_objectCanvas = new ZPixmap(canvasSize);
      m_objectCanvas->setOffset(-buddyDocument()->getStackOffset().getX(),
                                -buddyDocument()->getStackOffset().getY());
      m_objectCanvas->clearnUp();
      m_objectCanvasPainter.begin(m_objectCanvas);
      m_objectCanvasPainter.setCompositionMode(
            QPainter::CompositionMode_SourceOver);
      m_imageWidget->setObjectCanvas(m_objectCanvas);
//      m_imageWidget->setMask(m_objectCanvas, 1);
    } else {
      m_objectCanvasPainter.end();
      m_objectCanvas->clearnUp();
      m_objectCanvasPainter.begin(m_objectCanvas);
//      m_objectCanvas->setAlphaChannel(
#ifdef _DEBUG_2
      m_objectCanvas->save((GET_TEST_DATA_DIR + "/test.tif").c_str());
#endif
//      m_objectCanvas->fill(0);
    }
    m_objectCanvasPainter.setZOffset(buddyDocument()->getStackOffset().getZ());
  } else {
    if (m_objectCanvas != NULL) {
      m_objectCanvas->setVisible(false);
    }
  }
}

void ZStackView::updateTileCanvas()
{
  resetCanvasWithStack(m_tileCanvas, &m_tileCanvasPainter);

  QSize canvasSize = getCanvasSize();

  if (!canvasSize.isEmpty() &&
      buddyDocument()->hasDrawable(ZStackObject::TILE_CANVAS)) {
    if (m_tileCanvas != NULL) {
      if (m_tileCanvas->width() != canvasSize.width() ||
          m_tileCanvas->height() != canvasSize.height()) {
        clearTileCanvas();
      }
    }
    if (m_tileCanvas == NULL) {
//      double scale = 0.5;
//      m_tileCanvas = new ZPixmap(canvasSize * scale);
//      m_tileCanvas->setScale(scale, scale);
      m_tileCanvas = new ZPixmap(canvasSize);
      m_tileCanvasPainter.begin(m_tileCanvas);
      m_imageWidget->setTileCanvas(m_tileCanvas);
    }

    //m_tileCanvas->fill(Qt::transparent);
  }
}

void ZStackView::updateActiveDecorationCanvas()
{
//  if (m_activeDecorationCanvas != NULL) {
//    if (m_activeDecorationCanvas->width() != m_image->width() ||
//        m_activeDecorationCanvas->height() != m_image->height()) {
//      delete m_activeDecorationCanvas;
//      m_activeDecorationCanvas = NULL;
//    }
//  }

  resetCanvasWithStack(m_activeDecorationCanvas, NULL);

  if (m_activeDecorationCanvas == NULL) {
    QSize canvasSize = getCanvasSize();
    if (!canvasSize.isEmpty()) {
      m_activeDecorationCanvas = new ZPixmap(canvasSize);//m_image->createMask();
      m_activeDecorationCanvas->setOffset(
            -buddyDocument()->getStackOffset().getX(),
            -buddyDocument()->getStackOffset().getY());
      m_imageWidget->setActiveDecorationCanvas(m_activeDecorationCanvas);
//      m_imageWidget->setMask(m_activeDecorationCanvas, 2);
    }
  }

  if (m_activeDecorationCanvas != NULL) {
    m_activeDecorationCanvas->clearnUp();
  }
}

void ZStackView::paintMultiresImageTest(int resLevel)
{
  ZStackPatch *patch = new ZStackPatch(
        ZStackFactory::makeSlice(*(buddyDocument()->getStack()), getCurrentZ()));
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
#if 0
  updateCanvas();

  paintMultiresImageTest(1);
#endif

#if 1
  ZStack *stack = stackData();

  if (stack == NULL) {
    return;
  }

  updateImageCanvas();

  bool showImage = false;
  for (size_t i=0; i<m_chVisibleState.size(); ++i) {
    if (m_chVisibleState[i]->get()) {
      showImage = true;
      break;
    }
  }

  if (!showImage) {
    if (!buddyDocument()->hasSparseStack()) {
      return;
    }
  }

  if (buddyPresenter() != NULL) {
    if (buddyPresenter()->interactiveContext().isNormalView()) {
      if (!stack->isVirtual() && showImage) {
        if (stack->channelNumber() == 1) {   //grey
          paintSingleChannelStackSlice(stack, m_depthControl->value());
        } else { // multi channel image
          paintMultipleChannelStackSlice(stack, m_depthControl->value());
        }
      } else {
        m_image->setBackground();
        paintObjectBuffer(m_imagePainter, ZStackObject::STACK_CANVAS);

        if (buddyDocument()->hasSparseStack()) {
          ZStack *slice =
              buddyDocument()->getSparseStack()->getSlice(getCurrentZ());
          //paintSingleChannelStackSlice(slice, 0);
          slice->translate(-buddyDocument()->getStackOffset());
          slice->getOffset().setZ(0);

          m_image->setData(slice, 0, true);
          /*
          buddyDocument()->getSparseStack()->getObjectMask()->display(
                m_imagePainter, sliceIndex(), ZStackObject::BOUNDARY);
                */
          delete slice;
        }
      }
      //m_scrollEnabled = true;
    } else if (buddyPresenter()->interactiveContext().isProjectView()) {
      //m_scrollEnabled = false;
      if (!stack->isVirtual() && showImage) {
        if (stack->channelNumber() == 1) {
          paintSingleChannelStackMip(stack);
        } else {    // for color image
          paintMultipleChannelStackMip(stack);
        }
      } else {
        m_image->setBackground();
        if (buddyDocument()->hasSparseStack()) {
          ZStack *slice =
              buddyDocument()->getSparseStack()->getMip();
          if (slice != NULL) {
            slice->translate(-buddyDocument()->getStackOffset());
            slice->getOffset().setZ(0);

            m_image->setData(slice, 0);
            delete slice;
          }
        }
      }
    }
  }
#endif
}

void ZStackView::paintStack()
{
  paintStackBuffer();
  updateImageScreen();
}

void ZStackView::paintMaskBuffer()
{
  ZStack *stackMask = buddyDocument()->stackMask();
  if (stackMask == NULL) {
    return;
  }

  updateMaskCanvas();

  int slice = m_depthControl->value();
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
  if (buddyPresenter()->objectStyle() == ZStackObject::BOUNDARY) {
    m_imageMask->enhanceEdge();
  }
}

void ZStackView::paintMask()
{
  paintMaskBuffer();
  updateImageScreen();
}

void ZStackView::paintObjectBuffer(
    ZPainter &painter, ZStackObject::ETarget target)
{
#ifdef _DEBUG_2
    qDebug() << painter.getTransform();
#endif

  painter.setPainted(false);

  bool visible = true;
  if (target == ZStackObject::OBJECT_CANVAS) {
    visible = buddyPresenter()->isObjectVisible();
  }

  if (visible) {
    int slice = m_depthControl->value();
    int z = slice + buddyDocument()->getStackOffset().getZ();
    if (buddyPresenter()->interactiveContext().isProjectView()) {
      slice = -slice - 1;
    }

//    painter.setStackOffset(buddyDocument()->getStackOffset());

    if (buddyDocument()->hasDrawable()) {
      QList<ZStackObject*> *objs = buddyDocument()->drawableList();
      QList<const ZStackObject*> visibleObject;
      QList<ZStackObject*>::const_iterator iter = objs->end() - 1;
      for (;iter != objs->begin() - 1; --iter) {
        const ZStackObject *obj = *iter;
        if ((obj->isSliceVisible(z) || slice < 0) &&
            obj->getTarget() == target) {
          visibleObject.append(obj);
        }
      }
      std::sort(visibleObject.begin(), visibleObject.end(),
                ZStackObject::ZOrderCompare());

#ifdef _DEBUG_2
      std::cout << "---" << std::endl;
      std::cout << slice << " " << m_depthControl->value() <<  std::endl;
#endif

      for (QList<const ZStackObject*>::const_iterator
           iter = visibleObject.begin(); iter != visibleObject.end(); ++iter) {
        //(*obj)->display(m_objectCanvas, slice, buddyPresenter()->objectStyle());
        const ZStackObject *obj = *iter;
        if (slice == m_depthControl->value() || slice < 0) {
          obj->display(painter, slice, buddyPresenter()->objectStyle());
//          painted = true;
        }
      }
    }

    if (buddyPresenter()->hasObjectToShow()) {
//      if (buddyPresenter()->interactiveContext().isProjectView()) {
//        slice = -m_depthControl->value() - 1;
//      }
      QList<ZStackObject*> *objs = buddyPresenter()->decorations();
      for (QList<ZStackObject*>::const_iterator obj = objs->end() - 1;
           obj != objs->begin() - 1; --obj) {
        //(*obj)->display(m_objectCanvas, slice, buddyPresenter()->objectStyle());
        if ((*obj)->getTarget() == target) {
          (*obj)->display(painter, slice, buddyPresenter()->objectStyle());
//          painted = true;
        }
      }
    }
  }

//  return painted;
}

void ZStackView::paintObjectBuffer()
{
#ifdef _DEBUG_
  std::cout << "ZStackView::paintObjectBuffer" << std::endl;
#endif

  updateObjectCanvas();
  if (m_objectCanvas == NULL) {
    return;
  }

  paintObjectBuffer(m_objectCanvasPainter, ZStackObject::OBJECT_CANVAS);

  if (m_objectCanvasPainter.isPainted()) {
    m_objectCanvas->setVisible(true);
  }
}

bool ZStackView::paintTileCanvasBuffer()
{
#ifdef _DEBUG_
  std::cout << "ZStackView::paintTileCanvasBuffer" << std::endl;
#endif
  bool painted = false;

//  QElapsedTimer timer;
//  timer.start();
  if (buddyDocument()->hasObject(ZStackObject::TYPE_DVID_TILE_ENSEMBLE)) {
    updateTileCanvas();
    //std::cout << "update time canvas time: " << timer.elapsed() << std::endl;
    if (m_tileCanvas != NULL) {
      paintObjectBuffer(m_tileCanvasPainter, ZStackObject::TILE_CANVAS);
      painted = true;
    }
    //std::cout << "paint tile time: " << timer.elapsed() << std::endl;
  }

//  setDepthFrozen(false);
  setViewPortFrozen(false);

  return painted;
}

void ZStackView::paintObject()
{
  paintObjectBuffer();
  updateImageScreen();
}

void ZStackView::paintActiveTile()
{
  if (paintTileCanvasBuffer()) {
    updateImageScreen();
  }
}

void ZStackView::paintObject(
    QList<ZStackObject *> selected,
    QList<ZStackObject *> deselected)
{
  bool updatingObjectCanvas = false;
  bool updatingImageCanvas = false;
  foreach (ZStackObject *obj, selected) {
    if (obj->getTarget() == ZStackObject::OBJECT_CANVAS) {
      updatingObjectCanvas = true;
    } else if (obj->getTarget() == ZStackObject::STACK_CANVAS) {
      updatingImageCanvas = true;
    }
  }

  foreach (ZStackObject *obj, deselected) {
    if (obj->getTarget() == ZStackObject::OBJECT_CANVAS) {
      updatingObjectCanvas = true;
    } else if (obj->getTarget() == ZStackObject::STACK_CANVAS) {
      updatingImageCanvas = true;
    }
  }

  if (updatingObjectCanvas) {
    paintObjectBuffer();
  }

  if (updatingImageCanvas) {
    paintStackBuffer();
  }

  updateImageScreen();
}

void ZStackView::paintActiveDecorationBuffer()
{
#if 1
//  bool painted = false;

  updateActiveDecorationCanvas();
  const QList<ZStackObject*>& drawableList =
      buddyPresenter()->getActiveDecorationList();

  if (!drawableList.isEmpty()) {
    if (m_activeDecorationCanvas != NULL) {
      ZPainter painter(m_activeDecorationCanvas);
      painter.setStackOffset(buddyDocument()->getStackOffset());

      foreach (ZStackObject *obj, drawableList) {
        if (obj->getTarget() == ZStackObject::OBJECT_CANVAS) {
          obj->display(painter, sliceIndex(), ZStackObject::NORMAL);
//          painted = true;
        }
      }
      if (painter.isPainted()) {
        m_activeDecorationCanvas->setVisible(true);
      }
    }
  }

//  return painted;
#endif
}

void ZStackView::paintActiveDecoration()
{
  paintActiveDecorationBuffer();
  updateImageScreen();
}

ZStack* ZStackView::getStrokeMask(uint8_t maskValue)
{
#if 0
  if (m_objectCanvas != NULL){
    updateObjectCanvas();

    int slice = m_depthControl->value();
    if (buddyPresenter()->interactiveContext().isProjectView()) {
      slice = -1;
    }

    foreach (ZStroke2d *obj, buddyDocument()->getStrokeList()) {
      obj->display(m_objectCanvas, m_imageWidget->viewPort().x(), m_imageWidget->viewPort().y(),
                   m_imageWidget->projectSize().width() * 1.0 / m_imageWidget->viewPort().width(),
                   slice, buddyPresenter()->objectStyle());
    }
  }

  ZStack *stack = getObjectMask(maskValue);

  paintObjectBuffer();

  return stack;
#else
  if (m_objectCanvas != NULL){
      updateObjectCanvas();

      int slice = m_depthControl->value();
      if (buddyPresenter()->interactiveContext().isProjectView()) {
        slice = -slice - 1;
      }

      foreach (ZStroke2d *obj, buddyDocument()->getStrokeList()) {
//        ZPainter painter(m_objectCanvas);
//        painter.setZOffset(buddyDocument()->getStackOffset().getZ());
//        painter.setStackOffset(buddyDocument()->getStackOffset());
        obj->display(m_objectCanvasPainter,
                     slice, buddyPresenter()->objectStyle());
      }
    }

    ZStack *stack = getObjectMask(maskValue);

    paintObjectBuffer();

    return stack;
#endif
}

ZStack* ZStackView::getObjectMask(uint8_t maskValue)
{
  ZStack *stack = NULL;

  if (m_objectCanvas != NULL) {
    QImage image;
    image = m_objectCanvas->toImage();

    stack = new ZStack(GREY, m_objectCanvas->width(),
                       m_objectCanvas->height(), 1, 1);
    size_t offset = 0;
    uint8_t *array = stack->array8();
    for (int y = 0; y < m_objectCanvas->height(); ++y) {
      for (int x = 0; x < m_objectCanvas->width(); ++x) {
        QRgb rgb = image.pixel(x, y);
        if (qRed(rgb) > 0) {
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

ZStack* ZStackView::getObjectMask(NeuTube::EColor color, uint8_t maskValue)
{
  ZStack *stack = NULL;

  if (m_objectCanvas != NULL) {
    stack = new ZStack(GREY, m_objectCanvas->width(),
                       m_objectCanvas->height(), 1, 1);
    size_t offset = 0;
    uint8_t *array = stack->array8();
    QImage image = m_objectCanvas->toImage();
    for (int y = 0; y < m_objectCanvas->height(); ++y) {
      for (int x = 0; x < m_objectCanvas->width(); ++x) {
        QRgb rgb = image.pixel(x, y);
        bool isForeground = false;
        switch (color) {
        case NeuTube::RED:
          if ((qRed(rgb) > qGreen(rgb)) && (qRed(rgb) > qBlue(rgb))) {
            isForeground = true;
          }
          break;
        case NeuTube::GREEN:
          if ((qGreen(rgb) > qRed(rgb)) && (qGreen(rgb) > qBlue(rgb))) {
            isForeground = true;
          }
          break;
        case NeuTube::BLUE:
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
    NeuTube::EColor color, const string &filePath)
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

void ZStackView::increaseZoomRatio()
{
  increaseZoomRatio(0, 0, false);
  /*
  if (!isViewPortFronzen()) {
    setViewPortFrozen(true);
    imageWidget()->increaseZoomRatio();

    notifyViewChanged();
    notifyViewPortChanged();
  }
  */
}

void ZStackView::decreaseZoomRatio()
{
  decreaseZoomRatio(0, 0, false);
  /*
  if (!isViewPortFronzen()) {
    setViewPortFrozen(true);
    imageWidget()->decreaseZoomRatio();

    notifyViewChanged();
    notifyViewPortChanged();
  }
  */
}


void ZStackView::increaseZoomRatio(int x, int y, bool usingRef)
{
  if (!isViewPortFronzen()) {
    setViewPortFrozen(true);
    imageWidget()->increaseZoomRatio(x, y, usingRef);

    notifyViewChanged();
    notifyViewPortChanged();
  }
}

void ZStackView::decreaseZoomRatio(int x, int y, bool usingRef)
{
  if (!isViewPortFronzen()) {
    setViewPortFrozen(true);
    imageWidget()->decreaseZoomRatio(x, y, usingRef);

    notifyViewChanged();
    notifyViewPortChanged();
  }
}

int ZStackView::getZ(NeuTube::ECoordinateSystem coordSys) const
{
  int z = sliceIndex();
  if (coordSys == NeuTube::COORD_STACK) {
    z += buddyDocument()->getStackOffset().getZ();
  }

  return z;
}

QRect ZStackView::getViewPort(NeuTube::ECoordinateSystem coordSys) const
{
  QRect rect = m_imageWidget->viewPort();
  if (coordSys == NeuTube::COORD_RAW_STACK) {
    rect.translate(QPoint(-buddyDocument()->getStackOffset().getX(),
                          -buddyDocument()->getStackOffset().getY()));
  }

  return rect;
}

ZStackViewParam ZStackView::getViewParameter(
    NeuTube::ECoordinateSystem coordSys) const
{
  ZStackViewParam param(coordSys);
  param.setZ(getZ(coordSys));
  param.setViewPort(getViewPort(coordSys));
  //param.setViewPort(imageWidget()->viewPort());

  return param;
}

void ZStackView::setViewPortOffset(int x, int y)
{
  imageWidget()->setViewPortOffset(x, y);
  notifyViewChanged();
}

void ZStackView::setView(const ZStackViewParam &param)
{
  switch (param.getCoordinateSystem()) {
  case NeuTube::COORD_RAW_STACK:
  {
    QRect viewPort = param.getViewPort();
    viewPort.translate(QPoint(buddyDocument()->getStackOffset().getX(),
                              buddyDocument()->getStackOffset().getY()));
    m_imageWidget->setViewPort(param.getViewPort());
    setSliceIndex(param.getZ());
  }
    break;
  case NeuTube::COORD_STACK:
  {
    QRect viewPort = param.getViewPort();
    m_imageWidget->setViewPort(viewPort);
    setSliceIndex(param.getZ() - buddyDocument()->getStackOffset().getZ());
  }
    break;
  default:
    break;
  }

  updateView();
}

void ZStackView::processDepthSliderValueChange(int /*sliceIndex*/)
{
  redraw();

  notifyViewChanged();
}

void ZStackView::notifyViewChanged()
{
  notifyViewChanged(getViewParameter(NeuTube::COORD_STACK));
}

void ZStackView::notifyViewChanged(const ZStackViewParam &param)
{
#ifdef _DEBUG_2
  std::cout << "Signal: ZStackView::viewChanged" << std::endl;
#endif

  emit viewChanged(param);
}

void ZStackView::notifyViewPortChanged()
{
  emit viewPortChanged();
}

bool ZStackView::isImageMovable() const
{
  return (imageWidget()->viewPort() != imageWidget()->canvasRegion());
//  return (imageWidget()->viewPort().top() != 0 ||
//      imageWidget()->viewPort().left() != 0 ||
//      imageWidget()->viewPort().bottom() !=
//      imageWidget()->canvasSize().height() - 1 ||
//      imageWidget()->viewPort().right() !=
//      imageWidget()->canvasSize().width() - 1);
}

void ZStackView::customizeWidget()
{
  if (buddyDocument()->getTag() == NeuTube::Document::FLYEM_MERGE) {
    QPushButton *mergeButton = new QPushButton(this);
    mergeButton->setText("Merge");
    m_secondTopLayout->addWidget(mergeButton);
    connect(mergeButton, SIGNAL(clicked()), this, SLOT(requestMerge()));

    m_splitButton = new ZBodySplitButton(this);
    m_secondTopLayout->addWidget(m_splitButton);

    QPushButton *quickVis3dButton = new QPushButton(this);
    quickVis3dButton->setText("Low-res 3D");
    m_secondTopLayout->addWidget(quickVis3dButton);
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
    QPushButton *vis3dButton = new QPushButton(this);
    vis3dButton->setText("3D");
    m_secondTopLayout->addWidget(vis3dButton);
    connect(vis3dButton, SIGNAL(clicked()), this, SLOT(request3DVis()));
  }
}

void ZStackView::request3DVis()
{
  if (m_messageManager != NULL) {
    ZMessage message(this);
    if (buddyDocument()->getTag() == NeuTube::Document::FLYEM_SPLIT) {
      ZMessageFactory::MakeFlyEmSplit3DVisMessage(message);
    } else {
      ZMessageFactory::Make3DVisMessage(message);
    }
    //message.setBodyEntry("title", "3dvis");
    //message.setBodyEntry("body", "message test");

    m_messageManager->processMessage(&message, true);
  }
}

void ZStackView::requestQuick3DVis()
{
  if (m_messageManager != NULL) {
    ZMessage message(this);
    if (buddyDocument()->getTag() == NeuTube::Document::FLYEM_MERGE) {
      ZMessageFactory::MakeQuick3DVisMessage(message, 1);
    }
    m_messageManager->processMessage(&message, true);
  }
}

void ZStackView::requestHighresQuick3DVis()
{
  if (m_messageManager != NULL) {
    ZMessage message(this);
    if (buddyDocument()->getTag() == NeuTube::Document::FLYEM_MERGE) {
      ZMessageFactory::MakeQuick3DVisMessage(message, 0);
    }
    m_messageManager->processMessage(&message, true);
  }
}

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
    ZMessage */*message*/, QWidget */*host*/) const
{
#ifdef _DEBUG_
  std::cout << "ZStackView::MessageProcessor::processMessage" << std::endl;
#endif
}

ZStackFrame* ZStackView::getParentFrame() const
{
  return dynamic_cast<ZStackFrame*>(parent());
}

ZStackMvc* ZStackView::getParentMvc() const
{
  return dynamic_cast<ZStackMvc*>(parent());
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

void ZStackView::setDepthFrozen(bool state)
{
  m_depthFrozen = state;
}
