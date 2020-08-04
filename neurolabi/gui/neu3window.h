#ifndef NEU3WINDOW_H
#define NEU3WINDOW_H

#include <QMainWindow>

#include "zactionfactory.h"
#include "geometry/zpoint.h"
#include "common/zsharedpointer.h"

namespace Ui {
class Neu3Window;
}

class Z3DWindow;
class Z3DCanvas;
class ZFlyEmProofMvc;
class QProgressDialog;
class QToolBar;
class ZFlyEmBody3dDoc;
class ZFlyEmProofDoc;
class ZSwcTree;
class ZMesh;
class ZBodyListWidget;
class ZROIWidget;
class FlyEmSettingDialog;
class TaskProtocolWindow;
class ZActionLibrary;
class ZFlyEmArbMvc;
class ZArbSliceViewParam;
class ZNeu3SliceViewDialog;
class ZFlyEmMessageWidget;
class ZWidgetMessage;
class ZFlyEmBodyColorScheme;
class InformationDialog;

#if defined(_USE_WEBENGINE_)
class QWebEngineView;
#endif

/*!
 * \brief The class of the main window for Neu3
 */
class Neu3Window : public QMainWindow
{
  Q_OBJECT

public:
  explicit Neu3Window(QWidget *parent = 0);
  ~Neu3Window();

  void initialize();
  void initOpenglContext();
  bool loadDvidTarget(const QString &name);

  ZFlyEmBody3dDoc* getBodyDocument() const;
  ZFlyEmProofDoc* getDataDocument() const;

  static void enableZoomToLoadedBody(bool enable = true);
  static bool zoomToLoadedBodyEnabled();

  enum class EBrowseMode {
    NONE, NATIVE, NEUROGLANCER, NEUROGLANCER_EXT
  };

  QProgressDialog* getProgressDialog();

public slots:
  void showSynapse(bool on);
  void showTodo(bool on);

  /*!
   * \brief Remove a body from the current list.
   */
  void removeBody(uint64_t bodyId);

  /*!
   * \brief Remove all bodies from the current list, which can be more efficient
   * than removing them one at a time.
   */
  void removeAllBodies();

  /*!
   * \brief Add a body to the body list.
   */
  void addBody(uint64_t bodyId);

  /*!
   * \brief Load body data.
   *
   * This function will not update the list model.
   */
  void loadBody(uint64_t bodyId);

  /*!
   * \brief Unload body data.
   *
   * This function will not update the list model.
   */
  void unloadBody(uint64_t bodyId);

  /*!
   * \brief Update the selection states of a set of bodies
   *
   * A body absent from \a bodySet will be set to unselected.
   */
  void setBodyItemSelection(const QSet<uint64_t> &bodySet);

  void zoomToBodyMesh(int numMeshLoaded);

  /*!
   * \brief Start Neu3Window
   */
  void start();

  void processMessage(const ZWidgetMessage &msg);

  bool allowingSplit(uint64_t bodyId) const;
  bool cleaving() const;

  void gotoPosition(double x, double y, double z);

signals:
  void bodySelected(uint64_t bodyId);
  void bodyDeselected(uint64_t bodyId);
  void closed();
  void dvidLoaded();
//  void updatingSliceWidget();

protected:
  virtual void keyPressEvent(QKeyEvent *event);
  void closeEvent(QCloseEvent *event);

private slots:
  void processSwcChangeFrom3D(
      QList<ZSwcTree*> selected,QList<ZSwcTree*>deselected);
  void processMeshChangedFrom3D(
      QList<ZMesh*> selected, QList<ZMesh*>deselected);

  void syncBodyListModel();

  void meshArchiveLoadingStarted();
  void meshArchiveLoadingProgress(float fraction);
  void meshArchiveLoadingEnded();

  void updateRoiWidget();
  void browse(double x, double y, double z);
  void browse(int x, int y, int z, int);
//  void browseInPlace(double x, double y, double z);

  // Launch a native grayscale browser with a custom color mapping.
  void browse(double x, double y, double z, const QHash<uint64_t, QColor> &idToColor);

  void processKeyPressed(QKeyEvent* event);
  void updateBodyState();
  void setOption();
  void openNeuTu();

  void updateWidget();
  void updateUI();
  void exitSplit();
  void startSplit();


  void processSliceViewChange();
  void updateGrayscaleWidget();
  void updateSliceBrowser();
  void updateSliceBrowserSelection();
  void updateBrowserColor(const QHash<uint64_t, QColor> &idToColor);
  void applyBrowserColorScheme();

//  void hideGrayscale();
  void processCameraRotation();
//  void closeWebView();

  void updateSliceViewGraph();
  void updateSliceViewGraph(const ZArbSliceViewParam &param);
  void removeSliceViewGraph();

  void updateSliceWidget();
  void updateSliceWidgetPlane();

  void processSliceDockVisibility(bool on);

  void test();
  void testBodyChange();

  //progress interface
  void startProgress(const QString &title, int nticks);
  void startProgress(const QString &title);
  void startProgress();
  void startProgress(double alpha);
  void advanceProgress(double dp);
  void endProgress();

  void on_actionNeuTu_Proofread_triggered();

  void on_actionDiagnose_triggered();

  void diagnose();

private:
  void createDockWidget();
  void initNativeSliceBrowser();
  void createTaskWindow();
  void createRoiWidget();
  void configureToolBar();
  void connectSignalSlot();
  void createBodyListWidget();
  void createMessageWidget();
  void createMessageDock();
  QAction* getAction(ZActionFactory::EAction key);
  void initWebView();
  void initGrayscaleWidget();
  ZArbSliceViewParam getSliceViewParam(double x, double y, double z) const;
  ZArbSliceViewParam getSliceViewParam(const ZPoint &center) const;

  void startBrowser(EBrowseMode mode);
  void endBrowse();
  void updateWebView();
  void updateBrowseSize();

  QDockWidget* getSliceViewDoc() const;
  void createDialogs();

  void trackSliceViewPort() const;

private:
  Ui::Neu3Window *ui;

  Z3DCanvas *m_sharedContext = nullptr;
  Z3DWindow *m_3dwin = nullptr;
//  ZFlyEmProofMvc *m_dataContainer = nullptr;
  QToolBar *m_toolBar = nullptr;
  ZBodyListWidget *m_bodyListWidget = nullptr;
  QDockWidget *m_bodyListDock = nullptr;
  QDockWidget *m_nativeSliceDock = nullptr;
  ZROIWidget *m_roiWidget = nullptr;
  TaskProtocolWindow *m_taskProtocolWidget = nullptr;
  QDockWidget *m_messageDock = nullptr;
  ZFlyEmMessageWidget *m_messageWidget = nullptr;
//  QWidget *m_controlWidget = nullptr;
  bool m_doingBulkUpdate = false;
  class DoingBulkUpdate;
  QProgressDialog *m_progressDialog = nullptr;
  FlyEmSettingDialog *m_flyemSettingDlg = nullptr;
  ZNeu3SliceViewDialog *m_browseOptionDlg = nullptr;
  InformationDialog *m_infoDlg = nullptr;

  QDockWidget *m_webSliceDock = nullptr;
#if defined(_USE_WEBENGINE_)
  QWebEngineView *m_webView = nullptr;
#endif
  ZFlyEmProofMvc *m_sliceWidget = nullptr;

  ZPoint m_browsePos;

  constexpr static int DEFAULT_BROWSE_WIDTH = 512;
  constexpr static int DEFAULT_BROWSE_HEIGHT = 512;
  int m_browseWidth = DEFAULT_BROWSE_WIDTH;
  int m_browseHeight = DEFAULT_BROWSE_HEIGHT;
  EBrowseMode m_browseMode = EBrowseMode::NONE;
  ZSharedPointer<ZFlyEmBodyColorScheme> m_browserColorScheme;

  QSharedPointer<ZActionLibrary> m_actionLibrary;
  QTimer *m_testTimer;
};

#endif // NEU3WINDOW_H
