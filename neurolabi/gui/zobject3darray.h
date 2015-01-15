#ifndef ZOBJECT3DARRAY_H
#define ZOBJECT3DARRAY_H

#include <vector>
#include <string>
#include "zobject3d.h"
#include "tz_object_3d_linked_list.h"

class ZStack;

class ZObject3dArray : public std::vector<ZObject3d*>
{
public:
  ZObject3dArray();
  ZObject3dArray(size_t s);
  ~ZObject3dArray();

public:
  void clearAll();

  //<objList> will be freed by the function
  void append(Object_3d_List *objList);

  //Split an object
  void append(const ZObject3d &obj);
  void append(const ZObject3d &obj, const std::vector<int> &labelArray);

  void getRange(int *corner);

  void labelStack(Stack *stack);
  Stack* toStack();
  ZStack* toStackObject();

  void setSourceSize(int width, int height, int depth);

  //Note that the source size must be set first to get desirable results
  void sortByIndex();

  void readIndex(std::string filePath, int width, int height, int depth,
                 int indexOffset = 0);
  void writeIndex(std::string filePath, int width, int height, int depth);

  void exportCsvFile(std::string filePath);

  double radiusVariance();
  double angleShift();
  ZPoint averageDirection();

  /*!
   * \brief Take an object.
   *
   * The array no longer owns the taken object after the function call.
   */
  ZObject3d* take(size_t index);

private:
  int m_width;
  int m_height;
  int m_depth;
};

#endif // ZOBJECT3DARRAY_H
