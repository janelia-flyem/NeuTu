#include "zflyemtodoitem.h"

#include <iostream>

ZFlyEmToDoItem::ZFlyEmToDoItem()
{
  m_type = GetType();
  init();
}

void ZFlyEmToDoItem::init()
{
  setKind(KIND_INVALID);
}


std::ostream& operator<< (std::ostream &stream, const ZFlyEmToDoItem &item)
{
  //"Kind": (x, y, z)
  switch (item.getKind()) {
  case ZDvidAnnotation::KIND_POST_SYN:
    stream << "PostSyn";
    break;
  case ZDvidAnnotation::KIND_PRE_SYN:
    stream << "PreSyn";
    break;
  case ZDvidAnnotation::KIND_NOTE:
    stream << "Note";
    break;
  case ZDvidAnnotation::KIND_INVALID:
    stream << "Invalid";
    break;
  default:
    stream << "Unknown";
    break;
  }

  stream << ": " << "(" << item.getPosition().getX() << ", "
         << item.getPosition().getY() << ", "
         << item.getPosition().getZ() << ")";

  return stream;
}
