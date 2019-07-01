#ifndef ZSTACKMVC_H
#define ZSTACKMVC_H

#include <QObject>
#include <QWidget>
#include <QFrame>
#include <QLayout>
#include <QThread>

#include "common/zsharedpointer.h"
#include "common/neutube_def.h"
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
class ZStressTestOptionDialog;

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
                         neutu::EAxis axis);

  enum class ERole {
    ROLE_WIDGET, ROLE_DOCUMENT
  };

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
  QSize getViewScreenSize() const;
  QRect getViewPort() const;
  void setDefaultViewPort(const QRect &rect);

  void toggleStressTest();
  virtual void stressTest(ZStressTestOptionDialog *dlg);

  ERole getRole() const;
  void setRole(ERole role);


signals:
//  void stackChanged();
//  void objectChanged();
//  void objectSelectionChanged();
  void messageGenerated(const ZWidgetMessage&);
  void viewChanged();

public slots:
//  void updateActiveViewData();
  void processViewChange(const ZStackViewParam &viewParam);
  void processViewChange();

  void zoomTo(const ZIntPoint &pt);
  void zoomTo(int x, int y, int z);
  void zoomToL1(int x, int y, int z);
  void zoomTo(int x, int y, int z, int width);
  void zoomTo(const ZIntPoint &pt, double zoomRatio);
  void zoomTo(const ZStackViewParam &param);
//  void zoomWithWidthAligned(int x0, int x1, int cy);
//  void zoomWithWidthAligned(int x0, int x1, double pw, int cy, int cz);
//  void zoomWithHeightAligned(int y0, int y1, double ph, int cx, int cz);
  void goToSlice(int z);
  void stepSlice(int dz);

//  void zoomWithWidthAligned(const QRect &viewPort, int z, double pw);
  void zoomWithWidthAligned(const ZStackView *view);
  void zoomWithHeightAligned(const ZStackView *view);

  void dump(const QString &msg);

  void saveStack();

  virtual bool processKeyEvent(QKeyEvent *event);

  virtual void testSlot();

protected: // Events
  virtual void keyPressEvent(QKeyEvent *event);
  bool event(QEvent *event);

protected:
  void setStressTestEnv(ZStressTestOptionDialog *optionDlg);
  virtual void prepareStressTestEnv(ZStressTestOptionDialog *optionDlg);

protected:
  static void BaseConstruct(
      ZStackMvc *frame, ZSharedPointer<ZStackDoc> doc, neutu::EAxis axis);
  virtual void customInit();
  virtual void createPresenter();
  void createPresenter(neutu::EAxis axis);
  virtual void createView();
  virtual void createView(neutu::EAxis axis);
  virtual void dragEnterEvent(QDragEnterEvent *event);
  virtual void dropEvent(QDropEvent *event);
//  virtual void focusInEvent(QFocusEvent * event);
//  virtual void focusOutEvent(QFocusEvent * event);
//  virtual void changeEvent(QEvent * event);

  typedef bool FConnectAction(
      const QObject*, const char *,
      const QObject *, const char *,
      Qt::ConnectionType connetionType);

  static bool connectFunc(const QObject* obj1, const char *signal,
                          const QObject *obj2, const char *slot,
                          Qt::ConnectionType connetionType);
  static bool disconnectFunc(const QObject* obj1, const char *signal,
                             const QObject *obj2, const char *slot,
                             Qt::ConnectionType connetionType);

  void updateDocSignalSlot(FConnectAction connectAction);
  void updateSignalSlot(FConnectAction connectAction);

private:
  void dropDocument(ZSharedPointer<ZStackDoc> doc);
  void updateDocument();
  void construct(ZSharedPointer<ZStackDoc> doc,
                 neutu::EAxis axis = neutu::EAxis::Z);

private slots:
  void shortcutTest();

protected:
  ZSharedPointer<ZStackDoc> m_doc;
  ZStackPresenter *m_presenter;
  ZStackView *m_view;
  QLayout *m_layout;
  ZProgressSignal *m_progressSignal;
  QTimer *m_testTimer;
  ERole m_role;
};

#endif // ZSTACKMVC_H
