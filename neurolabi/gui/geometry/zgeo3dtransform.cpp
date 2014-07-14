#include "zgeo3dtransform.h"

ZGeo3dTransform::ZGeo3dTransform() : m_matrix(16, 0.0)
{
}

void ZGeo3dTransform::transform(double *ptArray, std::size_t n) const
{
  double result[3]; /* array to store temp result */
  for (std::size_t i = 0; i < n; i++) {
    result[0] = m_matrix[0] * ptArray[0] + m_matrix[1] * ptArray[1] +
      m_matrix[2] * ptArray[2] + m_matrix[3];
    result[1] = m_matrix[4] * ptArray[0] + m_matrix[5] * ptArray[1] +
      m_matrix[6] * ptArray[2] + m_matrix[7];
    result[2] = m_matrix[8] * ptArray[0] + m_matrix[9] * ptArray[1] +
      m_matrix[10] * ptArray[2] + m_matrix[11];

    ptArray[0] = result[0];
    ptArray[1] = result[1];
    ptArray[2] = result[2];

    ptArray += 3;
  }
}
