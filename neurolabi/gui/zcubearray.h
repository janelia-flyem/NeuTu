#ifndef ZCUBEARRAY_H
#define ZCUBEARRAY_H

#include <vector>
#include <string>
#include <QColor>
#include "geometry/zpoint.h"
#include "zpointnetwork.h"
#include "zjsonparser.h"
#include "zstackobject.h"
#include "zglmutils.h"
#include "geometry/zcuboid.h"
#include "geometry/zintcuboid.h"
#include "common/zsharedpointer.h"

class ZNormColorMap;
class ZObject3d;
class ZStackBall;
class ZMesh;

/// Mesh
class Mesh
{
public:
    Mesh();
    ~Mesh();

public:
    void setPositions(glm::vec3 a, glm::vec3 b); // line
    void setPositions(glm::vec3 a, glm::vec3 b, glm::vec3 c); // triangle
    void setPositions(glm::vec3 a, glm::vec3 b, glm::vec3 c, glm::vec3 d); // quadrilateral
    void setColor(std::vector<glm::vec4> &color, glm::vec4 c, int n);

public:
    std::vector<glm::vec3> positions;
    std::vector<glm::vec4> colors;
    std::vector<glm::vec3> normals;
    std::vector<glm::vec2> texCoords;
    std::vector<float> normalIndices;

    std::vector<glm::vec3> ePositions;
    std::vector<glm::vec4> eColors;

    std::vector<int> indices;
};

/// Triangle
class Triangle : public Mesh
{
public:
    Triangle();
    ~Triangle();

public:
    void init(glm::vec3 a, glm::vec3 b, glm::vec3 c);
    void setColor(glm::vec4 c);
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
    void init(glm::vec3 a, glm::vec3 b, glm::vec3 c, glm::vec3 d);
    void setColor(glm::vec4 c);
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
// Face vertices:
//   0: 2, 3, 6, 7
//   1: 0, 1, 4, 5
//   2: 4, 5, 6, 7
//   3: 0, 1, 2, 3
//   4: 1, 2, 5, 6
//   5: 0, 3, 4, 7
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
    void init(double sx, double sy, double sz, double tx, double ty, double tz);
    int init(std::vector<glm::vec3> nodes);

    void setVisible(std::vector<bool> v);

    void faces();
    void edges();

    void setFaceColor(glm::vec4 c);
    void setEdgeColor(glm::vec4 c);

public:
    std::vector<glm::vec3> points; // 8
    std::vector<glm::vec3> ePositions; // 24
    std::vector<glm::vec4> eColors; // 24

    float x,y,z; // center offsets

    std::vector<bool> b_visible;
    size_t nVertices;

    std::vector<glm::vec3> norm;
    std::vector<glm::vec2> texc;
    std::vector<float> normIndex;
};

//
class Z3DCube
{
public:
    Z3DCube();
    ~Z3DCube();

    bool hasVisibleFace() const;
    const std::vector<bool>& getFaceVisiblity() const;

public:
    double length;
    double x,y,z;
    std::vector<glm::vec3> nodes;
    glm::vec4 color;
    std::vector<bool> b_visible;
    bool initByNodes;
};

/*!
 * \brief A temporary fix for using meshes for ROIs
 */
class ZCubeArray : public ZStackObject
{
public:
  ZCubeArray();
  ~ZCubeArray();

  static ZStackObject::EType GetType() {
    return ZStackObject::EType::CUBE;
  }

public:
  bool isEmpty() const;

  bool display(QPainter */*painter*/, const DisplayConfig &/*config*/) const override {
    return false;
  }

  /*
  void display(ZPainter &painter, int slice, EDisplayStyle option,
               neutu::EAxis sliceAxis) const override;
               */

//  const std::string& className() const override;

  size_t size();
  void setMesh(ZMesh *mesh);
  ZSharedPointer<ZMesh> getMesh() const;
//  void setColor(const QColor &n) override;
  void pushObjectColor();

  void clear();

  ZCuboid getBoundBox() const override;

private:
  ZSharedPointer<ZMesh> m_mesh;
};


#endif // ZCUBEARRAY_H
