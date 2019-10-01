#include "taskutils.h"

#include "flyem/zflyembody3ddoc.h"
#include "z3dcamera.h"
#include "z3dcamerautils.h"
#include "z3dmeshfilter.h"
#include "z3dwindow.h"
#include "zstackdocproxy.h"

#include <QJsonArray>
#include <QJsonValue>

namespace TaskUtils
{

  bool pointFromJSON(const QJsonValue &value, ZPoint &result)
  {
    if (value.isArray()) {
      QJsonArray array = value.toArray();
      if (array.size() == 3) {
        result = ZPoint(array[0].toDouble(), array[1].toDouble(), array[2].toDouble());
        return true;
      }
    }
    return false;
  }

  void zoomToMeshes(ZFlyEmBody3dDoc *bodyDoc, const std::set<uint64_t> bodyIds, ZPoint center,
                    const std::vector<Z3DMeshFilter*> meshFilters, bool firstOnlySmallest,
                    size_t stride)
  {
    std::map<uint64_t, double> radii;
    for (uint64_t id : bodyIds) {
      radii[id] = 0;
    }

    std::vector<std::vector<glm::vec3>> vertices;
    std::map<uint64_t, size_t> indices;
    for (uint64_t id : bodyIds) {
      indices[id] = vertices.size();
      vertices.push_back(std::vector<glm::vec3>());
    }

    ZBBox<glm::dvec3> bbox;

    QList<ZMesh*> meshes = ZStackDocProxy::GetGeneralMeshList(bodyDoc);
    for (auto it = meshes.cbegin(); it != meshes.cend(); it++) {
      ZMesh *mesh = *it;
      uint64_t tarBodyId = bodyDoc->getMappedId(mesh->getLabel());
      if (bodyIds.find(tarBodyId) != bodyIds.end()) {
        std::vector<glm::vec3> &indexedVertices = vertices[indices[tarBodyId]];
        for (size_t j = 0; j < mesh->vertices().size(); j++) {
          glm::vec3 vertex = mesh->vertices()[j];
          double r = center.distanceTo(double(vertex.x), double(vertex.y), double(vertex.z));
          if (r > radii[tarBodyId]) {
            radii[tarBodyId] = r;
          }

          // To improve performance, use only a fraction of the vertices
          // when determining the zoom.

          if (j % stride == 0) {
            indexedVertices.push_back(vertex);
          }
        }

        // The code below will tighten the zoom to the body or bodies of interest.  If it is
        // only the smaller body, then the near clipping plane might be wrong for all the bodies
        // together. So compute their bounding box for use in a final adjustment.

        bbox.expand(mesh->boundBox());
      }
    }

    uint64_t smallestId, biggestId;
    double smallestRadius = std::numeric_limits<double>::max();
    double biggestRadius = std::numeric_limits<double>::min();
    for (uint64_t id : bodyIds) {
      if (radii[id] < smallestRadius) {
        smallestId = id;
        smallestRadius = radii[id];
      }
      if (radii[id] > biggestRadius) {
        biggestId = id;
        biggestRadius = radii[id];
      }
    }

    for (size_t i = 0; i < meshFilters.size(); i++) {
      Z3DMeshFilter *filter = meshFilters[i];
      double radius = (firstOnlySmallest && (i == 0)) ? radii[smallestId] : radii[biggestId];
      Z3DCameraUtils::resetCamera(center, radius, filter->camera());

      if (firstOnlySmallest && (i == 0)) {
        Z3DCameraUtils::tightenZoom(filter->camera(), vertices, indices[smallestId]);

        filter->camera().resetCameraNearFarPlane(bbox);
      } else {
        Z3DCameraUtils::tightenZoom(filter->camera(), vertices);
      }

      filter->invalidate();
    }
  }

}
