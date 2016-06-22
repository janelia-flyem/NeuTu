#include "zspgrowparser.h"

#include <iostream>

#include "assert.h"
#include "tz_stack_lib.h"
#include "zpoint.h"
#include "tz_stack_utils.h"
#include "tz_error.h"

using namespace std;

ZSpGrowParser::ZSpGrowParser()
{
  m_workspace = NULL;
  m_regionMask = NULL;
  m_checkedMask = NULL;
  m_pathMask = NULL;
}

ZSpGrowParser::ZSpGrowParser(Sp_Grow_Workspace *workspace)
{
  m_workspace = workspace;
  m_regionMask = NULL;
  m_checkedMask = NULL;
  m_pathMask = NULL;
}

ZSpGrowParser::~ZSpGrowParser()
{
  Kill_Sp_Grow_Workspace(m_workspace);
  if (m_regionMask != NULL) {
    Kill_Stack(m_regionMask);
  }
  if (m_checkedMask != NULL) {
    Kill_Stack(m_checkedMask);
  }
  if (m_pathMask != NULL) {
    Kill_Stack(m_pathMask);
  }
}

ZVoxelArray ZSpGrowParser::extractPath(ssize_t index)
{
  ZVoxelArray path;
  int width = m_workspace->width;
  int height = m_workspace->height;

  if (m_workspace != NULL) {
    while (index >= 0) {
      ZVoxel voxel;
      voxel.setFromIndex(index, width, height);
      path.append(voxel);

      if (m_pathMask != NULL) {
        if (m_pathMask->array[index] == 1) {
          break;
        }
      }

      assert(index != m_workspace->path[index]);
      index = m_workspace->path[index];
    }
  }

  return path;
}

int ZSpGrowParser::pathSize(ssize_t index)
{
  int count = 0;
  while (index >= 0) {
    count++;
    if (m_workspace->mask[index] == SP_GROW_SOURCE) {
      break;
    }
    if (m_checkedMask != NULL) {
      if (m_checkedMask->array[index] == 1) {
        break;
      }
    }
    index = m_workspace->path[index];
  }

  return count;
}

double index_distance(ssize_t index1, ssize_t index2, int width, int area)
{
  int indexDiff = abs(index2 - index1);

  if (indexDiff == 1 || indexDiff == width || indexDiff == area) {
    return 1.0;
  } else if (indexDiff == width + 1 || indexDiff == width - 1 ||
             indexDiff == area + 1 || indexDiff == area - 1 ||
             indexDiff == area + width || indexDiff == area - width) {
    return 1.41421356237;
  } else if (indexDiff == area + width + 1 || indexDiff == area + width - 1 ||
             indexDiff == area - width - 1 || indexDiff == area -width + 1) {
    return 1.73205080757;
  } else {
    TZ_ERROR(ERROR_DATA_VALUE);
  }

  return 0.0;
}

double ZSpGrowParser::pathLength(ssize_t index, bool masked)
{
  double len = 0.0;

  if (m_workspace->length != NULL) {
    ssize_t originalIndex = index;
    if (masked) {
      if (m_checkedMask != NULL) {
        while (index >= 0) {
          if (m_checkedMask->array[index] == 1) {
            break;
          }

          index = m_workspace->path[index];
        }
      } else {
        index = -1;
      }
    } else {
      index = -1;
    }

    len = m_workspace->length[originalIndex];
    if (index >= 0) {
      len -= m_workspace->length[index];
    } else {
      len += 1.0;
    }
  } else {
    ssize_t secondIndex;

    //ZPoint firstPoint;
    //ZPoint secondPoint;

    if (index >= 0) {
      if (m_workspace->mask[index] == SP_GROW_SOURCE) {
        return 0.0;
      }

      len = 1.0;
      /*
    int x, y, z;
    Stack_Util_Coord(index, m_workspace->width, m_workspace->height,
                     &x, &y, &z);
    secondPoint.set(x, y, z);
    */
      secondIndex = index;
      index = m_workspace->path[index];
    }

    while (index >= 0) {
      /*
    if (m_workspace->mask[index] == SP_GROW_SOURCE) {
      break;
    }
    */
      if (m_checkedMask != NULL) {
        if (m_checkedMask->array[index] == 1) {
          break;
        }
      }

      /*
    firstPoint = secondPoint;
    int x, y, z;
    Stack_Util_Coord(index, m_workspace->width, m_workspace->height,
                     &x, &y, &z);
    secondPoint.set(x, y, z);

    len += firstPoint.distanceTo(secondPoint);
*/
      ssize_t firstIndex = secondIndex;
      secondIndex = index;
      int area = m_workspace->width * m_workspace->height;
      len += index_distance(firstIndex, secondIndex, m_workspace->width, area);

      index = m_workspace->path[index];
    }
  }

#ifdef _DEBUG_2
  if (m_workspace->length != NULL) {
    double cachedLength = m_workspace->length[originalIndex];
    if (index >= 0) {
      cachedLength -= m_workspace->length[index];
    } else {
      cachedLength += 1.0;
    }
    std::cout << "Cached length: " <<  cachedLength << " | ";
    std::cout << "Computed length: " << len << std::endl;
    if (fabs(cachedLength - len) > 1.5) {
      std::cout << "Debug here" << std::endl;
    }
  }
#endif

  return len;
}

ZVoxelArray ZSpGrowParser::extractLongestPath(double *length, bool masked)
{

  ssize_t idx = -1;
  double maxLength = 0;
  size_t i;

  if (m_fgArray.empty()) {
    if (m_workspace->mask != NULL) {
      for (i = 0; i < m_workspace->size; i++) {
        if (m_workspace->mask[i] != SP_GROW_BARRIER) {
          m_fgArray.push_back(i);
        }
      }
    }
  }

  if (m_fgArray.empty()) {
    for (i = 0; i < m_workspace->size; i++) {
      double len = pathLength(i, masked);
      if (len > maxLength) {
        maxLength = len;
        idx = i;
      }
    }
  } else {
    for (i = 0; i < m_fgArray.size(); ++i) {
      double len = pathLength(m_fgArray[i], masked);
      if (len > maxLength) {
        maxLength = len;
        idx = m_fgArray[i];
      }
    }
  }

  if (length != NULL) {
    *length = maxLength;
  }

  return extractPath(idx);
}

static double DistanceWeight(double v)
{
  return sqrt(v);
}

vector<ZVoxelArray> ZSpGrowParser::extractAllPath(double lengthThreshold,
                                                  Stack *ballStack)
{
  bool isPathAvailable = true;

  const double maskExpansionRadius = 2.0;
  const double skeletonRadius = 3.0;

  //Calibrate
  lengthThreshold -= maskExpansionRadius;

  vector<ZVoxelArray> pathArray;

  std::cout << "Extracting path ..." << std::endl;
  //While any long path is available
  while (isPathAvailable) {
    //Extract the longest path
    double length = 0.0;
    ZVoxelArray path = extractLongestPath(&length, true);

#ifdef _DEBUG_
    cout << "Path length: " << length << endl;
    cout << "Path size: " << path.size() << endl;
    if (path.size() == 1) {
      cout << "Debug here." << endl;
    }
#endif

    if (length < lengthThreshold) {
      isPathAvailable = false;
    } else {
      pathArray.push_back(path);
      //Update checkedMask
      if (m_checkedMask == NULL) {
        m_checkedMask = Make_Stack(GREY, m_workspace->width,
                                   m_workspace->height, m_workspace->depth);
        Zero_Stack(m_checkedMask);
      }
      if (m_pathMask == NULL) {
        m_pathMask = Make_Stack(GREY, m_workspace->width,
                                m_workspace->height, m_workspace->depth);
        Zero_Stack(m_pathMask);
      }

      if (ballStack != NULL) { //Extract distance field values
        path.sample(ballStack, DistanceWeight);
        path.addValue(1.0);
        path.minimizeValue(skeletonRadius);
      } else {
        path.addValue(skeletonRadius);
      }

      path.labelStackWithBall(m_pathMask, 1); //Label skeletons with a certain width

      if (ballStack != NULL) {
        path.sample(ballStack, DistanceWeight);
        path.addValue(maskExpansionRadius);
      }
      path.labelStackWithBall(m_checkedMask, 1); //Label mask
    }
  }
  std::cout << pathArray.size() << " path(s) extracted." << std::endl;

  return pathArray;
}

Stack* ZSpGrowParser::createDistanceStack()
{
  Stack *stack = NULL;

  if (m_workspace != NULL) {
    if (m_workspace->dist != NULL) {
      size_t nvoxel = m_workspace->size;

      for (size_t i = 0; i < nvoxel; i++) {
        if (tz_isinf(m_workspace->dist[i])) {
          m_workspace->dist[i] = 0.0;
        }
      }
      stack = Scale_Double_Stack(m_workspace->dist, m_workspace->width,
                                 m_workspace->height, m_workspace->depth,
                                 GREY);
    }
  }

  return stack;
}

Stack* ZSpGrowParser::createEuclideanDistanceStack()
{
  Stack *stack = NULL;

  if (m_workspace != NULL) {
    if (m_workspace->length != NULL) {
      size_t nvoxel = m_workspace->size;

      for (size_t i = 0; i < nvoxel; i++) {
        if (tz_isinf(m_workspace->dist[i])) {
          m_workspace->length[i] = 0.0;
        }
      }
      stack = Scale_Double_Stack(m_workspace->length, m_workspace->width,
                                 m_workspace->height, m_workspace->depth,
                                 GREY);
    }
  }

  return stack;
}
