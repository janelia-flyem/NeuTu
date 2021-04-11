#ifndef ZJSONOBJECT_H
#define ZJSONOBJECT_H

#include <cstdint>
#include <string>
#include <vector>
#include <map>
#include <initializer_list>
#include <functional>

#include "zjsonvalue.h"

class ZJsonArray;

#define ZJsonObject_foreach(jsonObject, key, value) \
  json_object_foreach(jsonObject.getValue(), key, value)

class ZJsonObject : public ZJsonValue
{
public:
  ZJsonObject();
//  explicit ZJsonObject(json_t *json, bool asNew);
  explicit ZJsonObject(json_t *data, ESetDataOption option);
  ZJsonObject(const ZJsonObject &obj);
  ZJsonObject(const ZJsonValue &obj);

  virtual ~ZJsonObject();

  json_t* operator[] (const char *key);
  const json_t* operator[] (const char *key) const;

  ZJsonValue value(const char *key) const;
  ZJsonValue value(const std::string &key) const;
#ifndef SWIG
  ZJsonValue value(const std::initializer_list<const char*> &keyList) const;
#endif
  /*!
   * \brief Test if an object is empty
   *
   * An object is empty iff no key exists.
   */
  bool isEmpty() const;
  void denull();

public:
  /*!
   * \brief Decode a string.
   *
   * All old entries of the object will be removed no matter whether the decoding
   * succeeds or not.
   *
   * \param str Source string.
   * \return true iff the decoding succeeds.
   */
  bool decode(const std::string &str, bool reportingError);

  std::string summary();
  std::map<std::string, json_t*> toEntryMap(bool recursive) const;

  /*!
   * \brief Test if a key is valid
   *
   * \return true iff \a key is valid.
   */
  static bool isValidKey(const char *key);

  /*!
   * \brief Set an entry of the object
   *
   * The reference count of \a obj will be increased after the function call.
   */
  void setEntry(const char *key, json_t *obj);

  /*!
   * \brief Set an entry without increasing the reference count
   */
  void consumeEntry(const char *key, json_t *obj);

  /*!
   * \brief Set an entry of the object with string value
   *
   * The function does nothing if the key is empty.
   */
  void setEntry(const char *key, const std::string &value);
  void setEntry(const char *key, const char *value);
  void setEntry(const std::string &key, const std::string &value);

  /*!
   * \brief Set an entry to a string array
   *
   * Nothing will be done if \a value is empty. Any empty string in \a value
   * will also be ignored.
   *
   * \param key Key of the entry
   * \param value String array of the entry
   */
  void setEntry(const char *key, const std::vector<std::string> &value);

  /*!
   * \brief Set the entry if the value is not empty
   *
   * It does nothing if \a value is empty.
   */
  void setNonEmptyEntry(const char *key, const std::string &value);

  void setNonEmptyEntry(const char *key, const ZJsonObject &obj);
  void setNonEmptyEntry(const char *key, const ZJsonArray &obj);

  /*!
   * \brief Set an entry of the object with an array
   *
   * \param key Key
   * \param array Array buffer.
   * \param n Number of elements of the array.
   */
  void setEntry(const char *key, const double *array, size_t n);

  /*!
   * \brief Set an entry of the object with an integer array
   *
   * \param key Key
   * \param array Array buffer.
   * \param n Number of elements of the array.
   */
  void setEntry(const char *key, const int *array, size_t n);

  /*!
   * \brief setEntry Set an entry of the object with a boolean
   */
  void setEntry(const char *key, bool v);

  /*!
   * \brief Set the entry if the value is true.
   *
   * It does nothing if \a v is false.
   */
  void setTrueEntry(const char *key, bool v);

  /*!
   * \brief setEntry Set an entry of the object with an integer
   */
  void setEntry(const char *key, int64_t v);
  void setEntry(const char *key, uint64_t v);
  void setEntry(const char *key, int v);

  void setEntry(const std::string &key, int64_t v);
  void setEntry(const std::string &key, uint64_t v);
  void setEntry(const std::string &key, int v);

  /*!
   * \brief setEntry Set an entry of the object with a double
   */
  void setEntry(const char *key, double v);

  /*!
   * \brief setEntry Set an entry of the object
   */
  void setEntry(const char *key, ZJsonValue &value);
  void setEntry(const std::string &key, ZJsonValue &value);

  /*!
   * \brief Add an entry
   *
   * THe function adds the entry \a key -> \a value if \a key does not exist
   * in the current object.
   *
   * \return /for future/true iff the entry is added
   */
  void addEntry(const char *key, ZJsonValue &value);
  void addEntry(const char *key, const std::string &value);
  void addEntry(const char *key, const char *value);

  /*!
   * \brief Add fields from another object
   *
   * A field of \a obj is added to the current object iff its key does not exist
   * in the current object.
   */
  void addEntryFrom(const ZJsonObject &obj);

  /*!
   * \brief Test if a key exists
   *
   * \return true iff \a key exists in the object (only the first level).
   */
  bool hasKey(const char *key) const;
  bool hasKey(const std::string &key) const;

  /*!
   * \brief Set an entry as an array
   *
   * It returns NULL if the key already exists.
   *
   * \param key The entry key.
   * \return The json array added to the object.
   */
  json_t *setArrayEntry(const char *key);

  void setValue(const ZJsonValue &value);

  std::vector<std::string> getAllKey() const;

  void removeKey(const char *key);

//  std::string dumpString(int indent = 2) const;

  /*!
   * \brief Using flags in libjansson to produce a json string.
   */
  virtual std::string dumpJanssonString(size_t flags) const;

  void forEachValue(std::function<void(ZJsonValue)> f) const;

private:
  void setEntryWithoutKeyCheck(const char *key, json_t *obj, bool asNew = false);

private:
  static void appendEntries(const char *key, json_t *obj,
                            std::map<std::string, json_t*> *entryMap);
};

#endif // ZJSONOBJECT_H
