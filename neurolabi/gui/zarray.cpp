#include "zarray.h"

#include <iostream>
#include <string>
#include <cstdlib>
#include <cstring>

ZArray::ZArray() : m_data(NULL)
{
}

ZArray::ZArray(ZArray::Value_Type type, int ndims, mylib::Dimn_Type *dims)
{
  m_data = Make_Array(mylib::PLAIN_KIND, type, ndims, dims);
  setZero();
  m_startCoordinates.resize(ndims, 0);
}

ZArray::ZArray(const ZArray &array)
{
  if (array.m_data == NULL) {
    m_data = NULL;
  } else {
    m_data = mylib::Copy_Array(array.m_data);
  }
  m_startCoordinates = array.m_startCoordinates;
}

ZArray::~ZArray()
{
  if (m_data != NULL) {
    mylib::Kill_Array(m_data);
  }
}

void ZArray::printInfo() const
{
  if (m_data == NULL) {
    std::cout << "Empty array: null data" << std::endl;
  } else {
    if (isEmpty()) {
      std::cout << "Empty array" << std::endl;
    } else {
      std::cout << "Array(#): " << mylib::Array_Refcount(m_data) << std::endl;
      std::cout << "  size: " << dim(0);
      for (int i = 1; i < ndims(); i++) {
        std::cout << " x " << dim(i);
      }
      std::cout << std::endl;

      switch (valueType()) {
      case mylib::UINT8_TYPE:
        std::cout << "  uint8" << std::endl;
        break;
      case mylib::UINT16_TYPE:
        std::cout << "  uint16" << std::endl;
        break;
      case mylib::UINT32_TYPE:
        std::cout << "  uint32" << std::endl;
        break;
      case mylib::UINT64_TYPE:
        std::cout << "  uint64" << std::endl;
        break;
      case mylib::INT8_TYPE:
        std::cout << "  int8" << std::endl;
        break;
      case mylib::INT16_TYPE:
        std::cout << "  int16" << std::endl;
        break;
      case mylib::INT32_TYPE:
        std::cout << "  int32" << std::endl;
        break;
      case mylib::INT64_TYPE:
        std::cout << "  int64" << std::endl;
        break;
      case mylib::FLOAT32_TYPE:
        std::cout << "  single float" << std::endl;
      case mylib::FLOAT64_TYPE:
        std::cout << "  double float" << std::endl;
      default:
          std::cout << "  unknown type" << std::endl;
      }
    }
  }
}

size_t ZArray::getElementNumber() const
{
  if (m_data == NULL) {
    return 0;
  }

  size_t s = 0;
  if (ndims() > 0) {
    s = 1;
    for (int i = 0; i < ndims(); ++i) {
      s *= dim(i);
    }
  }

  return s;
}

bool ZArray::isEmpty() const
{
  return getElementNumber() == 0;
}

void ZArray::setZero()
{
  if (!isEmpty()) {
    bzero(m_data->data, getByteNumber());
  }
}

size_t ZArray::getByteNumber() const
{
  return getElementNumber() * getValueTypeSize(valueType());
}

void ZArray::copyDataFrom(const void *data)
{
  if (data != NULL) {
    memcpy(m_data->data, data, getByteNumber());
  }
}

size_t ZArray::getValueTypeSize(ZArray::Value_Type valueType)
{
  switch (valueType) {
  case mylib::UINT8_TYPE:
  case mylib::INT8_TYPE:
    return 1;
  case mylib::UINT16_TYPE:
  case mylib::INT16_TYPE:
    return 2;
  case mylib::UINT32_TYPE:
  case mylib::INT32_TYPE:
    return 4;
  case mylib::UINT64_TYPE:
  case mylib::INT64_TYPE:
    return 8;
  case mylib::FLOAT32_TYPE:
    return 4;
  case mylib::FLOAT64_TYPE:
    return 8;
  default:
    break;
  }

  return 0;
}

void ZArray::print() const
{
  printInfo();

  if (!isEmpty()) {
    const char *format = NULL;
    switch (valueType()) {
    case mylib::UINT8_TYPE:
    case mylib::UINT16_TYPE:
    case mylib::UINT32_TYPE:
      format = "%u";
      break;
    case mylib::UINT64_TYPE:
      format = "%llu";
      break;
    case mylib::INT8_TYPE:
    case mylib::INT16_TYPE:
    case mylib::INT32_TYPE:
      format = "%d";
      break;
    case mylib::INT64_TYPE:
      format = "%lld";
      break;
    case mylib::FLOAT32_TYPE:
    case mylib::FLOAT64_TYPE:
      format = "%g";
      break;
    default:
      break;
    }

    if (!isEmpty() && format != NULL) {
      mylib::Print_Array(m_data, stdout, 0, const_cast<char*>(format));
    }
  }
}

uint64_t ZArray::getUint64Value(size_t index) const
{
  if (index > getElementNumber()) {
    return 0;
  }

  uint64_t *array = getDataPointer<uint64_t>();

  return array[index];
}

int ZArray::getDim(int index)
{
  if (index < 0 || index >= ndims()) {
    return 1;
  }

  return dim(index);
}

int ZArray::getStartCoordinate(int index) const
{
  if (index < 0 || index >= ndims()) {
    return 0;
  }

  return m_startCoordinates[index];
}

void ZArray::setStartCoordinate(int index, int x)
{
  if (index >= 0 && index < ndims()) {
    m_startCoordinates[index] = x;
  }
}
