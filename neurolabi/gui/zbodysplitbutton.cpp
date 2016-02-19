#include "zbodysplitbutton.h"
#include "zmessagemanager.h"
#include "zmessage.h"

ZBodySplitButton::ZBodySplitButton(QWidget *parent) :
  QPushButton(parent), m_messageManager(NULL)
{
  setText("Launch Split");
  connect(this, SIGNAL(clicked()), this, SLOT(requestSplit()));
}


void ZBodySplitButton::enableMessageManager(ZMessageManager *parent)
{
  if (m_messageManager == NULL) {
    m_messageManager = ZMessageManager::Make(this, parent);
  }
}

void ZBodySplitButton::requestSplit()
{
  if (m_messageManager != NULL) {
    ZMessage message(this);
    message.setType(ZMessage::TYPE_FLYEM_SPLIT);
    m_messageManager->processMessage(&message, true);
  }
}
