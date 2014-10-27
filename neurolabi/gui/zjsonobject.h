#ifndef ZJSONOBJECT_H
#define ZJSONOBJECT_H

#include <string>
#include <map>

#include "zjsonvalue.h"

#define ZJsonObject_foreach(jsonObject, key, value) \
  json_object_foreach(jsonObject.getValue(), key, value)

class ZJsonObject : public ZJsonValue
{
public:
  explicit ZJsonObject(json_t *json, bool asNew);
  explicit ZJsonObject(json_t *data, ESetDataOption option);
  ZJsonObject();
  ZJsonObject(const ZJsonObject &obj);
  virtual ~ZJsonObject();

  json_t* operator[] (const char *key);
  const json_t* operator[] (const char *key) const;

  /*!
   * \brief Test if an object is empty
   *
   * An object is empty iff no key exists.
   */
  bool isEmpty() const;

public:
  bool load(const std::string &filePath);

  /*!
   * \brief Decode a string.
   *
   * All old entries of the object will be removed no matter whether the decoding
   * succeeds or not.
   *
   * \param str Source string.
   * \return true iff the decoding succeeds.
   */
  bool decode(const std::string &str);

  std::string summary();
  std::map<std::string, json_t*> toEntryMap(bool recursive = true) const;

  /*!
   * \brief Test if a key is valid
   *
   * \return true iff \a key is valid.
   */
  static bool isValidKey(const char *key);

  /*!
   * \brief Set an entry of the object
   */
  void setEntry(const char *key, json_t *obj);

  /*!
   * \brief Set an entry without increasing the reference count
   */
  void consumeEntry(const char *key, json_t *obj);

  /*!
   * \brief Set an entry of the object with string value
   *
   * The function does not if key is NULL or empty.
   */
  void setEntry(const char *key, const std::string &value);

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
   * \brief setEntry Set an entry of the object with an integer
   */
  void setEntry(const char *key, int v);

  /*!
   * \brief setEntry Set an entry of the object with a double
   */
  void setEntry(const char *key, double v);

  /*!
   * \brief setEntry Set an entry of the object
   */
  void setEntry(const char *key, ZJsonValue &value);


  /*!
   * \brief Test if a key exists
   *
   * \return true iff \a key exists in the object (only the first level).
   */
  bool hasKey(const char *key) const;

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

private:
  void setEntryWithoutKeyCheck(const char *key, json_t *obj, bool asNew = false);

private:
  static void appendEntries(const char *key, json_t *obj,
                            std::map<std::string, json_t*> *entryMap);
};

#endif // ZJSONOBJECT_H
