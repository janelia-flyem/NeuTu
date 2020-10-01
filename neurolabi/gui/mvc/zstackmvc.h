#ifndef ZSTACKMVC_H
#define ZSTACKMVC_H

#include <memory>

#include <QObject>
#include <QWidget>
#include <QFrame>
#include <QLayout>
#include <QThread>
#include <QLabel>

#include "common/neutudefs.h"
#include "logging/zloggable.h"
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
class ZAffineRect;
class ZStackObjectInfoSet;
class ZPlane;
class ZAffinePlane;

/*!
 * \brief The MVC class for stack operation
 *
 * This class encloses stack data, stack view and the data presenter. It is a
 * variant of ZStackFrame, with the flexibity of not using the frame.
 */
class ZStackMvc : public QWidget, public ZLoggable
{
  Q_OBJECT
public:
  explicit ZStackMvc(QWidget *parent = 0);
  virtual ~ZStackMvc();

  static ZStackMvc* Make(QWidget *parent, std::shared_ptr<ZStackDoc> doc);
  static ZStackMvc* Make(QWidget *parent, std::shared_ptr<ZStackDoc> doc,
                         neutu::EAxis axis);
  static ZStackMvc* Make(QWidget *parent, std::shared_ptr<ZStackDoc> doc,
                         const std::vector<neutu::EAxis> &axes);

  enum class ERole {
    ROLE_WIDGET, ROLE_DOCUMENT
  };

  void attachDocument(ZStackDoc *doc);
  void attachDocument(std::shared_ptr<ZStackDoc> doc);
  void detachDocument();

  std::shared_ptr<ZStackDoc> getDocument() const {
    return m_doc;
  }

  inline ZStackPresenter* getPresenter() const {
    return m_presenter;
  }

  ZStackView *getMainView() const;
  ZStackView* getDefaultView() const;
  ZStackView* getView(int viewId) const;
  std::vector<ZStackView*> getViewList() const;
  ZStackView* getMouseHoveredView() const;

  void forEachView(std::function<void(ZStackView*)> f) const;
  void forEachVisibleView(std::function<void(ZStackView*)> f) const;

  void setViewReady();

  /*!
   * \brief Get the global geometry of the view window.
   */
  QRect getViewGeometry(int viewId) const;

  virtual void connectSignalSlot();
  void disconnectAll();

  ZProgressSignal* getProgressSignal() const {
    return m_progressSignal;
  }

  QMainWindow* getMainWindow() const;

  virtual void processViewChangeCustom(const ZStackViewParam &/*viewParam*/) {}

  ZIntPoint getViewCenter() const;
  QSize getViewScreenSize() const;
  ZAffineRect getViewPort() const;
  void setDefaultViewPort(const QRect &rect);
  neutu::EAxis getSliceAxis() const;

  void setCutPlane(const ZPoint &v1, const ZPoint &v2);
  void setCutPlane(const ZPlane &plane);
  void setCutPlane(const ZAffinePlane &plane);
  void setCutPlane(const ZPoint &center, const ZPoint &v1, const ZPoint &v2);
  void setZoomScale(double s);
  void setInitialScale(double s);

  void toggleStressTest();
  virtual void stressTest(ZStressTestOptionDialog *dlg);

  ERole getRole() const;
  void setRole(ERole role);

  void blockViewChangeSignal(bool blocking);


signals:
//  void stackChanged();
//  void objectChanged();
//  void objectSelectionChanged();
  void messageGenerated(const ZWidgetMessage&);
  void viewChanged();

public slots:
//  void updateActiveViewData();
//  void processViewChange(const ZStackViewParam &viewParam);
  void processViewChange(int viewId);

  virtual void zoomTo(const ZIntPoint &pt);
  void zoomTo(int x, int y, int z);
  void zoomToL1(int x, int y, int z);
  void zoomTo(int x, int y, int z, int width);
  void zoomTo(const ZIntPoint &pt, double zoomRatio);
//  void zoomTo(const ZStackViewParam &param);
//  void zoomWithWidthAligned(int x0, int x1, int cy);
//  void zoomWithWidthAligned(int x0, int x1, double pw, int cy, int cz);
//  void zoomWithHeightAligned(int y0, int y1, double ph, int cx, int cz);
  void goToSlice(int z);
//  void stepSlice(int dz);

//  void zoomWithWidthAligned(const QRect &viewPort, int z, double pw);
  void zoomWithWidthAligned(const ZStackView *view);
  void zoomWithHeightAligned(const ZStackView *view);

  void updateViewLayout(std::vector<int> viewLayoutIndices);

  void dump(const QString &msg);

  void saveStack();

  virtual bool processKeyEvent(QKeyEvent *event);
  virtual void processObjectModified(const ZStackObjectInfoSet &objSet);

  virtual void testSlot();

protected: // Events
  virtual void keyPressEvent(QKeyEvent *event);
  bool event(QEvent *event);

protected:
  void setStressTestEnv(ZStressTestOptionDialog *optionDlg);
  virtual void prepareStressTestEnv(ZStressTestOptionDialog *optionDlg);

protected:
  static void BaseConstruct(
      ZStackMvc *frame, std::shared_ptr<ZStackDoc> doc, neutu::EAxis axis);
  static void BaseConstruct(
      ZStackMvc *frame, std::shared_ptr<ZStackDoc> doc,
      const std::vector<neutu::EAxis> &axes);
  virtual void customInit();
  virtual void createPresenter();
  void createPresenter(neutu::EAxis axis, int viewCount);
  virtual void createView();
  virtual void createView(neutu::EAxis axis);
  virtual void dragEnterEvent(QDragEnterEvent *event);
  virtual void dropEvent(QDropEvent *event);
//  virtual void focusInEvent(QFocusEvent * event);
//  virtual void focusOutEvent(QFocusEvent * event);
//  virtual void changeEvent(QEvent * event);

  void layoutView();
  void layoutView(ZStackView *view, int index);

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

  int getViewIndex(ZStackView *view);
  int getViewIndex(int viewId);

private:
  void dropDocument(std::shared_ptr<ZStackDoc> doc);
  void updateDocument();
  void construct(std::shared_ptr<ZStackDoc> doc,
                 neutu::EAxis axis = neutu::EAxis::Z);
  void construct(
      std::shared_ptr<ZStackDoc> doc,  const std::vector<neutu::EAxis> &axes);

private slots:
  void shortcutTest();

protected:
  std::shared_ptr<ZStackDoc> m_doc;
  ZStackPresenter *m_presenter;
  std::vector<ZStackView*> m_viewList;
  std::vector<int> m_viewLayoutIndices;
  QVBoxLayout *m_layout;
  QHBoxLayout *m_topLayout;
  QGridLayout *m_viewLayout;
  QLabel *m_infoLabel = nullptr;
  ZProgressSignal *m_progressSignal;
  QTimer *m_testTimer;
  ERole m_role;

  bool m_signalingViewChange = true;
};

#endif // ZSTACKMVC_H
