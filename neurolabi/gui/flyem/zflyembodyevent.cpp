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
    const ZFlyEmBodyEvent &event, neutube::EBiDirection direction)
{
  if (getBodyId() != event.getBodyId())  {
    return;
  }

  switch (direction) {
  case neutube::EBiDirection::FORWARD: //event comes first
    switch (getAction()) {
    case ACTION_UPDATE:
      switch (event.getAction()) {
      case ACTION_ADD:
        m_action = ACTION_ADD;
        setBodyColor(event.getBodyColor());
//        m_bodyColor = event.m_bodyColor;
        m_updateFlag |= event.m_updateFlag;
        if (getDsLevel() < event.getDsLevel()) {
          setDsLevel(event.getDsLevel());
        }
//        m_refreshing |= event.m_refreshing;
        break;
      case ACTION_UPDATE:
        addUpdateFlag(event.getUpdateFlag());
//        m_refreshing |= event.m_refreshing;
        break;
      case ACTION_REMOVE:
        m_action = ACTION_REMOVE;
        break;
      default:
        break;
      }
      break;
    case ACTION_ADD:
      if (event.getAction() == ACTION_ADD || event.getAction() == ACTION_UPDATE) {
//        m_refreshing |= event.m_refreshing;
      }
      break;
    case ACTION_NULL:
      *this = event;
      break;
    default:
      break;
    }
    break;
  case neutube::EBiDirection::BACKWARD:
  {
    ZFlyEmBodyEvent tmpEvent = event;
    tmpEvent.mergeEvent(*this, neutube::EBiDirection::FORWARD);
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
  return m_action == ACTION_NULL;
}

bool ZFlyEmBodyEvent::isValid() const
{
  return hasValidBody() && !isNull();
}

ZFlyEmBodyEvent ZFlyEmBodyEvent::makeHighResEvent(
    const ZFlyEmBodyConfig &config, int minDsLevel) const
{
  if (!isOneTime()) {
    if (getAction() == ACTION_ADD || getAction() == ACTION_UPDATE) {
      return MakeHighResEvent(config, minDsLevel);
    }
  }

  return ZFlyEmBodyEvent(ACTION_NULL, 0);
}

ZFlyEmBodyEvent ZFlyEmBodyEvent::makeHighResEvent(int minDsLevel) const
{
  return makeHighResEvent(m_config, minDsLevel);
}

ZFlyEmBodyEvent ZFlyEmBodyEvent::MakeHighResEvent(
    const ZFlyEmBodyConfig &config, int minDsLevel)
{
  ZFlyEmBodyEvent event(ACTION_UPDATE, config);

  if (event.getDsLevel() > minDsLevel) {
    event.decDsLevel();
    event.addUpdateFlag(UPDATE_MULTIRES);
  } else {
    event.setBodyId(0);
    event.setAction(ACTION_NULL); //Invalidate event
  }

  return event;
}

void ZFlyEmBodyEvent::print() const
{
  switch (m_action) {
  case ACTION_UPDATE:
    std::cout << "Update: ";
    break;
  case ACTION_ADD:
    std::cout << "Add: ";
    break;
  case ACTION_REMOVE:
    std::cout << "Remove: ";
    break;
  case ACTION_FORCE_ADD:
    std::cout << "Force add: ";
    break;
  case ACTION_NULL:
    std::cout << "No action: ";
    break;
  case ACTION_CACHE:
    std::cout << "Cache: ";
    break;
  }

  std::cout << getBodyId() << std::endl;
  std::cout << "  resolution: " << getDsLevel() << std::endl;
  std::cout << "  flag: " << getUpdateFlag() << std::endl;
}
