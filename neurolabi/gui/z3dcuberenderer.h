// cube renderer for rendering triangles, quads, and cubes
// Yang Yu, 11/11/2015

#ifndef Z3DCUBERENDERER_H
#define Z3DCUBERENDERER_H

#include "z3dprimitiverenderer.h"
#include "z3dshadergroup.h"

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

    std::vector<glm::vec3> ePositions;
    std::vector<glm::vec4> eColors;

    std::vector<unsigned int> indices;
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
    void init(double sx, double sy, double sz, double tx, double ty, double tz);
    void initIdentity();

    void setVisible(bool v[6]);

    void faces();
    void edges();

    void setFaceColor(glm::vec4 c);
    void setEdgeColor(glm::vec4 c);

public:
    std::vector<glm::vec3> points; // 8
    std::vector<glm::vec3> ePositions; // 24
    std::vector<glm::vec4> eColors; // 24

    float x,y,z; // center offsets

    bool b_visible[6];
    size_t nVertices;

    std::vector<glm::vec3> norm;
    std::vector<glm::vec2> texc;
};

/// Z3DCubeRenderer
class Z3DCubeRenderer : public Z3DPrimitiveRenderer
{
    Q_OBJECT
public:
    explicit Z3DCubeRenderer(QObject *parent = 0);
    virtual ~Z3DCubeRenderer();

    void addCube(double sx, double sy, double sz, double tx, double ty, double tz, glm::vec4 color, bool v[6]);
    void addCube(double l, double x, double y, double z, glm::vec4 color, bool v[6]);
    bool isEmpty();

protected:
    virtual void compile();
    virtual void initialize();
    virtual void deinitialize();
    virtual QString generateHeader();

    virtual void renderUsingOpengl();
    virtual void renderPickingUsingOpengl();

    virtual void render(Z3DEye eye);
    virtual void renderPicking(Z3DEye eye);

    Z3DShaderGroup m_cubeShaderGrp;
    QGLShaderProgram *oit2DComposeProgram;

private:
    std::vector<glm::vec4> m_pointAndRadius;
    std::vector<glm::vec4> m_specularAndShininess;
    std::vector<glm::vec4> m_pointColors;
    std::vector<glm::vec4> m_pointPickingColors;
    std::vector<GLfloat> m_allFlags;
    std::vector<GLuint> m_indexs;

    std::vector<Cube> m_cubes;

    QVector<QVector3D> m_screen;
    GLuint m_vao;
    GLint m_preFBO;
    GLuint m_fbo;
    GLuint m_renderbuffer;
    GLuint m_texture;

    std::vector<GLuint> m_VAOs;
    std::vector<GLuint> m_pickingVAOs;
    std::vector<GLuint> m_VBOs;
    std::vector<GLuint> m_pickingVBOs;

    bool m_dataChanged;
    bool m_pickingDataChanged;
    size_t m_oneBatchNumber;

    size_t nCubes;
};

#endif // Z3DCUBERENDERER_H
