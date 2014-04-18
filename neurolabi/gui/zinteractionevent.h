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
    EVENT_SWC_NODE_SELECTED,
    EVENT_SWC_NODE_ADDED, EVENT_SWC_NODE_DELETED, EVENT_SWC_NODE_ENLARGED,
    EVENT_SWC_NODE_MOVED, EVENT_SWC_BRANCH_TRACED, EVENT_SWC_NODE_TOGGLED_ON,
    EVENT_SWC_NODE_EXTENDED
  };

public:
  QString getMessage() const;
  void setEvent(EEvent event);

private:
  EEvent m_event;
};

#endif // ZINTERACTIONEVENT_H
