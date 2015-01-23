#ifndef _ZSTACKFRAME_H_
#define _ZSTACKFRAME_H_

/**@file zstackframe.h
 * @brief Stack frame
 * @author Ting Zhao
 */

#include <QMdiSubWindow>
#include <QString>
#include <QStringList>
#include <QUrl>
#include <QStatusBar>
#include "tz_image_lib_defs.h"
#include "plotsettings.h"
#include "zinteractivecontext.h"
#include "zstackdrawable.h"
#include "zqtbarprogressreporter.h"
#include "zrescaleswcdialog.h"
#include "zdocumentable.h"
#include "neutube.h"
#include "zreportable.h"
#include "neutube.h"
#include "ztilemanager.h"
#include "z3dwindow.h"
#include "zsharedpointer.h"

class ZStackView;
class ZStackPresenter;
class SettingDialog;
class ZLocsegChain;
class ZTraceProject;
class QProgressBar;
class QUndoStack;
class ZCurve;
class QUndoCommand;
class ZStack;
class ZStackDoc;
class ZTileManager;
class ZStackDocReader;
class MainWindow;

class ZStackFrame : public QMdiSubWindow, public ZReportable
{
  Q_OBJECT

public:
  ZStackFrame(QWidget *parent = 0, bool preparingModel = true);
  virtual ~ZStackFrame();

public:
  // A frame has three parts: view, document and presenter
  inline ZStackView* view() const { return m_view; }
  inline ZSharedPointer<ZStackDoc> document() const { return m_doc; }
  inline ZStackPresenter *presenter() const { return m_presenter; }

  virtual void constructFrame();
  virtual void createView();
  virtual void createPresenter();
  virtual void createDocument();
  virtual void connectSignalSlot();

  inline bool isClosing() const { return m_isClosing; }
  inline bool hasProject() const { return (m_traceProject != NULL); }
  bool isReadyToSave() const;
  static inline QString defaultTraceProjectFile() { return "project.xml"; }
  void addDocData(const ZStackDocReader &reader);

  inline virtual std::string name() { return "base"; }

public:
  void loadStack(Stack *stack, bool isOwner = true);
  void loadStack(ZStack *stack);
  int readStack(const char *filePath);
  //int loadTraceProject(const char *filePath, QProgressBar *pb = NULL);
  void saveTraceProject(const QString &filePath, const QString &output,
                        const QString &prefix);
  void readLocsegChain(const char *filePath);
  int importImageSequence(const char *filePath);

  void saveProject();
  void saveProjectAs(const QString &path);
  void importSwcAsReference(const QStringList &pathList);
  //void exportSwc(const QString &filePath);
  void exportPuncta(const QString &filePath);

  void importMask(const QString &filePath);
  void importSwc(const QString &filePath);

  void importSobj(const QStringList &fileList);

  void importPointList(const QString &filePath);

  void importSeedMask(const QString &filePath);

  //void exportVrml(const QString &filePath);
  void exportTube(const QString &filePath);
  void exportChainFileList(const QString &filePath);
  //void exportChainConnection(const QString &filePath);
  //void exportChainConnectionFeat(const QString &filePath);
  void exportObjectMask(const QString &filePath);
  void exportObjectMask(NeuTube::EColor color, const QString &filePath);
  ZStack* getObjectMask();
  ZStack* getObjectMask(NeuTube::EColor color);
  ZStack* getStrokeMask();
  ZTileManager* getTileManager() {return m_tile;}
  void setTileManager(ZTileManager *p) {m_tile = p; }

  void saveStack(const QString &filePath);

  void showSetting();
  void showManageObjsDialog();

  double displayGreyMin(int c=0) const;
  double displayGreyMax(int c=0) const;

  void displayActiveDecoration(bool enabled = true);

  ZInteractiveContext::ViewMode getViewMode() const;
  void setViewMode(ZInteractiveContext::ViewMode mode);
  void setObjectDisplayStyle(ZStackObject::Display_Style style);
  void setViewPortCenter(int x, int y, int z);
  void viewRoi(int x, int y, int z, int radius);

  ZStackObject::Display_Style getObjectStyle() const;
  void setObjectStyle(ZStackObject::Display_Style style);

  void hideObject();
  void showObject();

  Z3DWindow* open3DWindow(QWidget *parent,
                          Z3DWindow::EInitMode mode = Z3DWindow::NORMAL_INIT);
  /*!
   * \brief Get the main window ancestor of the frame.
   */
  MainWindow* getMainWindow();
  QAction* getBodySplitAction();

  void load(const QList<QUrl> &urls);
  void load(const QStringList &fileList);
  void load(const QString &filePath);
  void load(const std::string &filePath);

  void disconnectAll();

  virtual inline std::string classType() const { return "base"; }

  void takeScreenshot(const QString &filename);

  void findLoopInStack();
  void bwthin();

public:
  void setViewInfo(const QString &info);
  void setViewInfo();
  QString briefInfo() const;
  QString info() const;
  void updateInfo();
  QStringList toStringList() const;

  ZCurve curveToPlot(PlotSettings *settings = NULL, int option = 1) const;

public: //frame parameters
  double xResolution();
  double yResolution();
  double zResolution();
  double xReconstructScale();
  double zReconstructScale();
  char unit();
  int traceEffort();
  double traceMinScore();
  bool traceMasked();
  double reconstructDistThre();
  bool crossoverTest();
  bool singleTree();
  bool removeOvershoot();
  int reconstructRootOption();
  BOOL reconstructSpTest();
  void synchronizeSetting();
  void synchronizeDocument();

public: //set frame parameters
  void setResolution(const double *res);
  void setBc(double greyScale, double greyOffset, int channel);
  void autoBcAdjust();

public:
  void addDecoration(ZStackObject *obj);
  void clearDecoration();
  void updateView();
  void undo();
  void redo();
  void pushUndoCommand(QUndoCommand *command);
  void pushBinarizeCommand();
  void pushBwsolidCommand();
  void pushEnhanceLineCommand();

  void executeSwcRescaleCommand(const ZRescaleSwcSetting &setting);
  void executeAutoTraceCommand(bool doResample);
  void executeAutoTraceAxonCommand();
  void executeWatershedCommand();

  void executeAddObjectCommand(ZStackObject *obj);

  ZStackFrame *spinoffStackSelection(const std::vector<int> &selected);

  ZStackFrame *spinoffStackSelection(
      const std::vector<std::vector<double> > &selected);

  void invertStack();
  void removeChildFrame(ZStackFrame *frame);
  void detachParentFrame();
  void removeAllChildFrame();
  void setParentFrame(ZStackFrame *frame);
  inline ZStackFrame* getParentFrame() { return m_parentFrame; }

  void setSizeHintOption(NeuTube::ESizeHintOption option);
  /*!
   * Remove all existing decorations if isExclusive is true.
   */
  void loadRoi(const QString &filePath, bool isExclusive);
  void loadRoi(bool isExclusive);

  void prepareDisplay();

  void runSeededWatershed();

  QString swcFilename;
  void makeSwcProjection(ZStackDoc *doc);

  void clearData();

  void createMainWindowActions();

public slots:
  void setLocsegChainInfo(ZLocsegChain *chain, QString prefix = "",
                          QString suffix = "");
  void changeWindowTitle(bool clean);
  void detach3DWindow();
  void setupDisplay();
  void zoomToSelectedSwcNodes();
  void notifyUser(const QString &message);
  void locateSwcNodeIn3DView();

private slots:
  void updateSwcExtensionHint();

signals:
  void infoChanged();
  void closed(ZStackFrame*);
  void presenterChanged();
  void stackLoaded();
  void ready(ZStackFrame*);

protected: // Events
  virtual void keyPressEvent(QKeyEvent *event);

protected:
  void clear();
  void closeEvent(QCloseEvent *event);
  void resizeEvent(QResizeEvent *event);
  virtual void dragEnterEvent(QDragEnterEvent *event);
  virtual void dropEvent(QDropEvent *event);

  void consumeDocument(ZStackDoc *doc);
  void setDocument(ZSharedPointer<ZStackDoc> doc);

private:
  void setView(ZStackView *view);

protected:
  SettingDialog *m_settingDlg;
  QDialog *m_manageObjsDlg;

  ZSharedPointer<ZStackDoc> m_doc;
  ZStackPresenter *m_presenter;
  ZStackView *m_view;
  ZStackFrame *m_parentFrame;
  QList<ZStackFrame*> m_childFrameList;
  ZTileManager* m_tile;

  ZTraceProject *m_traceProject;

  QString m_statusInfo;
  bool m_isClosing;

  Z3DWindow *m_3dWindow;

  ZQtBarProgressReporter m_progressReporter;
};

#endif
