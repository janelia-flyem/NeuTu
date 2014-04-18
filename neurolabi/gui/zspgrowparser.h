#ifndef ZSPGROWPARSER_H
#define ZSPGROWPARSER_H

#include <vector>
#include "tz_sp_grow.h"

#include "zvoxel.h"
#include "zvoxelarray.h"

class ZSpGrowParser
{
public:
  ZSpGrowParser();
  ZSpGrowParser(Sp_Grow_Workspace *workspace);
  ~ZSpGrowParser();

public:
  size_t voxelNumber();
  Stack* createDistanceStack();

  /*!
   * \brief Create the stack for euclidean distance
   *
   * It returns NULL if the distance is not available
   */
  Stack* createEuclideanDistanceStack();

  ZVoxelArray extractPath(ssize_t index);
  ZVoxelArray extractLongestPath(double *length, bool masked);
  int pathSize(ssize_t index);
  double pathLength(ssize_t index, bool masked);
  std::vector<ZVoxelArray> extractAllPath(double lengthThreshold,
                                          Stack *ballStack = NULL);

private:
  Sp_Grow_Workspace *m_workspace;
  Stack *m_regionMask;
  Stack *m_checkedMask;
  Stack *m_pathMask;
  std::vector<size_t> m_fgArray;

};

#endif // ZSPGROWPARSER_H
