#include "zmouseeventrecorder.h"
#include <QMouseEvent>

const int ZMouseEventRecorder::m_maxEventNumber = 5;

ZMouseEventRecorder::ZMouseEventRecorder()
{
}

ZMouseEvent& ZMouseEventRecorder::record(const ZMouseEvent &event)
{
  m_eventList.prepend(event);
  if (m_eventList.size() > m_maxEventNumber) {
    m_eventList.removeLast();
  }

  m_eventMap[event.getButtons()][event.getAction()] = event;

  return m_eventList.first();
}

ZMouseEvent& ZMouseEventRecorder::record(
    QMouseEvent *event, ZMouseEvent::EAction action, int z)
{
  ZMouseEvent eventSnapshot;
  eventSnapshot.set(event, action, z);

  return record(eventSnapshot);
}

const ZMouseEvent& ZMouseEventRecorder::getLatestMouseEvent() const
{
  if (m_eventList.isEmpty()) {
    return m_emptyEvent;
  }

  return m_eventList.first();
}

const ZMouseEvent& ZMouseEventRecorder::getMouseEvent(
    Qt::MouseButtons buttons, ZMouseEvent::EAction action) const
{
  if (m_eventMap.contains(buttons)) {
    if (const_cast<TMouseEventMap&>(m_eventMap)[buttons].contains(action)) {
      return const_cast<TMouseEventMap&>(m_eventMap)[buttons][action];
    }
  }

  return m_emptyEvent;
}
