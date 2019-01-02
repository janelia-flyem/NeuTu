#include "zmesh.h"

#include "zmeshio.h"
#include "zmeshutils.h"
#include "zbbox.h"
#include "zexception.h"
#include "zcubearray.h"
#include <vtkPolyData.h>
#include <vtkPointData.h>
#include <vtkSphereSource.h>
#include <vtkTubeFilter.h>
#include <vtkFloatArray.h>
#include <vtkBooleanOperationPolyDataFilter.h>
#include <vtkMassProperties.h>
#include <vtkTriangleFilter.h>
#include <vtkCleanPolyData.h>
#include <vtkAppendPolyData.h>
#include <vtkOBBTree.h>
#include <vtkCellArray.h>
#include <boost/math/constants/constants.hpp>
#include <map>
#include "misc/zvtkutil.h"
#include "zpoint.h"
#include "zqslog.h"

ZMesh::ZMesh(GLenum type)
{
  setType(type);
  m_type = GetType();
}

ZMesh::ZMesh(const QString& filename)
{
  load(filename);
  m_type = GetType();
}

void ZMesh::swap(ZMesh& rhs) noexcept
{
  std::swap(m_ttype, rhs.m_ttype);

  m_vertices.swap(rhs.m_vertices);
  m_1DTextureCoordinates.swap(rhs.m_1DTextureCoordinates);
  m_2DTextureCoordinates.swap(rhs.m_2DTextureCoordinates);
  m_3DTextureCoordinates.swap(rhs.m_3DTextureCoordinates);
  m_normals.swap(rhs.m_normals);
  m_colors.swap(rhs.m_colors);
  m_indices.swap(rhs.m_indices);

  rhs.validateObbTree(false);
  validateObbTree(false);
}

/*
void ZMesh::setLabel(uint64_t label)
{
  m_label = label;
}

uint64_t ZMesh::getLabel() const
{
  return m_label;
}
*/

bool ZMesh::canReadFile(const QString& filename)
{
  return ZMeshIO::instance().canReadFile(filename);
}

bool ZMesh::canWriteFile(const QString& filename)
{
  return ZMeshIO::instance().canWriteFile(filename);
}

const QString& ZMesh::getQtReadNameFilter()
{
  return ZMeshIO::instance().getQtReadNameFilter();
}

void ZMesh::getQtWriteNameFilter(QStringList& filters, QList<std::string>& formats)
{
  ZMeshIO::instance().getQtWriteNameFilter(filters, formats);
}

void ZMesh::load(const QString& filename)
{
  ZMeshIO::instance().load(filename, *this);
  setSource(qUtf8Printable(filename));
  validateObbTree(false);
}

void ZMesh::save(const QString& filename, const std::string& format) const
{
  ZMeshIO::instance().save(*this, filename, format);
}

void ZMesh::save(const std::string& filename, const std::string& format) const
{
  save(filename.c_str(), format);
}

void ZMesh::save(const char* filename, const std::string& format) const
{
  save(QString(filename), format);
}

QByteArray ZMesh::writeToMemory(const std::string &format) const
{
  return ZMeshIO::instance().writeToMemory(*this, format);
}

ZBBox<glm::dvec3> ZMesh::boundBox() const
{
  ZBBox<glm::dvec3> result;
  for (size_t i = 0; i < m_vertices.size(); ++i) {
    result.expand(glm::dvec3(m_vertices[i]));
  }
  return result;
}

ZCuboid ZMesh::getBoundBox() const
{
  ZCuboid box;
  ZBBox<glm::dvec3> bbox = boundBox();
  box.setFirstCorner(bbox.minCorner().x, bbox.minCorner().y, bbox.minCorner().z);
  box.setLastCorner(bbox.maxCorner().x, bbox.maxCorner().y, bbox.maxCorner().z);

  return box;
}

ZBBox<glm::dvec3> ZMesh::boundBox(const glm::mat4& transform) const
{
  ZBBox<glm::dvec3> result;
  for (size_t i = 0; i < m_vertices.size(); ++i) {
    glm::vec3 vert = glm::applyMatrix(transform, m_vertices[i]);
    result.expand(glm::dvec3(vert));
  }
  return result;
}

QString ZMesh::typeAsString() const
{
  if (m_ttype == GL_TRIANGLES) {
    return "GL_TRIANGLES";
  }

  if (m_ttype == GL_TRIANGLE_STRIP) {
    return "GL_TRIANGLE_STRIP";
  }

  if (m_ttype == GL_TRIANGLE_FAN) {
    return "GL_TRIANGLE_FAN";
  }

  return "WrongType";
}

void ZMesh::setType(GLenum type)
{
  m_ttype = type;
  CHECK(m_ttype == GL_TRIANGLES || m_ttype == GL_TRIANGLE_FAN || m_ttype == GL_TRIANGLE_STRIP);
}

std::vector<glm::dvec3> ZMesh::doubleVertices() const
{
  std::vector<glm::dvec3> result;
  for (auto v : m_vertices)
    result.emplace_back(v.x, v.y, v.z);
  return result;
}

void ZMesh::setVertices(const std::vector<glm::dvec3>& vertices)
{
  for (const auto& v : vertices) {
    m_vertices.push_back(glm::vec3(v));
  }

  validateObbTree(false);
}

void ZMesh::setNormals(const std::vector<glm::dvec3>& normals)
{
  for (const auto& n : normals) {
    m_normals.push_back(glm::vec3(n));
  }
}

void ZMesh::interpolate(const ZMesh& ref)
{
  std::vector<glm::uvec3> triIdxs = ref.triangleIndices();
  if (!ref.m_1DTextureCoordinates.empty())
    m_1DTextureCoordinates.clear();
  if (!ref.m_2DTextureCoordinates.empty())
    m_2DTextureCoordinates.clear();
  if (!ref.m_3DTextureCoordinates.empty())
    m_3DTextureCoordinates.clear();
  if (!ref.m_colors.empty())
    m_colors.clear();
  for (size_t i = 0; i < m_vertices.size(); ++i) {
    bool match = false;
    // first check all ref vertices
    for (size_t j = 0; !match && j < ref.m_vertices.size(); ++j) {
      if (glm::length(ref.m_vertices[j] - m_vertices[i]) <= 1e-6) {
        match = true;
        if (!ref.m_1DTextureCoordinates.empty())
          m_1DTextureCoordinates.push_back(ref.m_1DTextureCoordinates[j]);
        if (!ref.m_2DTextureCoordinates.empty())
          m_2DTextureCoordinates.push_back(ref.m_2DTextureCoordinates[j]);
        if (!ref.m_3DTextureCoordinates.empty())
          m_3DTextureCoordinates.push_back(ref.m_3DTextureCoordinates[j]);
        if (!ref.m_colors.empty())
          m_colors.push_back(ref.m_colors[j]);
      }
    }
    // no vertice match, interpolate
    for (size_t j = 0; !match && j < triIdxs.size(); ++j) {
      glm::uvec3 triIdx = triIdxs[j];
      double s, t;
      if (ZMeshUtils::vertexTriangleSquaredDistance(glm::dvec3(m_vertices[i]), glm::dvec3(ref.m_vertices[triIdx[0]]),
                                                    glm::dvec3(ref.m_vertices[triIdx[1]]),
                                                    glm::dvec3(ref.m_vertices[triIdx[2]]),
                                                    s, t)
          <= 1e-6) {
        match = true;
        float fs = static_cast<float>(s);
        float ft = static_cast<float>(t);
        if (!ref.m_1DTextureCoordinates.empty())
          m_1DTextureCoordinates.push_back(ref.m_1DTextureCoordinates[triIdx[0]] +
                                           (ref.m_1DTextureCoordinates[triIdx[1]] -
                                            ref.m_1DTextureCoordinates[triIdx[0]]) * fs +
                                           (ref.m_1DTextureCoordinates[triIdx[2]] -
                                            ref.m_1DTextureCoordinates[triIdx[0]]) * ft);
        if (!ref.m_2DTextureCoordinates.empty())
          m_2DTextureCoordinates.push_back(ref.m_2DTextureCoordinates[triIdx[0]] +
                                           (ref.m_2DTextureCoordinates[triIdx[1]] -
                                            ref.m_2DTextureCoordinates[triIdx[0]]) * fs +
                                           (ref.m_2DTextureCoordinates[triIdx[2]] -
                                            ref.m_2DTextureCoordinates[triIdx[0]]) * ft);
        if (!ref.m_3DTextureCoordinates.empty())
          m_3DTextureCoordinates.push_back(ref.m_3DTextureCoordinates[triIdx[0]] +
                                           (ref.m_3DTextureCoordinates[triIdx[1]] -
                                            ref.m_3DTextureCoordinates[triIdx[0]]) * fs +
                                           (ref.m_3DTextureCoordinates[triIdx[2]] -
                                            ref.m_3DTextureCoordinates[triIdx[0]]) * ft);
        if (!ref.m_colors.empty())
          m_colors.push_back(ref.m_colors[triIdx[0]] +
                             (ref.m_colors[triIdx[1]] - ref.m_colors[triIdx[0]]) * fs +
                             (ref.m_colors[triIdx[2]] - ref.m_colors[triIdx[0]]) * ft);
      }
    }
    if (!match) {
      if (!ref.m_1DTextureCoordinates.empty())
        m_1DTextureCoordinates.push_back(0.0);
      if (!ref.m_2DTextureCoordinates.empty())
        m_2DTextureCoordinates.emplace_back(0.0);
      if (!ref.m_3DTextureCoordinates.empty())
        m_3DTextureCoordinates.emplace_back(0.0);
      if (!ref.m_colors.empty())
        m_colors.emplace_back(0.0);
    }
  }

  validateObbTree(false);
}

void ZMesh::clear()
{
  m_vertices.clear();
  m_1DTextureCoordinates.clear();
  m_2DTextureCoordinates.clear();
  m_3DTextureCoordinates.clear();
  m_normals.clear();
  m_colors.clear();
  m_indices.clear();
  validateObbTree(false);
}

size_t ZMesh::numTriangles() const
{
  size_t n = 0;
  if (m_indices.empty())
    n = m_vertices.size();
  else
    n = m_indices.size();
  if (m_ttype == GL_TRIANGLES)
    return n / 3;
  if (m_ttype == GL_TRIANGLE_STRIP || m_ttype == GL_TRIANGLE_FAN)
    return n - 2;

  return 0;
}

std::vector<glm::vec3> ZMesh::triangleVertices(size_t index) const
{
  std::vector<glm::vec3> triangle;
  glm::uvec3 tIs = triangleIndices(index);
  triangle.push_back(m_vertices[tIs[0]]);
  triangle.push_back(m_vertices[tIs[1]]);
  triangle.push_back(m_vertices[tIs[2]]);
  return triangle;
}

std::vector<glm::uvec3> ZMesh::triangleIndices() const
{
  std::vector<glm::uvec3> result;
  if (m_indices.empty()) {
    if (m_ttype == GL_TRIANGLES) {
      for (size_t i = 0; i < m_vertices.size() - 2; i += 3) {
        result.emplace_back(i, i + 1, i + 2);
      }
    } else if (m_ttype == GL_TRIANGLE_STRIP) {
      for (size_t i = 0; i < m_vertices.size() - 2; ++i) {
        glm::uvec3 triangle;
        if (i % 2 == 0) {
          triangle[0] = i;
          triangle[1] = i + 1;
        } else {
          triangle[0] = i + 1;
          triangle[1] = i;
        }
        triangle[2] = i + 2;
        result.push_back(triangle);
      }
    } else /*(m_type == GL_TRIANGLE_FAN)*/ {
      for (size_t i = 1; i < m_vertices.size() - 1; ++i) {
        result.emplace_back(0, i, i + 1);
      }
    }
  } else {
    if (m_ttype == GL_TRIANGLES) {
      for (size_t i = 0; i < m_indices.size() - 2; i += 3) {
        result.emplace_back(m_indices[i], m_indices[i + 1], m_indices[i + 2]);
      }
    } else if (m_ttype == GL_TRIANGLE_STRIP) {
      for (size_t i = 0; i < m_indices.size() - 2; ++i) {
        glm::uvec3 triangle;
        if (i % 2 == 0) {
          triangle[0] = m_indices[i];
          triangle[1] = m_indices[i + 1];
        } else {
          triangle[0] = m_indices[i + 1];
          triangle[1] = m_indices[i];
        }
        triangle[2] = m_indices[i + 2];
        result.push_back(triangle);
      }
    } else /*(m_type == GL_TRIANGLE_FAN)*/ {
      for (size_t i = 1; i < m_indices.size() - 1; ++i) {
        result.emplace_back(m_indices[0], m_indices[i], m_indices[i + 1]);
      }
    }
  }
  return result;
}

glm::uvec3 ZMesh::triangleIndices(size_t index) const
{
  glm::uvec3 triangle;
  CHECK(index < numTriangles());
  if (m_indices.empty()) {
    if (m_ttype == GL_TRIANGLES) {
      triangle[0] = index * 3;
      triangle[1] = index * 3 + 1;
      triangle[2] = index * 3 + 2;
    } else if (m_ttype == GL_TRIANGLE_STRIP) {
      if (index % 2 == 0) {
        triangle[0] = index;
        triangle[1] = index + 1;
      } else {
        triangle[0] = index + 1;
        triangle[1] = index;
      }
      triangle[2] = index + 2;
    } else if (m_ttype == GL_TRIANGLE_FAN) {
      triangle[0] = 0;
      triangle[1] = index + 1;
      triangle[2] = index + 2;
    }
  } else {
    if (m_ttype == GL_TRIANGLES) {
      triangle[0] = m_indices[index * 3];
      triangle[1] = m_indices[index * 3 + 1];
      triangle[2] = m_indices[index * 3 + 2];
    } else if (m_ttype == GL_TRIANGLE_STRIP) {
      if (index % 2 == 0) {
        triangle[0] = m_indices[index];
        triangle[1] = m_indices[index + 1];
      } else {
        triangle[0] = m_indices[index + 1];
        triangle[1] = m_indices[index];
      }
      triangle[2] = m_indices[index + 2];
    } else if (m_ttype == GL_TRIANGLE_FAN) {
      triangle[0] = m_indices[0];
      triangle[1] = m_indices[index + 1];
      triangle[2] = m_indices[index + 2];
    }
  }
  return triangle;
}

glm::vec3 ZMesh::triangleVertex(size_t triangleIndex, size_t vertexIndex) const
{
  CHECK(vertexIndex <= 2);
  return triangleVertices(triangleIndex)[vertexIndex];
}

void ZMesh::transformVerticesByMatrix(const glm::mat4& tfmat)
{
  if (tfmat == glm::mat4(1.0))
    return;
  for (size_t i = 0; i < m_vertices.size(); ++i) {
    m_vertices[i] = glm::applyMatrix(tfmat, m_vertices[i]);
  }

  validateObbTree(false);
}

std::vector<ZMesh> ZMesh::split(size_t numTriangle) const
{
  size_t totalNumTri = numTriangles();
  size_t numResult = std::ceil(1.0 * totalNumTri / numTriangle);
  std::vector<ZMesh> res(numResult);
  for (size_t i = 0; i < numResult; ++i) {
    size_t first = i * numTriangle;
    size_t last = std::min(totalNumTri, (i + 1) * numTriangle);
    for (size_t j = first; j < last; ++j) {
      res[i].appendTriangle(*this, triangleIndices(j));
    }
  }
  return res;
}

void ZMesh::prepareNormals(bool useAreaWeight)
{
  if (m_normals.size() < m_vertices.size()) {
    generateNormals(useAreaWeight);
  }
}

void ZMesh::generateNormals(bool useAreaWeight)
{
#ifdef _DEBUG_
  std::cout << "#vertex: " << m_vertices.size() << std::endl;
#endif
  m_normals.resize(m_vertices.size());
  for (size_t i = 0; i < m_normals.size(); ++i)
    m_normals[i] = glm::vec3(0.f);

  for (size_t i = 0; i < numTriangles(); ++i) {
    glm::uvec3 tri = triangleIndices(i);
    // get the three vertices that make the faces
    const glm::vec3 &p1 = m_vertices[tri[0]];
    const glm::vec3 &p2 = m_vertices[tri[1]];
    const glm::vec3 &p3 = m_vertices[tri[2]];

    glm::vec3 v1 = p2 - p1;
    glm::vec3 v2 = p3 - p1;
    glm::vec3 normal = glm::cross(v1, v2);
    if (!useAreaWeight)
      normal = glm::normalize(normal);
    m_normals[tri[0]] += normal;
    m_normals[tri[1]] += normal;
    m_normals[tri[2]] += normal;
  }

  for (size_t i = 0; i < m_normals.size(); ++i) {
#ifdef _DEBUG_2
    std::cout << "Normals: " << m_normals[i].x << " " << m_normals[i].y << " "
              << m_normals[i].z << std::endl;
#endif
    m_normals[i] = glm::normalize(m_normals[i]);
  }
}

//double ZMesh::volume() const
//{
//  double res = 0;
//  for (size_t i=0; i<numTriangles(); ++i) {
//    glm::uvec3 tIs = triangleIndices(i);
//    glm::vec3 normal = glm::normalize(glm::cross(m_vertices[tIs[1]] - m_vertices[tIs[0]],
//        m_vertices[tIs[2]] - m_vertices[tIs[0]]));
//    if (glm::dot(m_normals[tIs[0]], normal) +
//        glm::dot(m_normals[tIs[1]], normal) +
//        glm::dot(m_normals[tIs[2]], normal) < 0) {
//      res += signedVolumeOfTriangle(m_vertices[tIs[0]] - m_vertices[0], m_vertices[tIs[2]] - m_vertices[0], m_vertices[tIs[1]] - m_vertices[0]);
//    } else {
//      res += signedVolumeOfTriangle(m_vertices[tIs[0]] - m_vertices[0], m_vertices[tIs[1]] - m_vertices[0], m_vertices[tIs[2]] - m_vertices[0]);
//    }
//  }
//  return std::abs(res);
//}

ZMeshProperties ZMesh::properties() const
{
  vtkSmartPointer<vtkPolyData> poly = meshToVtkPolyData(*this);
  vtkSmartPointer<vtkMassProperties> massProperties = vtkSmartPointer<vtkMassProperties>::New();
  massProperties->SetInputData(poly);

  massProperties->Update();

  ZMeshProperties res;
  res.numVertices = numVertices();
  res.numTriangles = numTriangles();
  res.kx = massProperties->GetKx();
  res.ky = massProperties->GetKy();
  res.kz = massProperties->GetKz();
  res.maxTriangleArea = massProperties->GetMaxCellArea();
  res.minTriangleArea = massProperties->GetMinCellArea();
  res.normalizedShapeIndex = massProperties->GetNormalizedShapeIndex();
  res.surfaceArea = massProperties->GetSurfaceArea();
  res.volume = massProperties->GetVolume();
  res.volumeProjected = massProperties->GetVolumeProjected();
  res.volumeX = massProperties->GetVolumeX();
  res.volumeY = massProperties->GetVolumeY();
  res.volumeZ = massProperties->GetVolumeZ();
  return res;
}

void ZMesh::LogProperties(const ZMeshProperties& prop, const QString& str)
{
  LOG(INFO) << "";
  if (!str.isEmpty()) {
    LOG(INFO) << str;
  }
  LOG(INFO) << "Vertices Number: " << prop.numVertices;
  LOG(INFO) << "Triangles Number: " << prop.numTriangles;
  //LOG(INFO) << "volume old: " << volume();
  LOG(INFO) << "Surface Area: " << prop.surfaceArea;
  LOG(INFO) << "Min Triangle Area: " << prop.minTriangleArea;
  LOG(INFO) << "Max Triangle Area: " << prop.maxTriangleArea;
  LOG(INFO) << "Volume: " << prop.volume;
  LOG(INFO) << "Volume Projected: " << prop.volumeProjected;
  LOG(INFO) << "Volume X: " << prop.volumeX;
  LOG(INFO) << "Volume Y: " << prop.volumeY;
  LOG(INFO) << "Volume Z: " << prop.volumeZ;
  LOG(INFO) << "Kx: " << prop.kx;
  LOG(INFO) << "Ky: " << prop.ky;
  LOG(INFO) << "Kz: " << prop.kz;
  LOG(INFO) << "Normalized Shape Index: " << prop.normalizedShapeIndex;
  LOG(INFO) << "";
}

ZMesh ZMesh::CreateCuboidFaceMesh(
    const ZIntCuboid &cf, const std::vector<bool> &visible, const QColor &color)
{
  std::vector<glm::vec3> coordLlfs;
  coordLlfs.emplace_back(cf.getFirstCorner().getX(), cf.getFirstCorner().getY(),
                         cf.getFirstCorner().getZ());

  std::vector<glm::vec3> coordUrbs;
  coordUrbs.emplace_back(cf.getLastCorner().getX(), cf.getLastCorner().getY(),
                         cf.getLastCorner().getZ());

  std::vector<std::vector<bool> > faceVisbility;
  faceVisbility.push_back(visible);

  std::vector<glm::vec4> cubeColors;
  cubeColors.emplace_back(color.redF(), color.greenF(), color.blueF(), color.alphaF());

  return CreateCubesWithNormal(coordLlfs, coordUrbs, faceVisbility, &cubeColors);
}

void ZMesh::createCubesWithNormal(
    const std::vector<glm::vec3>& coordLlfs,
    const std::vector<glm::vec3>& coordUrbs,
    const std::vector<std::vector<bool> >& faceVisbility,
    const std::vector<glm::vec4>* cubeColors)
{
  CHECK(coordLlfs.size() == coordUrbs.size());
  CHECK(!cubeColors || cubeColors->size() >= coordLlfs.size());
  clear();
  setType(GL_TRIANGLES);

  GLuint idxes[6] = {0, 1, 2, 2, 1, 3};

  size_t offset = 0;
  for (size_t i = 0; i < coordLlfs.size(); ++i) {
    //CHECK(coordUrbs[i].z > coordLlfs[i].z && coordUrbs[i].y > coordLlfs[i].y && coordUrbs[i].x > coordLlfs[i].x);
    glm::vec3 p0(coordLlfs[i][0], coordLlfs[i][1], coordUrbs[i][2]); //4
    glm::vec3 p1(coordUrbs[i][0], coordLlfs[i][1], coordUrbs[i][2]); //5
    glm::vec3 p2(coordLlfs[i][0], coordUrbs[i][1], coordUrbs[i][2]); //6
    glm::vec3 p3(coordUrbs[i][0], coordUrbs[i][1], coordUrbs[i][2]); //7
    glm::vec3 p4(coordLlfs[i][0], coordLlfs[i][1], coordLlfs[i][2]); //0
    glm::vec3 p5(coordUrbs[i][0], coordLlfs[i][1], coordLlfs[i][2]); //1
    glm::vec3 p6(coordLlfs[i][0], coordUrbs[i][1], coordLlfs[i][2]); //2
    glm::vec3 p7(coordUrbs[i][0], coordUrbs[i][1], coordLlfs[i][2]); //3

    glm::vec3 frontFaceNormal = glm::normalize(p4 - p0); //z0 - z1
    glm::vec3 rightFaceNormal = glm::normalize(p5 - p4); //x1 - x0
    glm::vec3 upFaceNormal = glm::normalize(p4 - p6); //y0 - y1

    const std::vector<bool> fv = faceVisbility[i];

    size_t nvertices = 0;
    if (fv[5]) { //Front face
      m_vertices.push_back(p0);
      m_vertices.push_back(p1);
      m_vertices.push_back(p2);
      m_vertices.push_back(p3);

      m_normals.push_back(-frontFaceNormal);
      m_normals.push_back(-frontFaceNormal);
      m_normals.push_back(-frontFaceNormal);
      m_normals.push_back(-frontFaceNormal);

      for (GLuint j = 0; j < 6; ++j) {
        m_indices.push_back(idxes[j] + nvertices + offset);
      }
      nvertices += 4;
    }

    if (fv[3]) { //Up face
      m_vertices.push_back(p2);
      m_vertices.push_back(p3);
      m_vertices.push_back(p6);
      m_vertices.push_back(p7);

      m_normals.push_back(-upFaceNormal);
      m_normals.push_back(-upFaceNormal);
      m_normals.push_back(-upFaceNormal);
      m_normals.push_back(-upFaceNormal);

      for (GLuint j = 0; j < 6; ++j) {
        m_indices.push_back(idxes[j] + nvertices + offset);
      }
      nvertices += 4;
    }

    if (fv[0]) { //Left face
      m_vertices.push_back(p4);
      m_vertices.push_back(p0);
      m_vertices.push_back(p6);
      m_vertices.push_back(p2);

      m_normals.push_back(-rightFaceNormal);
      m_normals.push_back(-rightFaceNormal);
      m_normals.push_back(-rightFaceNormal);
      m_normals.push_back(-rightFaceNormal);

      for (GLuint j = 0; j < 6; ++j) {
        m_indices.push_back(idxes[j] + nvertices + offset);
      }
      nvertices += 4;
    }

    if (fv[1]) { //Right face
      m_vertices.push_back(p7);
      m_vertices.push_back(p3);
      m_vertices.push_back(p5);
      m_vertices.push_back(p1);

      m_normals.push_back(rightFaceNormal);
      m_normals.push_back(rightFaceNormal);
      m_normals.push_back(rightFaceNormal);
      m_normals.push_back(rightFaceNormal);

      for (GLuint j = 0; j < 6; ++j) {
        m_indices.push_back(idxes[j] + nvertices + offset);
      }
      nvertices += 4;
    }

    if (fv[2]) { //Down face
      m_vertices.push_back(p4);
      m_vertices.push_back(p5);
      m_vertices.push_back(p0);
      m_vertices.push_back(p1);

      m_normals.push_back(upFaceNormal);
      m_normals.push_back(upFaceNormal);
      m_normals.push_back(upFaceNormal);
      m_normals.push_back(upFaceNormal);

      for (GLuint j = 0; j < 6; ++j) {
        m_indices.push_back(idxes[j] + nvertices + offset);
      }
      nvertices += 4;
    }

    if (fv[4]) { //Back face
      m_vertices.push_back(p6);
      m_vertices.push_back(p7);
      m_vertices.push_back(p4);
      m_vertices.push_back(p5);

      m_normals.push_back(frontFaceNormal);
      m_normals.push_back(frontFaceNormal);
      m_normals.push_back(frontFaceNormal);
      m_normals.push_back(frontFaceNormal);

      for (GLuint j = 0; j < 6; ++j) {
        m_indices.push_back(idxes[j] + nvertices + offset);
      }
      nvertices += 4;
    }

    offset += nvertices;
    if (cubeColors) {
      for (size_t j = 0; j < nvertices; ++j)  {
        m_colors.push_back(cubeColors->at(i));
      }
    }
  }
}

void ZMesh::createCubesWithoutNormal(
    const std::vector<glm::vec3>& coordLlfs,
    const std::vector<glm::vec3>& coordUrbs,
    const std::vector<std::vector<bool> >& faceVisbility,
    const std::vector<glm::vec4>* cubeColors)
{
  CHECK(coordLlfs.size() == coordUrbs.size());
  CHECK(!cubeColors || cubeColors->size() >= coordLlfs.size());
  clear();
  setType(GL_TRIANGLES);

  GLuint idxes[6] = {0, 1, 2, 2, 1, 3};



  size_t offset = 0;
  for (size_t i = 0; i < coordLlfs.size(); ++i) {
    //CHECK(coordUrbs[i].z > coordLlfs[i].z && coordUrbs[i].y > coordLlfs[i].y && coordUrbs[i].x > coordLlfs[i].x);
    std::vector<glm::vec3> p(8);
    std::vector<bool> pv(8, false);
    std::vector<size_t> cubeTriangleIndices;

    p[0] = glm::vec3(coordLlfs[i][0], coordLlfs[i][1], coordUrbs[i][2]); //4
    p[1] = glm::vec3(coordUrbs[i][0], coordLlfs[i][1], coordUrbs[i][2]); //5
    p[2] = glm::vec3(coordLlfs[i][0], coordUrbs[i][1], coordUrbs[i][2]); //6
    p[3] = glm::vec3(coordUrbs[i][0], coordUrbs[i][1], coordUrbs[i][2]); //7
    p[4] = glm::vec3(coordLlfs[i][0], coordLlfs[i][1], coordLlfs[i][2]); //0
    p[5] = glm::vec3(coordUrbs[i][0], coordLlfs[i][1], coordLlfs[i][2]); //1
    p[6] = glm::vec3(coordLlfs[i][0], coordUrbs[i][1], coordLlfs[i][2]); //2
    p[7] = glm::vec3(coordUrbs[i][0], coordUrbs[i][1], coordLlfs[i][2]); //3

    const std::vector<bool> fv = faceVisbility[i];

//    size_t nvertices = 0;
    if (fv[5]) { //Front face
      int faceIndices[4] = {0, 1, 2, 3};
      for (size_t fi = 0; fi < 4; ++fi) {
        pv[faceIndices[fi]] = true;
      }
      for (GLuint j = 0; j < 6; ++j) {
        cubeTriangleIndices.push_back(faceIndices[idxes[j]]);
      }
    }

    if (fv[3]) { //Up face
      int faceIndices[4] = {2, 3, 6, 7};
      for (size_t fi = 0; fi < 4; ++fi) {
        pv[faceIndices[fi]] = true;
      }
      for (GLuint j = 0; j < 6; ++j) {
        cubeTriangleIndices.push_back(faceIndices[idxes[j]]);
      }
    }

    if (fv[0]) { //Left face
      int faceIndices[4] = {4, 0, 6, 2};
      for (size_t fi = 0; fi < 4; ++fi) {
        pv[faceIndices[fi]] = true;
      }
      for (GLuint j = 0; j < 6; ++j) {
        cubeTriangleIndices.push_back(faceIndices[idxes[j]]);
      }
    }

    if (fv[1]) { //Right face
      int faceIndices[4] = {7, 3, 5, 1};
      for (size_t fi = 0; fi < 4; ++fi) {
        pv[faceIndices[fi]] = true;
      }
      for (GLuint j = 0; j < 6; ++j) {
        cubeTriangleIndices.push_back(faceIndices[idxes[j]]);
      }
    }

    if (fv[2]) { //Down face
      int faceIndices[4] = {4, 5, 0, 1};
      for (size_t fi = 0; fi < 4; ++fi) {
        pv[faceIndices[fi]] = true;
      }
      for (GLuint j = 0; j < 6; ++j) {
        cubeTriangleIndices.push_back(faceIndices[idxes[j]]);
      }
    }

    if (fv[4]) { //Back face
      int faceIndices[4] = {6, 7, 4, 5};
      for (size_t fi = 0; fi < 4; ++fi) {
        pv[faceIndices[fi]] = true;
      }
      for (GLuint j = 0; j < 6; ++j) {
        cubeTriangleIndices.push_back(faceIndices[idxes[j]]);
      }
    }

    int vertexIndexMap[8];
    int currentIndex = 0;
    for (int ci = 0; ci < 8; ++ci) {
      if (pv[ci]) {
        m_vertices.push_back(p[ci]);
        vertexIndexMap[ci] = offset + currentIndex++;
      } else {
        vertexIndexMap[ci] = -1;
      }
    }

    for (size_t ti = 0; ti < cubeTriangleIndices.size(); ++ti) {
//      Q_ASSERT(vertexIndexMap[cubeTriangleIndices[ti]] >= 0);
      m_indices.push_back(vertexIndexMap[cubeTriangleIndices[ti]]);
    }

    offset += currentIndex;
    if (cubeColors) {
      for (int j = 0; j < currentIndex; ++j)  {
        m_colors.push_back(cubeColors->at(i));
      }
    }
  }
}

ZMesh ZMesh::CreateCubesWithNormal(const std::vector<glm::vec3>& coordLlfs,
                                   const std::vector<glm::vec3>& coordUrbs,
                                   const std::vector<std::vector<bool> >& faceVisbility,
                                   const std::vector<glm::vec4>* cubeColors)
{
  CHECK(coordLlfs.size() == coordUrbs.size());
  CHECK(!cubeColors || cubeColors->size() >= coordLlfs.size());
  ZMesh cubes(GL_TRIANGLES);
  std::vector<glm::vec3> vertices;
  std::vector<glm::vec3> normals;
  std::vector<GLuint> indexes;
  std::vector<glm::vec4> colors;
  GLuint idxes[6] = {0, 1, 2, 2, 1, 3};

  size_t offset = 0;
  for (size_t i = 0; i < coordLlfs.size(); ++i) {
    //CHECK(coordUrbs[i].z > coordLlfs[i].z && coordUrbs[i].y > coordLlfs[i].y && coordUrbs[i].x > coordLlfs[i].x);
    glm::vec3 p0(coordLlfs[i][0], coordLlfs[i][1], coordUrbs[i][2]); //4
    glm::vec3 p1(coordUrbs[i][0], coordLlfs[i][1], coordUrbs[i][2]); //5
    glm::vec3 p2(coordLlfs[i][0], coordUrbs[i][1], coordUrbs[i][2]); //6
    glm::vec3 p3(coordUrbs[i][0], coordUrbs[i][1], coordUrbs[i][2]); //7
    glm::vec3 p4(coordLlfs[i][0], coordLlfs[i][1], coordLlfs[i][2]); //0
    glm::vec3 p5(coordUrbs[i][0], coordLlfs[i][1], coordLlfs[i][2]); //1
    glm::vec3 p6(coordLlfs[i][0], coordUrbs[i][1], coordLlfs[i][2]); //2
    glm::vec3 p7(coordUrbs[i][0], coordUrbs[i][1], coordLlfs[i][2]); //3

    glm::vec3 frontFaceNormal = glm::normalize(p4 - p0); //z0 - z1
    glm::vec3 rightFaceNormal = glm::normalize(p5 - p4); //x1 - x0
    glm::vec3 upFaceNormal = glm::normalize(p4 - p6); //y0 - y1

    const std::vector<bool> fv = faceVisbility[i];

    size_t nvertices = 0;
    if (fv[5]) { //Front face
      vertices.push_back(p0);
      vertices.push_back(p1);
      vertices.push_back(p2);
      vertices.push_back(p3);

      normals.push_back(-frontFaceNormal);
      normals.push_back(-frontFaceNormal);
      normals.push_back(-frontFaceNormal);
      normals.push_back(-frontFaceNormal);

      for (GLuint j = 0; j < 6; ++j) {
        indexes.push_back(idxes[j] + nvertices + offset);
      }
      nvertices += 4;
    }

    if (fv[3]) { //Up face
      vertices.push_back(p2);
      vertices.push_back(p3);
      vertices.push_back(p6);
      vertices.push_back(p7);

      normals.push_back(-upFaceNormal);
      normals.push_back(-upFaceNormal);
      normals.push_back(-upFaceNormal);
      normals.push_back(-upFaceNormal);

      for (GLuint j = 0; j < 6; ++j) {
        indexes.push_back(idxes[j] + nvertices + offset);
      }
      nvertices += 4;
    }

    if (fv[0]) { //Left face
      vertices.push_back(p4);
      vertices.push_back(p0);
      vertices.push_back(p6);
      vertices.push_back(p2);

      normals.push_back(-rightFaceNormal);
      normals.push_back(-rightFaceNormal);
      normals.push_back(-rightFaceNormal);
      normals.push_back(-rightFaceNormal);

      for (GLuint j = 0; j < 6; ++j) {
        indexes.push_back(idxes[j] + nvertices + offset);
      }
      nvertices += 4;
    }

    if (fv[1]) { //Right face
      vertices.push_back(p7);
      vertices.push_back(p3);
      vertices.push_back(p5);
      vertices.push_back(p1);

      normals.push_back(rightFaceNormal);
      normals.push_back(rightFaceNormal);
      normals.push_back(rightFaceNormal);
      normals.push_back(rightFaceNormal);

      for (GLuint j = 0; j < 6; ++j) {
        indexes.push_back(idxes[j] + nvertices + offset);
      }
      nvertices += 4;
    }

    if (fv[2]) { //Down face
      vertices.push_back(p4);
      vertices.push_back(p5);
      vertices.push_back(p0);
      vertices.push_back(p1);

      normals.push_back(upFaceNormal);
      normals.push_back(upFaceNormal);
      normals.push_back(upFaceNormal);
      normals.push_back(upFaceNormal);

      for (GLuint j = 0; j < 6; ++j) {
        indexes.push_back(idxes[j] + nvertices + offset);
      }
      nvertices += 4;
    }

    if (fv[4]) { //Back face
      vertices.push_back(p6);
      vertices.push_back(p7);
      vertices.push_back(p4);
      vertices.push_back(p5);

      normals.push_back(frontFaceNormal);
      normals.push_back(frontFaceNormal);
      normals.push_back(frontFaceNormal);
      normals.push_back(frontFaceNormal);

      for (GLuint j = 0; j < 6; ++j) {
        indexes.push_back(idxes[j] + nvertices + offset);
      }
      nvertices += 4;
    }

    offset += nvertices;
    if (cubeColors) {
      for (size_t j = 0; j < nvertices; ++j)  {
        colors.push_back(cubeColors->at(i));
      }
    }
  }

#ifdef _DEBUG_
  std::cout << "Vertex size: " << vertices.size() << std::endl;
#endif

  cubes.m_vertices.swap(vertices);
  cubes.m_normals.swap(normals);
  cubes.m_indices.swap(indexes);
  cubes.m_colors.swap(colors);

#ifdef _DEBUG_
  std::cout << "Vertex size: " << cubes.numVertices() << std::endl;
#endif

  return cubes;
}


ZMesh ZMesh::CreateCubesWithNormal(const std::vector<glm::vec3>& coordLlfs,
                                   const std::vector<glm::vec3>& coordUrbs,
                                   const std::vector<glm::vec4>* cubeColors)
{
  CHECK(coordLlfs.size() == coordUrbs.size());
  CHECK(!cubeColors || cubeColors->size() >= coordLlfs.size());
  ZMesh cubes(GL_TRIANGLES);
  std::vector<glm::vec3> vertices;
  std::vector<glm::vec3> normals;
  std::vector<GLuint> indexes;
  std::vector<glm::vec4> colors;
  GLuint idxes[6] = {0, 1, 2, 2, 1, 3};

  for (size_t i = 0; i < coordLlfs.size(); ++i) {
    //CHECK(coordUrbs[i].z > coordLlfs[i].z && coordUrbs[i].y > coordLlfs[i].y && coordUrbs[i].x > coordLlfs[i].x);
    glm::vec3 p0(coordLlfs[i][0], coordLlfs[i][1], coordUrbs[i][2]); //1
    glm::vec3 p1(coordUrbs[i][0], coordLlfs[i][1], coordUrbs[i][2]); //2
    glm::vec3 p2(coordLlfs[i][0], coordUrbs[i][1], coordUrbs[i][2]); //5
    glm::vec3 p3(coordUrbs[i][0], coordUrbs[i][1], coordUrbs[i][2]); //6
    glm::vec3 p4(coordLlfs[i][0], coordLlfs[i][1], coordLlfs[i][2]); //0
    glm::vec3 p5(coordUrbs[i][0], coordLlfs[i][1], coordLlfs[i][2]); //3
    glm::vec3 p6(coordLlfs[i][0], coordUrbs[i][1], coordLlfs[i][2]); //4
    glm::vec3 p7(coordUrbs[i][0], coordUrbs[i][1], coordLlfs[i][2]); //7

    glm::vec3 frontFaceNormal = glm::normalize(p4 - p0);
    glm::vec3 rightFaceNormal = glm::normalize(p5 - p4);
    glm::vec3 upFaceNormal = glm::normalize(p4 - p6);

    vertices.push_back(p0);
    vertices.push_back(p1);
    vertices.push_back(p2);
    vertices.push_back(p3);

    normals.push_back(-frontFaceNormal);
    normals.push_back(-frontFaceNormal);
    normals.push_back(-frontFaceNormal);
    normals.push_back(-frontFaceNormal);

    for (GLuint j = 0; j < 6; ++j) {
      indexes.push_back(idxes[j] + 0 + i * 24);
    }

    vertices.push_back(p2);
    vertices.push_back(p3);
    vertices.push_back(p6);
    vertices.push_back(p7);

    normals.push_back(-upFaceNormal);
    normals.push_back(-upFaceNormal);
    normals.push_back(-upFaceNormal);
    normals.push_back(-upFaceNormal);

    for (GLuint j = 0; j < 6; ++j) {
      indexes.push_back(idxes[j] + 4 + i * 24);
    }

    vertices.push_back(p4);
    vertices.push_back(p0);
    vertices.push_back(p6);
    vertices.push_back(p2);

    normals.push_back(-rightFaceNormal);
    normals.push_back(-rightFaceNormal);
    normals.push_back(-rightFaceNormal);
    normals.push_back(-rightFaceNormal);

    for (GLuint j = 0; j < 6; ++j) {
      indexes.push_back(idxes[j] + 2 * 4 + i * 24);
    }

    vertices.push_back(p7);
    vertices.push_back(p3);
    vertices.push_back(p5);
    vertices.push_back(p1);

    normals.push_back(rightFaceNormal);
    normals.push_back(rightFaceNormal);
    normals.push_back(rightFaceNormal);
    normals.push_back(rightFaceNormal);

    for (GLuint j = 0; j < 6; ++j) {
      indexes.push_back(idxes[j] + 3 * 4 + i * 24);
    }

    vertices.push_back(p4);
    vertices.push_back(p5);
    vertices.push_back(p0);
    vertices.push_back(p1);

    normals.push_back(upFaceNormal);
    normals.push_back(upFaceNormal);
    normals.push_back(upFaceNormal);
    normals.push_back(upFaceNormal);

    for (GLuint j = 0; j < 6; ++j) {
      indexes.push_back(idxes[j] + 4 * 4 + i * 24);
    }

    vertices.push_back(p6);
    vertices.push_back(p7);
    vertices.push_back(p4);
    vertices.push_back(p5);

    normals.push_back(frontFaceNormal);
    normals.push_back(frontFaceNormal);
    normals.push_back(frontFaceNormal);
    normals.push_back(frontFaceNormal);

    for (GLuint j = 0; j < 6; ++j) {
      indexes.push_back(idxes[j] + 5 * 4 + i * 24);
    }

    if (cubeColors) {
      for (size_t j = 0; j < 24; ++j)  {
        colors.push_back(cubeColors->at(i));
      }
    }
  }

  cubes.m_vertices.swap(vertices);
  cubes.m_normals.swap(normals);
  cubes.m_indices.swap(indexes);
  cubes.m_colors.swap(colors);
  return cubes;
}

ZMesh ZMesh::CreateCube(const glm::vec3& coordLlf, const glm::vec3& coordUrb,
                        const glm::vec3& texLlf, const glm::vec3& texUrb)
{
  ZMesh cube(GL_TRIANGLE_STRIP);
  std::vector<glm::vec3> vertices;
  std::vector<glm::vec3> texCoords;
  vertices.emplace_back(coordLlf[0], coordLlf[1], coordUrb[2]);
  vertices.emplace_back(coordUrb[0], coordLlf[1], coordUrb[2]);
  vertices.emplace_back(coordLlf[0], coordUrb[1], coordUrb[2]);
  vertices.emplace_back(coordUrb[0], coordUrb[1], coordUrb[2]);
  vertices.emplace_back(coordLlf[0], coordLlf[1], coordLlf[2]);
  vertices.emplace_back(coordUrb[0], coordLlf[1], coordLlf[2]);
  vertices.emplace_back(coordLlf[0], coordUrb[1], coordLlf[2]);
  vertices.emplace_back(coordUrb[0], coordUrb[1], coordLlf[2]);

  texCoords.emplace_back(texLlf[0], texLlf[1], texUrb[2]);
  texCoords.emplace_back(texUrb[0], texLlf[1], texUrb[2]);
  texCoords.emplace_back(texLlf[0], texUrb[1], texUrb[2]);
  texCoords.emplace_back(texUrb[0], texUrb[1], texUrb[2]);
  texCoords.emplace_back(texLlf[0], texLlf[1], texLlf[2]);
  texCoords.emplace_back(texUrb[0], texLlf[1], texLlf[2]);
  texCoords.emplace_back(texLlf[0], texUrb[1], texLlf[2]);
  texCoords.emplace_back(texUrb[0], texUrb[1], texLlf[2]);

  GLuint idxes[14] = {0, 1, 2, 3, 7, 1, 5, 4, 7, 6, 2, 4, 0, 1};
  std::vector<GLuint> indexes(idxes, idxes + 14);
  cube.m_vertices.swap(vertices);
  cube.m_3DTextureCoordinates.swap(texCoords);
  cube.m_indices.swap(indexes);
  return cube;
}

ZMesh ZMesh::CreateCubeSlice(float coordIn3rdDim, float texCoordIn3rdDim, int alongDim,
                             const glm::vec2& coordlow, const glm::vec2& coordhigh,
                             const glm::vec2& texlow, const glm::vec2& texhigh)
{
  ZMesh quad(GL_TRIANGLE_STRIP);
  std::vector<glm::vec3> vertices;
  std::vector<glm::vec3> texCoords;
  if (alongDim == 0) {
    vertices.emplace_back(coordIn3rdDim, coordlow[0], coordlow[1]);
    vertices.emplace_back(coordIn3rdDim, coordhigh[0], coordlow[1]);
    vertices.emplace_back(coordIn3rdDim, coordlow[0], coordhigh[1]);
    vertices.emplace_back(coordIn3rdDim, coordhigh[0], coordhigh[1]);
    texCoords.emplace_back(texCoordIn3rdDim, texlow[0], texlow[1]);
    texCoords.emplace_back(texCoordIn3rdDim, texhigh[0], texlow[1]);
    texCoords.emplace_back(texCoordIn3rdDim, texlow[0], texhigh[1]);
    texCoords.emplace_back(texCoordIn3rdDim, texhigh[0], texhigh[1]);
  } else if (alongDim == 1) {
    vertices.emplace_back(coordlow[0], coordIn3rdDim, coordlow[1]);
    vertices.emplace_back(coordhigh[0], coordIn3rdDim, coordlow[1]);
    vertices.emplace_back(coordlow[0], coordIn3rdDim, coordhigh[1]);
    vertices.emplace_back(coordhigh[0], coordIn3rdDim, coordhigh[1]);
    texCoords.emplace_back(texlow[0], texCoordIn3rdDim, texlow[1]);
    texCoords.emplace_back(texhigh[0], texCoordIn3rdDim, texlow[1]);
    texCoords.emplace_back(texlow[0], texCoordIn3rdDim, texhigh[1]);
    texCoords.emplace_back(texhigh[0], texCoordIn3rdDim, texhigh[1]);
  } else if (alongDim == 2) {
    vertices.emplace_back(coordlow[0], coordlow[1], coordIn3rdDim);
    vertices.emplace_back(coordhigh[0], coordlow[1], coordIn3rdDim);
    vertices.emplace_back(coordlow[0], coordhigh[1], coordIn3rdDim);
    vertices.emplace_back(coordhigh[0], coordhigh[1], coordIn3rdDim);
    texCoords.emplace_back(texlow[0], texlow[1], texCoordIn3rdDim);
    texCoords.emplace_back(texhigh[0], texlow[1], texCoordIn3rdDim);
    texCoords.emplace_back(texlow[0], texhigh[1], texCoordIn3rdDim);
    texCoords.emplace_back(texhigh[0], texhigh[1], texCoordIn3rdDim);
  }

  quad.m_vertices.swap(vertices);
  quad.m_3DTextureCoordinates.swap(texCoords);
  return quad;
}

ZMesh ZMesh::CreateCubeSliceWith2DTexture(float coordIn3rdDim, int alongDim,
                                          const glm::vec2& coordlow, const glm::vec2& coordhigh,
                                          const glm::vec2& texlow, const glm::vec2& texhigh)
{
  ZMesh quad(GL_TRIANGLE_STRIP);
  std::vector<glm::vec3> vertices;
  std::vector<glm::vec2> texCoords;
  if (alongDim == 0) {
    vertices.emplace_back(coordIn3rdDim, coordlow[0], coordlow[1]);
    vertices.emplace_back(coordIn3rdDim, coordhigh[0], coordlow[1]);
    vertices.emplace_back(coordIn3rdDim, coordlow[0], coordhigh[1]);
    vertices.emplace_back(coordIn3rdDim, coordhigh[0], coordhigh[1]);
  } else if (alongDim == 1) {
    vertices.emplace_back(coordlow[0], coordIn3rdDim, coordlow[1]);
    vertices.emplace_back(coordhigh[0], coordIn3rdDim, coordlow[1]);
    vertices.emplace_back(coordlow[0], coordIn3rdDim, coordhigh[1]);
    vertices.emplace_back(coordhigh[0], coordIn3rdDim, coordhigh[1]);
  } else if (alongDim == 2) {
    vertices.emplace_back(coordlow[0], coordlow[1], coordIn3rdDim);
    vertices.emplace_back(coordhigh[0], coordlow[1], coordIn3rdDim);
    vertices.emplace_back(coordlow[0], coordhigh[1], coordIn3rdDim);
    vertices.emplace_back(coordhigh[0], coordhigh[1], coordIn3rdDim);
  }
  texCoords.emplace_back(texlow[0], texlow[1]);
  texCoords.emplace_back(texhigh[0], texlow[1]);
  texCoords.emplace_back(texlow[0], texhigh[1]);
  texCoords.emplace_back(texhigh[0], texhigh[1]);

  quad.m_vertices.swap(vertices);
  quad.m_2DTextureCoordinates.swap(texCoords);
  return quad;
}

ZMesh ZMesh::CreateImageSlice(float coordIn3rdDim, const glm::vec2& coordlow,
                              const glm::vec2& coordhigh, const glm::vec2& texlow, const glm::vec2& texhigh)
{
  ZMesh quad(GL_TRIANGLE_STRIP);
  std::vector<glm::vec3> vertices;
  std::vector<glm::vec2> texCoords;

  vertices.emplace_back(coordlow[0], coordlow[1], coordIn3rdDim);
  vertices.emplace_back(coordhigh[0], coordlow[1], coordIn3rdDim);
  vertices.emplace_back(coordlow[0], coordhigh[1], coordIn3rdDim);
  vertices.emplace_back(coordhigh[0], coordhigh[1], coordIn3rdDim);
  texCoords.emplace_back(texlow[0], texlow[1]);
  texCoords.emplace_back(texhigh[0], texlow[1]);
  texCoords.emplace_back(texlow[0], texhigh[1]);
  texCoords.emplace_back(texhigh[0], texhigh[1]);

  quad.m_vertices.swap(vertices);
  quad.m_2DTextureCoordinates.swap(texCoords);
  return quad;
}

ZMesh ZMesh::CreateCubeSerieSlices(int numSlices, int alongDim, const glm::vec3& coordfirst,
                                   const glm::vec3& coordlast, const glm::vec3& texfirst, const glm::vec3& texlast)
{
  ZMesh quad(GL_TRIANGLES);
  std::vector<glm::vec3> vertices;
  std::vector<glm::vec3> texCoords;
  std::vector<GLuint> indexes;
  GLuint idx[6] = {0, 1, 2, 2, 1, 3};

  bool reverse = true;
  if (alongDim == 0 && coordfirst.x > coordlast.x)
    reverse = false;
  if (alongDim == 1 && coordfirst.y > coordlast.y)
    reverse = false;
  if (alongDim == 2 && coordfirst.z > coordlast.z)
    reverse = false;

  for (int i = 0; i < numSlices; ++i) {
    float factor = 0.f;
    if (numSlices > 1)
      factor = i / (numSlices - 1.0);
    if (alongDim == 0) {
      vertices.emplace_back(glm::mix(coordfirst.x, coordlast.x, factor), coordfirst[1], coordfirst[2]);
      vertices.emplace_back(glm::mix(coordfirst.x, coordlast.x, factor), coordlast[1], coordfirst[2]);
      vertices.emplace_back(glm::mix(coordfirst.x, coordlast.x, factor), coordfirst[1], coordlast[2]);
      vertices.emplace_back(glm::mix(coordfirst.x, coordlast.x, factor), coordlast[1], coordlast[2]);
      texCoords.emplace_back(glm::mix(texfirst.x, texlast.x, factor), texfirst[1], texfirst[2]);
      texCoords.emplace_back(glm::mix(texfirst.x, texlast.x, factor), texlast[1], texfirst[2]);
      texCoords.emplace_back(glm::mix(texfirst.x, texlast.x, factor), texfirst[1], texlast[2]);
      texCoords.emplace_back(glm::mix(texfirst.x, texlast.x, factor), texlast[1], texlast[2]);
    } else if (alongDim == 1) {
      vertices.emplace_back(coordfirst[0], glm::mix(coordfirst.y, coordlast.y, factor), coordfirst[2]);
      vertices.emplace_back(coordlast[0], glm::mix(coordfirst.y, coordlast.y, factor), coordfirst[2]);
      vertices.emplace_back(coordfirst[0], glm::mix(coordfirst.y, coordlast.y, factor), coordlast[2]);
      vertices.emplace_back(coordlast[0], glm::mix(coordfirst.y, coordlast.y, factor), coordlast[2]);
      texCoords.emplace_back(texfirst[0], glm::mix(texfirst.y, texlast.y, factor), texfirst[2]);
      texCoords.emplace_back(texlast[0], glm::mix(texfirst.y, texlast.y, factor), texfirst[2]);
      texCoords.emplace_back(texfirst[0], glm::mix(texfirst.y, texlast.y, factor), texlast[2]);
      texCoords.emplace_back(texlast[0], glm::mix(texfirst.y, texlast.y, factor), texlast[2]);
    } else {
      vertices.emplace_back(coordfirst[0], coordfirst[1], glm::mix(coordfirst.z, coordlast.z, factor));
      vertices.emplace_back(coordlast[0], coordfirst[1], glm::mix(coordfirst.z, coordlast.z, factor));
      vertices.emplace_back(coordfirst[0], coordlast[1], glm::mix(coordfirst.z, coordlast.z, factor));
      vertices.emplace_back(coordlast[0], coordlast[1], glm::mix(coordfirst.z, coordlast.z, factor));
      texCoords.emplace_back(texfirst[0], texfirst[1], glm::mix(texfirst.z, texlast.z, factor));
      texCoords.emplace_back(texlast[0], texfirst[1], glm::mix(texfirst.z, texlast.z, factor));
      texCoords.emplace_back(texfirst[0], texlast[1], glm::mix(texfirst.z, texlast.z, factor));
      texCoords.emplace_back(texlast[0], texlast[1], glm::mix(texfirst.z, texlast.z, factor));
    }
    for (int j = 0; j < 6; ++j) {
      if (reverse)
        indexes.push_back(idx[5 - j] + i * 4);
      else
        indexes.push_back(idx[j] + i * 4);
    }
  }

  quad.m_vertices.swap(vertices);
  quad.m_3DTextureCoordinates.swap(texCoords);
  quad.m_indices.swap(indexes);
  return quad;
}

ZMesh ZMesh::CreateSphereMesh(const glm::vec3& center, float radius,
                              int thetaResolution, int phiResolution,
                              float startTheta, float endTheta,
                              float startPhi, float endPhi)
{
  vtkSmartPointer<vtkSphereSource> sphereSource = vtkSmartPointer<vtkSphereSource>::New();

  sphereSource->SetCenter(center.x, center.y, center.z);
  sphereSource->SetRadius(radius);
  sphereSource->SetThetaResolution(thetaResolution);
  sphereSource->SetPhiResolution(phiResolution);
  sphereSource->SetStartTheta(startTheta);
  sphereSource->SetEndTheta(endTheta);
  sphereSource->SetStartPhi(startPhi);
  sphereSource->SetEndPhi(endPhi);
  sphereSource->SetLatLongTessellation(false);
  sphereSource->SetOutputPointsPrecision(vtkAlgorithm::SINGLE_PRECISION);

  sphereSource->Update();
  return vtkPolyDataToMesh(sphereSource->GetOutput());
}

ZMesh ZMesh::CreateTubeMesh(const std::vector<glm::vec3>& line, const std::vector<float>& radius,
                            int numberOfSides, bool capping)
{
  CHECK(line.size() == radius.size());
  vtkSmartPointer<vtkPoints> points = vtkSmartPointer<vtkPoints>::New();
  points->SetDataType(VTK_FLOAT);
  for (size_t i = 0; i < line.size(); ++i) {
    points->InsertPoint(i, line[i].x, line[i].y, line[i].z);
  }

  vtkSmartPointer<vtkCellArray> lines = vtkSmartPointer<vtkCellArray>::New();
  lines->InsertNextCell(line.size());
  for (size_t i = 0; i < line.size(); ++i) {
    lines->InsertCellPoint(i);
  }

  vtkSmartPointer<vtkFloatArray> tubeRadius = vtkSmartPointer<vtkFloatArray>::New();
  tubeRadius->SetName("TubeRadius");
  tubeRadius->SetNumberOfTuples(radius.size());
  for (size_t i = 0; i < radius.size(); ++i) {
    tubeRadius->SetTuple1(i, radius[i]);
  }

  vtkSmartPointer<vtkPolyData> polyData = vtkSmartPointer<vtkPolyData>::New();
  polyData->SetPoints(points);
  polyData->SetLines(lines);
  polyData->GetPointData()->AddArray(tubeRadius);
  polyData->GetPointData()->SetActiveScalars("TubeRadius");

  vtkSmartPointer<vtkTubeFilter> tubeFilter = vtkSmartPointer<vtkTubeFilter>::New();

  tubeFilter->SetInputData(polyData);
  tubeFilter->SetNumberOfSides(numberOfSides);
  tubeFilter->SetCapping(capping);
  tubeFilter->SetSidesShareVertices(true);
  tubeFilter->SetVaryRadiusToVaryRadiusByAbsoluteScalar();

  vtkSmartPointer<vtkTriangleFilter> triangleFilter = vtkSmartPointer<vtkTriangleFilter>::New();
  triangleFilter->SetInputConnection(tubeFilter->GetOutputPort());

  triangleFilter->Update();
  return vtkPolyDataToMesh(triangleFilter->GetOutput());
}

ZMesh ZMesh::CreateConeMesh(glm::vec3 base, float baseRadius, glm::vec3 top, float topRadius,
                            int numberOfSides, bool capping)
{
  CHECK(baseRadius >= 0 && topRadius >= 0 && numberOfSides > 2);
  if (baseRadius > topRadius) {
    std::swap(base, top);
    std::swap(baseRadius, topRadius);
  }
  glm::vec3 axis = glm::normalize(top - base);
  // vector perpendicular to axis
  glm::vec3 v1, v2;
  glm::getOrthogonalVectors(axis, v1, v2);

  std::vector<glm::vec3> vertices;
  std::vector<glm::vec3> normals;
  std::vector<GLuint> triangles;

  using namespace boost::math::float_constants;
  float theta = two_pi / numberOfSides;

  if (baseRadius == 0) {
    vertices.push_back(base);
    normals.push_back(-axis);
    for (int i = 0; i < numberOfSides; ++i) {
      glm::vec3 normal = v1 * std::cos(i * theta) + v2 * std::sin(i * theta);
      glm::vec3 tpt = top + topRadius * normal;
      glm::vec3 edge = tpt - base;
      normal = glm::normalize(glm::cross(edge, glm::cross(tpt - top, edge)));
      vertices.push_back(tpt);
      normals.push_back(normal);
    }

    size_t numVertices = vertices.size();
    for (size_t i = 1; i < numVertices; ++i) {
      triangles.push_back(0);
      triangles.push_back(i);
      triangles.push_back((i + 1) < numVertices ? (i + 1) : 1);
    }

    if (capping) {
      for (int i = 0; i < numberOfSides; ++i) {
        glm::vec3 normal = v1 * std::cos(i * theta) + v2 * std::sin(i * theta);
        glm::vec3 tpt = top + topRadius * normal;
        vertices.push_back(tpt);
        normals.push_back(axis);
      }
      // top cap
      for (size_t i = numVertices + 1; i < numVertices + numberOfSides - 1; ++i) {
        triangles.push_back(numVertices);
        triangles.push_back(i);
        triangles.push_back(i + 1);
      }
    }
  } else {
    for (int i = 0; i < numberOfSides; ++i) {
      glm::vec3 normal = v1 * std::cos(i * theta) + v2 * std::sin(i * theta);
      glm::vec3 bpt = base + baseRadius * normal;
      glm::vec3 tpt = top + topRadius * normal;
      if (topRadius != baseRadius) {
        glm::vec3 edge = tpt - bpt;
        normal = glm::normalize(glm::cross(edge, glm::cross(tpt - top, edge)));
      }
      vertices.push_back(tpt);
      normals.push_back(normal);
      vertices.push_back(bpt);
      normals.push_back(normal);
    }

    size_t numVertices = vertices.size();
    for (size_t i = 0; i < numVertices; i += 2) {
      triangles.push_back(i);
      triangles.push_back(i + 1);
      triangles.push_back((i + 3) % numVertices);

      triangles.push_back(i);
      triangles.push_back((i + 3) % numVertices);
      triangles.push_back((i + 2) % numVertices);
    }

    if (capping) {
      for (int i = 0; i < numberOfSides; ++i) {
        glm::vec3 normal = v1 * std::cos(i * theta) + v2 * std::sin(i * theta);
        glm::vec3 tpt = top + topRadius * normal;
        vertices.push_back(tpt);
        normals.push_back(axis);
      }
      for (int i = 0; i < numberOfSides; ++i) {
        glm::vec3 normal = v1 * std::cos(i * theta) + v2 * std::sin(i * theta);
        glm::vec3 bpt = base + baseRadius * normal;
        vertices.push_back(bpt);
        normals.push_back(-axis);
      }
      // top cap
      for (size_t i = numVertices + 1; i < numVertices + numberOfSides - 1; ++i) {
        triangles.push_back(numVertices);
        triangles.push_back(i);
        triangles.push_back(i + 1);
      }
      // bot cap
      numVertices += numberOfSides;
      for (size_t i = numVertices + 1; i < numVertices + numberOfSides - 1; ++i) {
        triangles.push_back(numVertices);
        triangles.push_back(i);
        triangles.push_back(i + 1);
      }
    }
  }

  ZMesh msh;
  msh.setVertices(vertices);
  msh.setIndices(triangles);
  msh.setNormals(normals);
  return msh;
}

ZMesh ZMesh::FromZCubeArray(const ZCubeArray& ca)
{
  if (!ca.isEmpty()) {
    ZMesh mesh((*ca.getMesh()));

    return mesh;
  }

  return ZMesh();
#if 0
  std::vector<glm::vec3> coordLlfs;
  std::vector<glm::vec3> coordUrbs;
  std::vector<glm::vec4> cubeColors;
  std::vector<std::vector<bool> > faceVisibility;

  for (const Z3DCube& cube : ca.getCubeArray()) {
    if (cube.hasVisibleFace()) {
      coordLlfs.push_back(cube.nodes[0]);
      coordUrbs.push_back(cube.nodes[7]);
      cubeColors.push_back(cube.color);
      faceVisibility.push_back(cube.getFaceVisiblity());
    }
  }

  return CreateCubesWithNormal(
        coordLlfs, coordUrbs, faceVisibility, &cubeColors);
#endif
}

ZMesh ZMesh::Merge(const std::vector<ZMesh>& meshes)
{
  ZMesh res;
  if (meshes.empty())
    return res;

  vtkSmartPointer<vtkAppendPolyData> appendFilter =
      vtkSmartPointer<vtkAppendPolyData>::New();
  std::vector<vtkSmartPointer<vtkPolyData>> polys(meshes.size());
  for (size_t i = 0; i < meshes.size(); ++i) {
    polys[i] = meshToVtkPolyData(meshes[i]);
    appendFilter->AddInputData(polys[i]);
  }

  vtkSmartPointer<vtkCleanPolyData> cleanFilter =
      vtkSmartPointer<vtkCleanPolyData>::New();
  cleanFilter->SetInputConnection(appendFilter->GetOutputPort());

  cleanFilter->Update();
  return vtkPolyDataToMesh(cleanFilter->GetOutput());
}

ZMesh ZMesh::Merge(const std::vector<ZMesh*>& meshes)
{
  ZMesh res;
  if (meshes.empty())
    return res;

  vtkSmartPointer<vtkAppendPolyData> appendFilter =
      vtkSmartPointer<vtkAppendPolyData>::New();
  std::vector<vtkSmartPointer<vtkPolyData>> polys(meshes.size());
  for (size_t i = 0; i < meshes.size(); ++i) {
    polys[i] = meshToVtkPolyData(*meshes[i]);
    appendFilter->AddInputData(polys[i]);
  }

  vtkSmartPointer<vtkCleanPolyData> cleanFilter =
      vtkSmartPointer<vtkCleanPolyData>::New();
  cleanFilter->SetInputConnection(appendFilter->GetOutputPort());

  cleanFilter->Update();
  return vtkPolyDataToMesh(cleanFilter->GetOutput());
}

void ZMesh::append(const ZMesh &mesh)
{
#ifdef _DEBUG_
  std::cout << "Appending mesh:" << m_indices.size() << " " << mesh.m_indices.size() << std::endl;
#endif

  if (m_ttype == GL_TRIANGLES && mesh.m_ttype == GL_TRIANGLES) {
    size_t count = m_vertices.size();

    m_vertices.insert(
          m_vertices.end(), mesh.m_vertices.begin(), mesh.m_vertices.end());
    std::vector<GLuint> newIndices = mesh.m_indices;
    for (auto &index : newIndices) {
      index += count;
    }
    m_indices.insert(m_indices.end(), newIndices.begin(), newIndices.end());

    m_1DTextureCoordinates.insert(
          m_1DTextureCoordinates.end(), mesh.m_1DTextureCoordinates.begin(),
          mesh.m_1DTextureCoordinates.end());
    m_2DTextureCoordinates.insert(
          m_2DTextureCoordinates.end(), mesh.m_2DTextureCoordinates.begin(),
          mesh.m_2DTextureCoordinates.end());
    m_3DTextureCoordinates.insert(
          m_3DTextureCoordinates.end(), mesh.m_3DTextureCoordinates.begin(),
          mesh.m_3DTextureCoordinates.end());
    m_normals.insert(
          m_normals.end(), mesh.m_normals.begin(), mesh.m_normals.end());
    m_colors.insert(
          m_colors.end(), mesh.m_colors.begin(), mesh.m_colors.end());

  } else {
    std::vector<glm::uvec3> indices = mesh.triangleIndices();
    for (const auto &index : indices) {
      appendTriangle(mesh, index);
    }
  }
}

void ZMesh::appendTriangle(const ZMesh& mesh, const glm::uvec3& triangle)
{
  if (/*!m_indices.empty() ||*/ m_ttype != GL_TRIANGLES)
    return;

  if (!m_indices.empty()) {
    m_indices.push_back(m_vertices.size());
    m_indices.push_back(m_vertices.size() + 1);
    m_indices.push_back(m_vertices.size() + 2);
  }

  m_vertices.push_back(mesh.m_vertices[triangle[0]]);
  m_vertices.push_back(mesh.m_vertices[triangle[1]]);
  m_vertices.push_back(mesh.m_vertices[triangle[2]]);

  validateObbTree(false);

  if (mesh.num1DTextureCoordinates() > 0) {
    m_1DTextureCoordinates.push_back(mesh.m_1DTextureCoordinates[triangle[0]]);
    m_1DTextureCoordinates.push_back(mesh.m_1DTextureCoordinates[triangle[1]]);
    m_1DTextureCoordinates.push_back(mesh.m_1DTextureCoordinates[triangle[2]]);
  }

  if (mesh.num2DTextureCoordinates() > 0) {
    m_2DTextureCoordinates.push_back(mesh.m_2DTextureCoordinates[triangle[0]]);
    m_2DTextureCoordinates.push_back(mesh.m_2DTextureCoordinates[triangle[1]]);
    m_2DTextureCoordinates.push_back(mesh.m_2DTextureCoordinates[triangle[2]]);
  }

  if (mesh.num3DTextureCoordinates() > 0) {
    m_3DTextureCoordinates.push_back(mesh.m_3DTextureCoordinates[triangle[0]]);
    m_3DTextureCoordinates.push_back(mesh.m_3DTextureCoordinates[triangle[1]]);
    m_3DTextureCoordinates.push_back(mesh.m_3DTextureCoordinates[triangle[2]]);
  }

  if (mesh.numNormals() > 0) {
    m_normals.push_back(mesh.m_normals[triangle[0]]);
    m_normals.push_back(mesh.m_normals[triangle[1]]);
    m_normals.push_back(mesh.m_normals[triangle[2]]);
  }

  if (mesh.numColors() > 0) {
    m_colors.push_back(mesh.m_colors[triangle[0]]);
    m_colors.push_back(mesh.m_colors[triangle[1]]);
    m_colors.push_back(mesh.m_colors[triangle[2]]);
  }
}

double ZMesh::signedVolumeOfTriangle(const glm::vec3& v1, const glm::vec3& v2, const glm::vec3& v3) const
{
#if 1
  double v321 = v3.x * v2.y * v1.z;
  double v231 = v2.x * v3.y * v1.z;
  double v312 = v3.x * v1.y * v2.z;
  double v132 = v1.x * v3.y * v2.z;
  double v213 = v2.x * v1.y * v3.z;
  double v123 = v1.x * v2.y * v3.z;
  return (1.0 / 6.0) * (-v321 + v231 + v312 - v132 - v213 + v123);
#else
  return glm::dot(glm::dvec3(v1), glm::cross(glm::dvec3(v2), glm::dvec3(v3))) / 6.0;
#endif
}

size_t ZMesh::numCoverCubes(double cubeEdgeLength)
{
  float minx = std::numeric_limits<float>::max();
  float maxx = std::numeric_limits<float>::lowest();
  float miny = minx;
  float maxy = maxx;
  float minz = minx;
  float maxz = maxx;
  for (size_t i = 0; i < m_vertices.size(); ++i) {
    minx = std::min(minx, m_vertices[i].x);
    maxx = std::max(maxx, m_vertices[i].x);
    miny = std::min(miny, m_vertices[i].y);
    maxy = std::max(maxy, m_vertices[i].y);
    minz = std::min(minz, m_vertices[i].z);
    maxz = std::max(maxz, m_vertices[i].z);
  }
  int xdim = std::ceil((maxx - minx) / cubeEdgeLength);
  int ydim = std::ceil((maxy - miny) / cubeEdgeLength);
  int zdim = std::ceil((maxz - minz) / cubeEdgeLength);
  std::vector<ZBBox<glm::vec3>> boxes;
  std::vector<int> numPts;
  for (int x = 0; x < xdim; ++x) {
    for (int y = 0; y < ydim; ++y) {
      for (int z = 0; z < zdim; ++z) {
        glm::vec3 minCoord(minx + x * cubeEdgeLength,
                           miny + y * cubeEdgeLength,
                           minz + z * cubeEdgeLength);
        glm::vec3 maxCoord = minCoord + glm::vec3(cubeEdgeLength, cubeEdgeLength, cubeEdgeLength);
        ZBBox<glm::vec3> box(minCoord, maxCoord);
        boxes.push_back(box);
        numPts.push_back(0);
      }
    }
  }
  for (size_t i = 0; i < m_vertices.size(); ++i) {
    for (size_t j = 0; j < boxes.size(); ++j) {
      if (boxes[j].contains(m_vertices[i])) {
        numPts[j] += 1;
        break;
      }
    }
  }
  size_t res = 0;
  for (size_t j = 0; j < boxes.size(); ++j) {
    if (numPts[j] > 0)
      ++res;
  }
  return res;
}

ZMesh ZMesh::booleanOperation(const ZMesh& mesh1, const ZMesh& mesh2, ZMesh::BooleanOperationType type)
{
  vtkSmartPointer<vtkPolyData> input1 = meshToVtkPolyData(mesh1);
  vtkSmartPointer<vtkPolyData> input2 = meshToVtkPolyData(mesh2);

  vtkSmartPointer<vtkBooleanOperationPolyDataFilter> booleanOperationFilter =
    vtkSmartPointer<vtkBooleanOperationPolyDataFilter>::New();

  booleanOperationFilter->SetInputData(0, input1);
  booleanOperationFilter->SetInputData(1, input2);
  switch (type) {
    case BooleanOperationType::Union:
      booleanOperationFilter->SetOperationToUnion();
      break;
    case BooleanOperationType::Intersection:
      booleanOperationFilter->SetOperationToIntersection();
      break;
    case BooleanOperationType::Difference:
      booleanOperationFilter->SetOperationToDifference();
      break;
    default:
      break;
  }
  booleanOperationFilter->SetReorientDifferenceCells(1);
  booleanOperationFilter->SetTolerance(1e-6);

  vtkSmartPointer<vtkCleanPolyData> cleanFilter = vtkSmartPointer<vtkCleanPolyData>::New();
  cleanFilter->SetInputConnection(booleanOperationFilter->GetOutputPort());

  cleanFilter->Update();
  return vtkPolyDataToMesh(cleanFilter->GetOutput());
}

void ZMesh::pushObjectColor()
{
  m_colors.resize(m_vertices.size());
#ifdef _DEBUG_2
  qDebug() << "Push mesh color:" << getColor();
#endif
  for (size_t i = 0; i < m_colors.size(); ++i) {
    m_colors[i] = glm::vec4(getColor().redF(), getColor().greenF(),
                            getColor().blueF(), 1.0);
  }
}

void ZMesh::swapXZ()
{
  for (glm::vec3 &vertex : m_vertices) {
    std::swap(vertex[0], vertex[2]);
  }

  if (m_ttype == GL_TRIANGLES) {
    if (!m_indices.empty()) {
      for (size_t i = 0; i < m_indices.size() - 2; i += 3) {
        std::swap(m_indices[i], m_indices[i + 2]);
      }
    } else {
      for (size_t i = 0; i < m_vertices.size() - 2; i += 3) {
        std::swap(m_vertices[i], m_vertices[i + 2]);
      }
    }
  }
  validateObbTree(false);

  m_normals.clear();
  generateNormals();

//  for (glm::vec3 &normal : m_normals) {
//    std::swap(normal[0], normal[2]);
////    normal[0] = -normal[0];
////    normal[1] = -normal[1];
////    normal[2] = -normal[2];
//  }
}

void ZMesh::translate(double x, double y, double z)
{
  for (glm::vec3 &vertex : m_vertices) {
    vertex[0] += x;
    vertex[1] += y;
    vertex[2] += z;
  }
  validateObbTree(false);
}

void ZMesh::scale(double sx, double sy, double sz)
{
  for (glm::vec3 &vertex : m_vertices) {
    vertex[0] *= sx;
    vertex[1] *= sy;
    vertex[2] *= sz;
  }
  validateObbTree(false);
}

vtkSmartPointer<vtkOBBTree> ZMesh::getObbTree() const
{
  if (!isObbTreeValid()) {
    m_obbTreeData.m_obbTree = vtkSmartPointer<vtkOBBTree>::New();
    vtkSmartPointer<vtkPolyData> poly = meshToVtkPolyData(*this);
    m_obbTreeData.m_obbTree->SetDataSet(poly);
    m_obbTreeData.m_obbTree->BuildLocator();
    validateObbTree(true);
  }

  return m_obbTreeData.m_obbTree;
}
std::vector<ZPoint> ZMesh::intersectLineSeg(
    const ZPoint &start, const ZPoint &end) const
{
  vtkSmartPointer<vtkOBBTree> obbTree = getObbTree();

  vtkSmartPointer<vtkPoints> points = vtkSmartPointer<vtkPoints>::New();
  double a0[3] = {start.getX(), start.getY(), start.getZ()};
  double a1[3] = {end.getX(), end.getY(), end.getZ()};
  obbTree->IntersectWithLine(a0, a1, points, NULL);
#ifdef _DEBUG_
  std::cout << "Intersect start: " << a0[0] << " " << a0[1] << " " << a0[2] << std::endl;
  std::cout << "Intersect end: " << a1[0] << " " << a1[1] << " " << a1[2] << std::endl;
#endif
  std::vector<ZPoint> result;
  int n = points->GetNumberOfPoints();
#ifdef _DEBUG_
  std::cout << n << " intersections" << std::endl;
#endif
  double point[3] = {0, 0, 0};
  for (int i = 0; i < n; ++i) {
    points->GetPoint(i, point);
    result.emplace_back(point[0], point[1], point[2]);
  }


  return result;
}


ZSTACKOBJECT_DEFINE_CLASS_NAME(ZMesh)
