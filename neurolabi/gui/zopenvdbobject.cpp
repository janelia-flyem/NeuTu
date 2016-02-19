#include "zopenvdbobject.h"
#include <iostream>

#if defined(_USE_OPENVDB_)

ZOpenVdbObject::ZOpenVdbObject()
{
  openvdb::initialize();
  m_grid = openvdb::Int32Grid::create();
}

void ZOpenVdbObject::setValue(int x, int y, int z, int value)
{
  openvdb::Coord coord(x, y, z);

  m_grid->getAccessor().setValue(coord, value);
}

int ZOpenVdbObject::getValue(int x, int y, int z) const
{
  openvdb::Coord coord(x, y, z);

  return m_grid->getConstAccessor().getValue(coord);
}

void ZOpenVdbObject::repack()
{
  if (!isEmpty()) {
    m_grid->tree().prune();
  }
}

bool ZOpenVdbObject::isEmpty() const
{
  if (m_grid == NULL) {
    return true;
  }

  return m_grid->empty();
}

void ZOpenVdbObject::print() const
{
  if (m_grid == NULL) {
    std::cout << "Empty vdb object" << std::endl;
  } else {
    std::cout << m_grid->tree().activeVoxelCount() << " voxels" << std::endl;
    openvdb::CoordBBox box = m_grid->evalActiveVoxelBoundingBox();
    std::cout << "Bounding box: " << box << std::endl;
#ifdef _DEBUG_
    tic();
    openvdb::Coord firstCorner = box.min();
    openvdb::Coord lastCorner = box.max();
    int z = firstCorner[2];
    openvdb::Int32Grid::ConstAccessor accessor = m_grid->getConstAccessor();
    for (int y = 0; y <= 10000; ++y) {
      for (int x = 0; x <= 10000; ++x) {
        accessor.getValue(openvdb::Coord(x, y, z));
      }
    }
    ptoc();
#endif

  }
}

#endif
