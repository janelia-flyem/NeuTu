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
    TODO_TIP_INDEX_COLUMN,
    TODO_TIP_ROI_COLUMN,
    TODO_USERNAME_COLUMN
  };


  ZFlyEmTodoPresenter();
  virtual QVariant data(
      const ZFlyEmToDoItem &item, int index, int role) const;

  void setVisibleTest(std::function<bool(const ZFlyEmToDoItem&)> f);

private:
  static const std::string KEY_TIPQC_INDEX;
  static const std::string KEY_TIPQC_ROI;
  std::function<bool(const ZFlyEmToDoItem&)> m_visible =
          [](const ZFlyEmToDoItem&) {return true;};
  QString getTipRoi(const ZFlyEmToDoItem &item) const;
  QVariant getTipIndex(const ZFlyEmToDoItem &item) const;
};

#endif // ZFLYEMTODOPRESENTER_H
