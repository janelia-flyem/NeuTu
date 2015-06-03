#ifndef ZSTACKARRAY_H
#define ZSTACKARRAY_H

#include <vector>
#include "tz_cuboid_i.h"

class ZStack;
class ZIntCuboid;

class ZStackArray : public std::vector<ZStack*>
{
public:
  ZStackArray();
  ZStackArray(const std::vector<ZStack*> &stackArray);
  ~ZStackArray();

public:
  /*!
   * \brief Paste values to another stack
   */
  void paste(ZStack *stack, int valueIgnored = -1) const;

  /*!
   * \brief Get the bound box of the stack array.
   *
   * The result is stored in \a box.
   */
  void getBoundBox(Cuboid_I *box) const;
  ZIntCuboid getBoundBox() const;

  void downsampleMax(int xIntv, int yIntv, int zIntv);
};

#endif // ZSTACKARRAY_H
