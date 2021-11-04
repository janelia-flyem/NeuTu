#include "z3dcanvas.h"

#include <QWindow>
#include <QPainter>
#include <QGraphicsTextItem>
#include <algorithm>

#include "logging/zlog.h"
#include "logging/zqslog.h"
#include "qt/gui/loghelper.h"

#include "z3dnetworkevaluator.h"
#include "z3dcanvaseventlistener.h"
#include "z3dscene.h"
#include "zpainter.h"
#include "zopenglwidget.h"
#include "data3d/displayconfig.h"

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
  connect(&m_interaction, SIGNAL(browsing(int, int)),
          this, SIGNAL(browsing(int, int)));
  connect(&m_interaction, SIGNAL(showingDetail(int,int)),
          this, SIGNAL(viewingDetail(int,int)));

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
      m_interaction.isStateOn(ZInteractionEngine::STATE_LOCATE) ||
      m_interaction.isStateOn(ZInteractionEngine::STATE_BROWSE) ||
      m_interaction.isStateOn(ZInteractionEngine::STATE_SHOW_DETAIL)) {
    return true;
  }
//#endif

  return false;
}

void Z3DCanvas::mousePressEvent(QMouseEvent* e)
{
  neutu::LogMouseEvent(e, "press", "Z3DCanvas");
  m_pressedButtons = e->buttons();

  broadcastEvent(e, width(), height());

  m_interaction.processMousePressEvent(e);
}

void Z3DCanvas::mouseReleaseEvent (QMouseEvent* e)
{
  neutu::LogMouseReleaseEvent(m_pressedButtons, e->modifiers(), "Z3DCanvas");
  m_pressedButtons = Qt::NoButton;

#ifdef _DEBUG_2
  std::cout << "Z3DCanvas::mouseReleaseEvent" << std::endl;
#endif
  if (!m_interaction.processMouseReleaseEvent(e)) {
    broadcastEvent(e, width(), height());
  }
  setCursor(m_interaction.getCursorShape());
}

void Z3DCanvas::mouseMoveEvent(QMouseEvent*  e)
{
  neutu::LogMouseDragEvent(e, "Z3DCanvas");

  m_interaction.processMouseMoveEvent(e);

  if (!m_interaction.lockingMouseMoveEvent()) {
    broadcastEvent(e, width(), height());
  }
}

void Z3DCanvas::mouseDoubleClickEvent(QMouseEvent* e)
{
  neutu::LogMouseEvent(e, "double click", "Z3DCanvas");
//  KINFO << "Mouse double clicked in Z3DCanvas";

  broadcastEvent(e, width(), height());
}

void Z3DCanvas::wheelEvent(QWheelEvent* e)
{
//  KINFO << "Mouse scrolled in Z3DCanvas";
  neutu::LogMouseEvent(e, "Z3DCanvas");

  broadcastEvent(e, width(), height());
}

void Z3DCanvas::keyPressEvent(QKeyEvent* event)
{
  neutu::LogKeyPressEvent(event, "Z3DCanvas");

  if (!m_interaction.processKeyPressEvent(event)) {
    broadcastEvent(event, width(), height());
    updateView();
  }

  setCursor(m_interaction.getCursorShape());
}

void Z3DCanvas::keyReleaseEvent(QKeyEvent* event)
{
  neutu::LogKeyReleaseEvent(event, "Z3DCanvas");

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
  updateView();
  m_updatingDecoration = false;
}

/*
void Z3DCanvas::setCustomWidget(QWidget *widget)
{
  widget->setParent(this);
//  widget->setGeometry(QRect(0, 512, 512, 512));
}
*/

void Z3DCanvas::dump(const QString &message)
{
  m_message = message;
  updateView();
}

void Z3DCanvas::processMessage(const ZWidgetMessage &message)
{
  if (message.hasTarget(ZWidgetMessage::TARGET_CUSTOM_AREA)) {
    dump(message.toPlainString());
  }
}

void Z3DCanvas::updateView()
{
  viewport()->update();
}

void Z3DCanvas::paintCustomRegion(const QImage &image)
{
  if (m_customCanvas != NULL) {
    if (m_customCanvas->size() != image.size()) {
      delete m_customCanvas;
      m_customCanvas = NULL;
    }
  }

  if (m_customCanvas == NULL && !image.isNull()) {
    m_customCanvas = new QPixmap(image.size());
  }

  if (m_customCanvas != NULL) {
    QPainter painter(m_customCanvas);
    painter.drawImage(m_customCanvas->rect(), image);
  }

  updateView();
}

void Z3DCanvas::drawBackground(QPainter* painter, const QRectF& rect)
{
#ifdef _DEBUG_2
  std::cout << "Z3DCanvas::drawBackground: " << m_updatingDecoration << std::endl;
#endif

  if (!m_updatingDecoration) {
    QGraphicsView::drawBackground(painter, rect);
  }
  //m_3dScene->drawBackground(painter, rect);

  if (m_customCanvas != NULL) {
    double projWidth = width() * 0.25;
    double projHeight =
        projWidth / m_customCanvas->width() * m_customCanvas->height();

    painter->drawPixmap(QRectF(width() - projWidth, 0, projWidth, projHeight),
                        *m_customCanvas, m_customCanvas->rect());
  }

#if 1
  QList<ZStackObject*> drawableList = m_interaction.getDecorationList();

  painter->setRenderHint(QPainter::HighQualityAntialiasing, true);
  foreach (ZStackObject *drawable, drawableList) {
    //drawable->setVisible(true);
    drawable->display(
          painter,
          neutu::data3d::DisplayConfigBuilder().withStyle(
            neutu::data3d::EDisplayStyle::NORMAL).cutPlane(neutu::EAxis::Z, 0));

//    drawable->display(painter, 0, ZStackObject::EDisplayStyle::NORMAL,
//                      ZStackObject::EDisplaySliceMode::SINGLE, neutu::EAxis::Z);
  }
#else
  UNUSED_PARAMETER(painter);
#endif

#ifdef _DEBUG_2
  painter->setPen(QColor(255, 0, 0));
  painter->drawRect(QRect(10, 10, 40, 60));
#endif

  QStringList text;
  if (!m_message.isEmpty()) {
    text.append(m_message);
  }
  if (m_interaction.getKeyMode() == ZInteractionEngine::KM_SWC_SELECTION) {
    text.append("Selection mode on:");
    text.append("  1: downstream;");
    text.append("  2: upstream;");
    text.append("  3: connected nodes;");
    text.append("  4: inverse selection;");
    text.append("  5: select small trees;");
  }


  drawText(*painter, text);

#if QT_VERSION < QT_VERSION_CHECK(5, 9, 0) && defined(_USE_CORE_PROFILE_)
  glGetError(); //error from qt
#endif
}

void Z3DCanvas::drawText(QPainter &painter, const QStringList &text)
{
  if (!text.empty()) {
    int maxLength = 0;
    QString compText;
    foreach (const QString &str, text) {
      if (str.length() > maxLength) {
        maxLength = str.length();
      }
      compText += str + "\n";
    }

    maxLength = std::min(maxLength, 80);

    int width = maxLength * 8;
    int height = text.length() * 20;

    if (width > 0 && height > 0) {
      QPixmap pixmap(width, height);
      pixmap.fill(QColor(0, 0, 0, 64));

      QPainter bufferPainter(&pixmap);
      bufferPainter.setPen(QColor(64, 200, 64));

      //    bufferPainter.fillRect(pixmap.rect(), QColor(0, 0, 0, 0));
      bufferPainter.drawText(QRectF(10, 1, width, height), compText);
      painter.save();
      painter.setCompositionMode(QPainter::CompositionMode_SourceOver);
      painter.drawPixmap(0, 0, pixmap);
      painter.restore();
    }
  }
}

void Z3DCanvas::drawText(QPainter &painter, const QString &text)
{
  if (!text.isEmpty()) {
    QStringList textList;
    textList.append(text);
    drawText(painter, text);
  }
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
#ifdef _DEBUG_2
  std::cout << m_listeners.size() << " listeners" << std::endl;
#endif
  for (size_t i = 0 ; i < m_listeners.size() ; ++i) {
#ifdef _DEBUG_2
    std::cout << "  Listener: " << m_listeners[i]->getListnerName() << std::endl;
#endif
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

