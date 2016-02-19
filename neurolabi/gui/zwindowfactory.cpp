#include "zwindowfactory.h"
#include <QApplication>
#include <QDesktopWidget>
#include <QMessageBox>
#include "z3dapplication.h"
#include "neutubeconfig.h"
#include "z3dcompositor.h"
#include "z3dcanvas.h"
#include "z3dvolumeraycaster.h"
#include "zscalablestack.h"
#include "z3dvolume.h"
#include "z3dvolumesource.h"
#include "z3dwindow.h"
#include "z3dswcfilter.h"
#include "z3dpunctafilter.h"
#include "z3dutils.h"

ZWindowFactory::ZWindowFactory()
{
  init();
}

ZWindowFactory::~ZWindowFactory()
{

}

void ZWindowFactory::init()
{
  m_parentWidget = NULL;
  m_showVolumeBoundBox = false;
  m_showControlPanel = true;
  m_showObjectView = true;
  m_volumeMode = NeuTube3D::VR_AUTO;
}

Z3DWindow* ZWindowFactory::make3DWindow(
    ZStackDoc *doc, Z3DWindow::EInitMode mode)
{
  ZSharedPointer<ZStackDoc> sharedDoc(doc);

  return make3DWindow(sharedDoc, mode);
}

Z3DWindow* ZWindowFactory::open3DWindow(
    ZStackDoc *doc, Z3DWindow::EInitMode mode)
{
  Z3DWindow *window = make3DWindow(doc, mode);
  window->show();
  window->raise();

  return window;
}

Z3DWindow* ZWindowFactory::make3DWindow(ZSharedPointer<ZStackDoc> doc,
                                        Z3DWindow::EInitMode mode)
{
  if (Z3DApplication::app() == NULL) {
    QMessageBox::information(
          NULL, "3D Unavailable", "The 3D visualization is unavailable in this"
          "plug-in because of some technical problems. To obtain a "
          "fully-functioing version of neuTube, because visit "
          "<a href=www.neutracing.com>www.neutracing.com</a>");
    return NULL;
  }

  Z3DWindow *window = NULL;

  if (Z3DApplication::app()->is3DSupported() && doc) {
    window = new Z3DWindow(doc, mode, false, m_parentWidget);
    if (m_windowTitle.isEmpty()) {
      window->setWindowTitle("3D View");
    } else {
      window->setWindowTitle(m_windowTitle);
    }
    //connect(window, SIGNAL(destroyed()), m_hostFrame, SLOT(detach3DWindow()));
    /*
    if (GET_APPLICATION_NAME == "Biocytin") {
      window->getCompositor()->setBackgroundFirstColor(glm::vec3(1, 1, 1));
      window->getCompositor()->setBackgroundSecondColor(glm::vec3(1, 1, 1));
    }
    */

    if (!NeutubeConfig::getInstance().getZ3DWindowConfig().isBackgroundOn()) {
      window->getCompositor()->setShowBackground(false);
    }

    if (doc->getTag() == NeuTube::Document::FLYEM_SPLIT) {
      window->getSwcFilter()->setRenderingPrimitive("Sphere");
      window->getPunctaFilter()->setColorMode("Original Point Color");
    }

    if (m_volumeMode == NeuTube3D::VR_AUTO) {
      if (doc->getTag() == NeuTube::Document::FLYEM_BODY ||
          doc->getTag() == NeuTube::Document::FLYEM_SPLIT) {
        window->getVolumeRaycasterRenderer()->setCompositeMode(
              "Direct Volume Rendering");
      } else {
        //      doc->getTag() == NeuTube::Document::SEGMENTATION_TARGET
        window->getVolumeRaycasterRenderer()->setCompositeMode("MIP Opaque");
      }
    } else {
      window->getVolumeRaycasterRenderer()->setCompositeMode(
            Z3DUtils::GetVolumeRenderingName(m_volumeMode).c_str());
    }
    if (doc->getTag() != NeuTube::Document::FLYEM_SPLIT &&
        doc->getTag() != NeuTube::Document::SEGMENTATION_TARGET &&
        doc->getTag() != NeuTube::Document::FLYEM_PROOFREAD) {
//      window->getCanvas()->disableKeyEvent();
    }

    window->setZScale(doc->getPreferredZScale());

    if (!m_showVolumeBoundBox) {
      window->getVolumeRaycaster()->hideBoundBox();
    }

    if (m_windowGeometry.isEmpty()/* || m_parentWidget == NULL*/) {
      QRect screenRect = QApplication::desktop()->screenGeometry();
      window->setGeometry(screenRect.width() / 10, screenRect.height() / 10,
                          screenRect.width() - screenRect.width() / 5,
                          screenRect.height() - screenRect.height() / 5);
    } else {
      window->setGeometry(m_windowGeometry);
    }

    for (QMap<Z3DWindow::ERendererLayer, bool>::const_iterator
         iter = m_layerVisible.begin(); iter != m_layerVisible.end(); ++iter) {
      window->setVisible(iter.key(), iter.value());
    }

    if (!isControlPanelVisible()) {
      window->hideControlPanel();
    }

    if (!isObjectViewVisible()) {
      window->hideObjectView();
    }

    configure(window);
//    doc->registerUser(window);
  }

  return window;
}

Z3DWindow* ZWindowFactory::make3DWindow(ZScalableStack *stack)
{
  if (stack == NULL) {
    return NULL;
  }

  Z3DWindow *window = NULL;

  if (Z3DApplication::app()->is3DSupported()) {
    ZStackDoc *doc = new ZStackDoc;
    doc->loadStack(stack->getStack());
    window = make3DWindow(doc);
    window->getVolumeSource()->getVolume(0)->setScaleSpacing(
          glm::vec3(stack->getXScale(), stack->getYScale(), stack->getZScale()));
    ZPoint offset = stack->getOffset();
    window->getVolumeSource()->getVolume(0)->setOffset(
          glm::vec3(offset.x(), offset.y(), offset.z()));
    window->updateVolumeBoundBox();
    window->updateOverallBoundBox();
    //window->resetCameraClippingRange();
    window->resetCamera();
    stack->releaseStack();
    delete stack;
  }

  configure(window);

  return window;
}

void ZWindowFactory::setWindowTitle(const QString &title)
{
  m_windowTitle = title;
}

void ZWindowFactory::setParentWidget(QWidget *parentWidget)
{
  m_parentWidget = parentWidget;
}

void ZWindowFactory::setWindowGeometry(const QRect &rect)
{
  m_windowGeometry = rect;
}

void ZWindowFactory::configure(Z3DWindow */*window*/)
{

}

void ZWindowFactory::setVisible(Z3DWindow::ERendererLayer layer, bool visible)
{
  m_layerVisible[layer] = visible;
}
