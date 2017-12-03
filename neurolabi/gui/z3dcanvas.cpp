#include "z3dcanvas.h"

#include "z3dnetworkevaluator.h"
#include "z3dcanvaseventlistener.h"
#include "z3dscene.h"
#include "QsLog.h"
#include "zpainter.h"
#include "zstackdrawable.h"
#include "zopenglwidget.h"
#include <QWindow>
#include <QPainter>
#include <QGraphicsTextItem>
#include <algorithm>

Z3DCanvas::Z3DCanvas(const QString &title, int width, int height, QWidget* parent, Qt::WindowFlags f)
  : QGraphicsView(parent)
  , m_fullscreen(false)
  , m_glWidget(nullptr)
  , m_3dScene(nullptr)
{
  setAlignment(Qt::AlignLeft | Qt::AlignTop);
  resize(width, height);

  m_glWidget = new ZOpenGLWidget(nullptr, f);
  m_3dScene = new Z3DScene(width, height, m_glWidget->format().stereo(), this);

  setViewport(m_glWidget);
  setViewportUpdateMode(FullViewportUpdate);
  setScene(m_3dScene);

  setWindowTitle(title);
  setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform | QPainter::TextAntialiasing);
  setOptimizationFlags(QGraphicsView::DontSavePainterState |
                       QGraphicsView::DontAdjustForAntialiasing);

  setAcceptDrops(true);
  setFocusPolicy(Qt::StrongFocus);

#ifndef __APPLE__
  setStyleSheet("border-style: none;");
#endif
  setMouseTracking(true);

  connect(&m_interaction, SIGNAL(decorationUpdated()),
          this, SLOT(updateDecoration()));
  connect(&m_interaction, SIGNAL(strokePainted(ZStroke2d*)),
          this, SIGNAL(strokePainted(ZStroke2d*)));
  connect(&m_interaction, SIGNAL(shootingTodo(int,int)),
          this, SIGNAL(shootingTodo(int,int)));
  connect(&m_interaction, SIGNAL(locating(int, int)),
          this, SIGNAL(locating(int, int)));

  connect(m_glWidget, &ZOpenGLWidget::openGLContextInitialized, this, &Z3DCanvas::openGLContextInitialized);
}

QSurfaceFormat Z3DCanvas::format() const
{
  return m_glWidget->format();
}

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

void Z3DCanvas::updateAll()
{
  m_glWidget->update();
}

void Z3DCanvas::enterEvent(QEvent* e)
{
  broadcastEvent(e, width(), height());
}

void Z3DCanvas::leaveEvent(QEvent* e)
{
  broadcastEvent(e, width(), height());
}

bool Z3DCanvas::suppressingContextMenu() const
{
//#if defined(_FLYEM_)
  if (m_interaction.isStateOn(ZInteractionEngine::STATE_DRAW_STROKE) ||
      m_interaction.isStateOn(ZInteractionEngine::STATE_DRAW_RECT) ||
      m_interaction.isStateOn(ZInteractionEngine::STATE_MARK) ||
      m_interaction.isStateOn(ZInteractionEngine::STATE_LOCATE)) {
    return true;
  }
//#endif

  return false;
}

void Z3DCanvas::mousePressEvent(QMouseEvent* e)
{
  broadcastEvent(e, width(), height());

  m_interaction.processMousePressEvent(e);
}

void Z3DCanvas::mouseReleaseEvent (QMouseEvent* e)
{
  if (!m_interaction.processMouseReleaseEvent(e)) {
    broadcastEvent(e, width(), height());
  }
  setCursor(m_interaction.getCursorShape());
}

void Z3DCanvas::mouseMoveEvent(QMouseEvent*  e)
{
  m_interaction.processMouseMoveEvent(e);

  if (!m_interaction.lockingMouseMoveEvent()) {
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
  if (!m_interaction.processKeyPressEvent(event)) {
    broadcastEvent(event, width(), height());
  }

  setCursor(m_interaction.getCursorShape());
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

  emit canvasSizeChanged(event->size().width() * devicePixelRatioF(),
                         event->size().height() * devicePixelRatioF());
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

void Z3DCanvas::updateDecoration()
{
  m_updatingDecoration = true;
  viewport()->update();
  m_updatingDecoration = false;
}

void Z3DCanvas::drawBackground(QPainter* painter, const QRectF& rect)
{
  if (!m_updatingDecoration) {
    QGraphicsView::drawBackground(painter, rect);
  }
  //m_3dScene->drawBackground(painter, rect);

#if 1
  QList<ZStackObject*> drawableList = m_interaction.getDecorationList();

  foreach (ZStackObject *drawable, drawableList) {
    //drawable->setVisible(true);
    drawable->display(painter, 0, ZStackObject::NORMAL,
                      ZStackObject::DISPLAY_SLICE_SINGLE, neutube::Z_AXIS);
  }
#else
  UNUSED_PARAMETER(painter);
#endif

#ifdef _DEBUG_2
  painter->setPen(QColor(255, 0, 0));
  painter->drawRect(QRect(10, 10, 40, 60));
#endif

  if (m_interaction.getKeyMode() == ZInteractionEngine::KM_SWC_SELECTION) {
    painter->setPen(QColor(255, 255, 255));
    QFont font("Helvetica [Cronyx]", 24);
    painter->setFont(font);
    painter->drawText(
          QRectF(10, 10, 300, 200),
          "Selection mode on: \n"
          "  1: downstream;\n"
          "  2: upstream;\n"
          "  3: connected nodes;\n"
          "  4: inverse selection;\n"
          "  5: select small trees;");
  }

  //painter->drawRect(QRect(10, 10, 40, 60));

#if QT_VERSION < QT_VERSION_CHECK(5, 9, 0) && defined(_USE_CORE_PROFILE_)
  glGetError(); //error from qt
#endif
}

void Z3DCanvas::timerEvent(QTimerEvent* e)
{
  broadcastEvent(e, width(), height());
}

void Z3DCanvas::setNetworkEvaluator(Z3DNetworkEvaluator *n)
{
  m_3dScene->setNetworkEvaluator(n);
}

void Z3DCanvas::setFakeStereoOnce()
{
  m_3dScene->setFakeStereoOnce();
}

void Z3DCanvas::removeEventListener(Z3DCanvasEventListener& e)
{
  std::deque<Z3DCanvasEventListener*>::iterator pos;
  pos = std::find(m_listeners.begin(), m_listeners.end(), &e);

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

void Z3DCanvas::getGLFocus()
{
  m_glWidget->makeCurrent();
}

void Z3DCanvas::setKeyMode(ZInteractionEngine::EKeyMode mode)
{
  m_interaction.setKeyMode(mode);
  update(QRect(QPoint(0, 0), size()));
}

//double Z3DCanvas::devicePixelRatio()
//{
//  return (window() && window()->windowHandle()) ?
//        window()->windowHandle()->devicePixelRatio() : 1.0;
//}

void Z3DCanvas::disableKeyEvent()
{
  m_interaction.setKeyEventEnabled(false);
}

void Z3DCanvas::set3DInteractionHandler(Z3DTrackballInteractionHandler *handler)
{
  m_interaction.set3DInteractionHandler(handler);
}

void Z3DCanvas::updateCursor()
{
  setCursor(m_interaction.getCursorShape());
}

