#include "zflyembodymergeframe.h"
#include <QMdiArea>

#include "flyem/zflyembodymergedoc.h"
#include "zmessage.h"
#include "zmessagemanager.h"
#include "zstackview.h"

ZFlyEmBodyMergeFrame::ZFlyEmBodyMergeFrame(QWidget *parent) : ZStackFrame(parent)
{
  //constructFrame();
}

void ZFlyEmBodyMergeFrame::createDocument()
{
  setDocument(ZSharedPointer<ZStackDoc>(new ZFlyEmBodyMergeDoc));
}


void ZFlyEmBodyMergeFrame::setDvidTarget(const ZDvidTarget &target)
{
  getCompleteDocument()->setDvidTarget(target);
}

ZFlyEmBodyMergeDoc* ZFlyEmBodyMergeFrame::getCompleteDocument()
{
  return qobject_cast<ZFlyEmBodyMergeDoc*>(document().get());
}

ZStackFrame*
ZFlyEmBodyMergeFrame::Make(QMdiArea *parent)
{
  return Make(parent, ZSharedPointer<ZFlyEmBodyMergeDoc>(
                new ZFlyEmBodyMergeDoc));
}

ZStackFrame* ZFlyEmBodyMergeFrame::Make(
    QMdiArea *parent, ZSharedPointer<ZFlyEmBodyMergeDoc> doc)
{
  ZStackFrame *frame = new ZFlyEmBodyMergeFrame(parent);

  BaseConstruct(frame, doc);

  return frame;
}

void ZFlyEmBodyMergeFrame::enableMessageManager()
{
  if (m_messageManager == NULL) {
    m_messageManager = ZMessageManager::Make<MessageProcessor>(this);
  }

  view()->enableMessageManager();
}

void ZFlyEmBodyMergeFrame::MessageProcessor::processMessage(
    ZMessage *message, QWidget *host) const
{
  switch (message->getType()) {
  case ZMessage::TYPE_FLYEM_SPLIT:
  {
    ZFlyEmBodyMergeFrame *frame = qobject_cast<ZFlyEmBodyMergeFrame*>(host);
    if (frame != NULL) {
      uint64_t id = frame->getCompleteDocument()->getSelectedBodyId();
      if (id > 0) {
        message->setBodyEntry("body_id", id);
        if (frame->getCompleteDocument()->getDvidTarget().isValid()) {
          message->setBodyEntry(
                "dvid_target",
                frame->getCompleteDocument()->getDvidTarget().toJsonObject());
        }
      }
    }
  }
    break;
  default:
    break;
  }
}
