#ifndef ZFLYEMBODYMERGEPROJECT_H
#define ZFLYEMBODYMERGEPROJECT_H

#include <QObject>
#include <QList>
#include <QMap>

#include "neutube.h"
#include "common/zsharedpointer.h"
#include "tz_stdint.h"
#include "zstackobjectselector.h"
//#include "zstackviewparam.h"

#include "dvid/zdvidwriter.h"

#include "zflyembookmarkarray.h"
#include "zflyembodyannotation.h"
#include "zflyembodyannotationmerger.h"

class ZStackFrame;
class ZFlyEmBodyMergeFrame;
class Z3DWindow;
class ZStackObject;
class ZSwcTree;
class ZObject3dScan;
class ZFlyEmNeuron;
class ZIntPoint;
class ZStackDocReader;
class ZArray;
class Z3DMainWindow;
class Z3DTabWidget;
class Z3DWindow;
class ZStackDoc;
class ZFlyEmBodyMerger;
class ZWidgetMessage;
//class ZDvidInfo;
class ZProgressSignal;

//class ZStackViewParam;

class ZFlyEmBodyMergeProject : public QObject
{
  Q_OBJECT
public:
  explicit ZFlyEmBodyMergeProject(QObject *parent = 0);
  ~ZFlyEmBodyMergeProject();

  void test();
  void clear();

  bool hasDataFrame() const;
  void setDataFrame(ZStackFrame *frame);
  void closeDataFrame();

  void setDocData(ZStackDocReader &reader);

  void loadSlice(int x, int y, int z, int width, int height);
  void loadSliceFunc(int x, int y, int z, int width, int height);

  void changeDvidNode(const std::string &newUuid);

  const ZDvidReader& getDvidReader() const {
    return m_writer.getDvidReader();
  }

  inline const ZDvidTarget& getDvidTarget() const {
    return getDvidReader().getDvidTarget();
  }

  void setDvidTarget(const ZDvidTarget &target);
  void setAdmin(bool admin);

  inline ZFlyEmBodyMergeFrame* getDataFrame() {
    return m_dataFrame;
  }

  void setBodyStatusProtocol(const ZFlyEmBodyAnnotationMerger &protocol);

  //Obsolete functions
  uint64_t getSelectedBodyId() const;
  void addSelected(uint64_t label);
  void removeSelected(uint64_t label);

  bool lockNode(const QString &message);
  std::string createVersionBranch();

  int getCurrentZ() const;

//  void setDocument(ZSharedPointer<ZStackDoc> doc);
  ZStackDoc* getDocument() const;
  template<typename T>
  T* getDocument() const;

  void syncWithDvid();

#if 0
  void setSelection(
      const std::set<uint64_t> &selected, NeuTube::EBodyLabelType labelType);

  void addSelection(uint64_t bodyId, NeuTube::EBodyLabelType labelType);
  void updateSelection();
#endif

  std::set<uint64_t> getSelection(neutu::ELabelSource labelType) const;

  //void setSelectionFromOriginal(const std::set<uint64_t> &selected);

//  Z3DWindow* getBodyWindow() { return m_coarseBodyWindow; }
//  void closeBodyWindow();

  uint64_t getMappedBodyId(uint64_t label) const;

  void emitMessage(const QString msg, bool appending = true);
  void emitError(const QString msg, bool appending = true);

  ZProgressSignal* getProgressSignal() const;

  void notifySelected();

  void updateBookmarkDecoration(const ZFlyEmBookmarkArray &bookmarkArray);
  void clearBookmarkDecoration();
  void addBookmarkDecoration(const ZFlyEmBookmarkArray &bookmarkArray);

//  void attachBookmarkArray(ZFlyEmBookmarkArray *bookmarkArray);
//  ZFlyEmBookmarkArray* getBookmarkArray() const { return m_bookmarkArray; }

  void updateBookmarkDecoration();

  void setBookmarkVisible(bool visible);

  void clearBodyMerger();

  QList<QString> getBodyStatusList() const;
  QList<QString> getAdminStatusList() const;
  int getStatusRank(const std::string &status) const;
  bool isFinalStatus(const std::string &status) const;
  bool isExpertStatus(const std::string &status) const;
  bool isMergableStatus(const std::string &status) const;

  QString composeStatusConflictMessage(
      const QMap<uint64_t, ZFlyEmBodyAnnotation> &annotMap) const;
  QString composeFinalStatusMessage(
      const QMap<uint64_t, ZFlyEmBodyAnnotation> &annotMap) const;
  const ZFlyEmBodyAnnotationMerger& getAnnotationMerger() const {
    return m_bodyStatusProtocol;
  }

signals:
  void progressAdvanced(double dp);
  void progressStarted();
  void progressEnded();
  void newDocReady(ZStackDocReader *reader, bool readyForPaint);
//  void originalLabelUpdated(ZArray *label);
  void originalLabelUpdated(ZArray *label, QSet<uint64_t> *selectedSet);
  void selectionChanged(ZStackObjectSelector selector);
  void selectionChanged();
  void bodyMerged(QList<uint64_t> objLabelList);
  void mergeUploaded();
  void splitSent(ZDvidTarget target, uint64_t bodyId);
  void locating2DViewTriggered(int, int, int, int);
  void dvidLabelChanged();
  void messageGenerated(const ZWidgetMessage&);
  void coarseBodyWindowCreatedInThread();
  void checkingInBody(uint64_t bodyId, neutu::EBodySplitMode mode);

  /*
  void messageGenerated(QString, bool appending = true);
  void errorGenerated(QString, bool appending = true);
  */

public slots:
  void viewGrayscale(const ZIntPoint &offset, int width, int height);
  void loadGrayscaleFunc(int z, bool lowres);
  void shallowClear();
  void mergeBody();
  void setLoadingLabel(bool state);
  void uploadResult(bool mergingToLargest);
  void saveMergeOperation();
//  void update3DBodyView(const ZStackObjectSelector &selector);
//  void update3DBodyView(
//      bool showingWindow = true, bool resettingCamera = false);
//  void update3DBodyViewDeep();
//  void showCoarseBody3d();
//  void showBody3d();
//  void detachCoarseBodyWindow();
//  void detachBodyWindow();
  void notifySplit();

//  void highlightSelectedObject(bool hl);
//  void update3DBodyViewPlane(const ZStackViewParam &viewParam);
//  void highlightSelectedObject(bool hl);
//  void update3DBodyViewPlane(const ZStackViewParam &viewParam);

private slots:
//  void presentCoarseBodyView();

private:
  ZFlyEmBodyMerger* getBodyMerger() const;
//  void update3DBodyViewPlane(
//      const ZDvidInfo &dvidInfo, const ZStackViewParam &viewParam);
//  void update3DBodyViewBox(const ZDvidInfo &dvidInfo);
  void uploadResultFunc(bool mergingToLargest);
//  void makeCoarseBodyWindow(ZStackDoc *doc);
//  void makeBodyWindow();
  void connectSignalSlot();

  QString getSelectionMessage() const;

  void mergeBodyAnnotation(uint64_t targetId, const std::vector<uint64_t> &bodyIdArray);
  void refreshBodyAnnotationCache();
  void updateMergeMap();
  void updateAffliatedData(
      uint64_t targetId, const std::vector<uint64_t> &bodyArray,
      ZWidgetMessage &warnMsg);
  void updateSelection(const std::set<uint64_t> &newBodySet);
  void refreshSegmentationView();
  void unlockBody(const std::set<uint64_t> &bodySet);
  void unlockBody(uint64_t bodyId);
  void unlockBody(const std::vector<uint64_t> &bodyArray);
  bool mergeVerified(
      uint64_t targetId, const std::vector<uint64_t> &bodyArray) const;
  void removeMerge(uint64_t bodyId);
  void removeMerge(const std::vector<uint64_t> &bodyArray);

  void clearUndoStack();

  QList<QString> getBodyStatusList(
      std::function<bool(const ZFlyEmBodyStatus&)> pred) const;

  void logSynapseInfo(uint64_t bodyId);

private:
  ZFlyEmBodyMergeFrame *m_dataFrame;

  ZDvidWriter m_writer;
  ZFlyEmBodyAnnotationMerger m_bodyStatusProtocol;

  bool m_isBookmarkVisible;
  bool m_isAdmin = false;

  bool m_showingBodyMask;
  QSet<uint64_t> m_selectedOriginal; //the set of original ids of selected bodies
  QMap<uint64_t, ZFlyEmBodyAnnotation> m_annotationCache;
  QMap<uint64_t, std::vector<uint64_t>> m_mergeMap;

  ZProgressSignal *m_progressSignal;
};

template <typename T>
T* ZFlyEmBodyMergeProject::getDocument() const
{
  return qobject_cast<T*>(getDocument());
}

#endif // ZFLYEMBODYMERGEPROJECT_H
