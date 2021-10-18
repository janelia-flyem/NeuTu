#include "zarray.h"

#include <iostream>
#include <string>
#include <cstdint>
#include <cstring>

#include "geometry/zintcuboid.h"

ZArray::ZArray() : m_data(NULL)
{
}

ZArray::ZArray(ZArray::Value_Type type, int ndims, mylib::Dimn_Type *dims)
{
  m_data = Make_Array(mylib::PLAIN_KIND, type, ndims, dims);
  setZero();
  m_startCoordinates.resize(ndims, 0);
}

ZArray::ZArray(Value_Type type, const std::vector<int> &dims)
{
  if (dims.empty()) {
    return;
  }

  std::vector<mylib::Dimn_Type> cdims(dims.size());
  for (size_t i = 0; i < dims.size(); ++i) {
    cdims[i] = dims[i];
  }

  m_data = Make_Array(mylib::PLAIN_KIND, type, dims.size(), cdims.data());
  setZero();
  m_startCoordinates.resize(dims.size(), 0);
}

ZArray::ZArray(const ZArray &array)
{
  *this = array;
}

ZArray::~ZArray()
{
  if (m_data != NULL) {
    mylib::Kill_Array(m_data);
  }
}

ZArray& ZArray::operator = (const ZArray &array)
{
  if (array.m_data == NULL) {
    m_data = NULL;
  } else {
    m_data = mylib::Copy_Array(array.m_data);
  }
  m_startCoordinates = array.m_startCoordinates;

  return *this;
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
      std::cout << "  size: " << getDim(0);
      for (int i = 1; i < getRank(); i++) {
        std::cout << " x " << getDim(i);
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

void* ZArray::getDataPointer(size_t offset) const
{
  if (m_data == nullptr) {
    return nullptr;
  }

  if (m_data->data == nullptr) {
    return nullptr;
  }

  return reinterpret_cast<void*>(
        reinterpret_cast<uint8_t*>(m_data->data) + offset);
}


size_t ZArray::getElementNumber() const
{
  if (m_data == NULL) {
    return 0;
  }

  size_t s = 0;
  if (getRank() > 0) {
    s = 1;
    for (int i = 0; i < getRank(); ++i) {
      s *= getDim(i);
    }
  }

  return s;
}

bool ZArray::isEmpty() const
{
  return getElementNumber() == 0;
}

bool ZArray::withinDataRange(const std::vector<int> &coords) const
{
  if (int(coords.size()) != getRank()) {
    return false;
  }

  for (int i = 0; i < getRank(); ++i) {
    if (coords[i] < m_startCoordinates[i] ||
        coords[i] - m_startCoordinates[i] >= getDim(i)) {
      return false;
    }
  }

  return true;
}

void ZArray::setZero()
{
  if (!isEmpty()) {
    memset(m_data->data, 0, getByteNumber());
//    bzero(m_data->data, getByteNumber());
  }
}

size_t ZArray::getBytePerElement() const
{
  return getValueTypeSize(valueType());
}

size_t ZArray::getByteNumber() const
{
  return getElementNumber() * getBytePerElement();
}

void ZArray::copyDataFrom(const void *data)
{
  if (data != NULL) {
    memcpy(m_data->data, data, getByteNumber());
  }
}

void ZArray::copyDataFrom(
    const void *data, size_t elementOffset, size_t elementCount)
{
  if (data && (elementOffset < getElementNumber())) {
    elementCount = std::min(elementCount, getElementNumber() - elementOffset);
    memcpy((char*)(m_data->data) + elementOffset * getBytePerElement(),
           data, elementCount * getBytePerElement());
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
  if (index >= getElementNumber()) {
    return 0;
  }

  uint64_t *array = getDataPointer<uint64_t>();

  return array[index];
}

int ZArray::getDim(int index) const
{
  if (index < 0 || index >= getRank()) {
    return 1;
  }

  return m_data->dims[index];
}

int ZArray::getStartCoordinate(int index) const
{
  if (index < 0 || index >= getRank()) {
    return 0;
  }

  return m_startCoordinates[index];
}

void ZArray::setStartCoordinate(int index, int x)
{
  if (index >= 0 && index < getRank()) {
    m_startCoordinates[index] = x;
  }
}

void ZArray::setStartCoordinate(const std::vector<int> &coord)
{
  m_startCoordinates = coord;
}

void ZArray::setStartCoordinate(int x, int y, int z)
{
  m_startCoordinates[0] = x;
  m_startCoordinates[1] = y;
  m_startCoordinates[2] = z;
}

namespace {

struct MIndex {
  MIndex(const std::vector<int> &start, const std::vector<int> &dims):
    m_start(start), m_dims(dims) {
    startIter();
  }

  size_t getRank() const {
    return m_dims.size();
  }

  void startIter() {
    m_coords = m_start;
    m_coords[0] -= 1;
  }

  inline int getMaxCoord(int i) const {
    return m_start[i] + m_dims[i] - 1;
  }

  bool hasNext() const {
    for (int i = getRank() - 1; i >= 0; --i) {
      if (m_coords[i] < getMaxCoord(i)) {
        return true;
      }
    }

    return false;
  }

  bool next() {
    if (hasNext()) {
      size_t ndims = getRank();
      for (size_t i = 0; i < ndims; ++i) {
        if (m_coords[i] >= getMaxCoord(i)) {
          m_coords[i] = m_start[i];
        } else {
          ++m_coords[i];
          break;
        }
      }

      return true;
    }

    return false;
  }


  std::vector<int> m_start;
  std::vector<int> m_dims;
  std::vector<int> m_coords;
};

}

size_t ZArray::getIndex(const std::vector<int> &coords) const
{
  size_t index = 0;
  size_t stride = 1;
  for (int i = 0; i < getRank(); ++i) {
    index += stride * (coords[i] - m_startCoordinates[i]);
    stride *= getDim(i);
  }

  return index;
}

ZArray* ZArray::crop(
    const std::vector<int> &corner, const std::vector<int> &dims) const
{
  if (int(corner.size()) != getRank() && int(dims.size()) != getRank()) {
    return nullptr;
  }

  ZArray *array = new ZArray(m_data->type, dims);

  array->setZero();
  array->setStartCoordinate(corner);


  if (withinDataRange(corner)) {
    int minX = std::max(corner[0], getStartCoordinate(0));
    int maxX = std::min(
          corner[0] + dims[0] - 1, getStartCoordinate(0) + getDim(0) - 1);
    size_t nbyteToCopy = (maxX - minX + 1) * getBytePerElement();
    size_t targetOffset = 0;

    MIndex index(std::vector<int>(++(corner.begin()), corner.end()),
                 std::vector<int>(++(dims.begin()), dims.end()));
    while (index.next()) {
      std::vector<int> coords(1);
      coords[0] = corner[0];
      coords.insert(coords.end(), index.m_coords.begin(), index.m_coords.end());
      if (withinDataRange(coords)) {
        size_t sourceOffset = getIndex(coords) * getBytePerElement();
        memcpy(array->getDataPointer(targetOffset),
               this->getDataPointer(sourceOffset), nbyteToCopy);
      }
      targetOffset += dims[0] * getBytePerElement();
    }
  }

  return array;
}

std::vector<int> ZArray::getDimVector() const
{
  std::vector<mylib::Dimn_Type> dims(getRank());
  for (int i = 0; i < getRank(); ++i) {
    dims[i] = getDim(i);
  }

  return dims;
}

void ZArray::forEachCoordinates(
    std::function<void (const std::vector<int> &)> f) const
{
  MIndex index(m_startCoordinates, getDimVector());
  while (index.next()) {
    f(index.m_coords);
  }
}
