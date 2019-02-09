#ifndef ZLINESEGMENTARRAY_H
#define ZLINESEGMENTARRAY_H

#include <vector>
#include "zlinesegment.h"

/*!
 * \brief The ZLineSegmentArray class
 *
 * The array of line segments
 */
class ZLineSegmentArray : public std::vector<ZLineSegment>
{
public:
  ZLineSegmentArray();

  /*!
   * \brief Append a line segment.
   *
   * Add the line segment (\a v0, \a v1) to the end of the array.
   */
  void append(const ZPoint &v0, const ZPoint &v1);

  /*!
   * \brief Append a line segment array.
   */
  void append(const ZLineSegmentArray &lineArray);
};

#endif // ZLINESEGMENTARRAY_H
