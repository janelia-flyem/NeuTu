#ifndef ZPROOFREADWINDOW_H
#define ZPROOFREADWINDOW_H

#include <QMainWindow>
#include <QPalette>

#include "common/neutudefs.h"

class ZFlyEmProofMvc;
class QStackedWidget;
class ZFlyEmMessageWidget;
class QProgressDialog;
class ZProgressSignal;
class ZDvidTarget;
class ZWidgetMessage;
class ZDvidDialog;
class QSlider;
class DvidOperateDialog;
class FlyEmBodyFilterDialog;
class ZFlyEmDataLoader;
class FlyEmProofControlForm;
class FlyEmSplitControlForm;
class ZStressTestOptionDialog;
class ZFlyEmBodyScreenshotDialog;
class ZFlyEmBodySplitDialog;
class FlyEmAuthTokenDialog;
class ProtocolAssignmentDialog;
class ZActionLibrary;

/*!
 * \brief The mainwindow class of proofreading
 */
class ZProofreadWindow : public QMainWindow
{
  Q_OBJECT
public:
  explicit ZProofreadWindow(QWidget *parent = nullptr);
  ~ZProofreadWindow();

  static ZProofreadWindow* Make(QWidget *parent = nullptr);
  static ZProofreadWindow* Make(QWidget *parent, ZDvidDialog *dvidDlg);

  QProgressDialog* getProgressDialog();

  void setDvidDialog(ZDvidDialog *dvidDlg);

  ZFlyEmProofMvc* getMainMvc() const;

public:
  void stressTest();

signals:
  void splitTriggered(uint64_t bodyId);
  void proofreadWindowClosed();
  void showingMainWindow();
//  void splitTriggered();
  /*
  void progressStarted(const QString &title, int nticks);
  void progressStarted(const QString &title);
  void progressAdvanced(double dp);
  void progressEnded();
  */

public slots:
  void launchSplit(
      uint64_t bodyId, neutu::EBodySplitMode mode = neutu::EBodySplitMode::ONLINE);
  void launchSplit();
  void exitSplit();
  void presentSplitInterface(uint64_t bodyId);
  void updateDvidTargetWidget(const ZDvidTarget &target);

  void dump(const QString &message, bool appending = true);
  void dumpError(const QString &message, bool appending = true);
  void dump(const ZWidgetMessage &msg);

  void startProgress();
  void startProgress(const QString &title, int nticks);
  void startProgress(const QString &title);
  void advanceProgress(double dp);
  void endProgress();

  void operateDvid();
  void exploreBody();

  void exportNeuronScreenshot();
  void exportNeuronMeshScreenshot();
  void exportGrayscale();
  void exportBodyStack();

  void stressTestSlot();
  void diagnose();
  void profile();
  void testSlot();

  void showSettings();

  void showAndRaise();

  void loadDatabase();
  void loadDatabaseFromUrl();

  void showAuthTokenDialog();
  void updateAuthTokenIcon();
  void showProtocolAssignmentDialog();

protected:
  void dragEnterEvent(QDragEnterEvent *event);
  void changeEvent(QEvent * event);
  void keyPressEvent(QKeyEvent *event);
  void closeEvent(QCloseEvent *event);

private:
  void init();
  void initProgress(int nticks);
  void enableTargetAction(bool on);

  template <typename T>
  void connectMessagePipe(T *source);

  void createMenu();
  void createToolbar();
  void createDialog();
  void addSynapseActionToToolbar();

  void logMessage(const QString &msg);
  void logError(const QString &msg);
//  void logMessage(const ZWidgetMessage &msg);

  void displayActiveHint(bool on);

private slots:
  void processFeedback();
  void postDvidReady();

private:
  ZFlyEmProofMvc *m_mainMvc;
  QStackedWidget *m_controlGroup;
  ZFlyEmMessageWidget *m_messageWidget;

  QMenu *m_viewMenu;
  QMenu *m_toolMenu;
  QMenu *m_advancedMenu;

  QAction *m_viewSynapseAction;
  QAction *m_viewBookmarkAction;
  QAction *m_viewSegmentationAction;
  QAction *m_viewTodoAction;
  QAction *m_viewRoiAction;

  QAction *m_importBookmarkAction = nullptr;
  QAction *m_openSequencerAction = nullptr;
  QAction *m_openProtocolsAction = nullptr;
  QAction *m_contrastAction = nullptr;
  QAction *m_tuneContrastAction = nullptr;
  QAction *m_smoothAction = nullptr;
  QAction *m_openTodoAction = nullptr;
  QAction *m_roiToolAction = nullptr;
  QAction *m_bodyExplorerAction = nullptr;
  QAction *m_neuprintAction = nullptr;
  QAction *m_loadDvidAction = nullptr;
  QAction *m_loadDvidUrlAction = nullptr;
  QAction *m_openAuthDialogAction = nullptr;
  QAction *m_openProtocolAssignmentDialogAction = nullptr;

  QAction *m_openSkeletonAction;
  QAction *m_openExtNeuronWindowAction;
  QAction *m_openObject3dAction;
  QAction *m_openRoi3dAction;

  ZActionLibrary *m_actionLibrary = nullptr;

//  QAction *m_queryTableAction;

  QAction *m_dvidOperateAction;

  QSlider *m_segSlider;

  QToolBar *m_toolBar;
  QToolBar *m_synapseToolbar;

  QProgressDialog *m_progressDlg;
  ZProgressSignal *m_progressSignal;

  FlyEmProofControlForm *m_controlForm;
  FlyEmSplitControlForm *m_splitControlForm;

  QPalette m_defaultPal;

  ZFlyEmDataLoader *m_flyemDataLoader;
  DvidOperateDialog *m_dvidOpDlg;
  FlyEmBodyFilterDialog *m_bodyFilterDlg;
  ZStressTestOptionDialog *m_stressTestOptionDlg;
  ZFlyEmBodyScreenshotDialog *m_bodyScreenshotDlg;
  ZFlyEmBodySplitDialog *m_bodySplitDlg;
  FlyEmAuthTokenDialog *m_authTokenDlg;
  ProtocolAssignmentDialog * m_protocolAssignmentDlg;


};



#endif // ZPROOFREADWINDOW_H
