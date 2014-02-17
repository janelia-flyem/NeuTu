#include "zeigensolver.h"
#include <cmath>

ZEigenSolver::ZEigenSolver()
{
}

/*
bool ZEigenSolver::solve(const ZSymmetricMatrix &matrix)
{
  return false;
}
*/

bool ZEigenSolver::solveCovEigen(const std::vector<double> &cov)
{
  if (cov.size() == 3) {
    m_eigenValue.resize(2);
    double p = cov[0] + cov[1];
    double q =
        sqrt((cov[0] - cov[1]) * (cov[0] - cov[1]) + 4.0 * cov[2] * cov[2]);
    m_eigenValue[0] = (p + q) * 0.5;
    m_eigenValue[1] = (p - q) * 0.5;

    return true;
  }

  return false;
}

double ZEigenSolver::getEigenValue(size_t index)
{
  if (index < m_eigenValue.size()) {
    return m_eigenValue[index];
  }

  return 0.0;
}
