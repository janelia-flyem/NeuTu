#ifndef ZCUBOID_H
#define ZCUBOID_H

#include <vector>
#include "zpoint.h"

class ZIntPoint;
class ZIntCuboid;
class ZLineSegment;

/*************
 *
 *        4________5       z / front
 *       /|        /|       /
 *    0 /_______1 / |       -x--> right
 *     |  .      |  |     y |
 *     |  |      |  |       v up
 *     | 6._ . _ |_ |7
 *     | /       | /
 *     |_________|/
 *    2          3
 *
 * The order is determined by binary increment: 000 (zyx)->111, where 0 and 1
 * indicate min and max repectively.
 *
 * Face indexing: left 0, right 1, down 2, up 3, back 4, front 5
 ************/

class ZCuboid
{
public:
  ZCuboid();
  ZCuboid(double x1, double y1, double z1, double x2, double y2, double z2);
  ZCuboid(const ZPoint &minCorner, const ZPoint &maxCorner);
  ZCuboid(const ZCuboid &cuboid);

  void set(double x1, double y1, double z1, double x2, double y2, double z2);
  void set(const ZPoint &minCorner, const ZPoint &maxCorner);
  void set(const ZIntPoint &minCorner, const ZIntPoint &maxCorner);
  void set(const ZIntCuboid &cuboid);
  void set(const double *getCorner);

  void setMinCorner(const ZPoint &pt);
  void setMinCorner(double x, double y, double z);

  void setMaxCorner(const ZPoint &pt);
  void setMaxCorner(double x, double y, double z);

  void setSize(double width, double height, double depth);

  /*!
   * \brief Test if the bound box is valid
   *
   * \return true iff the locations of corners conform to the definition.
   */
  bool isValid() const;

  /*!
   * \brief Make the cuboid invalid
   */
  void invalidate();

  double width() const;
  double height() const;
  double depth() const;
  double volume() const;
  void intersect(const ZCuboid &cuboid);
  void bind(const ZCuboid &cuboid); //union

  double moveOutFrom(ZCuboid &cuboid, double margin = 0.0);

  void layout(std::vector<ZCuboid> *cuboidArray, double margin = 0.0);

  ZCuboid& operator= (const ZCuboid &cuboid);
  double& operator[] (int index);
  const double& operator[] (int index) const;

  double estimateSeparateScale(const ZCuboid &cuboid, const ZPoint &vec) const;

  void scale(double s);
  void scale(double sx, double sy, double sz);
  void expand(double margin);

  void joinX(double x);
  void joinY(double y);
  void joinZ(double z);

  void join(double x, double y, double z);
  void join(const ZPoint &pt);
  void join(const ZCuboid &box);
  void join(const ZIntCuboid &box);

  /*!
   * \brief Expand the box minimally to include a point.
   */
  void include(const ZPoint &point);

  void print();

  ZPoint getCorner(int index) const;
  ZPoint getCenter() const;

  inline const ZPoint& getMinCorner() const { return m_minCorner; }
  inline const ZPoint& getMaxCorner() const { return m_maxCorner; }

  ZPoint& getMinCorner() { return m_minCorner; }
  ZPoint& getMaxCorner() { return m_maxCorner; }

  double computeDistance(const ZCuboid &box) const;

  void translate(const ZPoint &pt);

  /*!
   * \brief Compute the intersection between a line and a cuboid
   *
   * \a slope defines the slope of the line and it does not have to be
   * normalized. \a p0 is a point on the line. The result is stored in \a seg
   * if it is not NULL. It returns false if the slope is 0.
   *
   * \return true if the line intersects with the cuboid.
   */
  bool intersectLine(
      const ZPoint &p0, const ZPoint &slope, ZLineSegment *seg) const;

  ZIntCuboid toIntCuboid() const;
  static ZCuboid FromIntCuboid(const ZIntCuboid &cuboid);
  double getDiagonalLength() const;
  int getMinSideLength() const;

  std::vector<double> toCornerVector() const;

  bool contains(const ZPoint &pt) const;
  bool contains(const ZCuboid &box) const;

  /*!
   * \brief Make a cuboid from two corners
   *
   * It assumes that \a c1 and \a c2 are two diagonal corners without a specific
   * order.
   */
  static ZCuboid MakeFromCorner(const ZPoint &c1, const ZPoint &c2);

private:
  static double computeDistance(double minX1, double maxX1,
                                double minX2, double maxX2);

private:
  ZPoint m_minCorner;
  ZPoint m_maxCorner;
};

#endif // ZCUBOID_H
