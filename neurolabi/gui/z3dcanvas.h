#ifndef Z3DCANVAS_H
#define Z3DCANVAS_H

#include <deque>
#include <memory>

#include <QGraphicsView>
#include <QSurfaceFormat>
#include <QInputEvent>
#include <QShortcut>

#include "zutils.h"
#include "zglmutils.h"
#include "zstroke2d.h"
#include "zwidgetmessage.h"
#include "flyem/zinteractionengine.h"

class ZOpenGLWidget;

class Z3DScene;

class Z3DNetworkEvaluator;

class Z3DCanvasEventListener;

class Z3DCanvas : public QGraphicsView
{
Q_OBJECT
public:
  Z3DCanvas(const QString& title, int width, int height, QWidget* parent = nullptr, Qt::WindowFlags f = 0);

  QSurfaceFormat format() const;

  void setNetworkEvaluator(Z3DNetworkEvaluator* n);

  void setFakeStereoOnce();

  void addEventListenerToBack(Z3DCanvasEventListener& e)
  { m_listeners.push_back(&e); }

  void addEventListenerToFront(Z3DCanvasEventListener& e)
  { m_listeners.push_front(&e); }

  void removeEventListener(Z3DCanvasEventListener& e);

  void clearEventListeners();

  void broadcastEvent(QEvent* e, int w, int h);

  // Set the opengl context of this canvas as the current one.
  void getGLFocus();

  void toggleFullScreen();

  void forceUpdate()
  {
    auto pe = std::make_unique<QPaintEvent>(rect());
    paintEvent(pe.get());
  }

  void updateAll();

  // for high dpi support like retina
  glm::uvec2 physicalSize()
  {
    return glm::uvec2(width() * devicePixelRatioF(),
                      height() * devicePixelRatioF());
  }

  glm::uvec2 logicalSize()
  { return glm::uvec2(width(), height()); }

  virtual void drawBackground(QPainter *painter, const QRectF &rect) override;

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

  void setCursor(const QCursor& c)
  { viewport()->setCursor(c); }

  void dump(const QString &message);
  void processMessage(const ZWidgetMessage &message);

  void paintCustomRegion(const QImage &image);

  void updateView();
//  void setCustomWidget(QWidget *widget);

signals:

  // w and h is physical size not logical size, opengl works in physical pixel
  void canvasSizeChanged(size_t w, size_t h);

  void activeDecorationUpdated();

  void strokePainted(ZStroke2d*);
  void shootingTodo(int x, int y);
  void locating(int x, int y);
  void browsing(int x, int y);
  void viewingDetail(int x, int y);

  void openGLContextInitialized();

protected:
  virtual void enterEvent(QEvent* e) override;

  virtual void leaveEvent(QEvent* e) override;

  virtual void mousePressEvent(QMouseEvent* e) override;

  virtual void mouseReleaseEvent(QMouseEvent* e) override;

  virtual void mouseMoveEvent(QMouseEvent* e) override;

  virtual void mouseDoubleClickEvent(QMouseEvent* e) override;

  virtual void wheelEvent(QWheelEvent* e) override;

  virtual void timerEvent(QTimerEvent* e) override;

  virtual void keyPressEvent(QKeyEvent* event) override;

  virtual void keyReleaseEvent(QKeyEvent* event) override;

  virtual void resizeEvent(QResizeEvent* event) override;

  virtual void paintEvent(QPaintEvent* event) override;

  virtual void dragEnterEvent(QDragEnterEvent* event) override;

  virtual void dropEvent(QDropEvent* event) override;

  void rotateX();

  void rotateY();

  void rotateZ();

  void rotateXM();

  void rotateYM();

  void rotateZM();

private:
  //double devicePixelRatio();
  void drawText(QPainter &painter, const QString &text);
  void drawText(QPainter &painter, const QStringList &text);

private slots:
  void updateDecoration();

private:
  bool m_fullscreen;

  ZOpenGLWidget* m_glWidget;
  Z3DScene* m_3dScene;
  std::deque<Z3DCanvasEventListener*> m_listeners;

  ZInteractionEngine m_interaction;
  bool m_updatingDecoration = false;

  QPixmap *m_customCanvas = NULL; //Not in use for now

  QString m_message;
  Qt::MouseButtons m_pressedButtons = Qt::NoButton;
};

#endif // Z3DCANVAS_H
