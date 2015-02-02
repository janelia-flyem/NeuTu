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

  /*!
   * \brief Get mouse event at a certain index
   * \param index The event is sorted from newest to oldest and 0 means the
   *        latest event, 1 means the second latest, and so on.
   * \return The correspoding event. It returns an empty event if \a index is
   *         out of range.
   */
  const ZMouseEvent& getMouseEvent(int index) const;

  /*!
   * \brief The number of events recorded.
   */
  int getEventCount() const;

  typedef QMap<Qt::MouseButtons, QMap<ZMouseEvent::EAction, ZMouseEvent> >
    TMouseEventMap;

  ZPoint getPositionOffset(NeuTube::ECoordinateSystem cs) const;

  /*!
   * \brief Get the offset from the last left-button-pressed position to the
   *        latest mouse position.
   *
   * It returns (0, 0, 0) if there is no left-button-pressed position recorded.
   */
  ZPoint
  getPositionOffsetFromLastLeftPress(NeuTube::ECoordinateSystem cs) const;

  ZPoint getPosition(Qt::MouseButtons buttons, ZMouseEvent::EAction action,
                     NeuTube::ECoordinateSystem cs) const;

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
