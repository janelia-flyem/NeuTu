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

  const std::string& className() const;
  void display(ZPainter &painter, int slice, EDisplayStyle option,
               NeuTube::EAxis sliceAxis) const;

  static ZStackObject::EType GetType() {
    return ZStackObject::TYPE_FLYEM_TODO_ITEM;
  }

  friend std::ostream& operator<< (
      std::ostream &stream, const ZFlyEmToDoItem &synapse);

  bool isChecked() const;
  void setChecked(bool checked);

  QColor getDisplayColor() const;

private:
  void init();
  void init(EKind kind);
};

#endif // ZFLYEMTODOITEM_H
