#ifndef ZFLYEMBODYEVENT_H
#define ZFLYEMBODYEVENT_H

#include "zflyembodyconfig.h"

class ZFlyEmBodyEvent
{
public:
  enum class EAction {
    NONE, REMOVE, ADD, FORCE_ADD, UPDATE, CACHE
  };
  typedef uint64_t TUpdateFlag;

  ZFlyEmBodyEvent();

  ZFlyEmBodyEvent(EAction action, uint64_t bodyId) :
    m_action(action), m_config(bodyId) {}
  ZFlyEmBodyEvent(EAction action, const ZFlyEmBodyConfig &config);

public:
  struct ComparePriority {
    bool operator() (const ZFlyEmBodyEvent &e1, const ZFlyEmBodyEvent &e2) {
      return (e1.getPriority() < e2.getPriority());
    }
  };

  EAction getAction() const { return m_action; }
  uint64_t getBodyId() const { return m_config.getBodyId(); }
  QColor getBodyColor() const { return m_config.getBodyColor(); }
//    bool isRefreshing() const { return m_refreshing; }

  void setBodyId(uint64_t id);
  void setAction(EAction action) { m_action = action; }
  void setBodyColor(const QColor &color);
  void setRange(const ZIntCuboid &range);

  void mergeEvent(const ZFlyEmBodyEvent &event, neutu::EBiDirection direction);

//    void syncBodySelection();

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
    return (m_updateFlag & flag) != UPDATE_NULL;
  }

  bool hasValidBody() const;
  bool isNull() const;
  bool isValid() const;

  int getDsLevel() const;
  void setDsLevel(int level);
  void decDsLevel();

  void setLocalDsLevel(int level);

  bool isOneTime() const;
  void setOneTime(bool on);
//  void setMaxDsLevel(int level);
//  void setMinDsLevel(int level);

  void print() const;

  int getPriority() const { return m_priority; }

  ZFlyEmBodyConfig getBodyConfig() const;
  void setBodyConfig(const ZFlyEmBodyConfig &config);

  ZFlyEmBodyEvent makeHighResEvent(int minDsLevel) const;
  ZFlyEmBodyEvent makeHighResEvent(
      const ZFlyEmBodyConfig &config, int minDsLevel) const;
  static ZFlyEmBodyEvent MakeHighResEvent(
      const ZFlyEmBodyConfig &config, int minDsLevel);

public:
  static const TUpdateFlag UPDATE_NULL;
  static const TUpdateFlag UPDATE_CHANGE_COLOR;
  static const TUpdateFlag UPDATE_ADD_SYNAPSE;
  static const TUpdateFlag UPDATE_ADD_TODO_ITEM;
  static const TUpdateFlag UPDATE_MULTIRES;
  static const TUpdateFlag UPDATE_SEGMENTATION;
  static const TUpdateFlag SYNC_BODY_COLOR;

private:
  EAction m_action = EAction::NONE;

  ZFlyEmBodyConfig m_config;
//    bool m_refreshing;
  TUpdateFlag m_updateFlag = UPDATE_NULL;
  bool m_oneTimeEvent = false;
  int m_priority = 0; //lower number means higher priority
};

#endif // ZFLYEMBODYEVENT_H
