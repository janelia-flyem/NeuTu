#ifndef ZCUBOID_H
#define ZCUBOID_H

#include <vector>
#include "zpoint.h"

class ZIntPoint;
class ZIntCuboid;
class ZLineSegment;

/*************
 *
 *        4________5       z /
 *       /|        /|       /
 *    0 /_______1 / |       x--->
 *     |  .      |  |     y |
 *     |  |      |  |       v
 *     | 6._ . _ |_ |7
 *     | /       | /
 *     |_________|/
 *    2          3
 *
 * The order is determined by binary increment: 000 (zyx)->111, where 0 and 1
 * indicate min and max repectively.
 ************/

class ZCuboid
{
public:
  ZCuboid();
  ZCuboid(double x1, double y1, double z1, double x2, double y2, double z2);
  ZCuboid(const ZCuboid &cuboid);

  void set(double x1, double y1, double z1, double x2, double y2, double z2);
  void set(const ZPoint &firstCorner, const ZPoint &lastCorner);
  void set(const ZIntPoint &firstCorner, const ZIntPoint &lastCorner);
  void set(const double *corner);

  void setFirstCorner(const ZPoint &pt);
  void setFirstCorner(double x, double y, double z);

  void setLastCorner(const ZPoint &pt);
  void setLastCorner(double x, double y, double z);

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
  void expand(double margin);

  void joinX(double x);
  void joinY(double y);
  void joinZ(double z);
  /*!
   * \brief Expand the box minimally to include a point.
   */
  void include(const ZPoint &point);

  void print();

  ZPoint corner(int index) const;
  ZPoint center() const;

  inline const ZPoint& firstCorner() const { return m_firstCorner; }
  inline const ZPoint& lastCorner() const { return m_lastCorner; }

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
  double getDiagonalLength() const;

  std::vector<double> toCornerVector() const;

private:
  static double computeDistance(double minX1, double maxX1,
                                double minX2, double maxX2);

private:
  ZPoint m_firstCorner;
  ZPoint m_lastCorner;
};

#endif // ZCUBOID_H
