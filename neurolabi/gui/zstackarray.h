#ifndef ZSTACKARRAY_H
#define ZSTACKARRAY_H

#include <vector>

class ZStack;

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
};

#endif // ZSTACKARRAY_H
