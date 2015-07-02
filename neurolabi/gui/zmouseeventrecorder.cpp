#include "zmouseeventrecorder.h"
#include <QMouseEvent>
#include <iostream>

const int ZMouseEventRecorder::m_maxEventNumber = 5;

ZMouseEventRecorder::ZMouseEventRecorder()
{
  m_eventList.clear();
}

ZMouseEvent& ZMouseEventRecorder::record(const ZMouseEvent &event)
{
  m_eventList.prepend(event);

#ifdef _DEBUG_2
  std::cout << "Mouse event recorded:" << std::endl;
  event.print();
#endif

  if (m_eventList.size() > m_maxEventNumber) {
    m_eventList.removeLast();
  }

  bool isRecorded = false;

  if (event.getButtons() & Qt::LeftButton) {
    m_eventMap[Qt::LeftButton][event.getAction()] = event;
    isRecorded = true;
  }

  if (event.getButtons() & Qt::RightButton) {
    m_eventMap[Qt::RightButton][event.getAction()] = event;
    isRecorded = true;
  }

  if (!isRecorded) {
    m_eventMap[event.getButtons()][event.getAction()] = event;
  }


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

const ZMouseEvent& ZMouseEventRecorder::getMouseEvent(int index) const
{
  if (index < 0 || index >= m_eventList.size()) {
    return m_emptyEvent;
  }

  return m_eventList.at(index);
}

int ZMouseEventRecorder::getEventCount() const
{
  return m_eventList.size();
}

ZPoint ZMouseEventRecorder::getPositionOffset(
    NeuTube::ECoordinateSystem cs) const
{
  ZPoint offset(0, 0, 0);
  if (getEventCount() > 1) {
    offset =
        getMouseEvent(0).getPosition(cs) - getMouseEvent(1).getPosition(cs);
  }

  return offset;
}

ZPoint ZMouseEventRecorder::getPositionOffsetFromLastLeftPress(
    NeuTube::ECoordinateSystem cs) const
{
  ZPoint offset(0, 0, 0);
  const ZMouseEvent &event = getMouseEvent(
        Qt::LeftButton, ZMouseEvent::ACTION_PRESS);
  if (!event.isNull()) {
    const ZMouseEvent &currentEvent = getLatestMouseEvent();
    if (!currentEvent.isNull()) {
      offset = currentEvent.getPosition(cs) - event.getPosition(cs);
    }
  }

  return offset;
}

ZPoint ZMouseEventRecorder::getPosition(
    Qt::MouseButtons buttons, ZMouseEvent::EAction action,
    NeuTube::ECoordinateSystem cs) const
{
  ZPoint pt(0, 0, 0);

  const ZMouseEvent &event = getMouseEvent(buttons, action);
  if (!event.isNull()) {
    pt = event.getPosition(cs);
  }

  return pt;
}
