#include "zarrayfactory.h"

#include "zstack.hxx"
#include "geometry/zintcuboid.h"

ZArrayFactory::ZArrayFactory()
{
}

ZArray* ZArrayFactory::MakeArray(const ZStack *stack)
{
  ZArray *array = NULL;
  if (stack != NULL) {
    int ndims = 3;
    mylib::Dimn_Type dims[3];
    dims[0] = stack->width();
    dims[1] = stack->height();
    dims[2] = stack->depth();

    array = new ZArray(mylib::UINT64_TYPE, ndims, dims);
    array->setStartCoordinate(0, stack->getOffset().getX());
    array->setStartCoordinate(1, stack->getOffset().getY());
    array->setStartCoordinate(2, stack->getOffset().getZ());

    uint64_t *labelArray = array->getDataPointer<uint64_t>();
    size_t voxelNumber = stack->getVoxelNumber();
    for (size_t i = 0; i < voxelNumber; ++i) {
      labelArray[i] = stack->getIntValue(i);
    }
  }

  return array;
}

ZArray* ZArrayFactory::MakeArray(const ZIntCuboid box, mylib::Value_Type type)
{
  ZArray *array = NULL;
  if (!box.isEmpty()) {
    int ndims = 3;
    mylib::Dimn_Type dims[3];
    dims[0] = box.getWidth();
    dims[1] = box.getHeight();
    dims[2] = box.getDepth();

    array = new ZArray(type, ndims, dims);
    array->setStartCoordinate(0, box.getMinCorner().getX());
    array->setStartCoordinate(1, box.getMinCorner().getY());
    array->setStartCoordinate(2, box.getMinCorner().getZ());
  }

  return array;
}
