#ifndef ZFLYEMBODYSPLITPROJECT_H
#define ZFLYEMBODYSPLITPROJECT_H

#include <QObject>
#include <QList>
#include <QMutex>

#include <set>
#include "dvid/zdvidreader.h"
#include "dvid/zdvidwriter.h"
#include "flyem/zflyembookmarklistmodel.h"
#include "flyem/zflyembookmarkarray.h"
#include "zthreadfuturemap.h"
#include "zsharedpointer.h"
#include "dvid/zdvidinfo.h"
#include "zprogresssignal.h"

class ZStackFrame;
class Z3DWindow;
class ZStackObject;
class ZSwcTree;
class ZObject3dScan;
class ZFlyEmNeuron;
class ZStack;
class ZStackDoc;
class ZStackViewParam;
class ZWidgetMessage;
class ZFlyEmSplitUploadOptionDialog;

class ZFlyEmBodySplitProject : public QObject
{
  Q_OBJECT

public:
  explicit ZFlyEmBodySplitProject(QObject *parent = 0);
  virtual ~ZFlyEmBodySplitProject();

  /*!
   * \brief Clear all resources associated with the project.
   */
  void clear();

  /*!
   * \brief Exit the working state.
   *
   *
   */
  void exit();

  void setDvidTarget(const ZDvidTarget &target);
  void setBodyId(uint64_t bodyId);

  uint64_t getBodyId() const;
  inline const ZDvidTarget& getDvidTarget() const {
    return m_reader.getDvidTarget(); }
  void setDvidInfo(const ZDvidInfo &info);

  ZFlyEmNeuron getFlyEmNeuron() const;

  bool hasDataFrame() const;
  void setDataFrame(ZStackFrame *frame);
  inline ZStackFrame* getDataFrame() const {
    return m_dataFrame;
  }

  void setDocument(ZSharedPointer<ZStackDoc> doc);
  ZStackDoc* getDocument() const;
  ZSharedPointer<ZStackDoc> getSharedDocument() const;

  template<typename T>
  T* getDocument() const;

  void loadBookmark(const QString &filePath);
  std::set<int> getBookmarkBodySet() const;

  /*
  const ZFlyEmBookmarkArray* getBookmarkArray() const {
    return m_bookmarkArray;
  }
  */

  bool hasBookmark() const;
  int getBookmarkCount() const;

  void locateBookmark(const ZFlyEmBookmark &bookmark);

//  void clearBookmarkDecoration();
  void addBookmarkDecoration(const ZFlyEmBookmarkArray &bookmarkArray);
//  void updateBookmarkDecoration();

  void setBookmarkVisible(bool visible);
//  void updateBookmarkDecoration(const ZFlyEmBookmarkArray &bookmarkArray);

//  void removeAllBookmark();

//  void showSkeleton(ZSwcTree *tree);
//  void showBodyQuickView();

//  ZObject3dScan* readBody(ZObject3dScan *out) const;

  void saveSeed(bool emphasizingMessage);
  void deleteSavedSeed();
  void downloadSeed();
  void recoverSeed();
  void exportSeed(const QString &fileName);
  void importSeed(const QString &fileName);
  int selectSeed(int label);
  int selectAllSeed();
  void loadSeed(const ZJsonObject &obj);
  void swapMainSeedLabel(int label);

  std::string saveTask() const;
  std::string saveTask(uint64_t bodyId) const;

  void exportSplits();
  void commitResult();
  void commitResultFunc(ZObject3dScan *wholeBody, const ZStack *stack,
      /*const ZIntPoint &dsIntv,*/ size_t minObjSize);
  void commitResultFunc(
      ZObject3dScan *wholeBody, const std::vector<ZObject3dScan*> &objArray,
      size_t minObjSize, bool checkingIsolation);

  void commitCoarseSplit(const ZObject3dScan &splitPart);
  void decomposeBody(ZFlyEmSplitUploadOptionDialog *dlg);
  void cropBody(ZFlyEmSplitUploadOptionDialog *dlg);
  void chopBody(int v, neutube::EAxis axis, ZFlyEmSplitUploadOptionDialog *dlg);
  void chopBodyX(int x, ZFlyEmSplitUploadOptionDialog *dlg);
  void chopBodyY(int y, ZFlyEmSplitUploadOptionDialog *dlg);
  void chopBodyZ(int z, ZFlyEmSplitUploadOptionDialog *dlg);

  void viewPreviousSlice();
  void viewNextSlice();
  void viewFullGrayscale();
  void viewFullGrayscale(bool viewing);
  void updateBodyMask();
//  void downloadBodyMask();

  void setShowingBodyMask(bool state){
    m_showingBodyMask = state;
  }

  void setSplitMode(flyem::EBodySplitMode mode) {
    m_splitMode = mode;
  }

  flyem::EBodySplitMode getSplitMode() const {
    return m_splitMode;
  }

  void setMinObjSize(size_t s) { m_minObjSize = s; }
  void keepMainSeed(bool keeping) { m_keepingMainSeed = keeping; }
  void enableCca(bool state) { m_runningCca = state; }

  std::string getSplitStatusName() const;
  std::string getSplitLabelName() const;

  std::string getSeedKey(uint64_t bodyId) const;
  std::string getBackupSeedKey(uint64_t bodyId) const;
  bool isSeedProcessed(uint64_t bodyId) const;
  void setSeedProcessed(uint64_t bodyId);

  class ThreadManager {
  public:
    enum EThreadName {
      THREAD_SPLIT, THREAD_PROCESS_ALL_SEED
    };

    void updateThread(EThreadName name, QFuture<void> future);
    bool isAlive(EThreadName);

  private:
    ZThreadFutureMap m_futureMap;
  };

  void closeBodyWindow();

  bool isReadyForSplit(const ZDvidTarget &target);

  void emitMessage(const QString &msg, bool appending = true);
  void emitPopoupMessage(const QString &msg);
  void emitError(const QString &msg, bool appending = true);

  ZProgressSignal* getProgressSignal() const;

  void attachBookmarkArray(ZFlyEmBookmarkArray *bookmarkArray);

  void waitResultQuickView();

signals:
  void messageGenerated(QString, bool appending = true);
  void errorGenerated(QString, bool appending = true);

  void messageGenerated(const ZWidgetMessage&);
//  void errorGenerated(QStringList);
  void resultCommitted();

  void progressStarted(const QString &title, int nticks);
  void progressDone();
  void progressAdvanced(double dp);
  void locating2DViewTriggered(int x, int y, int z, int width);
  void bodyQuickViewReady();
  void result3dQuickViewReady();
  void rasingResultQuickView();
  void rasingBodyQuickView();

public slots:
  void start();
  void showDataFrame() const;
  void showDataFrame3d();
//  void showResult3d();
  void showResultQuickView();
//  void showBookmark(bool visible);
  void runSplit();
  void runFullSplit();
  void runLocalSplit();
  void updateResult3dQuick();
  void backupSeed();
//  void startBodyQuickView();
  void startResultQuickView();
  void startQuickView(Z3DWindow *window);
//  void raiseBodyQuickView();
  void raiseResultQuickView();
  void updateSplitQuick();

  /*!
   * \brief Clear the project without deleting the associated widgets
   *
   * This is usually used when the widgets are destroyed outside.
   */
  void shallowClear();
  void shallowClearDataFrame();
  void shallowClearResultWindow();
  void shallowClearQuickResultWindow();
  void shallowClearQuickViewWindow();
  //void shallowClearBodyWindow();

  void update3DViewPlane();

  void updateSplitDocument();

private slots:
  void resetQuickResultWindow();
  void updateSplitQuickFunc();
  void invalidateSplitQuick();

private:
  bool showingBodyMask() const { return m_showingBodyMask; }
  void clear(QWidget *widget);
  void loadResult3dQuick(ZStackDoc *doc);
  void loadResult3dQuick(ZSharedPointer<ZStackDoc> doc);
  void downloadSeed(const std::string &seedKey);
  void removeAllSeed();
  void removeAllSideSeed();
//  void updateResult3dQuickFunc();

  void clearQuickResultWindow();
//  void quickViewFunc();
//  void showBodyQuickView();
//  void showResultQuickView();
  void showQuickView(Z3DWindow *window);
  void result3dQuickFunc();
  void quitResultUpdate();
  void cancelResultUpdate();

  int getMinObjSize() const { return m_minObjSize; }
  bool keepingMainSeed() const { return m_keepingMainSeed; }

  ZJsonArray getSeedJson() const;
  ZJsonArray getRoiJson() const;
  ZIntCuboid getSeedBoundBox() const;

  void updateBodyId();
  void processSmallBodyGroup(
      ZObject3dScan *body, size_t minObjSize, ZObject3dScan *smallBodyGroup);
  void processIsolation(ZObject3dScan &currentBody, ZObject3dScan *body,
      QList<ZObject3dScan> &splitList, QList<uint64_t> &oldBodyIdList,
      const ZObject3dScan *obj, size_t minIsolationSize);

  const ZDvidReader &getMainReader() const {
    return m_reader;
  }

  ZDvidReader &getMainReader() {
    return m_reader;
  }

  ZDvidReader& getCommitReader();
  ZDvidWriter& getCommitWriter();
  ZDvidWriter& getMainWriter();

  void updateBodyDep(uint64_t bodyId);
  void updateBodyDep(uint64_t bodyId1, uint64_t bodyId2);
  void updateBodyDep(const std::vector<uint64_t> &bodyArray);


private:
  ZDvidReader m_reader;
  ZDvidWriter m_writer;
  ZDvidReader m_commitReader;
  ZDvidWriter m_commitWriter;

//  ZDvidTarget m_dvidTarget;
  ZDvidInfo m_dvidInfo;
  uint64_t m_bodyId;
  ZStackFrame *m_dataFrame;
  ZSharedPointer<ZStackDoc> m_doc;
  ZSharedPointer<ZStackDoc> m_quickResultDoc;
//  Z3DWindow *m_bodyWindow;
//  Z3DWindow *m_resultWindow; //Result window with split volumes * obsolete
  Z3DWindow *m_quickResultWindow; //Result window with split surfaces
//  Z3DWindow *m_quickViewWindow; //Obsolete

  size_t m_minObjSize;
  bool m_keepingMainSeed;
  bool m_runningCca;
  flyem::EBodySplitMode m_splitMode;

//  ZFlyEmBookmarkArray *m_bookmarkArray; //aggregation

//  std::vector<ZStackObject*> m_bookmarkDecoration;
  bool m_isBookmarkVisible;
  bool m_showingBodyMask;
  size_t m_skelThre;

  ZThreadFutureMap m_futureMap;
  QTimer *m_timer;

  QMutex m_splitWindowMutex;
  bool m_cancelSplitQuick = false;
  bool m_splitUpdated = false;

  ZProgressSignal *m_progressSignal;

  static const char* THREAD_RESULT_QUICK;
};

template <typename T>
T* ZFlyEmBodySplitProject::getDocument() const
{
  return qobject_cast<T*>(getDocument());
}

#endif // ZFLYEMBODYSPLITPROJECT_H
