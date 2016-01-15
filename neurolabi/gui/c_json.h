#ifndef C_JSON_H
#define C_JSON_H

#include "jansson.h"

#include <string>
#include <vector>

namespace C_Json {
json_t* makeObject();
json_t* makeInteger(int v);
json_t* makeNumber(double v);
json_t* makeBoolean(bool v);
json_t* makeArray();
json_t* makeString(const char *v);
json_t* makeString(const std::string &v);

/*!
 * \brief Make a JSON array from a double array
 * \param array The array buffer.
 * \param n Number of elements.
 * \return The JSON object if it succeeds, otherwise returns NULL (if \a array
 *         is NULL or \a n is 0).
 */
json_t* makeArray(const double *array, size_t n);

/*!
 * \brief Make a JSON array from an integer array
 * \param array The array buffer.
 * \param n Number of elements.
 * \return The JSON object if it succeeds, otherwise returns NULL (if \a array
 *         is NULL or \a n is 0).
 */
json_t* makeArray(const int *array, size_t n);

void appendArray(json_t *array, json_t *v);

void decref(json_t *json);
void incref(json_t *json);

bool isObject(const json_t *value);
bool isArray(const json_t *value);
bool isInteger(const json_t *value);
bool isReal(const json_t *value);
bool isNumber(const json_t *value);
bool isBoolean(const json_t *value);

bool dump(const json_t *obj, const char *filePath);

json_type type(const json_t *value);

const char* stringValue(const json_t *value);
double numberValue(const json_t *value);

/*!
 * \brief Integer value of a json value.
 *
 * \return 0 if \a value is not in integer type or it is NULL.
 */
int64_t integerValue(const json_t *value);
bool booleanValue(const json_t *value);

//It returns 0 if <array> is not an array
size_t arraySize(const json_t *array);

json_t* arrayValue(const json_t *array, size_t index);

const char* stringValue(const json_t *value, size_t index);
double numberValue(const json_t *value, size_t index);
int integerValue(const json_t *value, size_t index);

std::vector<int> integerArray(const json_t *value);

void print(const char *key, json_t *value, int indent);

json_t* clone(json_t *value);
}

#endif // C_JSON_H
