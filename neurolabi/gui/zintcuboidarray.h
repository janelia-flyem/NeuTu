#ifndef ZINTCUBOIDARRAY_H
#define ZINTCUBOIDARRAY_H

#include <vector>
#include <string>
#include <map>
#include "tz_cuboid_i.h"
#include "zintcuboidface.h"

class ZJsonObject;
class ZSwcTree;

/*!
 * \brief The class of cuboid arrays
 *
 * The class assumes that there is not overlap between any two cuboids.
 */
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

  /*!
   * \brief Append a cuboid
   *
   * Append cuboid starting at (\a x, \a y, \a z) with size
   * (\a width x \a height x \a depth).
   */
  void append(int x, int y, int z, int width, int height, int depth);

  /*!
   * \brief Test a point is contained by which cuboid.
   *
   * \return The index of the hit box. -1 if no hit occurs.
   */
  int hitTest(double x, double y, double z) const;
  int hitTest(const ZPoint &pt) const;

  /*!
   * \brief Test if a point hits the internal region of a cuboid
   *
   * \return The index of the hit box. -1 if no hit occurs.
   */
  int hitInternalTest(double x, double y, double z) const;

  std::vector<int> loadSubstackList(const std::string filePath);
  void translate(int x, int y, int z);
  void rescale(double factor);

  void exportSwc(const std::string &filePath) const;
  ZSwcTree* toSwc() const;

  void removeInvalidCuboid();

  static bool isInvalid(const Cuboid_I &cuboid);

  void intersect(const Cuboid_I &cuboid);

  Cuboid_I getBoundBox() const;

  ZIntCuboidArray getFace() const;
  ZIntCuboidArray getInnerFace() const;

  void print() const;

  size_t getVolume() const;

  ZIntCuboidFaceArray getBorderFace() const;
  ZIntCuboidFaceArray getSideBorderFace() const;

private:
  mutable ZIntCuboidFaceArray m_borderFace;
};

namespace FlyEm {
class SubstackRegionCalbration {
public:
  SubstackRegionCalbration();

  void setMargin(int x, int y, int z);
  void setBounding(bool x, bool y, bool z);
  void calibrate(ZIntCuboidArray &roi) const;
  bool importJsonObject(const ZJsonObject &obj);

private:
  int m_margin[3];
  bool m_bounding[3];
};

class ZIntCuboidCutter {
public:
  ZIntCuboidCutter();

  bool loadJsonObject(const ZJsonObject &obj);
  void cut(Cuboid_I *cuboid);

private:
  Cuboid_I m_cuboid;
};

/*!
 * \brief Class of substack ROI
 *
 * Presumed properties:
 *   All blocks have the same size
 *   Side faces are matched exactly
 *   No gap allowed
 *   The blocks are sorted by the ascending order (z, y, x)
 */
class ZSubstackRoi
{
public:
  void clear();

  Cuboid_I& getSubstack(int index) const;

  void importJsonFile(const std::string &filePath);

  Cuboid_I* getCuboidFromId(int id);

  void exportSwc(const std::string &filePath);

  ZIntCuboidArray& getCuboidArray() {
    return m_cuboidArray;
  }

private:
  std::vector<int> m_idArray;
  ZIntCuboidArray m_cuboidArray;

  static const char* m_blockFileKey;
  static const char* m_calbrationKey;
  static const char* m_cutterKey;
};
}
#endif // ZINTCUBOIDARRAY_H
