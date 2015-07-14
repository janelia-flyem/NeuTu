#ifndef ZFLYEMBODY3DDOC_H
#define ZFLYEMBODY3DDOC_H

#include "zstackdoc.h"
#include "dvid/zdvidtarget.h"
#include "dvid/zdvidinfo.h"
#include "qthreadfuturemap.h"

#include <QSet>
#include <QTimer>
#include <QQueue>

class ZFlyEmBody3dDoc : public ZStackDoc
{
  Q_OBJECT
public:
  explicit ZFlyEmBody3dDoc(QObject *parent = 0);

public:
  class BodyEvent {
  public:
    BodyEvent();

    enum EAction {
      ACTION_REMOVE, ACTION_ADD, ACTION_UPDATE, ACTION_CHANGE_COLOR
    };

    EAction getAction() const { return m_action; }
    uint64_t getBodyId() const { return m_bodyId; }

  private:
    EAction m_action;
    uint64_t m_bodyId;
  };

  void addBody(uint64_t bodyId);
  void removeBody(uint64_t bodyId);

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
};

#endif // ZFLYEMBODY3DDOC_H
