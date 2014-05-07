#ifndef ZINTCUBOIDARRAY_H
#define ZINTCUBOIDARRAY_H

#include <vector>
#include <tz_cuboid_i.h>
#include <string>
#include "zintcuboidface.h"

namespace FlyEm {
class ZIntCuboidArray : public std::vector<Cuboid_I>
{
public:
  ZIntCuboidArray();

public:
  enum EComponent {
    BORDER_FACE, ALL_COMPONENT
  };

  void deprecate(EComponent component) const;
  void deprecateDependent(EComponent component) const;
  bool isDeprecated(EComponent component) const;

  void append(int x, int y, int z, int width, int height, int depth);
  int hitTest(double x, double y, double z) const;

  /*!
   * \brief Test if a point hits the internal region of a cuboid
   *
   * \return The index of the hit box. -1 if no hit occurs.
   */
  int hitInternalTest(double x, double y, double z) const;

  void loadSubstackList(const std::string filePath);
  void translate(int x, int y, int z);
  void rescale(double factor);

  void exportSwc(const std::string &filePath) const;

  void removeInvalidCuboid();

  static bool isInvalid(const Cuboid_I &cuboid);

  void intersect(const Cuboid_I &cuboid);

  Cuboid_I getBoundBox() const;

  ZIntCuboidArray getFace() const;
  ZIntCuboidArray getInnerFace() const;

  void print() const;

  size_t getVolume() const;

  ZIntCuboidFaceArray getBorderFace() const;

private:
  mutable ZIntCuboidFaceArray m_borderFace;
};
}

#endif // ZINTCUBOIDARRAY_H
