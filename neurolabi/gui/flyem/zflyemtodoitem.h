#ifndef ZFLYEMTODOITEM_H
#define ZFLYEMTODOITEM_H

#include "dvid/zdvidannotation.h"

class ZFlyEmToDoItem : public ZDvidAnnotation
{
public:
  ZFlyEmToDoItem();

  static ZStackObject::EType GetType() {
    return ZStackObject::TYPE_FLYEM_TODO_ITEM;
  }

  friend std::ostream& operator<< (
      std::ostream &stream, const ZFlyEmToDoItem &synapse);

private:
  void init();
};

#endif // ZFLYEMTODOITEM_H
