#ifndef ZJSONARRAY_H
#define ZJSONARRAY_H

#include "tz_stdint.h"
#include "zjsonvalue.h"
#include "zuncopyable.h"

class ZJsonArray : public ZJsonValue
{
public:
  ZJsonArray();
  explicit ZJsonArray(json_t *data, bool asNew);
  explicit ZJsonArray(const json_t *data, bool asNew);
  explicit ZJsonArray(json_t *data, ESetDataOption option);
  explicit ZJsonArray(const ZJsonValue &v);
  virtual ~ZJsonArray();

public:
  ::size_t size() const;
  json_t* at(::size_t index);
  const json_t* at(::size_t index) const;

  ZJsonValue value(::size_t index) const;

  /*!
   * \brief Append an element
   */
  void append(const ZJsonValue &obj);
  void setValue(::size_t i, const ZJsonValue &obj);

  void append(int v);
  void append(int64_t v);
  void append(uint64_t v);
  void append(double v);
  void append(const char *str);
  void append(const std::string &str);

  void remove(::size_t index);

  /*!
   * \brief Append an element.
   * \param obj The element to be appended. Nothing is done if it is NULL.
   */
  void append(json_t *obj);

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

  /*!
   * \brief Get a boolean array from the json array.
   *
   * Any json array element that is not boolean will be ignored.
   */
  std::vector<bool> toBoolArray() const;

  ZJsonArray& operator << (double e);

  /*!
   * \brief Decode a string.
   *
   * The object will be cleared first no matter whether the decoding
   * succeeds or not.
   *
   * \param str Source string.
   * \return true iff the decoding succeeds.
   */
  bool decode(const std::string &str);

  bool isEmpty() const;

  std::string dumpString(int indent = 2) const;
};

#endif // ZJSONARRAY_H
