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

  void setVisibleTest(std::function<bool(const ZFlyEmToDoItem&)> f);

private:
  std::function<bool(const ZFlyEmToDoItem&)> m_visible =
      [](const ZFlyEmToDoItem&) {return true;};
};

#endif // ZFLYEMTODOPRESENTER_H
