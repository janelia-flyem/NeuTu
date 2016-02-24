#ifndef ZFLYEMBODY3DDOC_H
#define ZFLYEMBODY3DDOC_H

#include "zstackdoc.h"

#include <QSet>
#include <QTimer>
#include <QQueue>
#include <QMutex>
#include <QColor>
#include <QList>

#include "neutube_def.h"
#include "dvid/zdvidtarget.h"
#include "dvid/zdvidinfo.h"
#include "zthreadfuturemap.h"
#include "zsharedpointer.h"

class ZFlyEmProofDoc;
class ZFlyEmBodyMerger;

class ZFlyEmBody3dDoc : public ZStackDoc
{
  Q_OBJECT
public:
  explicit ZFlyEmBody3dDoc(QObject *parent = 0);
  ~ZFlyEmBody3dDoc();


  void setDataDoc(ZSharedPointer<ZStackDoc> doc);

public:
  class BodyEvent {
  public:
    enum EAction {
      ACTION_NULL, ACTION_REMOVE, ACTION_ADD, ACTION_UPDATE
    };

    typedef uint64_t TUpdateFlag;

  public:
    BodyEvent() : m_action(ACTION_NULL), m_bodyId(0), /*m_refreshing(false),*/
    m_updateFlag(0) {}
    BodyEvent(BodyEvent::EAction action, uint64_t bodyId) :
      m_action(action), m_bodyId(bodyId) {}

    EAction getAction() const { return m_action; }
    uint64_t getBodyId() const { return m_bodyId; }
    const QColor& getBodyColor() const { return m_bodyColor; }
//    bool isRefreshing() const { return m_refreshing; }

    void setAction(EAction action) { m_action = action; }
    void setBodyColor(const QColor &color) { m_bodyColor = color; }

    void mergeEvent(const BodyEvent &event, NeuTube::EBiDirection direction);

    void syncBodySelection();

    bool updating(TUpdateFlag flag) const {
      return (m_updateFlag & flag) > 0;
    }

    void addUpdateFlag(TUpdateFlag flag) {
      m_updateFlag |= flag;
    }

    void removeUpdateFlag(TUpdateFlag flag) {
      m_updateFlag &= ~flag;
    }

    TUpdateFlag getUpdateFlag() const {
      return m_updateFlag;
    }

    void print() const;

  public:
    static const TUpdateFlag UPDATE_CHANGE_COLOR;
    static const TUpdateFlag UPDATE_ADD_SYNAPSE;

  private:
    EAction m_action;
    uint64_t m_bodyId;
    QColor m_bodyColor;
//    bool m_refreshing;
    TUpdateFlag m_updateFlag;


  };

  enum EBodyType {
    BODY_FULL, BODY_COARSE, BODY_SKELETON
  };

  void setBodyType(EBodyType type);
  EBodyType getBodyType() { return m_bodyType; }

  void addBody(uint64_t bodyId, const QColor &color);
  void removeBody(uint64_t bodyId);
  void updateBody(uint64_t bodyId, const QColor &color);

  void addSynapse(uint64_t bodyId);

  void addEvent(BodyEvent::EAction action, uint64_t bodyId,
                BodyEvent::TUpdateFlag flag = 0, QMutex *mutex = NULL);

  template <typename InputIterator>
  void addBodyChangeEvent(const InputIterator &first, const InputIterator &last);

  bool hasBody(uint64_t bodyId);

  inline const ZDvidTarget& getDvidTarget() const {
    return m_dvidTarget;
  }

  void setDvidTarget(const ZDvidTarget &target);

  const ZDvidInfo& getDvidInfo() const;

//  void updateFrame();

  ZFlyEmProofDoc* getDataDocument() const;

  void printEventQueue() const;

  void dumpAllSwc();

  void dumpGarbage(ZStackObject *obj);
  void mergeBodyModel(const ZFlyEmBodyMerger &merger);

  void processEventFunc();

public slots:
  void showSynapse(bool on);// { m_showingSynapse = on; }
  void addSynapse(bool on);

protected:
  void autoSave() {}

private:
  ZSwcTree* retrieveBodyModel(uint64_t bodyId);
  ZSwcTree* getBodyModel(uint64_t bodyId);

  ZSwcTree* makeBodyModel(uint64_t bodyId);
  void updateDvidInfo();

  void addBodyFunc(uint64_t bodyId, const QColor &color);

  void removeBodyFunc(uint64_t bodyId);

  void connectSignalSlot();
  void updateBodyFunc();

  void processBodySetBuffer();



signals:

public slots:

private slots:
//  void updateBody();
  void processEvent();
  void processEvent(const BodyEvent &event);
  void clearGarbage();

private:
  void processEventFunc(const BodyEvent &event);

private:
  QSet<uint64_t> m_bodySet;
  EBodyType m_bodyType;

  bool m_quitting;
  bool m_showingSynapse;
//  QSet<uint64_t> m_bodySetBuffer;
//  bool m_isBodySetBufferProcessed;

  ZDvidTarget m_dvidTarget;
  ZDvidInfo m_dvidInfo;

  ZThreadFutureMap m_futureMap;
  QTimer *m_timer;
  QTimer *m_garbageTimer;

  ZSharedPointer<ZStackDoc> m_dataDoc;

  QList<ZStackObject*> m_garbageList;
  bool m_garbageJustDumped;

  QQueue<BodyEvent> m_eventQueue;

  QMutex m_eventQueueMutex;
  QMutex m_garbageMutex;
};

template <typename InputIterator>
void ZFlyEmBody3dDoc::addBodyChangeEvent(
    const InputIterator &first, const InputIterator &last)
{
  std::cout << "Locking mutex ..." << std::endl;
  QMutexLocker locker(&m_eventQueueMutex);

  m_eventQueue.clear();

  for (QSet<uint64_t>::const_iterator iter = m_bodySet.begin();
       iter != m_bodySet.end(); ++iter) {
    uint64_t bodyId = *iter;
    addEvent(BodyEvent::ACTION_REMOVE, bodyId, 0, NULL);
  }

  for (InputIterator iter = first; iter != last; ++iter) {
    uint64_t bodyId = *iter;
    addEvent(BodyEvent::ACTION_ADD, bodyId, 0, NULL);
  }
}

#endif // ZFLYEMBODY3DDOC_H
