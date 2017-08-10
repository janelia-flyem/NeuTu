#ifndef ZMESH_H
#define ZMESH_H

#include "z3dgl.h"
#include "zbbox.h"
#include "zstackobject.h"
#include "QsLog.h"
#include <vector>

struct ZMeshProperties
{
  double surfaceArea = -1.0;
  double minTriangleArea = -1.0;
  double maxTriangleArea = -1.0;
  double volume = -1.0;
  // Typically you should compare this volume to the value returned by GetVolume
  // if you get an error (GetVolume()-GetVolumeProjected())*10000 that is greater
  // than GetVolume() this should identify a problem:
  // * Either the polydata is not closed
  // * Or the polydata contains triangle that are flipped
  double volumeProjected = -1.0; // == Projected area of triangles * average z values
  // volume projected on to each axis aligned plane.
  double volumeX = -1.0;
  double volumeY = -1.0;
  double volumeZ = -1.0;
  // weighting factors for the maximum unit normal component (MUNC)
  double kx = -1.0;
  double ky = -1.0;
  double kz = -1.0;
  // This characterizes the
  // deviation of the shape of an object from a sphere. A sphere's NSI
  // is one. This number is always >= 1.0.
  double normalizedShapeIndex = -1.0;

  size_t numVertices = 0;
  size_t numTriangles = 0;
};

class ZMesh : public ZStackObject
{
public:
  // one of GL_TRIANGLES, GL_TRIANGLE_STRIP and GL_TRIANGLE_FAN
  explicit ZMesh(GLenum type = GL_TRIANGLES);

  // might throw ZIOException
  explicit ZMesh(const QString& filename);

  ZMesh(ZMesh&&) = default;

  ZMesh& operator=(ZMesh&&) = default;

  ZMesh(const ZMesh&) = default;

  ZMesh& operator=(const ZMesh&) = default;

  void swap(ZMesh& rhs) noexcept;

  virtual const std::string& className() const override;

  virtual void display(
      ZPainter &, int , EDisplayStyle ,
      NeuTube::EAxis ) const override
  {}

  // qt style read write name filter for filedialog
  static bool canReadFile(const QString& filename);

  static bool canWriteFile(const QString& filename);

  static const QString& getQtReadNameFilter();

  static void getQtWriteNameFilter(QStringList& filters, QList<std::string>& formats);

  // might throw ZIOException
  void load(const QString& filename);

  void save(const QString& filename, const std::string& format = "") const;

  ZBBox<glm::dvec3> boundBox() const;

  ZBBox<glm::dvec3> boundBox(const glm::mat4& transform) const;

  using ZStackObject::boundBox;

  GLenum type() const
  { return m_type; }

  QString typeAsString() const;

  void setType(GLenum type)
  {
    m_type = type;
    CHECK(m_type == GL_TRIANGLES || m_type == GL_TRIANGLE_FAN || m_type == GL_TRIANGLE_STRIP);
  }

  const std::vector<glm::vec3>& vertices() const
  { return m_vertices; }

  void setVertices(const std::vector<glm::vec3>& vertices)
  { m_vertices = vertices; }

  std::vector<glm::dvec3> doubleVertices() const;

  void setVertices(const std::vector<glm::dvec3>& vertices);

  const std::vector<float>& textureCoordinates1D() const
  { return m_1DTextureCoordinates; }

  void setTextureCoordinates(const std::vector<float>& textureCoordinates)
  { m_1DTextureCoordinates = textureCoordinates; }

  const std::vector<glm::vec2>& textureCoordinates2D() const
  { return m_2DTextureCoordinates; }

  void setTextureCoordinates(const std::vector<glm::vec2>& textureCoordinates)
  { m_2DTextureCoordinates = textureCoordinates; }

  const std::vector<glm::vec3>& textureCoordinates3D() const
  { return m_3DTextureCoordinates; }

  void setTextureCoordinates(const std::vector<glm::vec3>& textureCoordinates)
  { m_3DTextureCoordinates = textureCoordinates; }

  const std::vector<glm::vec3>& normals() const
  { return m_normals; }

  void setNormals(const std::vector<glm::vec3>& normals)
  { m_normals = normals; }

  void setNormals(const std::vector<glm::dvec3>& normals);

  const std::vector<glm::vec4>& colors() const
  { return m_colors; }

  void setColors(const std::vector<glm::vec4>& colors)
  { m_colors = colors; }

  const std::vector<GLuint>& indices() const
  { return m_indices; }

  void setIndices(const std::vector<GLuint>& indices)
  { m_indices = indices; }

  bool hasIndices() const
  { return !m_indices.empty(); }

  // use ref to interpolate texture coordinate and colors. all vertices should be on ref surface
  void interpolate(const ZMesh& ref);

  // return true if no vertex
  bool empty() const
  { return m_vertices.empty(); }

  void clear();

  size_t numVertices() const
  { return m_vertices.size(); }

  size_t numTriangles() const;

  size_t numColors() const
  { return m_colors.size(); }

  size_t numNormals() const
  { return m_normals.size(); }

  size_t num1DTextureCoordinates() const
  { return m_1DTextureCoordinates.size(); }

  size_t num2DTextureCoordinates() const
  { return m_2DTextureCoordinates.size(); }

  size_t num3DTextureCoordinates() const
  { return m_3DTextureCoordinates.size(); }

  std::vector<glm::vec3> triangleVertices(size_t index) const;

  std::vector<glm::uvec3> triangleIndices() const;

  glm::uvec3 triangleIndices(size_t index) const;

  glm::vec3 triangleVertex(size_t triangleIndex, size_t vertexIndex) const;

  void transformVerticesByMatrix(const glm::mat4& tfmat);

  std::vector<ZMesh> split(size_t numTriangle = 100000) const;

  void generateNormals(bool useAreaWeight = true);

  //double volume() const;
  ZMeshProperties properties() const;

  void logProperties(const QString& str = "") const
  { logProperties(properties(), str); }

  static void logProperties(const ZMeshProperties& prop, const QString& str = "");

  // a list of cubes with normal
  static ZMesh createCubesWithNormal(const std::vector<glm::vec3>& coordLlfs,
                                     const std::vector<glm::vec3>& coordUrbs);

  // a cube with six surfaces
  static ZMesh createCube(
    const glm::vec3& coordLlf = glm::vec3(0.f, 0.f, 0.f),
    const glm::vec3& coordUrb = glm::vec3(1.f, 1.f, 1.f),
    const glm::vec3& texLlf = glm::vec3(0.f, 0.f, 0.f),
    const glm::vec3& texUrb = glm::vec3(1.f, 1.f, 1.f));

  // one slice from a cube, it is a x slice if alongDim == 0, a y slice if alongDim == 1,
  // a z slice if alongDim == 2
  static ZMesh createCubeSlice(
    float coordIn3rdDim,
    float texCoordIn3rdDim,
    int alongDim = 2,     // 0, 1, or 2
    const glm::vec2& coordlow = glm::vec2(0.f, 0.f),
    const glm::vec2& coordhigh = glm::vec2(1.f, 1.f),
    const glm::vec2& texlow = glm::vec2(0.f, 0.f),
    const glm::vec2& texhigh = glm::vec2(1.f, 1.f));

  // one slice from a cube, it is a x slice if alongDim == 0, a y slice if alongDim == 1,
  // a z slice if alongDim == 2
  static ZMesh createCubeSliceWith2DTexture(
    float coordIn3rdDim,
    int alongDim = 2,     // 0, 1, or 2
    const glm::vec2& coordlow = glm::vec2(0.f, 0.f),
    const glm::vec2& coordhigh = glm::vec2(1.f, 1.f),
    const glm::vec2& texlow = glm::vec2(0.f, 0.f),
    const glm::vec2& texhigh = glm::vec2(1.f, 1.f));

  // a 2d image quad with 2d texture coordinates
  static ZMesh createImageSlice(
    float coordIn3rdDim,
    const glm::vec2& coordlow = glm::vec2(0.f, 0.f),
    const glm::vec2& coordhigh = glm::vec2(1.f, 1.f),
    const glm::vec2& texlow = glm::vec2(0.f, 0.f),
    const glm::vec2& texhigh = glm::vec2(1.f, 1.f));

  // create a serie of slices from a cube, slices are cut along a specified dimension
  // if number of slices if 1, first slice will be returned (use first coordinate)
  // if number of slices is 2, first and last slices will be returned (use first and last coordinate)
  // other slices are interpolated between first and last slice
  // assume that the last slice is nearest to camera, then created triangles will face camera if first
  // coordinate is smaller than last coordinatesin the two fixed dimensions.
  // in cut dimension, last coordinate can be smaller than first coordinate to create inverse order series
  static ZMesh createCubeSerieSlices(
    int numSlices,
    int alongDim = 2,     // 0, 1, or 2
    const glm::vec3& coordfirst = glm::vec3(0.f, 0.f, 0.f),
    const glm::vec3& coordlast = glm::vec3(1.f, 1.f, 1.f),
    const glm::vec3& texfirst = glm::vec3(0.f, 0.f, 0.f),
    const glm::vec3& texlast = glm::vec3(1.f, 1.f, 1.f));

  static ZMesh createSphereMesh(const glm::vec3& center, float radius,
                                int thetaResolution = 32, int phiResolution = 32,
                                float startTheta = 0.f, float endTheta = 360.f,
                                float startPhi = 0.f, float endPhi = 180.f);

  static ZMesh createTubeMesh(const std::vector<glm::vec3>& line, const std::vector<float>& radius,
                              int numberOfSides = 32, bool capping = true);

  static ZMesh createConeMesh(glm::vec3 base, float baseRadius, glm::vec3 top, float topRadius,
                              int numberOfSides = 32, bool capping = true);

  // these functions only deal with meshes with normal, other fields (texture, color) are ignored
  static ZMesh unite(const ZMesh& mesh1, const ZMesh& mesh2)
  { return booleanOperation(mesh1, mesh2, BooleanOperationType::Union); }

  static ZMesh intersect(const ZMesh& mesh1, const ZMesh& mesh2)
  { return booleanOperation(mesh1, mesh2, BooleanOperationType::Intersection); }

  static ZMesh subtract(const ZMesh& mesh1, const ZMesh& mesh2)
  { return booleanOperation(mesh1, mesh2, BooleanOperationType::Difference); }

  static ZMesh merge(const std::vector<ZMesh>& meshes);

private:
  enum class BooleanOperationType
  {
    Union, Intersection, Difference
  };

  void appendTriangle(const ZMesh& mesh, const glm::uvec3& triangle);

  double signedVolumeOfTriangle(const glm::vec3& v1,
                                const glm::vec3& v2,
                                const glm::vec3& v3) const;

  size_t numCoverCubes(double cubeEdgeLength);

  static ZMesh booleanOperation(const ZMesh& mesh1, const ZMesh& mesh2, BooleanOperationType type);

private:
  friend class ZMeshIO;

  GLenum m_type;

  std::vector<glm::vec3> m_vertices;
  std::vector<float> m_1DTextureCoordinates;
  std::vector<glm::vec2> m_2DTextureCoordinates;
  std::vector<glm::vec3> m_3DTextureCoordinates;
  std::vector<glm::vec3> m_normals;
  std::vector<glm::vec4> m_colors;
  std::vector<GLuint> m_indices;
};

#endif // ZMESH_H
