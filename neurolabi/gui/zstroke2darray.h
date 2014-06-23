#ifndef ZSTROKE2DARRAY_H
#define ZSTROKE2DARRAY_H

#include <QVector>
#include "zstroke2d.h"
#include "c_stack.h"
#include "zsparseobject.h"

class ZStack;

class ZStroke2dArray : public QVector<ZStroke2d*>
{
public:
  ZStroke2dArray();
  ~ZStroke2dArray();

  void labelStack(Stack *stack) const;
  void labelStack(ZStack *stack) const;

  ZStack* toStack() const;

  /*!
   * \brief Get bound box of the stroke array.
   *
   * It's not necessary to be the minimal bounding box.
   */
  ZCuboid getBoundBox() const;

  /*!
   * \brief Convert the stroke array into a 3D object
   */
  ZObject3d* toObject3d() const;
};

#endif // ZSTROKE2DARRAY_H
