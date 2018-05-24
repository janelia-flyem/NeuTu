#ifndef ZFLYEMTODOPRESENTER_H
#define ZFLYEMTODOPRESENTER_H

#include "zabstractmodelpresenter.h"

class ZFlyEmToDoItem;

class ZFlyEmTodoPresenter : public ZAbstractModelPresenter
{
  Q_OBJECT

public:
  ZFlyEmTodoPresenter();
  virtual QVariant data(
      const ZFlyEmToDoItem &item, int index, int role) const;
};

#endif // ZFLYEMTODOPRESENTER_H
