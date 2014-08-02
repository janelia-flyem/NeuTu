#include "z3dcanvas.h"

#include "z3dnetworkevaluator.h"
#include <algorithm>
#include "z3dcanvaseventlistener.h"
#include "QsLog/QsLog.h"
#ifdef _QT5_
#include <QWindow>
#endif
#include "zpainter.h"
#include "zstackdrawable.h"

Z3DCanvas::Z3DCanvas(const QString &title, int width, int height, const QGLFormat &format,
                     QWidget* parent, const QGLWidget *shareWidget, Qt::WindowFlags f)
  : QGraphicsView(parent)
  , m_fullscreen(false)
  , m_glWidget(NULL)
  , m_3dScene(NULL)
  , m_networkEvaluator(NULL)
  , m_fakeStereoOnce(false)
{
  setAlignment(Qt::AlignLeft | Qt::AlignTop);
  resize(width, height);

  m_glWidget = new QGLWidget(format, NULL, shareWidget, f);
  m_glWidget->makeCurrent();
  m_isStereoScene = m_glWidget->format().stereo();
  m_3dScene = new QGraphicsScene(0, 0, width, height, this);

  setViewport(m_glWidget);
  setViewportUpdateMode(FullViewportUpdate);
  setScene(m_3dScene);

  setWindowTitle(title);
  setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform);
  setOptimizationFlags(QGraphicsView::DontSavePainterState | QGraphicsView::DontAdjustForAntialiasing);

  setAcceptDrops(true);
  setFocusPolicy(Qt::StrongFocus);

  setStyleSheet("border-style: none;");
  setMouseTracking(true);

#if defined(_FLYEM_)
  connect(&m_interaction, SIGNAL(decorationUpdated()),
          this->viewport(), SLOT(update()));
  connect(&m_interaction, SIGNAL(strokePainted(ZStroke2d*)),
          this, SIGNAL(strokePainted(ZStroke2d*)));
#endif
}

Z3DCanvas::~Z3DCanvas() {}

void Z3DCanvas::toggleFullScreen()
{
  if (m_fullscreen) {
    m_fullscreen = false;
    showNormal();
  } else {
    showFullScreen();
    m_fullscreen = !m_fullscreen;
  }
}

void Z3DCanvas::enterEvent(QEvent* e)
{
  broadcastEvent(e, width(), height());
}

void Z3DCanvas::leaveEvent(QEvent* e)
{
  broadcastEvent(e, width(), height());
}

void Z3DCanvas::mousePressEvent(QMouseEvent* e)
{
  broadcastEvent(e, width(), height());


#if defined(_FLYEM_)
  m_interaction.processMousePressEvent(e);
#endif
}

bool Z3DCanvas::suppressingContextMenu() const
{
#if defined(_FLYEM_)
  if (m_interaction.isStateOn(ZInteractionEngine::STATE_DRAW_STROKE)) {
    return true;
  }
#endif

  return false;
}

void Z3DCanvas::mouseReleaseEvent (QMouseEvent* e)
{
  broadcastEvent(e, width(), height());
#if defined(_FLYEM_)
  m_interaction.processMouseReleaseEvent(e);
  setCursor(m_interaction.getCursorShape());
#endif
}

void Z3DCanvas::mouseMoveEvent(QMouseEvent*  e)
{
#if defined(_FLYEM_)
  m_interaction.processMouseMoveEvent(e);
#endif

#if defined(_FLYEM_)
  if (!m_interaction.lockingMouseMoveEvent()) {
#else
  {
#endif
    broadcastEvent(e, width(), height());
  }
}

void Z3DCanvas::mouseDoubleClickEvent(QMouseEvent* e)
{
  broadcastEvent(e, width(), height());
}

void Z3DCanvas::wheelEvent(QWheelEvent* e)
{
  broadcastEvent(e, width(), height());
}

void Z3DCanvas::keyPressEvent(QKeyEvent* event)
{
  broadcastEvent(event, width(), height());

#if defined(_FLYEM_)
  m_interaction.processKeyPressEvent(event);
  setCursor(m_interaction.getCursorShape());
#endif
}

void Z3DCanvas::keyReleaseEvent(QKeyEvent* event)
{
  broadcastEvent(event, width(), height());
}

void Z3DCanvas::resizeEvent(QResizeEvent *event)
{
  getGLFocus();
  QGraphicsView::resizeEvent(event);
  if (m_3dScene)
    m_3dScene->setSceneRect(QRect(QPoint(0, 0), event->size()));

  emit canvasSizeChanged(event->size().width() * getDevicePixelRatio(),
                         event->size().height() * getDevicePixelRatio());
}

void Z3DCanvas::paintEvent(QPaintEvent *event)
{
  getGLFocus();
  QGraphicsView::paintEvent(event);
}

void Z3DCanvas::dragEnterEvent(QDragEnterEvent *event)
{
  event->ignore();
}

void Z3DCanvas::dropEvent(QDropEvent *event)
{
  event->ignore();
}

void Z3DCanvas::drawBackground(QPainter *painter, const QRectF &)
{
  if (!m_networkEvaluator) {
    return;
  }

  // QPainter set glclearcolor to white, we set it back
  glClearColor(0.f, 0.f, 0.f, 0.f);

  m_networkEvaluator->process(m_isStereoScene || m_fakeStereoOnce);
  m_fakeStereoOnce = false;


#if defined(_FLYEM_)
  QList<ZStackObject*> drawableList = m_interaction.getDecorationList();

  foreach (ZStackObject *drawable, drawableList) {
    //drawable->setVisible(true);
    drawable->display(painter);
  }
#else
  UNUSED_PARAMETER(painter);
#endif

#ifdef _DEBUG_2
  painter->setPen(QColor(255, 0, 0));
  painter->drawRect(QRect(10, 10, 40, 60));
#endif

  //ZPainter painter()
  //painter->drawRect(QRect(10, 10, 40, 60));

}

void Z3DCanvas::timerEvent(QTimerEvent* e)
{
  broadcastEvent(e, width(), height());
}

void Z3DCanvas::setNetworkEvaluator(Z3DNetworkEvaluator *n)
{
  //m_3dScene->setNetworkEvaluator(n);
  m_networkEvaluator = n;
  if (n)
    n->setOpenGLContext(this);
}

void Z3DCanvas::setFakeStereoOnce()
{
  m_fakeStereoOnce = true;
  //m_3dScene->setFakeStereoOnce();
}

void Z3DCanvas::addEventListenerToBack(Z3DCanvasEventListener *e)
{
  if (e)
    m_listeners.push_back(e);
}

void Z3DCanvas::addEventListenerToFront(Z3DCanvasEventListener *e)
{
  if (e)
    m_listeners.push_front(e);
}

void Z3DCanvas::removeEventListener(Z3DCanvasEventListener *e)
{
  std::deque<Z3DCanvasEventListener*>::iterator pos;
  pos = std::find(m_listeners.begin(), m_listeners.end(), e);

  if (pos != m_listeners.end())
    m_listeners.erase(pos);
}

void Z3DCanvas::clearEventListeners()
{
  m_listeners.clear();
}

void Z3DCanvas::broadcastEvent(QEvent *e, int w, int h)
{
  getGLFocus();
  for (size_t i = 0 ; i < m_listeners.size() ; ++i) {
    m_listeners[i]->onEvent(e, w, h);
    if (e->isAccepted()) {
      break;
    }
  }
}

double Z3DCanvas::getDevicePixelRatio()
{
#ifdef _QT5_
  return (window() && window()->windowHandle()) ?
        window()->windowHandle()->devicePixelRatio() : 1.0;
#else
  return 1.0;
#endif
}

void Z3DCanvas::disableKeyEvent()
{
#if defined(_FLYEM_)
  m_interaction.setKeyEventEnabled(false);
#endif
}

void Z3DCanvas::set3DInteractionHandler(Z3DTrackballInteractionHandler *handler)
{
#if defined(_FLYEM_)
  m_interaction.set3DInteractionHandler(handler);
#else
  UNUSED_PARAMETER(handler);
#endif
}

void Z3DCanvas::updateCursor()
{
#if defined(_FLYEM_)
  setCursor(m_interaction.getCursorShape());
#endif
}
