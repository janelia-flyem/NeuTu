#ifndef ZSTACKARRAY_H
#define ZSTACKARRAY_H

#include <vector>
#include <algorithm>

#include "tz_cuboid_i.h"
#include "zstackptr.h"

class ZIntCuboid;

class ZStackArray : public std::vector<ZStackPtr>
{
public:
  ZStackArray();
  explicit ZStackArray(const std::vector<ZStackPtr> &stackArray);
  explicit ZStackArray(const std::vector<ZStack*> &stackArray);
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

  /*!
   * \brief Append a stack.
   *
   * \a stack will be owned by the array, but nothing will happen if \a stack
   * is NULL.
   *
   * \param Append a stack to the end of the array.
   */
  void append(ZStack *stack);
  void append(const ZStackPtr &stack);

  template <class InputIterator>
  void append(const InputIterator &first, const InputIterator &last);

  std::vector<ZStack*> toRawArray() const;

  template<class Compare>
  void sort(Compare comp);
};

template <class InputIterator>
void ZStackArray::append(const InputIterator &first, const InputIterator &last)
{
  for (InputIterator iter = first; iter != last; ++iter) {
    append(*iter);
  }
}

template <class Compare>
void ZStackArray::sort(Compare comp)
{
  std::sort(begin(), end(), comp);
}

#endif // ZSTACKARRAY_H
