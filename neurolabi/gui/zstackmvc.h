#ifndef ZSTACKMVC_H
#define ZSTACKMVC_H

#include <QObject>
#include <QWidget>
#include <QFrame>
#include <QLayout>

#if QT_VERSION >= 0x050000
#include <QtWidgets>
#endif

#include "zsharedpointer.h"
#include "neutube_def.h"
//#include "zwidgetmessage.h"

class ZStackDoc;
class ZStackView;
class ZStackPresenter;
class ZStackViewParam;
class ZProgressSignal;
class ZWidgetMessage;
class QMainWindow;
class ZIntPoint;
class ZPoint;

/*!
 * \brief The MVC class for stack operation
 *
 * This class encloses stack data, stack view and the data presenter. It is a
 * variant of ZStackFrame, with the flexibity of not using the frame.
 */
class ZStackMvc : public QWidget
{
  Q_OBJECT
public:
  explicit ZStackMvc(QWidget *parent = 0);
  virtual ~ZStackMvc();

  static ZStackMvc* Make(QWidget *parent, ZSharedPointer<ZStackDoc> doc);
  static ZStackMvc* Make(QWidget *parent, ZSharedPointer<ZStackDoc> doc,
                         NeuTube::EAxis axis);

  void attachDocument(ZStackDoc *doc);
  void attachDocument(ZSharedPointer<ZStackDoc> doc);
  void detachDocument();

  ZSharedPointer<ZStackDoc> getDocument() const {
    return m_doc;
  }

  inline ZStackPresenter* getPresenter() const {
    return m_presenter;
  }

  inline ZStackView* getView() const {
    return m_view;
  }

  /*!
   * \brief Get the global geometry of the view window.
   */
  QRect getViewGeometry() const;

  void connectSignalSlot();
  void disconnectAll();

  ZProgressSignal* getProgressSignal() const {
    return m_progressSignal;
  }

  QMainWindow* getMainWindow() const;

  virtual void processViewChangeCustom(const ZStackViewParam &/*viewParam*/) {}

  ZIntPoint getViewCenter() const;
  double getWidthZoomRatio() const;
  double getHeightZoomRatio() const;

protected: // Events
  virtual void keyPressEvent(QKeyEvent *event);

signals:
  void stackChanged();
  void objectChanged();
  void objectSelectionChanged();
  void messageGenerated(const ZWidgetMessage&);
  void viewChanged();

public slots:
  void updateActiveViewData();
  void processViewChange(const ZStackViewParam &viewParam);
  void processViewChange();

  void zoomTo(const ZIntPoint &pt);
  void zoomTo(int x, int y, int z);
  void zoomTo(int x, int y, int z, int width);
  void zoomTo(const ZIntPoint &pt, double zoomRatio);
  void zoomWithWidthAligned(int x0, int x1, int cy);
  void zoomWithWidthAligned(int x0, int x1, double pw, int cy, int cz);
  void zoomWithHeightAligned(int y0, int y1, double ph, int cx, int cz);

  void zoomWithWidthAligned(const QRect &viewPort, int z, double pw);
  void zoomWithWidthAligned(const ZStackView *view);
  void zoomWithHeightAligned(const ZStackView *view);

  void dump(const QString &msg);

  void test();

protected:
  static void BaseConstruct(
      ZStackMvc *frame, ZSharedPointer<ZStackDoc> doc, NeuTube::EAxis axis);
  virtual void customInit();
  virtual void createPresenter();
  void createPresenter(NeuTube::EAxis axis);
  virtual void createView();
  virtual void createView(NeuTube::EAxis axis);
  virtual void dragEnterEvent(QDragEnterEvent *event);
  virtual void dropEvent(QDropEvent *event);
//  virtual void focusInEvent(QFocusEvent * event);
//  virtual void focusOutEvent(QFocusEvent * event);
//  virtual void changeEvent(QEvent * event);

  typedef bool FConnectAction(
      const QObject*, const char *,
      const QObject *, const char *);

  static bool connectFunc(const QObject* obj1, const char *signal,
                          const QObject *obj2, const char *slot);

  void updateDocSignalSlot(FConnectAction connectAction);
  void updateSignalSlot(FConnectAction connectAction);

private:
  void dropDocument(ZSharedPointer<ZStackDoc> doc);
  void updateDocument();
  void construct(ZSharedPointer<ZStackDoc> doc,
                 NeuTube::EAxis axis = NeuTube::Z_AXIS);

protected:
  ZSharedPointer<ZStackDoc> m_doc;
  ZStackPresenter *m_presenter;
  ZStackView *m_view;
  QLayout *m_layout;
  ZProgressSignal *m_progressSignal;
};

#endif // ZSTACKMVC_H
