#ifndef ZJSONARRAY_H
#define ZJSONARRAY_H

#include <stddef.h>
#include "zjsonvalue.h"
#include "zuncopyable.h"

class ZJsonArray : public ZJsonValue, ZUncopyable
{
public:
  ZJsonArray();
  ZJsonArray(json_t *data, bool asNew);
  virtual ~ZJsonArray();

public:
  ::size_t size() const;
  json_t* at(::size_t index);
  const json_t* at(::size_t index) const;

  /*!
   * \brief Append an element.
   * \param obj The element to be appended. Nothing is done if it is NULL.
   */
  void append(json_t *obj);

  /*!
   * \brief Append an element
   */
  void append(ZJsonValue &obj);

  /*!
   * \brief Get a number array from the json array.
   *
   * \return Returns an empty array if the object can not be converted into a
   *         number array.
   */
  std::vector<double> toNumberArray() const;

  /*!
   * \brief Get an integer array from the json array.
   *
   * Any json array element that is not integer will be ignored.
   */
  std::vector<int> toIntegerArray() const;

  ZJsonArray& operator << (double e);
};

#endif // ZJSONARRAY_H
