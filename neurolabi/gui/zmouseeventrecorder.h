#ifndef ZMOUSEEVENTRECORDER_H
#define ZMOUSEEVENTRECORDER_H

#include <QList>
#include <QMap>
#include "qnamespace.h"
#include "zmouseevent.h"

class QMouseEvent;

class ZMouseEventRecorder
{
public:
  ZMouseEventRecorder();

  ZMouseEvent& record(QMouseEvent *event, ZMouseEvent::EAction, int z = 0);
  ZMouseEvent &record(const ZMouseEvent &event);
  const ZMouseEvent &getLatestMouseEvent() const;
  const ZMouseEvent& getMouseEvent(
      Qt::MouseButtons buttons, ZMouseEvent::EAction action) const;

  typedef QMap<Qt::MouseButtons, QMap<ZMouseEvent::EAction, ZMouseEvent> >
    TMouseEventMap;

  /*
  bool hasPosition(const Qt::MouseButtons &button,
                   const ZMouseEvent::EAction &action) const;
  ZIntPoint getPosition(const Qt::MouseButtons &button,
                               const ZMouseEvent::EAction &action) const;
*/
private:
  QList<ZMouseEvent> m_eventList;
  TMouseEventMap m_eventMap;

  ZMouseEvent m_emptyEvent;

  static const int m_maxEventNumber;
};

#endif // ZMOUSEEVENTRECORDER_H
