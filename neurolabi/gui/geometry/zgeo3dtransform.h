#ifndef ZGEO3DTRANSFORM_H
#define ZGEO3DTRANSFORM_H

#include <vector>

class ZGeo3dTransform
{
public:
  ZGeo3dTransform();

public:
  /*!
   * \brief Transform n points in place
   *
   * \param ptArray (x, y, z, ...)
   * \param n Number of points
   */
  void transform(double *ptArray, std::size_t n) const;

private:
  std::vector<double> m_matrix;
};

#endif // ZGEO3DTRANSFORM_H
