/**@file mainwindow.h
 * @brief Main window
 * @author Ting Zhao
 */
#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QMap>
#include <QFileDialog>
#include <QSettings>

#include "tz_image_lib_defs.h"
#include "dialogs/frameinfodialog.h"
#include "zstackpresenter.h"
#include "zmessagereporter.h"
#include "dialogs/moviedialog.h"
#include "dialogs/autosaveswclistdialog.h"
#include "zactionactivator.h"
#include "dialogs/flyemneuronthumbnaildialog.h"
#include "zstackdoc.h"
#include "newprojectmainwindow.h"
#include "zqtbarprogressreporter.h"
#include "zmessageprocessor.h"
#include "zwindowfactory.h"

class ZStackFrame;
class QMdiArea;
class QActionGroup;
class ZStackDoc;
class QProgressDialog;
class BcAdjustDialog;
class QUndoGroup;
class QUndoView;
class QMdiSubWindow;
class Z3DCanvas;
class ZStack;
class ZFlyEmDataFrame;
class HelpDialog;
class QProgressBar;
class DiagnosisDialog;
class PenWidthDialog;
class ZDvidClient;
class DvidObjectDialog;
class ResolutionDialog;
class DvidImageDialog;
class TileManager;
class ZTiledStackFrame;
class FlyEmBodyIdDialog;
class FlyEmHotSpotDialog;
class ZDvidDialog;
class FlyEmBodyFilterDialog;
class FlyEmBodySplitProjectDialog;
class ZFlyEmNewBodySplitProjectDialog;
class DvidSkeletonizeDialog;
class ZFlyEmRoiDialog;
class NewProjectMainWindow;
class ShapePaperDialog;
class DvidOperateDialog;
class SynapseImportDialog;
class FlyEmBodyMergeProjectDialog;
class ZSegmentationProjectDialog;
class ZStackViewManager;
class ZFlyEmProjectManager;
class ZFlyEmDataLoader;
class ZFlyEmHackathonConfigDlg;
class ZProgressManager;
class ZMessageManager;
class ZTestDialog;
class ZTestDialog2;
class ZAutoTraceDialog;
class QTimer;
class ProjectionDialog;
class ZStackSkeletonizer;
class FlyEmSkeletonizationDialog;
class ZWidgetMessage;
class FlyEmSettingDialog;
class ZDvidBodyPositionDialog;

namespace Ui {
  class MainWindow;
}

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
  MainWindow(QWidget *parent = 0);
  ~MainWindow();

  /*!
   * \brief Run configuration.
   */
  void configure();

  void initOpenglContext();

  class MessageProcessor : public ZMessageProcessor {
  public:
    void processMessage(ZMessage *message, QWidget *host) const;
  };

public: /* frame operation */
  ZStackFrame* activeStackFrame();
  ZStackFrame* currentStackFrame();
  ZFlyEmDataFrame* currentFlyEmDataFrame();
  ZTiledStackFrame* currentTiledStackFrame();
  int frameNumber();

  //Add a flyem data frame. Nothing happens if <frame> is NULL.
  void addFlyEmDataFrame(ZFlyEmDataFrame *frame);

public: /* Get useful widgets/objects */
  QProgressDialog* getProgressDialog();
  QProgressBar* getProgressBar();

  inline QUndoGroup* undoGroup() const { return m_undoGroup; }

public: /* File and message dialogs */
  QString getOpenFileName(const QString &caption,
                          const QString &filter = QString());
  QStringList getOpenFileNames(const QString &caption,
                               const QString &filter = QString());
  QString getSaveFileName(const QString &caption,
                          const QString &filter = QString());
  QString getSaveFileName(const QString &caption,
                          const QString &filter,
                          const QString &dir);
  QString getDirectory(const QString &caption);

  void report(const std::string &title, const std::string &msg,
              NeuTube::EMessageType msgType);
  bool ask(const std::string &title, const std::string &msg);

  QMenu* getSandboxMenu() const;

public:
  bool initBodySplitProject();

  static void createWorkDir();

  QAction* getBodySplitAction() const;
  void runBodySplit();

  void processArgument(const QString &arg);

public: //Testing routines
  void testFlyEmProofread();

signals:
  void dvidRequestCanceled();
  void progressDone();
  void progressAdvanced(double dp);
  void progressStarted(QString title, int nticks);
  void docReaderReady(ZStackDocReader*);
  void docReady(ZStackDocPtr);
  void fileOpenFailed(QString fileName, QString reason);

public slots:
  void addStackFrame(ZStackFrame *frame, bool isReady = true);
  void presentStackFrame(ZStackFrame *frame);
  void openFile(const QString &fileName);
  void openFile(const QStringList &fileNameList);

  void dump(const ZWidgetMessage &msg);

  void initProgress(int maxValue);
  void advanceProgress(double dp);
  void startProgress(const QString &title, int nticks);
  void startProgress();
  void endProgress();

  void updateAction();
  void updateMenu();
  void updateStatusBar();
  void runRoutineCheck();

  void on_actionTile_Manager_2_triggered();
  void cancelDvidRequest();

  //Report the problem when a file cannot be opened correctly.
  void reportFileOpenProblem(const QString &filePath,
                             const QString &reason = "");


  ZStackFrame* createEmptyStackFrame(ZStackFrame *parentFrame = NULL);

  ZStackFrame* createStackFrame(
      ZStack *stack,NeuTube::Document::ETag tag = NeuTube::Document::NORMAL,
      ZStackFrame *parentFrame = NULL);

  ZStackFrame* createStackFrame(
      Stack *stack,NeuTube::Document::ETag tag = NeuTube::Document::NORMAL,
      ZStackFrame *parentFrame = NULL);

  ZStackFrame* createStackFrame(
      ZStackDocReader *reader, ZStackFrame *parentFrame = NULL);
  ZStackFrame* createStackFrame(ZStackDocReader &reader,
      NeuTube::Document::ETag tag = NeuTube::Document::NORMAL);

  ZStackFrame* createStackFrame(ZStackDocPtr doc);

  void showStackFrame(
      const QStringList &fileList, bool opening3DWindow = false);
  void createDvidFrame();
  void createStackFrameFromDocReader(ZStackDocReader *reader);

  void launchSplit(const QString &str);

private:
  Ui::MainWindow *m_ui;

  void setActionActivity();

  void initDialog();
  void checkVersion();

  void testProgressBarFunc();

protected:
  //a virtual function from QWidget. It is called when the window is closed.
  void changeEvent(QEvent *e);
  virtual void closeEvent(QCloseEvent *event);
  virtual void dragEnterEvent(QDragEnterEvent *event);
  virtual void dropEvent(QDropEvent *event);

  void createActionMap();

  ZStackDocReader* openFileFunc(const QString &filePath);
  void openFileFunc2(const QString &filePath);
  void openFileListFunc(const QStringList fileList);
  void runSplitFunc(ZStackFrame *frame);

private slots:
  // slots for 'File' menu
  //void on_actionProjectToggle_triggered();
  void connectedThreshold(int x, int y, int z);
  void on_actionConnected_Threshold_triggered();
  void on_actionCanny_Edge_triggered();
  void on_actionWatershed_triggered();
  void on_actionSave_Stack_triggered();
  void on_actionExtract_Channel_triggered();
  void on_actionAutoMerge_triggered();
  void on_actionLoad_from_a_file_triggered();
  void on_actionSave_As_triggered();
  //void on_actionFrom_SWC_triggered();
  void on_actionAdd_Reference_triggered();
  void on_actionLoad_triggered();
  void on_actionSave_triggered();
  void on_actionManual_triggered();
  void on_actionAbout_iTube_triggered();
  void on_actionBrightnessContrast_triggered();
  void on_actionProject_triggered();
  //void on_actionRefine_Ends_triggered();
  void on_actionRemove_Small_triggered();
  void on_actionUpdate_triggered();
  void on_actionAutomatic_triggered();
  void on_actionAutomatic_Axon_triggered();
  void on_actionDisable_triggered();
  void on_actionEnhance_Line_triggered();
  void on_actionSolidify_triggered();
  void on_actionBinarize_triggered();
  void on_actionOpen_triggered();
  void on_actionEdit_Swc_triggered();
  void on_actionRescale_Swc_triggered();
  void on_action3DView_triggered();
  void save();
  void saveAs();

  //void openTrace();
  void openRecentFile();
  void expandCurrentFrame();

  // for 'File->Export'
  //void exportSwc();
  void exportBinary();
  void exportChainFileList();
  //void exportTubeConnection();
  //void exportTubeConnectionFeat();
  void exportSvg();
  void exportTraceProject();
  void exportPuncta();

  // for 'File->Import'
  //void openTraceProject();
  void importBinary();
  void importSwc();
  //void importGoodTube();
  //void importBadTube();
  //void importTubeConnection();
  void importPuncta();
  void importImageSequence();

  // slots for 'Edit' menu
//  void undo();
//  void redo();

  // slots for 'View' menu
  void updateViewMode(QAction *action);
  void viewObject(QAction *action);
  void showFrameInfo();
  void checkViewAction(QAction *action);
  void checkTraceAction(QAction *action);
  void takeScreenshot();

  // slots for 'Tools' menu
  void activateInteractiveTrace(QAction *action);
  void activateInteractiveMarkPuncta(QAction *action);
  //void buildConn();
  void manageObjs();
  void binarize();
  void bwsolid();
  void enhanceLine();
  void subtractSwcs();

  // slots for 'Options'
  void setOption();

  // slots for 'Help'
  void about();
  void test();
  void test2();

  // slots for frame
  void updateFrameInfoDlg();
  void updateActiveUndoStack();
  void removeStackFrame(ZStackFrame *frame);
  void addStackFrame(Stack *stack, bool isOwner = true);
  void addStackFrame(ZStack *stack);
  void updateViewMenu(ZInteractiveContext::ViewMode viewMode);
  void stretchStackFrame(ZStackFrame *frame);

  void updateBcDlg(const ZStackFrame *frame);
  void updateBcDlg();
  void bcAdjust();
  void autoBcAdjust();
  void on_actionMedian_Filter_triggered();

  void on_actionDistance_Map_triggered();
  void on_actionShortest_Path_Flow_triggered();
  void on_actionExpand_Region_triggered();
  void on_actionDilate_triggered();
  void on_actionExtract_Neuron_triggered();
  void on_actionSkeletonization_triggered();
  void on_actionPixel_triggered();

  //Addictional actions of an frame when it's activated
  void evokeStackFrame(QMdiSubWindow *frame);
  void on_actionImport_Network_triggered();
  void on_actionAddSWC_triggered();
  void on_actionImage_Sequence_triggered();
  void on_actionAddFlyEmNeuron_Network_triggered();
  void on_actionSynapse_Annotation_triggered();
  void on_actionPosition_triggered();
  void on_actionImportMask_triggered();
  void on_actionFlyEmSelect_triggered();
  void on_actionImportSegmentation_triggered();
  void on_actionFlyEmClone_triggered();
  void on_actionClear_Decoration_triggered();
  void on_actionFlyEmGrow_triggered();
  void on_actionFlyEmSelect_connection_triggered();
  void on_actionAxon_Export_triggered();
  void on_actionExtract_body_triggered();
//  void on_actionPredict_errors_triggered();
  void on_actionCompute_Features_triggered();
  void on_actionMexican_Hat_triggered();
  void on_actionInvert_triggered();
  void on_actionFlyEmQATrain_triggered();
  void on_actionUpdate_Configuration_triggered();
  void on_actionErrorClassifcationTrain_triggered();
  void on_actionErrorClassifcationPredict_triggered();
  void on_actionErrorClassifcationEvaluate_triggered();
  void on_actionErrorClassifcationComputeFeatures_triggered();
  void on_actionTem_Paper_Volume_Rendering_triggered();
  void on_actionTem_Paper_Neuron_Type_Figure_triggered();
  void on_actionBinary_SWC_triggered();
  void on_actionImportFlyEmDatabase_triggered();
  void on_actionMake_Movie_triggered();
  void on_actionOpen_3D_View_Without_Volume_triggered();
  void on_actionLoop_Analysis_triggered();
  void on_actionMorphological_Thinning_triggered();
  void on_actionAddMask_triggered();
  void on_actionMask_triggered();
  void on_actionShortcut_triggered();
  void on_actionMake_Projection_triggered();
  void on_actionMask_SWC_triggered();
  void on_actionAutosaved_Files_triggered();
  void on_actionDiagnosis_triggered();
  void on_actionSave_SWC_triggered();
  void on_actionSimilarity_Matrix_triggered();
  void on_actionSparse_objects_triggered();
  void on_actionDendrogram_triggered();
  void on_actionPen_Width_for_SWC_Display_triggered();
//  void on_actionDVID_Object_triggered();
//  void on_actionDvid_Object_triggered();
  void on_actionAssign_Clustering_triggered();
  void on_actionSWC_Rescaling_triggered();
  void on_actionSurface_detection_triggered();
  void on_actionMorphological_Features_triggered();
  void on_actionFeature_Selection_triggered();
  void on_actionGet_Grayscale_triggered();
  void on_actionTiles_triggered();
  void on_actionNewProject_triggered();
  void on_actionThumbnails_triggered();
  void on_actionBundle_triggered();
  void on_actionVolume_field_triggered();
  void on_actionThumbnails_2_triggered();
  void on_actionJSON_Point_List_triggered();
  void on_actionIdentify_Hot_Spot_triggered();
  void on_actionHot_Spot_Demo_triggered();
  void on_actionHDF5_Body_triggered();
  void on_actionDVID_Bundle_triggered();
  void on_actionSubmit_Skeletonize_triggered();
  void on_actionSplit_Region_triggered();
  void on_actionLoad_Body_with_Grayscale_triggered();
  void on_actionFlyEmSettings_triggered();

  void on_actionView_Labeled_Regions_triggered();

  void on_actionLoad_Large_Body_triggered();

  void on_actionBody_Split_Project_triggered();

  void on_actionSplit_Body_triggered();

  void on_actionUpdate_Skeletons_triggered();

  void on_actionCreate_Databundle_triggered();

//  void on_actionCreate_Thumbnails_triggered();
  
  void on_actionCreate_ROI_triggered();

  void on_actionFlyEmROI_triggered();

  void on_actionShape_Matching_triggered();

  void on_actionOne_Column_triggered();

  void on_actionOperateDvid_triggered();

  void on_actionGenerate_Local_Grayscale_triggered();

  void on_actionExport_Segmentation_Result_triggered();

  void on_actionBody_Touching_Analysis_triggered();

  void on_actionImportBoundBox_triggered();

  void on_actionImportSeeds_triggered();

  void on_actionUpload_Annotations_triggered();

  void on_actionMerge_Body_Project_triggered();

  void on_actionHierarchical_Split_triggered();

  void on_actionSegmentation_Project_triggered();

  void on_actionHackathonConfigure_triggered();

  void on_actionLoad_Named_Bodies_triggered();

  void on_actionHackathonSimmat_triggered();

  void on_actionHackathonEvaluate_triggered();

  void on_actionProof_triggered();

  void on_actionSubtract_Background_triggered();

  void on_actionImport_Sparsevol_Json_triggered();

  void on_actionNeuroMorpho_triggered();

  void on_actionRemove_Obsolete_Annotations_triggered();

  void on_actionGenerate_KC_c_Actor_triggered();

  void on_actionMake_Movie_MB_triggered();

  void on_actionGenerate_KC_s_Actor_triggered();

  void on_actionGenerate_MB_Actor_triggered();

  void on_actionGenerate_KC_p_Actor_triggered();

  void on_actionGenerate_All_KC_Actor_triggered();

  void on_actionGenerate_PAM_Actor_triggered();

  void on_actionGenerate_MB_Conn_Actor_triggered();

  void on_actionGet_Body_Positions_triggered();

private:
  void createActions();
  void createFileActions();
  void createEditActions();
  void createViewActions();
  void createToolActions();
  void createTraceActions();
  void createSwcActions();

  void updateActionGroup(QActionGroup *group, QAction *triggeredAction);
  void updateObjectDisplayStyle(ZStackFrame *frame, QAction *action);
  void updateTraceMode(ZStackFrame *frame, QAction *action);

  void customizeActions();
  void createMenus();
  void createContextMenu();
  void createToolBars();
  void createStatusBar();
  void readSettings();
  void writeSettings();
  bool okToContinue();
  void saveFile(const QString &fileName);
  //void openTraceProject(QString fileName);
  void setCurrentFile(const QString &fileName);
  void updateRecentFileActions();
  QString strippedName(const QString &fullFileName);

  static QSettings& getSettings();

  void enableStackActions(bool b);
  void createUndoView();
  //bool loadTraceFile(const QString &fileName);

  //Record <path> as the path (could be a file or directory) opened last time.
  void recordLastOpenPath(const QString &path);

  //Get the path opened last time.
  QString getLastOpenPath() const;

  ZStackDocReader* hotSpotDemo(int bodyId, const QString &dvidAddress,
                           const QString &dvidUuid);
  /*!
   * \brief Hotspot demo for false split
   */
  ZStackDocReader *hotSpotDemoFs(uint64_t bodyId, const QString &dvidAddress,
                           const QString &dvidUuid);

  ZStackDoc* importHdf5Body(int bodyId, const QString &hdf5Path);
  ZStackDoc* importHdf5BodyM(const std::vector<int> &bodyIdArray,
                             const QString &hdf5Path,
                             const std::vector<int> &downsampleInterval);

  ZStackDocReader* readDvidGrayScale(const QString &dvidAddress,
                                       const QString &dvidUuid,
                                       int x, int y, int z,
                                       int width, int height, int depth);

  void autoTrace(ZStackFrame *frame);

  void setSkeletonizer(
      ZStackSkeletonizer &skeletonizer,
      const FlyEmSkeletonizationDialog &dlg);

  void makeMovie();

  void generateMBKcCast(const std::string &movieFolder);
  void generateMBAllKcCast(const std::string &movieFolder);
  void generateMBPAMCast(const std::string &movieFolder);
  void generateMBONCast(const std::string &movieFolder);
  void generateMBONConnCast(const std::string &movieFolder);

private:
  QMdiArea *mdiArea;

  QStringList recentFiles;
  QString curFile;
  QString m_lastOpenedFilePath;

  enum { MaxRecentFiles = 10 };
  QAction *recentFileActions[MaxRecentFiles];
  QAction *separatorAction;

  QMenu *exportMenu;
  QMenu *importMenu;
  QMenu *objectViewMenu;
  QMenu *traceMenu;
//  QMenu *processMenu;

  // toolbars
  QToolBar *fileToolBar;
  QToolBar *editToolBar;

  // 'File' menu: 'New', 'Open', 'Import', 'Export', 'Save', 'Save As',
  //              'Close', 'Exit'
  QActionGroup *m_readActionGroup;
  QActionGroup *m_writeActionGroup;
  QActionGroup *m_viewActionGroup;
  QAction *newAction;
  QAction *openAction;
  QAction *expandAction;
  QAction *swcExportAction;
  QAction *svgExportAction;
  QAction *vrmlExportAction;
  QAction *bnExportAction;
  QAction *nsExportAction;
  QAction *nsMultipleSwcExportAction;
  QAction *connExportAction;
  QAction *connFeatExportAction;
  QAction *chainSourceExportAction;
  //QAction *projectExportAction;
  //QAction *projectImportAction;
  QAction *bnImportAction;
  QAction *swcImportAction;
  QAction *gtImportAction;
  QAction *btImportAction;
  QAction *connImportAction;
  QAction *closeAction;
  QAction *exitAction;
  QAction *punctaImportAction;
  QAction *punctaExportAction;
  QAction *imageDirImportAction;

  // 'Edit' menu: 'Cut', 'Copy', 'Paste', 'Delete', 'Undo'
  QAction *undoAction;
  QAction *redoAction;

  // 'View' menu: 'Mode' ('Normal', 'Project', 'Zoom'),
  //   'Objects' ('Hide', 'Normal', 'Solid', 'Surface', 'Skeleton')
  //   'Document'
  QActionGroup *viewMode;
  //QAction *normalAction;
  //QAction *projAction;
  QAction *zoomAction;
  QActionGroup *objectView;
  //QAction *objectViewHideAction;
  QAction *objectViewNormalAction;
  QAction *objectViewSolidAction;
  QAction *objectViewSurfaceAction;
  QAction *objectViewSkeletonAction;
  QAction *infoViewAction;
  QAction *screenshotAction;

  // 'Tools' menu:
  //   'Trace' ('Disable', 'Fit segment', 'Trace tube', 'Automatic')
  //   'Build Conn'
  //   'Process' ('Binarize', 'Solidify', ...)
  QActionGroup *interactiveTrace;
  QAction *noTraceAction;
  QAction *fitsegAction;
  QAction *traceTubeAction;
  QAction *autoTraceAction;
  QAction *subtractSwcsAction;

  QActionGroup *interactiveMarkPuncta;
  QAction *noMarkPunctaAction;
  QAction *markPunctaAction;

  QAction *buildConnAction;
  QAction *processBinarizeAction;
  QAction *processBwsolidAction;

  QAction *manageObjsAction;

  // 'Options' menu: 'Settings'
  QAction *settingAction;

  // 'Help' menu: 'About'
  //QAction *aboutAction;
  //QAction *aboutQtAction;
  QAction *testAction;
  QAction *testAction2;

  QAction *openTraceAction;

  ZStackActionActivator m_stackActionActivator;
  ZSwcActionActivator m_swcActionActivator;
  QVector<ZActionActivator*> m_actionActivatorList;

  FrameInfoDialog *m_frameInfoDlg;
  QProgressDialog *m_progress;
  BcAdjustDialog *m_bcDlg;
  HelpDialog *m_helpDlg;
  DiagnosisDialog *m_DiagnosisDlg;
  ResolutionDialog *m_resDlg;
  ZFlyEmHackathonConfigDlg *m_hackathonConfigDlg;


  // undo redo
  QUndoGroup *m_undoGroup;
  QUndoView *m_undoView;

  Z3DCanvas *m_sharedContext;

  ZMessageReporter *m_reporter;

  int m_frameCount;


  MovieDialog *m_movieDlg;
  AutosaveSwcListDialog *m_autosaveSwcDialog;

  PenWidthDialog *m_penWidthDialog;

  QMap<QString, QAction*> m_actionMap;

  ZDvidClient *m_dvidClient;
  ZStackFrame *m_dvidFrame;
  //DvidObjectDialog *m_dvidObjectDlg;
  DvidImageDialog *m_dvidImageDlg;
  TileManager *m_tileDlg;
  FlyEmBodyIdDialog *m_bodyDlg;
  FlyEmHotSpotDialog *m_hotSpotDlg;
  ZDvidDialog *m_dvidDlg;
  FlyEmBodyFilterDialog *m_bodyFilterDlg;
  FlyEmBodySplitProjectDialog *m_bodySplitProjectDialog;
  ZFlyEmNewBodySplitProjectDialog *m_newBsProjectDialog;
  DvidSkeletonizeDialog *m_dvidSkeletonizeDialog;
  ZFlyEmRoiDialog *m_roiDlg;
  ShapePaperDialog *m_shapePaperDlg;
  DvidOperateDialog *m_dvidOpDlg;
  SynapseImportDialog *m_synapseDlg;
  FlyEmBodyMergeProjectDialog *m_mergeBodyDlg;
  ZSegmentationProjectDialog *m_segmentationDlg;
  ZAutoTraceDialog *m_autoTraceDlg;
  ProjectionDialog *m_projDlg;
  FlyEmSkeletonizationDialog *m_skeletonDlg;
  FlyEmSettingDialog *m_flyemSettingDlg;
  ZDvidBodyPositionDialog *m_bodyPosDlg;

  ZStackViewManager *m_stackViewManager;
  ZFlyEmProjectManager *m_flyemProjectManager;
  ZFlyEmDataLoader *m_flyemDataLoader;
  //new project main window
  NewProjectMainWindow *m_newProject;

  //FlyEmNeuronThumbnailDialog *m_thumbnailDlg;
  QFileDialog::Options m_fileDialogOption;

  //QSettings m_settings;
  QString m_version;

  ZProgressManager *m_progressManager;
  ZQtBarProgressReporter m_specialProgressReporter;

  ZMessageManager *m_messageManager;
  ZTestDialog *m_testDlg;
  ZTestDialog2 *m_testDlg2;
  ZWindowFactory m_3dWindowFactory;

  QTimer *m_autoCheckTimer;
  //ZStackDocReader *m_docReader;
};

#endif // MAINWINDOW_H
