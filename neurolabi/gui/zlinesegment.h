#ifndef ZLINESEGMENT_H
#define ZLINESEGMENT_H

#include "zpoint.h"

/*!
 * \brief The class of line segment
 *
 * ZLineSegment represents a 3D line segment which has two end points. Different
 * than usual traditional geometrical definition, the class also specifies the
 * direction of the line segment by defining a start point and an end point.
 */
class ZLineSegment
{
public:
  /*!
   * \brief Constructor.
   *
   * The default start and end points are both set to (0, 0, 0)
   */
  ZLineSegment();

  /*!
   * \brief Constructor.
   *
   * Set the line segment to (\a x0, \a y0, \a z0) -> (\a x1, \a y1, \a z1)
   */
  ZLineSegment(double x0, double y0, double z0,
               double x1, double y1, double z1);

  /*!
   * \brief Constructor
   *
   * Set the line segment to (\a v0, \a v1).
   */
  ZLineSegment(const ZPoint &v0, const ZPoint &v1);

  /*!
   * \brief Set start point.
   */
  void setStartPoint(double x, double y, double z);
  void setStartPoint(const ZPoint &pt);

  /*!
   * \brief Set end point.
   */
  void setEndPoint(double x, double y, double z);
  void setEndPoint(const ZPoint &pt);

  void set(const ZPoint &start, const ZPoint &end);

  inline const ZPoint& getStartPoint() const { return m_start; }
  inline const ZPoint& getEndPoint() const { return m_end; }

  /*!
   * \brief Get the interpolation point of a segment.
   *
   * \param ds The distance to the start point.
   */
  ZPoint getInterpolation(double ds) const;

  /*!
   * \brief Get the length of the line segment
   */
  double getLength() const;

  /*!
   * \brief Invert the line segment.
   *
   * It swaps the start and end points.
   */
  void invert();

  ZPoint getVector() const;
  ZPoint getDirection() const;

  double getLowerX() const;
  double getUpperX() const;
  double getLowerY() const;
  double getUpperY() const;
  double getLowerZ() const;
  double getUpperZ() const;

  void print() const;

  void shiftSliceAxis(NeuTube::EAxis axis);

private:
  ZPoint m_start;
  ZPoint m_end;
};

#endif // ZLINESEGMENT_H
