#ifndef ZJSONVALUE_H
#define ZJSONVALUE_H

#include <vector>
#include <string>
#include <iostream>

#include "neurolabi_config.h"
#include "c_json.h"
//#include "tz_json.h"

class ZJsonValue
{
public:
  enum ESetDataOption {
    SET_INCREASE_REF_COUNT, SET_AS_IT_IS, SET_DEEP_COPY, SET_SHALLOW_COPY
  };

  ZJsonValue();
  ZJsonValue(const ZJsonValue &value);

  /*!
   * \brief Constructor
   *
   * \param data json value pointer
   * \param asNew Take it as a new value or just increase its reference count
   */
//  ZJsonValue(json_t *data, bool asNew);

  ZJsonValue(json_t *data, ESetDataOption option);
  ZJsonValue(const json_t *data, ESetDataOption option);

  /*!
   * \brief Constructor
   *
   * Create a json object with integer data
   *
   * \param data The input integer
   */
  explicit ZJsonValue(int data);
  explicit ZJsonValue(double data);
  explicit ZJsonValue(const char *data);

  ZJsonValue& operator= (const ZJsonValue &value);

  virtual ~ZJsonValue();

  ZJsonValue clone() const;

public:
  inline json_t *getData() const { return m_data; }
  inline json_t *getValue() const { return m_data; }

  bool isObject() const;
  bool isArray() const;
  bool isString() const;
  bool isInteger() const;
  bool isReal() const;
  bool isNumber() const;
  bool isBoolean() const;
  virtual bool isEmpty() const;
  bool isNull() const;

  std::string getType() const;

  /*!
   * \brief Get the integer value of the json value.
   *
   * \return 0 if the object is not a json integer.
   */
  int toInteger() const;

  /*!
   * \brief Get the real value of the json value
   *
   * \return 0 if the object is not an integer or real type.
   */
  double toReal() const;

  /*!
   * \brief Get the boolean value of the json value
   *
   * \return false if the object is not a bool type.
   */
  bool toBoolean() const;

  std::string toString() const;

  //std::string toString() const;

  void set(json_t *data, bool asNew);
  void set(json_t *data, ESetDataOption option);
  void set(const ZJsonValue &value);

  virtual void denull();

  /*!
   * \brief Obsolete. Will be removed.
   */
  void decodeString(const char *str, json_error_t *error);
  void decodeString(const char *str);

  void clear();

  void print() const;

  /*!
   * \brief Get elements of a JSON array
   * \return Empty array if the object is not an array.
   */
  std::vector<ZJsonValue> toArray();

  /*!
   * \brief Get a string describing the current error
   */
  static std::string GetErrorString(const json_error_t &error);
  static void PrintError(const json_error_t &error);

  /*!
   * \brief Dump the object to a string.
   */
  std::string dumpString(int indent = 2) const;

  /*!
   * \brief Using flags in libjansson to produce a json string.
   */
  virtual std::string dumpJanssonString(size_t flags) const;

  /*!
   * \brief Save the json value into a file
   *
   * \return true iff write succeeds.
   */
  bool dump(const std::string &path) const;

  bool load(const std::string &filePath);

  std::string getSource() const;

  int getRefCount() const;

  friend std::ostream& operator << (std::ostream &stream, const ZJsonValue &v);

protected:
//  json_error_t m_error;
  json_t *m_data;
  std::string m_source;
};

#endif
