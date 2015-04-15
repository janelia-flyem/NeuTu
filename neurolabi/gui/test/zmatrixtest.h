#ifndef ZMATRIXTEST_H
#define ZMATRIXTEST_H

#include "ztestheader.h"
#include "zmatrix.h"
#include "zeigensolver.h"
#include "zvectorgenerator.h"

#ifdef _USE_GTEST_
TEST(ZMatrix, resize)
{
  ZMatrix mat(5, 3);

  for (int i = 0; i < 5; ++i) {
    for (int j = 0; j < 3; ++j) {
      mat.set(i, j, i * 10 + j);
    }
  }

  for (int i = 0; i < 5; ++i) {
    for (int j = 0; j < 3; ++j) {
      ASSERT_EQ(mat.at(i, j), i * 10 + j);
    }
  }

  mat.resize(5, 3);
  for (int i = 0; i < 5; ++i) {
    for (int j = 0; j < 3; ++j) {
      ASSERT_EQ(mat.at(i, j), i * 10 + j);
    }
  }

  mat.resize(6, 3);
  for (int i = 0; i < 5; ++i) {
    for (int j = 0; j < 3; ++j) {
//      std::cout << i << ", " << j << std::endl;
      ASSERT_EQ(i * 10 + j, mat.at(i, j));
    }
  }

  for (int j = 0; j < 3; ++j) {
    ASSERT_EQ(mat.at(5, j), 0);
  }

  mat.resize(5, 3);
  for (int i = 0; i < 5; ++i) {
    for (int j = 0; j < 3; ++j) {
//      std::cout << i << ", " << j << std::endl;
      ASSERT_EQ(i * 10 + j, mat.at(i, j));
    }
  }

  mat.resize(5, 4);
  for (int i = 0; i < 5; ++i) {
    for (int j = 0; j < 3; ++j) {
//      std::cout << i << ", " << j << std::endl;
      ASSERT_EQ(i * 10 + j, mat.at(i, j));
    }
    ASSERT_EQ(0, mat.at(i, 3));
  }

  mat.resize(6, 4);
  for (int i = 0; i < 5; ++i) {
    for (int j = 0; j < 3; ++j) {
//      std::cout << i << ", " << j << std::endl;
      ASSERT_EQ(i * 10 + j, mat.at(i, j));
    }
    ASSERT_EQ(0, mat.at(i, 3));
  }
  for (int j = 0; j < 3; ++j) {
    ASSERT_EQ(mat.at(5, j), 0);
  }

  mat.resize(2, 4);
  for (int i = 0; i < 2; ++i) {
    for (int j = 0; j < 3; ++j) {
//      std::cout << i << ", " << j << std::endl;
      ASSERT_EQ(i * 10 + j, mat.at(i, j));
    }
    ASSERT_EQ(0, mat.at(i, 3));
  }

  mat.resize(2, 1);
  for (int i = 0; i < 2; ++i) {
    for (int j = 0; j < 1; ++j) {
//      std::cout << i << ", " << j << std::endl;
      ASSERT_EQ(i * 10 + j, mat.at(i, j));
    }
  }

  mat.printInfo();
}

TEST(ZMatrix, copyValue)
{
  ZMatrix mat(5, 3);
  std::vector<double> array(10, 0);

  for (int i = 0; i < 15; ++i) {
    mat.set(i, i);
  }

  for (int i = 0; i < 15; ++i) {
    ASSERT_EQ(i, mat.getValue(i));
  }

  ASSERT_EQ(1, mat.copyRowValueTo(1, 2, 4, &(array[1])));
  ASSERT_EQ(0.0, array[0]);
  ASSERT_EQ(5.0, array[1]);
  ASSERT_EQ(0.0, array[2]);
  ASSERT_EQ(0.0, array[3]);
  ASSERT_EQ(0.0, array[4]);

  ASSERT_EQ(3, mat.copyRowValueTo(1, -1, 2, &(array[1])));
  ASSERT_EQ(0.0, array[0]);
  ASSERT_EQ(5.0, array[1]);
  ASSERT_EQ(3.0, array[2]);
  ASSERT_EQ(4.0, array[3]);
  ASSERT_EQ(5.0, array[4]);
  ASSERT_EQ(0.0, array[5]);
}

TEST(ZMatrix, RowColumn)
{
  ZMatrix mat(2, 3);
  mat.set(0, 0, 1);
  mat.set(0, 1, 2);

  ASSERT_EQ(2, mat.getRowMax(0));

  mat.set(1, 0, 3);
  mat.set(2, 0, 2);

  ASSERT_EQ(3, mat.getColumnMax(0));

  mat.set(1, 1, 4);
  mat.set(2, 1, 2);
  ASSERT_EQ(4, mat.getColumnMax(1));

  double dst[3];
  mat.copyRowValueTo(1, 0, 2, dst);
  ASSERT_EQ(4, dst[1]);

  dst[0] = 5;
  dst[1] = 6;
  dst[2] = 7;
  mat.copyColumnValueFrom(dst, 1, 1);

  ASSERT_EQ(5, mat.at(0, 1));
  ASSERT_EQ(6, mat.at(1, 1));

  mat.resize(4, 5);
  for (int i = 0; i < 4; ++i) {
    for (int j = 0; j < 5; ++j) {
      mat.set(i, j, i * 10 + j);
    }
  }

  ZMatrix mat2 = mat.makeRowSlice(1, 3);
  ASSERT_EQ(10, mat2.at(0, 0));
  ASSERT_EQ(11, mat2.at(0, 1));
  ASSERT_EQ(12, mat2.at(0, 2));
  ASSERT_EQ(3, mat2.getRowNumber());

  ZMatrix mat3 = mat2.makeColumnSlice(1, 2);
  ASSERT_EQ(11, mat3.at(0, 0));
  ASSERT_EQ(12, mat3.at(0, 1));

  ASSERT_EQ(3, mat3.getRowNumber());
  ASSERT_EQ(2, mat3.getColumnNumber());
}

TEST(ZEigenSolver, solve)
{
  ZEigenSolver solver;
  std::vector<double> cov =
      ZVectorGenerator<double>() << 4.2667 << 0.7667 << 1.4000;
  solver.solveCovEigen(cov);
  ASSERT_NEAR(4.75779, solver.getEigenValue(0), 0.0001);
  ASSERT_NEAR(0.275607, solver.getEigenValue(1), 0.0001);
}

#endif

#endif // ZMATRIXTEST_H
