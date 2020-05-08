#include "zlinesegment.h"

#include <algorithm>
#include <utility>

ZLineSegment::ZLineSegment()
{
}

ZLineSegment::ZLineSegment(
    double x0, double y0, double z0, double x1, double y1, double z1)
{
  m_start.set(x0, y0, z0);
  m_end.set(x1, y1, z1);
}

ZLineSegment::ZLineSegment(const ZPoint &v0, const ZPoint &v1)
{
  m_start = v0;
  m_end = v1;
}

void ZLineSegment::setStartPoint(double x, double y, double z)
{
  m_start.set(x, y, z);
}

void ZLineSegment::setStartPoint(const ZPoint &pt)
{
  m_start = pt;
}

void ZLineSegment::setEndPoint(double x, double y, double z)
{
  m_end.set(x, y, z);
}

void ZLineSegment::setEndPoint(const ZPoint &pt)
{
  m_end = pt;
}

void ZLineSegment::set(const ZPoint &start, const ZPoint &end)
{
  setStartPoint(start);
  setEndPoint(end);
}

void ZLineSegment::print() const
{
  std::cout << "Line segment: " << m_start.toString() << " --> "
            << m_end.toString() << std::endl;
}

double ZLineSegment::getLength() const
{
  return m_start.distanceTo(m_end);
}

void ZLineSegment::invert()
{
  std::swap(m_start, m_end);
//  ZPoint tmp;
//  SWAP2(m_start, m_end, tmp);
}

ZPoint ZLineSegment::getDirection() const
{
  ZPoint direction = m_end - m_start;
  direction.normalize();
  return direction;
}

ZPoint ZLineSegment::getVector() const
{
  return m_end - m_start;
}

ZPoint ZLineSegment::getInterpolation(double ds) const
{
  double length = getLength();
  if (length == 0.0) {
    return m_start;
  }

  return m_start + getDirection() * ds;
}

ZPoint ZLineSegment::getIntercept(double lambda) const
{
  return m_start * (1.0 - lambda) + m_end * lambda;
}

void ZLineSegment::shiftSliceAxis(neutu::EAxis axis)
{
  m_start.shiftSliceAxis(axis);
  m_end.shiftSliceAxis(axis);
}

bool ZLineSegment::isValid() const
{
  return m_start != m_end;
}

double ZLineSegment::getLowerX() const
{
  return std::min(getStartPoint().x(), getEndPoint().x());
}

double ZLineSegment::getUpperX() const
{
  return std::max(getStartPoint().x(), getEndPoint().x());
}

double ZLineSegment::getLowerY() const
{
  return std::min(getStartPoint().y(), getEndPoint().y());
}

double ZLineSegment::getUpperY() const
{
  return std::max(getStartPoint().y(), getEndPoint().y());
}

double ZLineSegment::getLowerZ() const
{
  return std::min(getStartPoint().z(), getEndPoint().z());
}

double ZLineSegment::getUpperZ() const
{
  return std::max(getStartPoint().z(), getEndPoint().z());
}

void ZLineSegment::flip()
{
  std::swap(m_start, m_end);
}

ZLineSegment ZLineSegment::flipped() const
{
  ZLineSegment seg = *this;
  seg.flip();
  return seg;
}

bool ZLineSegment::approxEquals(const ZLineSegment &seg) const
{
  if (m_start.approxEquals(seg.m_start) && m_end.approxEquals(seg.m_end)) {
    return true;
  }

  ZLineSegment flipped = seg.flipped();

  return m_start.approxEquals(flipped.m_start) &&
      m_end.approxEquals(flipped.m_end);
}

bool ZLineSegment::operator==(const ZLineSegment &seg) const
{
  return (m_start == seg.m_start) && (m_end == seg.m_end);
}

ZLineSegment& ZLineSegment::operator -= (const ZPoint &pt)
{
  m_start -= pt;
  m_end -= pt;

  return *this;
}

ZLineSegment& ZLineSegment::operator += (const ZPoint &pt)
{
  m_start += pt;
  m_end += pt;

  return *this;
}

ZLineSegment operator+ (const ZLineSegment &seg, const ZPoint &pt)
{
  return ZLineSegment(seg.getStartPoint() + pt, seg.getEndPoint() + pt);
}

ZLineSegment operator- (const ZLineSegment &seg, const ZPoint &pt)
{
  return  ZLineSegment(seg.getStartPoint() - pt, seg.getEndPoint() - pt);
}


std::ostream& operator <<(std::ostream &stream, const ZLineSegment &seg)
{
  stream << "[" << seg.getStartPoint() << ", " << seg.getEndPoint() << "]";

  return stream;
}
