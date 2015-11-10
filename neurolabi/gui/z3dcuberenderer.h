// triangle renderer for rendering triangles, quads, and cubes
// Yang Yu, 11/11/2015

#ifndef Z3DTRIANGLERENDERER_H
#define Z3DTRIANGLERENDERER_H

#include "z3dprimitiverenderer.h"
#include "z3dshadergroup.h"

/// Mesh
class Mesh
{
public:
    Mesh();
    ~Mesh();

public:
    void setPositions(QVector3D a, QVector3D b); // line
    void setPositions(QVector3D a, QVector3D b, QVector3D c); // triangle
    void setPositions(QVector3D a, QVector3D b, QVector3D c, QVector3D d); // quadrilateral
    void setColor(QVector<QVector4D> &color, QVector4D c, int n);

public:
    QVector<QVector3D> positions;
    QVector<QVector4D> colors;
    QVector<QVector3D> normals;
    QVector<QVector2D> texCoords;

    QVector<QVector3D> ePositions;
    QVector<QVector4D> eColors;

    QVector<unsigned int> indices;
};

/// Triangle
class Triangle : public Mesh
{
public:
    Triangle();
    ~Triangle();

public:
    void init(QVector3D a, QVector3D b, QVector3D c);
    void setColor(QVector4D c);
};

/// Quadrilateral
//  d_______c
//   |      |
//   |      |
//  a|______|b
//
// triangle (a,b,c) + triangle (c,d,a)
//
class Quadrilateral : public Triangle
{
public:
    Quadrilateral();
    ~Quadrilateral();

public:
    void init(QVector3D a, QVector3D b, QVector3D c, QVector3D d);
    void setColor(QVector4D c);
};


/// cube
//
//     5______6
//   1/_____2/ |
//    | |    | |
//    |__4___|/7
//    0      3
//
//
// 12 triangles: 36 vertices, normals, and colors
// 12 lines: 24 vertices and colors
//
//
// ---------------> U = ((-Z/|X|) + 1)/2
// |       ____
// |      | U 2|
// | _____|_+y_|__________
// | | L 1| B 4| R 0| F 5|
// | |_-x_|_+z_|_+x_|_-z_|
// |      | D 3|
// |      |_-y_|
// v
// V = ((-Y/|X|) + 1)/2
//
// GL_TEXTURE_CUBE_MAP_POSITIVE_X 	0 +x Right
// GL_TEXTURE_CUBE_MAP_NEGATIVE_X 	1 -x Left
// GL_TEXTURE_CUBE_MAP_POSITIVE_Y 	2 +y Up (Top)
// GL_TEXTURE_CUBE_MAP_NEGATIVE_Y 	3 -y Down (Bottom)
// GL_TEXTURE_CUBE_MAP_POSITIVE_Z 	4 +z Back
// GL_TEXTURE_CUBE_MAP_NEGATIVE_Z 	5 -z Front
//
class Cube : public Quadrilateral
{
public:
    Cube();
    ~Cube();

public:
    void init(float sx, float sy, float sz, float tx, float ty, float tz);
    void initIdentity();

    void faces();
    void edges();

    void setFaceColor(QVector4D c);
    void setEdgeColor(QVector4D c);

public:
    QVector<QVector3D> points; // 8
    QVector<QVector3D> ePositions; // 24
    QVector<QVector4D> eColors; // 24

    float x,y,z; // center offsets
};

/// Z3DTriangleRenderer
class Z3DTriangleRenderer : public Z3DPrimitiveRenderer
{
  Q_OBJECT
public:
  // default use display list and lighting for opengl mode
  explicit Z3DTriangleRenderer(QObject *parent = 0);
  virtual ~Z3DTriangleRenderer();

  void setData(std::vector<glm::vec4> *pointAndRadiusInput, std::vector<glm::vec4> *specularAndShininessInput = NULL);
  void setDataColors(std::vector<glm::vec4> *pointColorsInput);
  void setDataPickingColors(std::vector<glm::vec4> *pointPickingColorsInput = NULL);

protected:
  virtual void compile();
  virtual void initialize();
  virtual void deinitialize();
  virtual QString generateHeader();

  virtual void renderUsingOpengl();
  virtual void renderPickingUsingOpengl();

  virtual void render(Z3DEye eye);
  virtual void renderPicking(Z3DEye eye);

  void appendDefaultColors();

  Z3DShaderGroup m_sphereShaderGrp;

  ZIntParameter m_cube;
  ZBoolParameter m_useDynamicMaterial;

private:
  std::vector<glm::vec4> m_pointAndRadius;
  std::vector<glm::vec4> m_specularAndShininess;
  std::vector<glm::vec4> m_pointColors;
  std::vector<glm::vec4> m_pointPickingColors;
  std::vector<GLfloat> m_allFlags;
  std::vector<GLuint> m_indexs;

  //std::vector<GLuint> m_VBOs;
  //std::vector<GLuint> m_pickingVBOs;
  std::vector<GLuint> m_VAOs;
  std::vector<GLuint> m_pickingVAOs;
  std::vector<std::vector<GLuint> > m_VBOs;
  std::vector<std::vector<GLuint> > m_pickingVBOs;
  bool m_dataChanged;
  bool m_pickingDataChanged;
  size_t m_oneBatchNumber;
};

#endif // Z3DTRIANGLERENDERER_H
