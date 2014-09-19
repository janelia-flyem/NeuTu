#ifndef ZINTERACTIONEVENT_H
#define ZINTERACTIONEVENT_H

#include <QString>
#include <QSet>

/*!
 * \brief The class of representing event occured during user interaction
 */
class ZInteractionEvent
{
public:
  ZInteractionEvent();

  enum EEvent {
    EVENT_NULL,
    EVENT_SWC_NODE_SELECTED,
    EVENT_SWC_NODE_ADDED, EVENT_SWC_NODE_DELETED, EVENT_SWC_NODE_ENLARGED,
    EVENT_SWC_NODE_MOVED, EVENT_SWC_BRANCH_TRACED, EVENT_SWC_NODE_TOGGLED_ON,
    EVENT_SWC_NODE_EXTENDED,
    EVENT_VIEW_PROJECTION, EVENT_VIEW_SLICE,
    EVENT_ACTIVE_DECORATION_UPDATED
  };

public:
  QString getMessage() const;
  void setEvent(EEvent event);
  inline EEvent getEvent() const {
    return m_event;
  }

private:
  EEvent m_event;
};

#endif // ZINTERACTIONEVENT_H
