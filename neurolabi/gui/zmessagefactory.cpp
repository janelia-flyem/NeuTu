#include "zmessagefactory.h"

#include "zmessage.h"

ZMessageFactory::ZMessageFactory()
{
}

ZMessage ZMessageFactory::Make3DVisMessage(QWidget *source)
{
  ZMessage message(source);
  Make3DVisMessage(message);
  return message;
}

void ZMessageFactory::Make3DVisMessage(ZMessage &message)
{
  message.setType(ZMessage::TYPE_3D_VIS);
}

void ZMessageFactory::MakeFlyEmSplit3DVisMessage(ZMessage &message)
{
  message.setType(ZMessage::TYPE_FLYEM_SPLIT_VIEW_3D_BODY);
}

void ZMessageFactory::MakeQuick3DVisMessage(ZMessage &message, int coarseLevel)
{
  message.setType(ZMessage::TYPE_FLYEM_COARSE_3D_VIS);
  message.setBodyEntry("coarse_level", coarseLevel);
}
