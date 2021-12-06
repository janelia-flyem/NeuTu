#ifndef ZINTCUBOID_H
#define ZINTCUBOID_H

#include <iostream>

#include "zintpoint.h"
#include "tz_cuboid_i.h"
#include "common/neutudefs.h"

class ZJsonArray;
class ZPoint;

/*!
 * \brief The class of cuboid with integer coordinates
 *
 * Note: Any operation on an empty cuboid is undefined, except those that may
 * turn the object into a non-empty one.
 *
 */
class ZIntCuboid
{
public:
  /*!
   * \brief Default constructor
   *
   * Construct an empty cuboid
   */
  ZIntCuboid();

  ZIntCuboid(int x1, int y1, int z1, int x2, int y2, int z2);
  ZIntCuboid(const ZIntPoint &firstCorner, const ZIntPoint &lastCorner);
  ZIntCuboid(const Cuboid_I &cuboid);

  /*!
   * \brief Check if a cuboid is empty.
   *
   * A cuboid is empty if any of its dimension <= 0.
   */
  bool isEmpty() const;

  inline const ZIntPoint& getMinCorner() const { return m_minCorner; }
  inline const ZIntPoint& getMaxCorner() const { return m_maxCorner; }
  ZIntPoint getCorner(int index) const;

  int getWidth() const;
  int getHeight() const;
  int getDepth() const;
  ZIntPoint getSize() const;

  double getDiagonalLength() const;
  double getMinSideLength() const;

  /*!
   * \brief Get the volume of the cuboid.
   *
   * It returns 0 if the cuboid is invalid.
   */
  size_t getVolume() const;
  size_t getDsMaxVolume(int xIntv, int yIntv, int zIntv) const;


  inline void setMinCorner(const ZIntPoint &corner) {
    m_minCorner = corner;
  }

  inline void setMinCorner(int x, int y, int z) {
    m_minCorner.set(x, y, z);
  }

  inline void setMaxCorner(const ZIntPoint &corner) {
    m_maxCorner = corner;
  }

  inline void setMaxCorner(int x, int y, int z) {
    m_maxCorner.set(x, y, z);
  }

  /*!
   * \brief Reset to the default constructing state.
   */
  void reset();

  void set(int x1, int y1, int z1, int x2, int y2, int z2);
  void set(const ZIntPoint &firstCorner, const ZIntPoint &lastCorner);
  //void set(const int *corner);

  int getMinX() const;
  int getMaxX() const;
  int getMinY() const;
  int getMaxY() const;
  int getMinZ() const;
  int getMaxZ() const;

  void setMinX(int x);
  void setMaxX(int x);
  void setMinY(int y);
  void setMaxY(int y);
  void setMinZ(int z);
  void setMaxZ(int z);

  void translateX(int dx);

  void translate(const ZIntPoint &offset);
  void translate(int dx, int dy, int dz);

  /*!
   * \brief Scale the box
   *
   * The min corner and the size of the box will be scaled by \a s. This is
   * useful for upsampling a downsampled object.
   */
  void scaleUp(const ZIntPoint &s);
  void scaleUp(int s);

  /*!
   * \brief Downsize the box
   *
   * The size of the box will be scaled by \a 1 / s. Nothing will be done if any
   * of the coordinate in \a s is non-positive.
   */
  void scaleDown(const ZIntPoint &s);
  void scaleDown(int s);

  /*!
   * \brief Downsize the box in a way of filling blocks.
   *
   * When the box is scaled back up in the same ratio after being scaled down,
   * it should cover the original box.
   */
  void scaleDownBlock(int s);
  void scaleDownBlock(const ZIntPoint &s);

  /*!
   * \brief Change the size of the cuboid by fixing the first corner
   */
  void setSize(int width, int height, int depth);
  void setSize(const ZIntPoint &size);

  void setWidth(int width);
  void setHeight(int height);

  /*!
   * \brief Set the depth
   *
   * It change the depth of the cuboid with the min corner fixed.
   *
   * \param depth New depth of the cuboid.
   */
  void setDepth(int depth);

  void setDepth(int depth, neutu::ERangeReference ref);


  /*!
   * \brief Join two cuboids
   *
   * Note that if the object is empty, the result will become the same as \a cuboid.
   * If \a cuboid is empty, nothing will be done.
   *
   * \return The current object after joining.
   */
  ZIntCuboid& join(const ZIntCuboid &cuboid);

  void join(int x, int y, int z);
  void join(const ZIntPoint &pt);

  void joinX(int x);
  void joinY(int y);
  void joinZ(int z);

  /*!
   * \brief Expand the cuboid to make sure it includes a point in float coordinates
   */
  void join(const ZPoint &pt);

  /*!
   * \brief Expand a box with a given margin.
   *
   * It expands both sides.
   */
  void expandX(int dx);
  void expandY(int dy);
  void expandZ(int dz);
  void expand(int dx, int dy, int dz);

  //intersect
  ZIntCuboid& intersect(const ZIntCuboid &cuboid);

  /*!
   * \brief Check if a point is in the cuboid.
   */
  bool contains(int x, int y, int z) const;
  bool contains(const ZIntPoint &pt) const;
  bool contains(const ZIntCuboid &box) const;

  bool containsYZ(int y, int z) const;


  bool equals(const ZIntCuboid &cuboid) const;

  bool hasOverlap(const ZIntCuboid &box) const;

  //double distanceTo(const ZIntPoint &pt);

  int computeBlockDistance(const ZIntCuboid &box);
  double computeDistance(const ZIntCuboid &box);

  void shiftSliceAxis(neutu::EAxis axis);
  void shiftSliceAxisInverse(neutu::EAxis axis);
  int getDim(neutu::EAxis axis) const;

  ZIntPoint getCenter() const;
  void setCenter(const ZIntPoint &center);

  ZPoint getExactCenter() const;

  /*!
   * \brief Downscale the cuboid.
   *
   * The coordinates of the corners are divided by (sx, sy, sz).
   * The cuboid stays the same if any scale is less than 1.
   */
  void downScale(int sx, int sy, int sz);
  void downScale(int s);

  /*!
   * \brief Turn the cuboid into a JSON array
   *
   * \return [x1, y1, z1, x2, y2, z2]
   */
  ZJsonArray toJsonArray() const;

  /*!
   * \brief Set corners from a json array
   *
   * Array: [x1, y1, z1, x2, y2, z2]. The object is reset to default if the json
   * array is invalid.
   */
  void loadJson(const ZJsonArray &json);

  std::string toString() const;

  bool operator == (const ZIntCuboid &box) const;
  bool operator != (const ZIntCuboid &box) const;

  friend ZIntCuboid operator + (const ZIntCuboid &box, const ZIntPoint &pt);
  friend ZIntCuboid operator - (const ZIntCuboid &box, const ZIntPoint &pt);
  friend ZIntCuboid operator * (const ZIntCuboid &box, const ZIntPoint &pt);

  friend std::ostream& operator<< (std::ostream &stream, const ZIntCuboid &box);

  static ZIntCuboid Empty();

private:
  ZIntPoint m_minCorner;
  ZIntPoint m_maxCorner;
};

#endif // ZINTCUBOID_H
