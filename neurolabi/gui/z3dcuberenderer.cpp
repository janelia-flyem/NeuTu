// cube renderer for rendering triangles, quads, and cubes
// Yang Yu, 11/11/2015

#include "zglew.h"
#include "z3dcuberenderer.h"

#include "z3dgpuinfo.h"
#include "zglmutils.h"

//
Mesh::Mesh()
{

}

Mesh::~Mesh()
{

}

void Mesh::setPositions(QVector3D a, QVector3D b)
{
    ePositions.append(a);
    ePositions.append(b);
}

void Mesh::setPositions(QVector3D a, QVector3D b, QVector3D c)
{
    positions.append(a);
    positions.append(b);
    positions.append(c);
}

void Mesh::setPositions(QVector3D a, QVector3D b, QVector3D c, QVector3D d)
{
    setPositions(a,b,c);
    setPositions(c,d,a);
}

void Mesh::setColor(QVector<QVector4D> &color, QVector4D c, int n)
{
    for(int i=0; i<n; i++)
    {
        color.append(c);
    }
}

//
Triangle::Triangle() : Mesh()
{

}

Triangle::~Triangle()
{

}

void Triangle::init(QVector3D a, QVector3D b, QVector3D c)
{
    QVector3D norm = QVector3D::crossProduct(b - a, c - b).normalized();

    setPositions(a,b,c);

    for(int i=0; i<3; i++)
    {
        normals.append(norm);
    }
}

void Triangle::setColor(QVector4D c)
{
    Mesh::setColor(colors,c,3);
}

//
Quadrilateral::Quadrilateral() : Triangle()
{

}

Quadrilateral::~Quadrilateral()
{

}

void Quadrilateral::init(QVector3D a, QVector3D b, QVector3D c, QVector3D d)
{
    QVector3D norm = QVector3D::crossProduct(b - a, c - b).normalized();

    setPositions(a,b,c,d);

    for(int i=0; i<4; i++)
    {
        normals.append(norm);
    }
}

void Quadrilateral::setColor(QVector4D c)
{
    Mesh::setColor(colors,c,6);
}

//
Cube::Cube() : Quadrilateral()
{
    QVector<QVector3D> norm;
    QVector<QVector2D> texc;

    norm << QVector3D(1.0f, 0.0f, 0.0f)   // +x
         << QVector3D(-1.0f, 0.0f, 0.0f)  // -x
         << QVector3D(0.0f, 1.0f, 0.0f)   // +y
         << QVector3D(0.0f, -1.0f, 0.0f)  // -y
         << QVector3D(0.0f, 0.0f, 1.0f)   // +z
         << QVector3D(0.0f, 0.0f, -1.0f); // -z

    texc << QVector2D(0, 0) << QVector2D(1, 0) << QVector2D(1, 1)  // a,b,c
         << QVector2D(1, 1) << QVector2D(0, 1) << QVector2D(0, 0); // c,d,a

    for (int i=0; i<6; i++)
    {
        for (int j=0; j<6; j++)
        {
            normals.append( norm[i] );
            texCoords.append( texc[j] );
        }
    }
}

Cube::~Cube()
{
}

void Cube::init(float sx, float sy, float sz, float tx, float ty, float tz)
{
    float cx = sx/2.0;
    float cy = sy/2.0;
    float cz = sz/2.0;

    float l = -cx - tx;
    float r = cx - tx;
    float u = cy - ty;
    float d = -cy - ty;
    float f = -cz - tz;
    float b = cz - tz;

    x = tx;
    y = ty;
    z = tz;

    //
    points  << QVector3D( l, d, f )  // 0
            << QVector3D( l, u, f )  // 1
            << QVector3D( r, u, f )  // 2
            << QVector3D( r, d, f )  // 3
            << QVector3D( l, d, b )  // 4
            << QVector3D( l, u, b )  // 5
            << QVector3D( r, u, b )  // 6
            << QVector3D( r, d, b ); // 7

    //
    faces();

    //
    edges();
}

void Cube::initIdentity()
{
    init(1,1,1,0,0,0);
}

void Cube::faces()
{
    // 12 triangles: 36 vertices and 36 colors

    //
    setPositions(points[7], points[3], points[2], points[6]); // GL_TEXTURE_CUBE_MAP_POSITIVE_X 	0 +x Right
    setPositions(points[0], points[4], points[5], points[1]); // GL_TEXTURE_CUBE_MAP_NEGATIVE_X 	1 -x Left
    setPositions(points[1], points[5], points[6], points[2]); // GL_TEXTURE_CUBE_MAP_POSITIVE_Y 	2 +y Up (Top)
    setPositions(points[4], points[0], points[3], points[7]); // GL_TEXTURE_CUBE_MAP_NEGATIVE_Y 	3 -y Down (Bottom)
    setPositions(points[6], points[5], points[4], points[7]); // GL_TEXTURE_CUBE_MAP_POSITIVE_Z 	4 +z Back
    setPositions(points[3], points[0], points[1], points[2]); // GL_TEXTURE_CUBE_MAP_NEGATIVE_Z 	5 -z Front

}

void Cube::edges()
{
    // 12 lines: 24 vertices and 24 colors

    // edges of the top face
    setPositions(points[0], points[1]);
    setPositions(points[1], points[2]);
    setPositions(points[2], points[3]);
    setPositions(points[3], points[0]);

    // edges of the bottom face
    setPositions(points[4], points[5]);
    setPositions(points[5], points[6]);
    setPositions(points[6], points[7]);
    setPositions(points[7], points[4]);

    // edges connecting top face to bottom face
    setPositions(points[1], points[5]);
    setPositions(points[0], points[2]);
    setPositions(points[2], points[6]);
    setPositions(points[3], points[7]);
}

void Cube::setFaceColor(QVector4D c)
{
    Mesh::setColor(colors, c, 36);
}

void Cube::setEdgeColor(QVector4D c)
{
    Mesh::setColor(eColors, c, 24);
}

//
Z3DCubeRenderer::Z3DCubeRenderer(QObject *parent)
  : Z3DPrimitiveRenderer(parent)
  , m_cubeShaderGrp()
  , m_screenShaderGrp()
  , m_dataChanged(false)
  , m_pickingDataChanged(false)
  , m_oneBatchNumber(4e6)
{
  setNeedLighting(true);
  setUseDisplayList(true);

  //
  m_vao = 0;
  m_vbo = 0;

  m_fbo = 0;
  m_db = 0;
  m_rt = 0;

  //
  m_numCubes = 0;
}

Z3DCubeRenderer::~Z3DCubeRenderer()
{
}

void Z3DCubeRenderer::addCube(int sx, int sy, int sz, int tx, int ty, int tz, QVector4D color)
{
    Cube cube;

    cube.setFaceColor(color);
    cube.init(sx,sy,sz,tx,ty,tz);
    m_cubes.push_back(cube);

    m_dataChanged = true;
}

void Z3DCubeRenderer::compile()
{
  m_dataChanged = true;
  m_cubeShaderGrp.rebuild(generateHeader());
  m_screenShaderGrp.rebuild(generateHeader());
}

void Z3DCubeRenderer::initialize()
{
  Z3DPrimitiveRenderer::initialize();
  QStringList cubeShaders, screenShaders;
  cubeShaders << "cube_wboit.vert" << "cube_wboit.frag" << "lighting.frag";
  m_cubeShaderGrp.init(cubeShaders, generateHeader(), m_rendererBase);
  m_cubeShaderGrp.addAllSupportedPostShaders();

  screenShaders << "cube_wboit_compose.vert" << "cube_wboit_compose.frag";
  m_screenShaderGrp.init(screenShaders, generateHeader(), m_rendererBase);
  m_screenShaderGrp.addAllSupportedPostShaders();

  m_initialized = true;
}

void Z3DCubeRenderer::deinitialize()
{
  if (!m_VAOs.empty())
  {
    glDeleteVertexArrays(m_VAOs.size(), &m_VAOs[0]);
  }
  m_VAOs.clear();

  if (!m_pickingVAOs.empty())
  {
    glDeleteVertexArrays(m_pickingVAOs.size(), &m_pickingVAOs[0]);
  }
  m_pickingVAOs.clear();

  if (!m_VBOs.empty())
  {
    glDeleteVertexArrays(m_VBOs.size(), &m_VBOs[0]);
  }
  m_VBOs.clear();

  if (!m_pickingVBOs.empty())
  {
    glDeleteVertexArrays(m_pickingVBOs.size(), &m_pickingVBOs[0]);
  }
  m_pickingVBOs.clear();


  if(m_vao)
  {
    glDeleteBuffers(1, &m_vao);
  }

  if(m_vbo)
  {
    glDeleteBuffers(1, &m_vbo);
  }

  if(m_fbo)
  {
    glDeleteBuffers(1, &m_fbo);
  }

  if(m_db)
  {
    glDeleteBuffers(1, &m_db);
  }

  if(m_rt)
  {
    glDeleteBuffers(1, &m_rt);
  }

  m_cubeShaderGrp.removeAllShaders();
  m_screenShaderGrp.removeAllShaders();
  CHECK_GL_ERROR;
  Z3DPrimitiveRenderer::deinitialize();
}

QString Z3DCubeRenderer::generateHeader()
{
  QString headerSource = Z3DPrimitiveRenderer::generateHeader();
  return headerSource;
}

void Z3DCubeRenderer::renderUsingOpengl()
{
    // deprecated
}

void Z3DCubeRenderer::renderPickingUsingOpengl()
{
    // deprecated
}

void Z3DCubeRenderer::render(Z3DEye eye)
{
  if (!m_initialized)
    return;

  //
  m_cubeShaderGrp.bind();
  Z3DShaderProgram &oit3DTransparencyPassShader = m_cubeShaderGrp.get();

  m_rendererBase->setGlobalShaderParameters(oit3DTransparencyPassShader, eye);
  oit3DTransparencyPassShader.setUniformValue("lighting_enabled", m_needLighting);

  m_numCubes = m_cubes.size();

  Z3DShaderProgram &oit2DCompositingPassShader = m_screenShaderGrp.get();

//  if (m_hardwareSupportVAO) {
//    if (m_dataChanged) {
//      if (!m_VAOs.empty()) {
//        glDeleteVertexArrays(m_VAOs.size(), &m_VAOs[0]);
//      }
//      m_VAOs.resize(numBatch);
//      glGenVertexArrays(m_VAOs.size(), &m_VAOs[0]);

//      for (size_t ivbo=0; ivbo<m_VBOs.size(); ++ivbo) {
//        glDeleteBuffers(m_VBOs[ivbo].size(), &m_VBOs[ivbo][0]);
//      }
//      m_VBOs.resize(numBatch);
//      for (size_t ivbo=0; ivbo<m_VBOs.size(); ++ivbo) {
//        m_VBOs[ivbo].resize(5);
//        glGenBuffers(m_VBOs[ivbo].size(), &m_VBOs[ivbo][0]);
//      }

//      //glBindVertexArray(m_VAO);
//      // set vertex data
//      GLint attr_a_vertex_radius = shader.attributeLocation("attr_vertex_radius");
//      GLint attr_a_specular_shininess;
//      if (m_useDynamicMaterial.get() && !m_specularAndShininess.empty()) {
//        attr_a_specular_shininess = shader.attributeLocation("attr_specular_shininess");
//      }
//      GLint attr_color = shader.attributeLocation("attr_color");
//      GLint attr_flags = shader.attributeLocation("attr_flags");

//      for (size_t i=0; i<numBatch; ++i) {
//        glBindVertexArray(m_VAOs[i]);
//        size_t size = m_oneBatchNumber;
//        if (i == numBatch-1)
//          size = m_pointAndRadius.size() - (numBatch - 1) * m_oneBatchNumber;
//        size_t start = m_oneBatchNumber * i;

//        glEnableVertexAttribArray(attr_a_vertex_radius);
//        glBindBuffer(GL_ARRAY_BUFFER, m_VBOs[i][0]);
//        glBufferData(GL_ARRAY_BUFFER, size*4*sizeof(GLfloat), &(m_pointAndRadius[start]), GL_STATIC_DRAW);
//        glVertexAttribPointer(attr_a_vertex_radius, 4, GL_FLOAT, GL_FALSE, 0, 0);

//        if (m_useDynamicMaterial.get() && !m_specularAndShininess.empty()) {
//          glEnableVertexAttribArray(attr_a_specular_shininess);
//          glBindBuffer(GL_ARRAY_BUFFER, m_VBOs[i][3]);
//          glBufferData(GL_ARRAY_BUFFER, size*4*sizeof(GLfloat), &(m_specularAndShininess[start]), GL_STATIC_DRAW);
//          glVertexAttribPointer(attr_a_specular_shininess, 4, GL_FLOAT, GL_FALSE, 0, 0);
//        }

//        glEnableVertexAttribArray(attr_color);
//        glBindBuffer(GL_ARRAY_BUFFER, m_VBOs[i][1]);
//        glBufferData(GL_ARRAY_BUFFER, size*4*sizeof(GLfloat), &(m_pointColors[start]), GL_STATIC_DRAW);
//        glVertexAttribPointer(attr_color, 4, GL_FLOAT, GL_FALSE, 0, 0);

//        glEnableVertexAttribArray(attr_flags);
//        glBindBuffer(GL_ARRAY_BUFFER, m_VBOs[i][2]);
//        glBufferData(GL_ARRAY_BUFFER, size*sizeof(GLfloat), &(m_allFlags[start]), GL_STATIC_DRAW);
//        glVertexAttribPointer(attr_flags, 1, GL_FLOAT, GL_FALSE, 0, 0);

//        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_VBOs[i][4]);
//        glBufferData(GL_ELEMENT_ARRAY_BUFFER, size*6/4*sizeof(GLuint), &(m_indexs[0]), GL_STATIC_DRAW);

//        glBindBuffer(GL_ARRAY_BUFFER, 0);
//        glBindVertexArray(0);
//      }

//      m_dataChanged = false;
//    }

//    for (size_t i=0; i<numBatch; ++i) {
//      size_t size = m_oneBatchNumber;
//      if (i == numBatch-1)
//        size = m_pointAndRadius.size() - (numBatch - 1) * m_oneBatchNumber;
//      glBindVertexArray(m_VAOs[i]);
//      glDrawElements(GL_TRIANGLES, size * 6 / 4, GL_UNSIGNED_INT, 0);
//      glBindVertexArray(0);
//    }

//  } else {
//    if (m_dataChanged) {
//      for (size_t ivbo=0; ivbo<m_VBOs.size(); ++ivbo) {
//        glDeleteBuffers(m_VBOs[ivbo].size(), &m_VBOs[ivbo][0]);
//      }
//      m_VBOs.resize(numBatch);
//      for (size_t ivbo=0; ivbo<m_VBOs.size(); ++ivbo) {
//        m_VBOs[ivbo].resize(5);
//        glGenBuffers(m_VBOs[ivbo].size(), &m_VBOs[ivbo][0]);
//      }
//    }
//    // set vertex data
//    GLint attr_a_vertex_radius = shader.attributeLocation("attr_vertex_radius");
//    GLint attr_a_specular_shininess;
//    if (m_useDynamicMaterial.get() && !m_specularAndShininess.empty()) {
//      attr_a_specular_shininess = shader.attributeLocation("attr_specular_shininess");
//    }
//    GLint attr_color = shader.attributeLocation("attr_color");
//    GLint attr_flags = shader.attributeLocation("attr_flags");

//    for (size_t i=0; i<numBatch; ++i) {
//      size_t size = m_oneBatchNumber;
//      if (i == numBatch-1)
//        size = m_pointAndRadius.size() - (numBatch - 1) * m_oneBatchNumber;
//      size_t start = m_oneBatchNumber * i;

//      glEnableVertexAttribArray(attr_a_vertex_radius);
//      glBindBuffer(GL_ARRAY_BUFFER, m_VBOs[i][0]);
//      if (m_dataChanged)
//        glBufferData(GL_ARRAY_BUFFER, size*4*sizeof(GLfloat), &(m_pointAndRadius[start]), GL_STATIC_DRAW);
//      glVertexAttribPointer(attr_a_vertex_radius, 4, GL_FLOAT, GL_FALSE, 0, 0);

//      if (m_useDynamicMaterial.get() && !m_specularAndShininess.empty()) {
//        glEnableVertexAttribArray(attr_a_specular_shininess);
//        glBindBuffer(GL_ARRAY_BUFFER, m_VBOs[i][3]);
//        if (m_dataChanged)
//          glBufferData(GL_ARRAY_BUFFER, size*4*sizeof(GLfloat), &(m_specularAndShininess[start]), GL_STATIC_DRAW);
//        glVertexAttribPointer(attr_a_specular_shininess, 4, GL_FLOAT, GL_FALSE, 0, 0);
//      }

//      glEnableVertexAttribArray(attr_color);
//      glBindBuffer(GL_ARRAY_BUFFER, m_VBOs[i][1]);
//      if (m_dataChanged)
//        glBufferData(GL_ARRAY_BUFFER, size*4*sizeof(GLfloat), &(m_pointColors[start]), GL_STATIC_DRAW);
//      glVertexAttribPointer(attr_color, 4, GL_FLOAT, GL_FALSE, 0, 0);

//      glEnableVertexAttribArray(attr_flags);
//      glBindBuffer(GL_ARRAY_BUFFER, m_VBOs[i][2]);
//      if (m_dataChanged)
//        glBufferData(GL_ARRAY_BUFFER, size*sizeof(GLfloat), &(m_allFlags[start]), GL_STATIC_DRAW);
//      glVertexAttribPointer(attr_flags, 1, GL_FLOAT, GL_FALSE, 0, 0);

//      glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_VBOs[i][4]);
//      if (m_dataChanged)
//        glBufferData(GL_ELEMENT_ARRAY_BUFFER, size*6/4*sizeof(GLuint), &(m_indexs[0]), GL_STATIC_DRAW);

//      glDrawElements(GL_TRIANGLES, size*6/4, GL_UNSIGNED_INT, 0);

//      glBindBuffer(GL_ARRAY_BUFFER, 0);
//      glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
//      glDisableVertexAttribArray(attr_a_vertex_radius);
//      if (m_useDynamicMaterial.get() && !m_specularAndShininess.empty())
//        glDisableVertexAttribArray(attr_a_specular_shininess);
//      glDisableVertexAttribArray(attr_color);
//      glDisableVertexAttribArray(attr_flags);
//    }

//    m_dataChanged = false;
//  }

  m_cubeShaderGrp.release();
}

void Z3DCubeRenderer::renderPicking(Z3DEye eye)
{
}
