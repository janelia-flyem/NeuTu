#ifndef Z3DCANVAS_H
#define Z3DCANVAS_H

#include "z3dgl.h"
#include <QGraphicsView>
#include <QWidget>
#ifdef _QT5_
#include <QOpenGLWidget>
#else
#include <QGLWidget>
#endif
#include <QInputEvent>
#include <deque>
#include <QList>
#include "zstroke2d.h"

#  include "flyem/zinteractionengine.h"

class Z3DScene;
class Z3DNetworkEvaluator;
class Z3DCanvasEventListener;
class ZStackObject;
class ZStroke2d;
class Z3DTrackballInteractionHandler;

class Z3DCanvas : public QGraphicsView
{
  Q_OBJECT
public:
#ifdef _QT5_
  Z3DCanvas(const QString &title, int width = 512, int height = 512,
            QWidget* parent = 0, Qt::WindowFlags f = 0);
#else
  Z3DCanvas(const QString &title, int width = 512, int height = 512,
            const QGLFormat &format = QGLFormat(QGL::DoubleBuffer | QGL::DepthBuffer | QGL::Rgba | QGL::SampleBuffers | QGL::AlphaChannel),
            QWidget* parent = 0, const QGLWidget * shareWidget = 0, Qt::WindowFlags f = 0);
#endif


  virtual ~Z3DCanvas();

#ifdef _QT5_
  inline QSurfaceFormat format() const { return m_glWidget->format(); }
#else
  inline QGLFormat format() const { return m_glWidget->format(); }
#endif

  void setNetworkEvaluator(Z3DNetworkEvaluator *n);
  void setFakeStereoOnce();

  void addEventListenerToBack(Z3DCanvasEventListener* e);
  void addEventListenerToFront(Z3DCanvasEventListener* e);
  void removeEventListener(Z3DCanvasEventListener *e);
  void clearEventListeners();
  void broadcastEvent(QEvent* e, int w, int h);

  // Set the opengl context of this canvas as the current one.
#ifdef _QT5_
  inline void getGLFocus() {}
#else
  inline void getGLFocus() { m_glWidget->makeCurrent(); }
#endif
  void toggleFullScreen();
  void forceUpdate() {
    QPaintEvent *pe = new QPaintEvent(rect()); paintEvent(pe); delete pe;
  }
  void updateAll() { m_glWidget->update(); }

  // for high dpi support like retina
  glm::ivec2 getPhysicalSize() { return glm::ivec2(width() * getDevicePixelRatio(),
                                                   height() * getDevicePixelRatio()); }
  glm::ivec2 getLogicalSize() { return glm::ivec2(width(), height()); }

  virtual void enterEvent(QEvent* e);
  virtual void leaveEvent(QEvent* e);
  virtual void mousePressEvent(QMouseEvent* e);
  virtual void mouseReleaseEvent (QMouseEvent* e);
  virtual void mouseMoveEvent(QMouseEvent*  e);
  virtual void mouseDoubleClickEvent(QMouseEvent* e);
  virtual void wheelEvent(QWheelEvent* e);
  virtual void timerEvent(QTimerEvent* e);

  virtual void keyPressEvent(QKeyEvent* event);
  virtual void keyReleaseEvent(QKeyEvent* event);

  virtual void resizeEvent(QResizeEvent *event);
  virtual void paintEvent(QPaintEvent *event);
  virtual void dragEnterEvent(QDragEnterEvent *event);
  virtual void dropEvent(QDropEvent *event);

#ifdef _QT5_
  void setCursor(const QCursor &c) { viewport()->setCursor(c); }
#endif

  virtual void drawBackground(QPainter *painter, const QRectF &rect);

  bool processMouseMoveEventForPaint(QMouseEvent *e);
  bool suppressingContextMenu() const;
  void disableKeyEvent();
  void setKeyMode(ZInteractionEngine::EKeyMode mode);

  void set3DInteractionHandler(Z3DTrackballInteractionHandler *handler);

  void updateCursor();

  inline ZInteractionEngine* getInteractionEngine() {
    return &m_interaction;
  }

  inline const ZInteractionEngine* getInteractionEngine() const {
    return &m_interaction;
  }

  inline ZInteractiveContext& getInteractionContext() {
    return getInteractionEngine()->getInteractiveContext();
  }

signals:
  // w and h is physical size not logical size, opengl works in physical pixel
  void canvasSizeChanged(int w, int h);
  void activeDecorationUpdated();
  void strokePainted(ZStroke2d*);

protected:
  double getDevicePixelRatio();

  bool m_fullscreen;

#ifdef _QT5_
  QOpenGLWidget* m_glWidget;
#else
  QGLWidget* m_glWidget;
#endif
  QGraphicsScene* m_3dScene;
  std::deque<Z3DCanvasEventListener*> m_listeners;

  Z3DNetworkEvaluator* m_networkEvaluator;
  bool m_isStereoScene;
  bool m_fakeStereoOnce;

private:
  ZInteractionEngine m_interaction;
};

#endif // Z3DCANVAS_H
