#include "zflyembodyevent.h"

const ZFlyEmBodyEvent::TUpdateFlag
ZFlyEmBodyEvent::UPDATE_NULL = 0;

const ZFlyEmBodyEvent::TUpdateFlag
ZFlyEmBodyEvent::UPDATE_CHANGE_COLOR = 1;

const ZFlyEmBodyEvent::TUpdateFlag
ZFlyEmBodyEvent::UPDATE_ADD_SYNAPSE = 2;

const ZFlyEmBodyEvent::TUpdateFlag
ZFlyEmBodyEvent::UPDATE_ADD_TODO_ITEM = 4;

const ZFlyEmBodyEvent::TUpdateFlag
ZFlyEmBodyEvent::UPDATE_MULTIRES = 8;

const ZFlyEmBodyEvent::TUpdateFlag
ZFlyEmBodyEvent::UPDATE_SEGMENTATION = 16;

const ZFlyEmBodyEvent::TUpdateFlag
ZFlyEmBodyEvent::SYNC_BODY_COLOR = 32;

ZFlyEmBodyEvent::ZFlyEmBodyEvent()
{

}

ZFlyEmBodyEvent::ZFlyEmBodyEvent(EAction action, const ZFlyEmBodyConfig &config)
  : m_action(action), m_config(config)
{

}

int ZFlyEmBodyEvent::getDsLevel() const
{
  return m_config.getDsLevel();
}

void ZFlyEmBodyEvent::setDsLevel(int level)
{
  m_config.setDsLevel(level);
}


void ZFlyEmBodyEvent::decDsLevel()
{
  m_config.decDsLevel();
}

void ZFlyEmBodyEvent::setLocalDsLevel(int level)
{
  m_config.setLocalDsLevel(level);
}

bool ZFlyEmBodyEvent::isOneTime() const
{
  return m_oneTimeEvent;
}

void ZFlyEmBodyEvent::setOneTime(bool on)
{
  m_oneTimeEvent = on;
}

/*
void ZFlyEmBodyEvent::setMaxDsLevel(int level)
{
  m_config.setMaxDsLevel(level);
}

void ZFlyEmBodyEvent::setMinDsLevel(int level)
{
  m_config.setMinDsLevel(level);
}
*/

void ZFlyEmBodyEvent::mergeEvent(
    const ZFlyEmBodyEvent &event, neutu::EBiDirection direction)
{
  if (getBodyId() != event.getBodyId())  {
    return;
  }

  switch (direction) {
  case neutu::EBiDirection::FORWARD: //event comes first
    switch (getAction()) {
    case EAction::UPDATE:
      switch (event.getAction()) {
      case EAction::ADD:
        m_action = EAction::ADD;
        setBodyColor(event.getBodyColor());
//        m_bodyColor = event.m_bodyColor;
        m_updateFlag |= event.m_updateFlag;
        if (getDsLevel() < event.getDsLevel()) {
          setDsLevel(event.getDsLevel());
        }
//        m_refreshing |= event.m_refreshing;
        break;
      case EAction::UPDATE:
        addUpdateFlag(event.getUpdateFlag());
//        m_refreshing |= event.m_refreshing;
        break;
      case EAction::REMOVE:
        m_action = EAction::REMOVE;
        break;
      default:
        break;
      }
      break;
    case EAction::ADD:
      if (event.getAction() == EAction::ADD ||
          event.getAction() == EAction::UPDATE) {
//        m_refreshing |= event.m_refreshing;
      }
      break;
    case EAction::NONE:
      *this = event;
      break;
    default:
      break;
    }
    break;
  case neutu::EBiDirection::BACKWARD:
  {
    ZFlyEmBodyEvent tmpEvent = event;
    tmpEvent.mergeEvent(*this, neutu::EBiDirection::FORWARD);
    *this = tmpEvent;
  }
    break;
  }
}

void ZFlyEmBodyEvent::setRange(const ZIntCuboid &range)
{
  m_config.setRange(range);
}

void ZFlyEmBodyEvent::setBodyColor(const QColor &color)
{
  m_config.setBodyColor(color);
}

void ZFlyEmBodyEvent::setBodyId(uint64_t id)
{
  m_config.setBodyId(id);
}

ZFlyEmBodyConfig ZFlyEmBodyEvent::getBodyConfig() const
{
  return m_config;
}

void ZFlyEmBodyEvent::setBodyConfig(const ZFlyEmBodyConfig &config)
{
  m_config = config;
}

bool ZFlyEmBodyEvent::hasValidBody() const
{
  return getBodyId() > 0;
}

bool ZFlyEmBodyEvent::isNull() const
{
  return m_action == EAction::NONE;
}

bool ZFlyEmBodyEvent::isValid() const
{
  return hasValidBody() && !isNull();
}

ZFlyEmBodyEvent ZFlyEmBodyEvent::makeHighResEvent(
    const ZFlyEmBodyConfig &config, int minDsLevel) const
{
  if (!isOneTime()) {
    if (getAction() == EAction::ADD ||
        getAction() == EAction::UPDATE) {
      return MakeHighResEvent(config, minDsLevel);
    }
  }

  return ZFlyEmBodyEvent(EAction::NONE, 0);
}

ZFlyEmBodyEvent ZFlyEmBodyEvent::makeHighResEvent(int minDsLevel) const
{
  return makeHighResEvent(m_config, minDsLevel);
}

ZFlyEmBodyEvent ZFlyEmBodyEvent::MakeHighResEvent(
    const ZFlyEmBodyConfig &config, int minDsLevel)
{
  ZFlyEmBodyEvent event(EAction::UPDATE, config);

  if (config.hasNextDsLevel(minDsLevel)) {
    event.decDsLevel();
    event.addUpdateFlag(UPDATE_MULTIRES);
  } else {
    event.setBodyId(0);
    event.setAction(EAction::NONE); //Invalidate event
  }

  return event;
}

void ZFlyEmBodyEvent::print() const
{
  switch (m_action) {
  case EAction::UPDATE:
    std::cout << "Update: ";
    break;
  case EAction::ADD:
    std::cout << "Add: ";
    break;
  case EAction::REMOVE:
    std::cout << "Remove: ";
    break;
  case EAction::FORCE_ADD:
    std::cout << "Force add: ";
    break;
  case EAction::NONE:
    std::cout << "No action: ";
    break;
  case EAction::CACHE:
    std::cout << "Cache: ";
    break;
  }

  std::cout << getBodyId() << std::endl;
  std::cout << "  resolution: " << getDsLevel() << std::endl;
  std::cout << "  flag: " << getUpdateFlag() << std::endl;
}
