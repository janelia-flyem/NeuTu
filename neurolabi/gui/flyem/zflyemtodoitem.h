#ifndef ZFLYEMTODOITEM_H
#define ZFLYEMTODOITEM_H

#include "dvid/zdvidannotation.h"

class ZFlyEmToDoItem : public ZDvidAnnotation
{
public:
  /*!
   * \brief Default constructor
   *
   * The object is always set to invalid kind by default.
   */
  ZFlyEmToDoItem();

  /*!
   * \brief Constructor with initial position
   *
   * With a position given, the object is set to a valid type.
   */
  ZFlyEmToDoItem(const ZIntPoint &pos);
  ZFlyEmToDoItem(int x, int y, int z);

  const std::string& className() const;
  void display(ZPainter &painter, int slice, EDisplayStyle option,
               neutube::EAxis sliceAxis) const;

  static ZStackObject::EType GetType() {
    return ZStackObject::TYPE_FLYEM_TODO_ITEM;
  }

  friend std::ostream& operator<< (
      std::ostream &stream, const ZFlyEmToDoItem &synapse);

  bool isChecked() const;
  void setChecked(bool checked);

  void setAction(neutube::EToDoAction action);
  neutube::EToDoAction getAction() const;

  QColor getDisplayColor() const;

  void removeActionTag();

private:
  void syncActionTag();
  static std::string GetActionTag(neutube::EToDoAction action);

public:
  static const char *ACTION_KEY;
  static const char *ACTION_SPLIT;
  static const char *ACTION_SUPERVOXEL_SPLIT;
  static const char *ACTION_IRRELEVANT;
  static const char *ACTION_MERGE;
  static const char *ACTION_SPLIT_TAG;
  static const char *ACTION_SUPERVOXEL_SPLIT_TAG;
  static const char *ACTION_IRRELEVANT_TAG;
  static const char *ACTION_MERGE_TAG;

private:
  void init();
  void init(EKind kind);
};

#endif // ZFLYEMTODOITEM_H
