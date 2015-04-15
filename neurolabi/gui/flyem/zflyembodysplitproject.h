#ifndef ZFLYEMBODYSPLITPROJECT_H
#define ZFLYEMBODYSPLITPROJECT_H

#include <QObject>
#include <set>
#include "dvid/zdvidtarget.h"
#include "flyem/zflyembookmarklistmodel.h"
#include "qthreadfuturemap.h"

class ZStackFrame;
class Z3DWindow;
class ZStackObject;
class ZSwcTree;
class ZObject3dScan;
class ZFlyEmNeuron;
class ZStack;
class ZStackDoc;

class ZFlyEmBodySplitProject : public QObject
{
  Q_OBJECT

public:
  explicit ZFlyEmBodySplitProject(QObject *parent = 0);
  virtual ~ZFlyEmBodySplitProject();


  void clear();

  void setDvidTarget(const ZDvidTarget &target);
  inline void setBodyId(int bodyId) {
    m_bodyId = bodyId;
  }

  inline int getBodyId() const { return m_bodyId; }
  inline const ZDvidTarget& getDvidTarget() const { return m_dvidTarget; }

  ZFlyEmNeuron getFlyEmNeuron() const;

  bool hasDataFrame() const;
  void setDataFrame(ZStackFrame *frame);
  inline ZStackFrame* getDataFrame() const {
    return m_dataFrame;
  }

  void loadBookmark(const QString &filePath);
  std::set<int> getBookmarkBodySet() const;

  const ZFlyEmBookmarkArray& getBookmarkArray() const {
    return m_bookmarkArray;
  }

  void locateBookmark(const ZFlyEmBookmark &bookmark);

  void clearBookmarkDecoration();
  void addBookmarkDecoration(const ZFlyEmBookmarkArray &bookmarkArray);
  void updateBookDecoration();

  void removeAllBookmark();

  void showSkeleton(ZSwcTree *tree);
  void quickView();

  ZObject3dScan* readBody(ZObject3dScan *out) const;

  void saveSeed();
  void downloadSeed();

  void exportSplits();
  void commitResult();
  void commitResultFunc(
      const ZObject3dScan *wholeBody, const ZStack *stack, const ZIntPoint &dsIntv);

  void viewPreviousSlice();
  void viewNextSlice();
  void viewFullGrayscale();
  void updateBodyMask();

  void setShowingBodyMask(bool state){
    m_showingBodyMask = state;
  }

  std::string getSplitStatusName() const;
  std::string getSplitLabelName() const;

  std::string getSeedKey(int bodyId) const;
  bool isSeedProcessed(int bodyId) const;
  void setSeedProcessed(int bodyId);

  class ThreadManager {
  public:
    enum EThreadName {
      THREAD_SPLIT, THREAD_PROCESS_ALL_SEED
    };

    void updateThread(EThreadName name, QFuture<void> future);
    bool isAlive(EThreadName);

  private:
    QThreadFutureMap m_futureMap;
  };

signals:
  void messageGenerated(QString);
  void resultCommitted();

  void progressStarted(const QString &title, int nticks);
  void progressDone();
  void progressAdvanced(double dp);

public slots:
  void showDataFrame() const;
  void showDataFrame3d();
  void showResult3d();
  void showResult3dQuick();
  void showBookmark(bool visible);
  void runSplit();
  void updateResult3dQuick();

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

private:
  bool showingBodyMask() const { return m_showingBodyMask; }
  void clear(QWidget *widget);
  void loadResult3dQuick(ZStackDoc *doc);

private:
  ZDvidTarget m_dvidTarget;
  int m_bodyId;
  ZStackFrame *m_dataFrame;
  Z3DWindow *m_resultWindow;
  Z3DWindow *m_quickResultWindow;
  Z3DWindow *m_quickViewWindow;
  ZFlyEmBookmarkArray m_bookmarkArray;
  std::vector<ZStackObject*> m_bookmarkDecoration;
  bool m_isBookmarkVisible;
  bool m_showingBodyMask;
};

#endif // ZFLYEMBODYSPLITPROJECT_H
