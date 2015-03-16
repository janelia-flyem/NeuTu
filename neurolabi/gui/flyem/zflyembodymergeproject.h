#ifndef ZFLYEMBODYMERGEPROJECT_H
#define ZFLYEMBODYMERGEPROJECT_H

#include <QObject>
#include <QList>
#include "dvid/zdvidtarget.h"
#include "tz_stdint.h"
#include "zstackobjectselector.h"

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
class Z3DWindow;

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

  inline const ZDvidTarget& getDvidTarget() const {
    return m_dvidTarget;
  }

  inline void setDvidTarget(const ZDvidTarget &target) {
    m_dvidTarget = target;
  }

  inline ZFlyEmBodyMergeFrame* getDataFrame() {
    return m_dataFrame;
  }

  int getSelectedBodyId() const;

  void addSelected(uint64_t label);
  void removeSelected(uint64_t label);

  bool lockNode(const QString &message);
  std::string createVersionBranch();

signals:
  void progressAdvanced(double dp);
  void progressStarted();
  void progressEnded();
  void newDocReady(ZStackDocReader *reader, bool readyForPaint);
//  void originalLabelUpdated(ZArray *label);
  void originalLabelUpdated(ZArray *label, QSet<uint64_t> *selectedSet);
  void selectionChanged(ZStackObjectSelector selector);
  void bodyMerged(QList<uint64_t> objLabelList);
  void splitSent(ZDvidTarget target, int bodyId);

public slots:
  void viewGrayscale(const ZIntPoint &offset, int width, int height);
  void loadGrayscaleFunc(int z, bool lowres);
  void shallowClear();
  void mergeBody();
  void setLoadingLabel(bool state);
  void uploadResult();
  void update3DBodyView(const ZStackObjectSelector &selector);
  void showBody3d();
  void detachBodyWindow();
  void notifySplit();

private:
  ZFlyEmBodyMergeFrame *m_dataFrame;
  Z3DWindow *m_bodyWindow;
  ZDvidTarget m_dvidTarget;
//  Z3DWindow *m_resultWindow;
//  Z3DWindow *m_quickViewWindow;
//  ZFlyEmBookmarkArray m_bookmarkArray;
//  std::vector<ZStackObject*> m_bookmarkDecoration;
//  bool m_isBookmarkVisible;
  bool m_showingBodyMask;
  QSet<uint64_t> m_currentSelected;
};

#endif // ZFLYEMBODYMERGEPROJECT_H
