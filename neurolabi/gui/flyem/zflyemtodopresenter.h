#ifndef ZFLYEMTODOPRESENTER_H
#define ZFLYEMTODOPRESENTER_H

#include <functional>

#include "zabstractmodelpresenter.h"

class ZFlyEmToDoItem;

class ZFlyEmTodoPresenter : public ZAbstractModelPresenter
{
  Q_OBJECT

public:
  // keep this in sync with the m_fieldList in the .cpp file!
  enum TodoTableColumns {
    TODO_BODYID_COLUMN,
    TODO_ACTION_COLUMN,
    TODO_COMMENT_COLUMN,
    TODO_PRIORITY_COLUMN,
    TODO_Z_COLUMN,
    TODO_X_COLUMN,
    TODO_Y_COLUMN,
    TODO_USERNAME_COLUMN
  };


  ZFlyEmTodoPresenter();
  virtual QVariant data(
      const ZFlyEmToDoItem &item, int index, int role) const;

  void setVisibleTest(std::function<bool(const ZFlyEmToDoItem&)> f);

private:
  std::function<bool(const ZFlyEmToDoItem&)> m_visible =
      [](const ZFlyEmToDoItem&) {return true;};
};

#endif // ZFLYEMTODOPRESENTER_H
