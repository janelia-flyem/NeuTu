#ifndef ZFLYEMBODY3DDOC_H
#define ZFLYEMBODY3DDOC_H

#include "zstackdoc.h"

#include <QSet>
#include <QTimer>
#include <QQueue>
#include <QMutex>

#include "dvid/zdvidtarget.h"
#include "dvid/zdvidinfo.h"
#include "qthreadfuturemap.h"

class ZFlyEmBody3dDoc : public ZStackDoc
{
  Q_OBJECT
public:
  explicit ZFlyEmBody3dDoc(QObject *parent = 0);

public:
  class BodyEvent {
  public:
    enum EAction {
      ACTION_NULL, ACTION_REMOVE, ACTION_ADD, ACTION_UPDATE, ACTION_CHANGE_COLOR
    };

  public:
    BodyEvent() : m_action(ACTION_NULL), m_bodyId(0) {}
    BodyEvent(BodyEvent::EAction action, uint64_t bodyId) :
      m_action(action), m_bodyId(bodyId) {}

    EAction getAction() const { return m_action; }
    uint64_t getBodyId() const { return m_bodyId; }

  private:
    EAction m_action;
    uint64_t m_bodyId;
  };

  void addBody(uint64_t bodyId);
  void removeBody(uint64_t bodyId);

  void addEvent(BodyEvent::EAction action, uint64_t bodyId);

  template <typename InputIterator>
  void addBodyChangeEvent(const InputIterator &first, const InputIterator &last);

  bool hasBody(uint64_t bodyId);

  inline const ZDvidTarget& getDvidTarget() const {
    return m_dvidTarget;
  }

  void setDvidTarget(const ZDvidTarget &target);

  const ZDvidInfo& getDvidInfo() const;

  void updateFrame();

private:
  ZSwcTree* retrieveBodyModel(uint64_t bodyId);

  ZSwcTree* makeBodyModel(uint64_t bodyId);
  void updateDvidInfo();

  void addBodyFunc(uint64_t bodyId);
  void removeBodyFunc(uint64_t bodyId);

  void connectSignalSlot();
  void updateBodyFunc();

signals:

public slots:

private slots:
//  void updateBody();
  void processEvent();

private:
  QSet<uint64_t> m_bodySet;
  ZDvidTarget m_dvidTarget;
  ZDvidInfo m_dvidInfo;

  QThreadFutureMap m_futureMap;
  QTimer *m_timer;

  QQueue<BodyEvent> m_eventQueue;
  QMutex m_eventQueueMutex;
};

template <typename InputIterator>
void ZFlyEmBody3dDoc::addBodyChangeEvent(
    const InputIterator &first, const InputIterator &last)
{
  QSet<uint64_t> newSet;
  for (InputIterator iter = first; iter != last; ++iter) {
    newSet.insert(*iter);
  }

  for (QSet<uint64_t>::const_iterator iter = newSet.begin();
       iter != newSet.end(); ++iter) {
    if (!m_bodySet.contains(*iter)) {
      addEvent(BodyEvent::ACTION_ADD, *iter);
    }
  }

  for (QSet<uint64_t>::const_iterator iter = m_bodySet.begin();
       iter != m_bodySet.end(); ++iter) {
    if (!newSet.contains(*iter)) {
      addEvent(BodyEvent::ACTION_REMOVE, *iter);
    }
  }
}

#endif // ZFLYEMBODY3DDOC_H
