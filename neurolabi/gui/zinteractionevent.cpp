#include "zinteractionevent.h"

ZInteractionEvent::ZInteractionEvent() : m_event(ZInteractionEvent::EVENT_NULL)
{
}

void ZInteractionEvent::setEvent(EEvent event)
{
  m_event = event;
}

QString ZInteractionEvent::getMessage() const
{
  QString message;
  switch (m_event) {
  case EVENT_SWC_NODE_SELECTED:
    message = "Node selected: Use keyboard "
        "(x: delete; q: smaller; e: bigger; a/s/w/d: move) to operate or "
        "right click for more options";
    break;
  case EVENT_SWC_NODE_ADDED:
    message = "Node added";
    break;
  case  EVENT_SWC_NODE_DELETED:
    message = "Node deleted";
    break;
  case EVENT_SWC_NODE_ENLARGED:
    message = "Node became bigger";
    break;
  case EVENT_SWC_NODE_MOVED:
    message = "Node moved";
    break;
  case EVENT_SWC_BRANCH_TRACED:
    message = "New branch traced";
    break;
  case EVENT_SWC_NODE_TOGGLED_ON:
    message = "Click mouse at the desired position to add node. "
        "Ctrl/Cmd+E for automatically adjust node size based on local signal";
    break;
  case EVENT_SWC_NODE_EXTENDED:
    message = "Node extended. You can click to extend more. "
        "Tip: Ctrl/Cmd+Click for extending with a single node";
    break;
  case EVENT_VIEW_PROJECTION:
    message = "Switch to projection view.";
    break;
  case EVENT_VIEW_SLICE:
    message = "Switch to slice view.";
    break;
  default:
    break;
  }

  return message;
}
