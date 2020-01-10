#ifndef TASKUTILS_H
#define TASKUTILS_H

#include "geometry/zpoint.h"
#include <QJsonValue>

#include <set>
#include <vector>

class ZFlyEmBody3dDoc;
class Z3DMeshFilter;

namespace TaskUtils
{
  bool pointFromJSON(const QJsonValue &value, ZPoint &result);

  // If "firstOnlySmallest" is true, then the first camera in "meshFilters" will be set to
  // frame only the smallest mesh.
  void zoomToMeshes(ZFlyEmBody3dDoc *bodyDoc, const std::set<uint64_t> bodyIDs, ZPoint center,
                    const std::vector<Z3DMeshFilter*> meshFilters,
                    bool firstOnlySmallest = false,
                    size_t stride = 2);
};

#endif // TASKUTILS_H
