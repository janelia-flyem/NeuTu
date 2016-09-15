#ifndef ZFLYEMBODY3DDOC_H
#define ZFLYEMBODY3DDOC_H

#include "zstackdoc.h"

#include <QSet>
#include <QTimer>
#include <QQueue>
#include <QMutex>
#include <QColor>
#include <QList>
#include <QTime>

#include "neutube_def.h"
#include "dvid/zdvidtarget.h"
#include "dvid/zdvidinfo.h"
#include "zthreadfuturemap.h"
#include "zsharedpointer.h"

class ZFlyEmProofDoc;
class ZFlyEmBodyMerger;
class ZFlyEmToDoItem;

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
      ACTION_NULL, ACTION_REMOVE, ACTION_ADD, ACTION_FORCE_ADD,
      ACTION_UPDATE
    };

    typedef uint64_t TUpdateFlag;

  public:
    BodyEvent() : m_action(ACTION_NULL), m_bodyId(0), /*m_refreshing(false),*/
    m_updateFlag(0), m_resLevel(0) {}
    BodyEvent(BodyEvent::EAction action, uint64_t bodyId) :
      m_action(action), m_bodyId(bodyId), m_updateFlag(0), m_resLevel(0) {}

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

    bool hasUpdateFlag(TUpdateFlag flag) {
      return (m_updateFlag & flag) > 0;
    }

    int getResLevel() const {
      return m_resLevel;
    }

    void setResLevel(int level) {
      m_resLevel = level;
    }

    void decResLevel() {
      --m_resLevel;
    }

    void print() const;

  public:
    static const TUpdateFlag UPDATE_CHANGE_COLOR;
    static const TUpdateFlag UPDATE_ADD_SYNAPSE;
    static const TUpdateFlag UPDATE_ADD_TODO_ITEM;
    static const TUpdateFlag UPDATE_MULTIRES;

  private:
    EAction m_action;
    uint64_t m_bodyId;
    QColor m_bodyColor;
//    bool m_refreshing;
    TUpdateFlag m_updateFlag;
    int m_resLevel;
  };

  enum EBodyType {
    BODY_FULL, BODY_COARSE, BODY_SKELETON
  };

  class ObjectStatus {
  public:
    ObjectStatus(int timeStamp = 0);
    void setRecycable(bool on);
    void setTimeStamp(int t);
    void setResLevel(int level);

    bool isRecycable() const;
    int getTimeStamp() const;
    int getResLevel() const;

  private:
    void init(int timeStatus);

  private:
    bool m_recycable;
    int m_timeStamp;
    int m_resLevel;
  };

  void setBodyType(EBodyType type);
  EBodyType getBodyType() { return m_bodyType; }

  void addBody(uint64_t bodyId, const QColor &color);
  void removeBody(uint64_t bodyId);
  void updateBody(uint64_t bodyId, const QColor &color);
  void updateBody(uint64_t bodyId, const QColor &color, EBodyType type);

  void addSynapse(uint64_t bodyId);
  void addTodo(uint64_t bodyId);

  void addEvent(BodyEvent::EAction action, uint64_t bodyId,
                BodyEvent::TUpdateFlag flag = 0, QMutex *mutex = NULL);
  void addEvent(const BodyEvent &event);

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

  void dumpAllBody(bool recycable);

  void dumpGarbage(ZStackObject *obj, bool recycable);
  template<typename InputIterator>
  void dumpGarbage(const InputIterator &first, const InputIterator &last,
                   bool recycable);
  void mergeBodyModel(const ZFlyEmBodyMerger &merger);

  void processEventFunc();
  void cancelEventThread();

  void setTodoItemSelected(ZFlyEmToDoItem *item, bool select);

  bool hasTodoItemSelected() const;

  ZFlyEmToDoItem* getOneSelectedTodoItem() const;

  void forceBodyUpdate();

public slots:
  void showSynapse(bool on);// { m_showingSynapse = on; }
  void addSynapse(bool on);
  void showTodo(bool on);
  void addTodo(bool on);
  void updateTodo(uint64_t bodyId);
  void setUnrecycable(const QSet<uint64_t> &bodySet);

protected:
  void autoSave() {}

private:
  ZSwcTree* retrieveBodyModel(uint64_t bodyId, EBodyType bodyType);
  ZSwcTree* getBodyModel(uint64_t bodyId, EBodyType bodyType);

  ZSwcTree* makeBodyModel(uint64_t bodyId);
  ZSwcTree* makeBodyModel(uint64_t bodyId, EBodyType bodyType);
  void updateDvidInfo();

  void addBodyFunc(uint64_t bodyId, const QColor &color, int resLevel);

  void removeBodyFunc(uint64_t bodyId, bool removingAnnotation);

  void connectSignalSlot();
  void updateBodyFunc();

  void processBodySetBuffer();

  QMap<uint64_t, BodyEvent> makeEventMap(bool synced, QSet<uint64_t> &bodySet);

  static std::string GetBodyTypeName(EBodyType bodyType);

  template<typename T>
  T* recoverFromGarbage(const std::string &source);

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
  bool m_showingTodo;
//  QSet<uint64_t> m_bodySetBuffer;
//  bool m_isBodySetBufferProcessed;

  ZDvidTarget m_dvidTarget;
  ZDvidInfo m_dvidInfo;

  ZThreadFutureMap m_futureMap;
  QTimer *m_timer;
  QTimer *m_garbageTimer;
  QTime m_objectTime;

  ZSharedPointer<ZStackDoc> m_dataDoc;

//  QList<ZStackObject*> m_garbageList;
  QMap<ZStackObject*, ObjectStatus> m_garbageMap;
  QMap<uint64_t, int> m_bodyUpdateMap;
//  QSet<uint64_t> m_unrecycableSet;

  bool m_garbageJustDumped;

  QQueue<BodyEvent> m_eventQueue;

  QMutex m_eventQueueMutex;
  QMutex m_garbageMutex;

  const static int OBJECT_GARBAGE_LIFE;
  const static int OBJECT_ACTIVE_LIFE;
};

template <typename InputIterator>
void ZFlyEmBody3dDoc::addBodyChangeEvent(
    const InputIterator &first, const InputIterator &last)
{
  std::cout << "Locking mutex ..." << std::endl;
  QMutexLocker locker(&m_eventQueueMutex);

  QSet<uint64_t> bodySet = m_bodySet;
  QMap<uint64_t, ZFlyEmBody3dDoc::BodyEvent> actionMap = makeEventMap(
        false, bodySet);

//  m_eventQueue.clear();

  QSet<uint64_t> newBodySet;
  for (InputIterator iter = first; iter != last; ++iter) {
    uint64_t bodyId = *iter;
    newBodySet.insert(bodyId);
  }

  for (QSet<uint64_t>::const_iterator iter = m_bodySet.begin();
       iter != m_bodySet.end(); ++iter) {
    uint64_t bodyId = *iter;
    if (!newBodySet.contains(bodyId)) {
      addEvent(BodyEvent::ACTION_REMOVE, bodyId, 0, NULL);
    }
  }

//  QList<BodyEvent> oldEventList;
  for (QMap<uint64_t, ZFlyEmBody3dDoc::BodyEvent>::iterator
       iter = actionMap.begin(); iter != actionMap.end(); ++iter) {
    if (newBodySet.contains(iter.key())) {
      if (iter.value().getAction() != BodyEvent::ACTION_REMOVE) {
        //In the new body set had the bodyID and not remove, add event
        addEvent(iter.value());
      } else {
        addEvent(BodyEvent::ACTION_ADD, iter.key(), 0, NULL);
      }
    }
  }

  for (InputIterator iter = first; iter != last; ++iter) {
    uint64_t bodyId = *iter;
    if (!actionMap.contains(bodyId)) { //If the action map has no such body id
      addEvent(BodyEvent::ACTION_ADD, bodyId, 0, NULL);
    }
  }
}

#endif // ZFLYEMBODY3DDOC_H
