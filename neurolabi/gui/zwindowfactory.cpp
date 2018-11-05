#include "zwindowfactory.h"
#include <QApplication>
#include <QDesktopWidget>
#include <QMessageBox>
#include "neutubeconfig.h"
#include "z3dcompositor.h"
#include "z3dcanvas.h"
#include "z3dvolumefilter.h"
#include "zscalablestack.h"
#include "z3dwindow.h"
#include "z3dswcfilter.h"
#include "z3dpunctafilter.h"
#include "zstackdoc.h"
#include "zstackframe.h"
#include "zdialogfactory.h"
#include "mainwindow.h"
#include "zsysteminfo.h"

ZWindowFactory::ZWindowFactory()
{
  init();
}

ZWindowFactory::~ZWindowFactory()
{

}

void ZWindowFactory::init()
{
}

Z3DWindow* ZWindowFactory::make3DWindow(
    ZStackDoc *doc, Z3DView::EInitMode mode)
{
  ZSharedPointer<ZStackDoc> sharedDoc(doc);

  return make3DWindow(sharedDoc, mode);
}

Z3DWindow* ZWindowFactory::open3DWindow(
    ZStackDoc *doc, Z3DView::EInitMode mode)
{
  Z3DWindow *window = make3DWindow(doc, mode);

  return window;
}

Z3DWindow* ZWindowFactory::make3DWindow(ZSharedPointer<ZStackDoc> doc,
                                        Z3DView::EInitMode mode)
{
  if (!ZSystemInfo::instance().is3DSupported()) {
    if (GET_APPLICATION_NAME == "neuTube") {
      QMessageBox::information(
            NULL, "3D Unavailable", "The 3D visualization is unavailable in this"
                                    "plug-in because of some technical problems. To obtain a "
                                    "fully-functioing version of neuTube, because visit "
                                    "<a href=www.neutracing.com>www.neutracing.com</a>");
    } else {
      QMessageBox::information(
            NULL, "3D Unavailable",
            "The 3D visualization is unavailable in this"
            "plug-in because of some technical problems.");
    }
    return NULL;
  }

  Z3DWindow *window = NULL;

  if (ZSystemInfo::instance().is3DSupported() && doc) {
    window = new Z3DWindow(
          doc, mode, m_windowType, false, m_parentWidget);
    window->show();
    window->raise();

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

    if (doc->getTag() == neutube::Document::ETag::FLYEM_SPLIT) {
      window->getSwcFilter()->setRenderingPrimitive("Sphere");
      window->getPunctaFilter()->setColorMode("Original Point Color");
    }

    if (m_volumeMode == neutube3d::VR_AUTO) {
      if (doc->getTag() == neutube::Document::ETag::FLYEM_BODY ||
          doc->getTag() == neutube::Document::ETag::FLYEM_SPLIT ||
          doc->getTag() == neutube::Document::ETag::FLYEM_PROOFREAD) {
        window->getVolumeFilter()->setCompositeMode(
              "Direct Volume Rendering");
      } else {
        //      doc->getTag() == NeuTube::Document::SEGMENTATION_TARGET
        window->getVolumeFilter()->setCompositeMode("MIP Opaque");
      }
    } else {
      window->getVolumeFilter()->setCompositeMode(
            neutube3d::GetVolumeRenderingModeName(m_volumeMode));
    }
    if (doc->getTag() != neutube::Document::ETag::FLYEM_SPLIT &&
        doc->getTag() != neutube::Document::ETag::SEGMENTATION_TARGET &&
        doc->getTag() != neutube::Document::ETag::FLYEM_PROOFREAD) {
//      window->getCanvas()->disableKeyEvent();
    }

    window->setZScale(doc->getPreferredZScale());

    if (!m_showVolumeBoundBox) {
      window->getVolumeFilter()->hideBoundBox();
    }

    if (m_windowGeometry.isEmpty()/* || m_parentWidget == NULL*/) {
      QRect screenRect = QApplication::desktop()->screenGeometry();
      window->setGeometry(screenRect.width() / 10, screenRect.height() / 10,
                          screenRect.width() - screenRect.width() / 5,
                          screenRect.height() - screenRect.height() / 5);
    } else {
      window->setGeometry(m_windowGeometry);
    }

    for (QMap<neutube3d::ERendererLayer, bool>::const_iterator
         iter = m_layerVisible.begin(); iter != m_layerVisible.end(); ++iter) {
      window->setLayerVisible(iter.key(), iter.value());
    }

    if (!isControlPanelVisible()) {
      window->hideControlPanel();
    }

    if (!isObjectViewVisible()) {
      window->hideObjectView();
    }

    if (!isStatusBarVisible()) {
      window->hideStatusBar();
    }

    configure(window);
//    doc->registerUser(window);
  }

  return window;
}

Z3DWindow* ZWindowFactory::Open3DWindow(
    ZStackFrame *frame, Z3DView::EInitMode mode)
{
  if (!ZSystemInfo::instance().is3DSupported()) {
    ZDialogFactory::Notify3DDisabled(frame);

    return NULL;
  }

  ZSharedPointer<ZStackDoc> doc = frame->document();
  if (doc->getTag() == neutube::Document::ETag::BIOCYTIN_PROJECTION) {
    doc = doc->getParentDoc();
  }

  Z3DWindow *window = doc->getParent3DWindow();

  if (window == NULL) {
    if (frame->getMainWindow() != NULL) {
      frame->getMainWindow()->startProgress("Opening 3D View ...", 0);
    }

    ZWindowFactory factory;
    //factory.setParentWidget(parent);
    window = factory.make3DWindow(doc, mode);
    QString title = frame->windowTitle();
    if (title.endsWith(" *")) {
      title.resize(title.size()-2);
    }
    window->setWindowTitle(title);

    doc->registerUser(window);

    frame->connect(frame, SIGNAL(closed(ZStackFrame*)), window, SLOT(close()));
//    connect(window, SIGNAL(destroyed()), this, SLOT(detach3DWindow()));
    if (frame->getMainWindow() != NULL) {
      frame->getMainWindow()->endProgress();
    }
  }

  if (window != NULL) {
    window->show();
    window->raise();
  } else {
    ZDialogFactory::Notify3DDisabled(frame);
  }

  return window;
}

Z3DWindow* ZWindowFactory::make3DWindow(ZScalableStack *stack)
{
  if (stack == NULL) {
    return NULL;
  }

  Z3DWindow *window = NULL;

  if (ZSystemInfo::instance().is3DSupported()) {
    ZStackDoc *doc = new ZStackDoc;
    doc->loadStack(stack->getStack());
    window = make3DWindow(doc);

    window->getVolumeFilter()->setScale(stack->getXScale(), stack->getYScale(), stack->getZScale());
    ZPoint offset = stack->getOffset();
    window->getVolumeFilter()->setOffset(offset.x(), offset.y(), offset.z());
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

void ZWindowFactory::setWindowType(neutube3d::EWindowType type)
{
  m_windowType = type;
}


void ZWindowFactory::configure(Z3DWindow * /*window*/)
{

}

void ZWindowFactory::setVisible(neutube3d::ERendererLayer layer, bool visible)
{
  m_layerVisible[layer] = visible;
}
