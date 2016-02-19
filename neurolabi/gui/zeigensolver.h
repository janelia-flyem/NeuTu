#ifndef ZEIGENSOLVER_H
#define ZEIGENSOLVER_H

#include "zsymmetricmatrix.h"

class ZEigenSolver
{
public:
  ZEigenSolver();

public:
  //bool solve(const ZSymmetricMatrix &matrix);
  bool solveCovEigen(const std::vector<double> &cov);
  double getEigenValue(size_t index);

private:
  std::vector<double> m_eigenValue;
};

#endif // ZEIGENSOLVER_H
