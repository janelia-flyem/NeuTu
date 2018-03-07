#ifndef NEU3WINDOW_H
#define NEU3WINDOW_H

#include <QMainWindow>

#include "zactionfactory.h"
#include "zpoint.h"

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
  bool loadDvidTarget();

  ZFlyEmBody3dDoc* getBodyDocument() const;
  ZFlyEmProofDoc* getDataDocument() const;

  static void enableZoomToLoadedBody(bool enable = true);
  static bool zoomToLoadedBodyEnabled();

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

  void zoomToBodyMesh();

signals:
  void bodySelected(uint64_t bodyId);
  void bodyDeselected(uint64_t bodyId);
  void closed();

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
  void browseInPlace(double x, double y, double z);
  void processKeyPressed(QKeyEvent* event);
  void updateBodyState();
  void setOption();

  void updateWidget();
  void exitSplit();
  void startSplit();

  void updateBrowser();
  void updateEmbeddedGrayscale();
  void hideGrayscale();
  void processCameraRotation();
  void closeWebView();

  void test();

private:
  void createDockWidget();
  void createTaskWindow();
  void createRoiWidget();
  void createToolBar();
  void connectSignalSlot();
  QAction* getAction(ZActionFactory::EAction key);
  void initWebView();

private:
  Ui::Neu3Window *ui;

  Z3DCanvas *m_sharedContext = nullptr;
  Z3DWindow *m_3dwin = nullptr;
  ZFlyEmProofMvc *m_dataContainer = nullptr;
  QToolBar *m_toolBar = nullptr;
  ZBodyListWidget *m_bodyListWidget = nullptr;
  QDockWidget *m_bodyListDock = nullptr;
  ZROIWidget *m_roiWidget = nullptr;
  TaskProtocolWindow *m_taskProtocolWidget = nullptr;
//  QWidget *m_controlWidget = nullptr;
  bool m_doingBulkUpdate = false;
  class DoingBulkUpdate;
  QProgressDialog *m_progressDialog = nullptr;
  FlyEmSettingDialog *m_flyemSettingDlg = nullptr;

#if defined(_USE_WEBENGINE_)
  QWebEngineView *m_webView = nullptr;
#endif

  ZPoint m_browsePos;

  QSharedPointer<ZActionLibrary> m_actionLibrary;
};

#endif // NEU3WINDOW_H
