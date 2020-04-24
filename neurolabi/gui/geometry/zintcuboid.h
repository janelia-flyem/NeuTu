#ifndef ZINTCUBOID_H
#define ZINTCUBOID_H

#include "zintpoint.h"
#include "tz_cuboid_i.h"
#include "common/neutudefs.h"

class ZJsonArray;

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

  //ZIntCuboid(const ZIntCuboid &cuboid);

  inline const ZIntPoint& getMinCorner() const { return m_minCorner; }
  inline const ZIntPoint& getMaxCorner() const { return m_maxCorner; }

  inline void setMinCorner(const ZIntPoint &corner) {
    m_minCorner = corner;
  }

  inline void setMinCorner(int x, int y, int z) {
    m_minCorner.set(x, y, z);
  }

  ZIntPoint getCorner(int index) const;

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
  /*!
   * \brief Scale the box
   *
   * The size of the box will be scaled by \a s.
   */
  void scale(const ZIntPoint &s);
  void scale(int s);

  /*!
   * \brief Downsize the box
   *
   * The size of the box will be scaled by \a 1 / s. Nothing will be done if any
   * of the coordinate in \a s is non-positive.
   */
  void scaleDown(const ZIntPoint &s);
  void scaleDown(int s);

  /*!
   * \brief Downsize the box in a way of filling blocks
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

  int getWidth() const;
  int getHeight() const;
  int getDepth() const;
  ZIntPoint getSize() const;

  double getDiagonalLength() const;

  /*!
   * \brief Set the depth
   *
   * It change the depth of the cuboid with the first corner fixed.
   *
   * \param depth New depth of the cuboid.
   */
  void setDepth(int depth);


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

  void joinX(int x);
  void joinY(int y);
  void joinZ(int z);

  void expandX(int dx);
  void expandY(int dy);
  void expandZ(int dz);
  void expand(int dx, int dy, int dz);

  //intersect
  ZIntCuboid& intersect(const ZIntCuboid &cuboid);

  /*!
   * \brief Get the volume of the cuboid.
   *
   * It returns 0 if the cuboid is invalid.
   */
  size_t getVolume() const;
  size_t getDsMaxVolume(int xIntv, int yIntv, int zIntv) const;

  /*!
   * \brief Check if a point is in the cuboid.
   */
  bool contains(int x, int y, int z) const;
  bool contains(const ZIntPoint &pt) const;
  bool contains(const ZIntCuboid &box) const;

  bool containYZ(int y, int z) const;

  /*!
   * \brief Check if a cuboid is empty.
   *
   * A cuboid is empty if any of its dimension <= 0.
   */
  bool isEmpty() const;

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

private:
  ZIntPoint m_minCorner;
  ZIntPoint m_maxCorner;
};

#endif // ZINTCUBOID_H
