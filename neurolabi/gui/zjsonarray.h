#ifndef ZJSONARRAY_H
#define ZJSONARRAY_H

#include <cstdlib>
#include <functional>

#include "zjsonvalue.h"
#include "zuncopyable.h"

class ZJsonArray : public ZJsonValue
{
public:
  ZJsonArray();
//  explicit ZJsonArray(json_t *data, bool asNew);
//  explicit ZJsonArray(const json_t *data, bool asNew);
  explicit ZJsonArray(json_t *data, ESetDataOption option);
  explicit ZJsonArray(const json_t *data, ESetDataOption option);
  explicit ZJsonArray(const ZJsonValue &v);
  virtual ~ZJsonArray();

public:
  size_t size() const;
  json_t* at(size_t index);
  const json_t* at(size_t index) const;

  ZJsonValue value(size_t index) const;

  /*!
   * \brief Append an element
   */
  void append(const ZJsonValue &obj);
  void setValue(size_t i, const ZJsonValue &obj);

  void append(int v);
  void append(int64_t v);
  void append(uint64_t v);
  void append(bool v);
  void append(double v);
  void append(const char *str);
  void append(const std::string &str);

  void concat(ZJsonArray &array);

  void remove(size_t index);

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
   * Any json array element that is not an integer will be ignored.
   */
  std::vector<int> toIntegerArray() const;

  /*!
   * \brief Get a boolean array from the json array.
   *
   * Any json array element that is not a boolean will be ignored.
   */
  std::vector<bool> toBoolArray() const;

  /*!
   * \brief Get a string array from the json array.
   *
   * Any json array element that is not a string will be ignored.
   */
  std::vector<std::string> toStringArray() const;

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
  bool decode(const std::string &str, bool reportingError = false);

  bool isEmpty() const;
  void denull();

  std::string dumpJanssonString(size_t flags) const;

  /*!
   * \brief Go through a string array.
   *
   * \param f The function of processing a given string element.
   */
  void forEachString(std::function<void(const std::string &str)>f);
  ZJsonArray filter(std::function<bool(ZJsonValue)> pred);
};

#endif // ZJSONARRAY_H
