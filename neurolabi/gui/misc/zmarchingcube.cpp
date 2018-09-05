#include "zmarchingcube.h"

#include "ilastik/marching_cubes.h"
#include "ilastik/laplacian_smoothing.h"
#include "zmesh.h"
#include "zstack.hxx"

ZMarchingCube::ZMarchingCube()
{

}

namespace {

ZMesh * ConvertMeshToZMesh(
    const ilastik::Mesh &mesh, const ZIntPoint &offset,
    const ZIntPoint &dsIntv, bool offsetAdjust, ZMesh *out)
{
  if (out == NULL) {
    out = new ZMesh(GL_TRIANGLES);
  } else {
    out->clear();
  }

  std::vector<glm::vec3> vertices(mesh.vertexCount);
//  std::vector<glm::vec3> normals(mesh.vertexCount);

  double dx = offset.getX();
  double dy = offset.getY();
  double dz = offset.getZ();

  if (offsetAdjust) {
    dx += 0.5;
    dy += 0.5;
    dz += 0.5;
  }

  int sx = dsIntv.getX() + 1;
  int sy = dsIntv.getY() + 1;
  int sz = dsIntv.getZ() + 1;

  //Transfer vertices
  for (size_t i = 0; i < mesh.vertexCount; ++i) {
    vertices[i] = glm::vec3(
          (mesh.vertices[i][0] + dx) * sx,
          (mesh.vertices[i][1] + dy) * sy,
          (mesh.vertices[i][2] + dz) * sz);

//    normals[i] = glm::vec3(
//          mesh.normals[i][0], mesh.normals[i][1], mesh.normals[i][2]);
  }
  out->setVertices(vertices);
//  out->setNormals(normals);

  //Transfer faces
  std::vector<GLuint> indices(mesh.faceCount * 3);
  for (size_t i = 0; i < indices.size(); i+=3) {
    for (int j = 0; j < 3; ++j) {
      indices[i] = int(mesh.faces[i + 2]);
      indices[i + 1] = int(mesh.faces[i + 1]);
      indices[i + 2] = int(mesh.faces[i]);
    }
  }
  out->setIndices(indices);

  return out;
}

}

ZMesh* ZMarchingCube::March(
    const ZStack &stack, int smooth, bool offsetAdjust, ZMesh *out)
{
  if (stack.hasData()) {
    tic();
    ilastik::Mesh mesh = ilastik::march(
          stack.array8(), stack.width(), stack.height(), stack.depth(), 1);
    std::cout << "Mesh extracting time:" << toc() << std::endl;

    ilastik::smooth(mesh, smooth);

    out = ConvertMeshToZMesh(
          mesh, stack.getOffset(), stack.getDsIntv(), offsetAdjust, out);
  }

  return out;
}

ZMesh* ZMarchingCube::march(const ZStack &stack)
{
  ilastik::Mesh mesh = ilastik::march(
        stack.array8(), stack.width(), stack.height(), stack.depth(), 1);

  ilastik::smooth(mesh, m_smooth);

  ZMesh *out = ConvertMeshToZMesh(
        mesh, stack.getOffset(), stack.getDsIntv(), m_offsetAdjust, m_result);

  return out;
}

/*
ZMesh* ZMarchingCube::March(const ZStack &stack, ZMesh *out)
{
  ilastik::Mesh mesh = ilastik::march(
        stack.array8(), stack.width(), stack.height(), stack.depth(), 1);

  ilastik::smooth(mesh, 3);

  out = ConvertMeshToZMesh(
        mesh, stack.getOffset(), stack.getDsIntv(), true, out);

  return out;
}
*/
