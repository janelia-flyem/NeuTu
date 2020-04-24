#ifndef ZMESH_H
#define ZMESH_H

#include "z3dgl.h"

#include <vector>
#include <memory>
#include <vtkSmartPointer.h>

#include "zbbox.h"
#include "zstackobject.h"
//#include "QsLog.h"

class ZPoint;
class vtkOBBTree;
class ZCuboid;

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

class ZCubeArray;
//class ZIntCuboidFace;

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

//  virtual const std::string& className() const override;

  static ZStackObject::EType GetType() {
    return ZStackObject::EType::MESH;
  }

  virtual void display(
      ZPainter &, int , EDisplayStyle ,
      neutu::EAxis ) const override
  {}

//  void setLabel(uint64_t label) override;
//  uint64_t getLabel() const;

  // qt style read write name filter for filedialog
  static bool canReadFile(const QString& filename);

  static bool canWriteFile(const QString& filename);

  static const QString& getQtReadNameFilter();

  static void getQtWriteNameFilter(QStringList& filters, QList<std::string>& formats);

  // might throw ZIOException
  void load(const QString& filename);

  void save(const QString& filename, const std::string& format = "") const;
  void save(const std::string& filename, const std::string& format = "") const;
  void save(const char* filename, const std::string& format = "") const;

  QByteArray writeToMemory(const std::string &format) const;

  ZBBox<glm::dvec3> boundBox() const;

  ZBBox<glm::dvec3> boundBox(const glm::mat4& transform) const;

  ZCuboid getBoundBox() const override;

  using ZStackObject::boundBox;

  GLenum type() const
  { return m_ttype; }

  QString typeAsString() const;

  void setType(GLenum type);

  const std::vector<glm::vec3>& vertices() const
  { return m_vertices; }

  void setVertices(const std::vector<glm::vec3>& vertices)
  { m_vertices = vertices; invalidateCachedProperties();}

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
  { m_indices = indices; invalidateCachedProperties();}

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

  /*!
   * \brief Generate normals if the normals are not ready.
   */
  void prepareNormals(bool useAreaWeight = true);

  /*!
   * \brief Recompute normals.
   */
  void generateNormals(bool useAreaWeight = true);

  //double volume() const;
  ZMeshProperties properties() const;

  void logProperties(const QString& str = "") const
  { LogProperties(properties(), str); }


  void createCubesWithNormal(
      const std::vector<glm::vec3>& coordLlfs,
      const std::vector<glm::vec3>& coordUrbs,
      const std::vector<std::vector<bool> > &faceVisbility,
      const std::vector<glm::vec4>* cubeColors = nullptr);

  void createCubesWithoutNormal(
      const std::vector<glm::vec3>& coordLlfs,
      const std::vector<glm::vec3>& coordUrbs,
      const std::vector<std::vector<bool> > &faceVisbility,
      const std::vector<glm::vec4>* cubeColors = nullptr);

  static void LogProperties(const ZMeshProperties& prop, const QString& str = "");

  // a list of cubes with normal
  static ZMesh CreateCubesWithNormal(
      const std::vector<glm::vec3>& coordLlfs,
      const std::vector<glm::vec3>& coordUrbs,
      const std::vector<glm::vec4>* cubeColors = nullptr);

  static ZMesh CreateCubesWithNormal(
      const std::vector<glm::vec3>& coordLlfs,
      const std::vector<glm::vec3>& coordUrbs,
      const std::vector<std::vector<bool> > &faceVisbility,
      const std::vector<glm::vec4>* cubeColors = nullptr);

  // a cube with six surfaces
  static ZMesh CreateCube(
    const glm::vec3& coordLlf = glm::vec3(0.f, 0.f, 0.f),
    const glm::vec3& coordUrb = glm::vec3(1.f, 1.f, 1.f),
    const glm::vec3& texLlf = glm::vec3(0.f, 0.f, 0.f),
    const glm::vec3& texUrb = glm::vec3(1.f, 1.f, 1.f));

  // one slice from a cube, it is a x slice if alongDim == 0, a y slice if alongDim == 1,
  // a z slice if alongDim == 2
  static ZMesh CreateCubeSlice(
    float coordIn3rdDim,
    float texCoordIn3rdDim,
    int alongDim = 2,     // 0, 1, or 2
    const glm::vec2& coordlow = glm::vec2(0.f, 0.f),
    const glm::vec2& coordhigh = glm::vec2(1.f, 1.f),
    const glm::vec2& texlow = glm::vec2(0.f, 0.f),
    const glm::vec2& texhigh = glm::vec2(1.f, 1.f));

  // one slice from a cube, it is a x slice if alongDim == 0, a y slice if alongDim == 1,
  // a z slice if alongDim == 2
  static ZMesh CreateCubeSliceWith2DTexture(
    float coordIn3rdDim,
    int alongDim = 2,     // 0, 1, or 2
    const glm::vec2& coordlow = glm::vec2(0.f, 0.f),
    const glm::vec2& coordhigh = glm::vec2(1.f, 1.f),
    const glm::vec2& texlow = glm::vec2(0.f, 0.f),
    const glm::vec2& texhigh = glm::vec2(1.f, 1.f));

  // a 2d image quad with 2d texture coordinates
  static ZMesh CreateImageSlice(
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
  static ZMesh CreateCubeSerieSlices(
    int numSlices,
    int alongDim = 2,     // 0, 1, or 2
    const glm::vec3& coordfirst = glm::vec3(0.f, 0.f, 0.f),
    const glm::vec3& coordlast = glm::vec3(1.f, 1.f, 1.f),
    const glm::vec3& texfirst = glm::vec3(0.f, 0.f, 0.f),
    const glm::vec3& texlast = glm::vec3(1.f, 1.f, 1.f));

  static ZMesh CreateSphereMesh(const glm::vec3& center, float radius,
                                int thetaResolution = 32, int phiResolution = 32,
                                float startTheta = 0.f, float endTheta = 360.f,
                                float startPhi = 0.f, float endPhi = 180.f);

  static ZMesh CreateTubeMesh(const std::vector<glm::vec3>& line, const std::vector<float>& radius,
                              int numberOfSides = 32, bool capping = true);

  static ZMesh CreateConeMesh(glm::vec3 base, float baseRadius, glm::vec3 top, float topRadius,
                              int numberOfSides = 32, bool capping = true);

  // from ZCubeArray
  static ZMesh FromZCubeArray(const ZCubeArray& ca);

  static ZMesh CreateCuboidFaceMesh(
      const ZIntCuboid &cf, const std::vector<bool> &visible, const QColor &color);

  // these functions only deal with meshes with normal, other fields (texture, color) are ignored
  static ZMesh Unite(const ZMesh& mesh1, const ZMesh& mesh2)
  { return booleanOperation(mesh1, mesh2, BooleanOperationType::Union); }

  static ZMesh Intersect(const ZMesh& mesh1, const ZMesh& mesh2)
  { return booleanOperation(mesh1, mesh2, BooleanOperationType::Intersection); }

  static ZMesh Subtract(const ZMesh& mesh1, const ZMesh& mesh2)
  { return booleanOperation(mesh1, mesh2, BooleanOperationType::Difference); }

  static ZMesh Merge(const std::vector<ZMesh>& meshes);
  static ZMesh Merge(const std::vector<ZMesh*>& meshes);

  void swapXZ();
  void translate(double x, double y, double z);
  void scale(double sx, double sy, double sz);

  void pushObjectColor();
  void pushObjectColor(const QColor &color);

  std::vector<ZPoint> intersectLineSeg(
      const ZPoint &start, const ZPoint &end) const;

  void append(const ZMesh &mesh);

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

  static ZMesh booleanOperation(
      const ZMesh& mesh1, const ZMesh& mesh2, BooleanOperationType type);

  struct ObbTreeData {
    ObbTreeData() {}
    ObbTreeData(const ObbTreeData &){
      reset();
    }
    ObbTreeData(const ObbTreeData &&){
      reset();
    }

    ObbTreeData& operator =(const ObbTreeData&) {
      reset();
      return *this;
    }
    ObbTreeData& operator =(const ObbTreeData&&) {
      reset();
      return *this;
    }

    void reset() {
      m_isObbTreeValid = false;
    }

    bool m_isObbTreeValid = false;
    vtkSmartPointer<vtkOBBTree> m_obbTree;
  };

  bool isObbTreeValid() const {
    return m_obbTreeData.m_isObbTreeValid;
  }

  void validateObbTree(bool valid) const {
   m_obbTreeData.m_isObbTreeValid = valid;
  }

  vtkSmartPointer<vtkOBBTree> getObbTree() const;

  void invalidateCachedProperties() {
    validateObbTree(false);
    m_boundBox.reset();
  }

private:
  friend class ZMeshIO;

  GLenum m_ttype;

  std::vector<glm::vec3> m_vertices;
  std::vector<float> m_1DTextureCoordinates;
  std::vector<glm::vec2> m_2DTextureCoordinates;
  std::vector<glm::vec3> m_3DTextureCoordinates;
  std::vector<glm::vec3> m_normals;
  std::vector<glm::vec4> m_colors;
  std::vector<GLuint> m_indices;
//  uint64_t m_label = 0;

  mutable ZBBox<glm::dvec3> m_boundBox;
  mutable ObbTreeData m_obbTreeData;
  mutable std::vector<std::shared_ptr<ZMesh>> m_splitMeshList;
//  mutable bool m_isObbTreeValid = false;
//  mutable vtkSmartPointer<vtkOBBTree> m_obbTree;
};

#endif // ZMESH_H
