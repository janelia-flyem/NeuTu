#include "zstackyzview.h"
#include "zimage.h"
#include "zslider.h"
#include "widgets/zimagewidget.h"

ZStackYZView::ZStackYZView(QWidget *parent) :
  ZStackView(parent)
{ 
}

ZStackYZView::ZStackYZView(ZStackFrame *parent) : ZStackView(parent)
{

}

ZStackYZView::~ZStackYZView()
{

}

int ZStackYZView::getDepth() const
{
  ZStack *stack = stackData();
  if (stack != NULL) {
    return stack->width();
  }

  return 0;
}

void ZStackYZView::paintSingleChannelStackSlice(ZStack *stack, int slice)
{
//  void *dataArray = stack->getDataPointer(0, slice);

  switch (stack->kind()) {
  case GREY:
    m_image->setData(stack->array8(), slice,
                     stack->depth(), NeuTube::X_AXIS);
    break;
  default:
    break;
  }
}

template<typename T>
void ZStackYZView::resetCanvasWithStack(T &canvas, ZPainter *painter)
{
  if (canvas != NULL) {
    if (canvas->width() != buddyDocument()->getStackHeight() ||
        canvas->height() != buddyDocument()->getStackDepth() ||
        iround(canvas->getTransform().getTx()) !=
        -buddyDocument()->getStackOffset().getY() ||
        iround(canvas->getTransform().getTy()) !=
        -buddyDocument()->getStackOffset().getZ()) {
      if (painter != NULL) {
        painter->end();
      }
      m_imageWidget->removeCanvas(canvas);
      delete canvas;
      canvas = NULL;
    }
  }
}

void ZStackYZView::updateImageCanvas()
{
  resetCanvasWithStack(m_image, &m_imagePainter);
  if (buddyDocument()->hasStackPaint()) {
    if (m_image != NULL) {
      m_image->setOffset(-buddyDocument()->getStackOffset().getY(),
                         -buddyDocument()->getStackOffset().getZ());
      if ((m_image->width() != buddyDocument()->getStackHeight()) ||
          (m_image->height() != buddyDocument()->getStackDepth())) {
        clearCanvas();
      }
    }

    if (m_image == NULL) {
//      double scale = 0.5;
      m_image = new ZImage(buddyDocument()->getStackHeight(),
                           buddyDocument()->getStackDepth());
      m_image->setOffset(-buddyDocument()->getStackOffset().getY(),
                         -buddyDocument()->getStackOffset().getZ());
//      m_image->setScale(scale, scale);
      m_imagePainter.begin(m_image);
      m_imagePainter.setZOffset(buddyDocument()->getStackOffset().getX());
      m_imageWidget->setImage(m_image);
    }
  }
}

void ZStackYZView::paintStackBuffer()
{
  ZStack *stack = stackData();

  if (stack == NULL) {
    return;
  }

  bool showImage = false;
  for (size_t i=0; i<m_chVisibleState.size(); ++i) {
    if (m_chVisibleState[i]->get()) {
      showImage = true;
      break;
    }
  }

  if (!showImage) {
    if (!buddyDocument()->hasVisibleSparseStack()) {
      return;
    }
  }

  updateImageCanvas();

  if (buddyPresenter() != NULL) {
    if (!stack->isVirtual() && showImage) {
      if (stack->channelNumber() == 1) {   //grey
        paintSingleChannelStackSlice(stack, m_depthControl->value());
      }
    }
  }
}
