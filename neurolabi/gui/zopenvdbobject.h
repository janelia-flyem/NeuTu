#ifndef ZOPENVDBOBJECT_H
#define ZOPENVDBOBJECT_H

#include "openvdb_header.h"
#include "zobject3d.h"
#include "zuncopyable.h"

#if defined(_USE_OPENVDB_)

/*!
 * \brief An experimental class of using OpenVdb
 */
class ZOpenVdbObject
{
public:
  ZOpenVdbObject();

  void setValue(int x, int y, int z, int value);
  int getValue(int x, int y, int z) const;

  /*!
   * \brief Improve sparse representation
   */
  void repack();

  void print() const;

  bool isEmpty() const;

private:
  openvdb::Int32Grid::Ptr m_grid;
};

#endif

#endif // ZOPENVDBOBJECT_H
