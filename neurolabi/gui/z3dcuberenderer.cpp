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
Z3DTriangleRenderer::Z3DTriangleRenderer(QObject *parent)
  : Z3DPrimitiveRenderer(parent)
  , m_sphereShaderGrp()
  , m_sphereSlicesStacks("Sphere Slices/Stacks", 36, 20, 100)
  , m_useDynamicMaterial("Calculate Material Property From Intensity", false)
  //  , m_VBOs(5)
  //  , m_pickingVBOs(4)
  , m_dataChanged(false)
  , m_pickingDataChanged(false)
  , m_oneBatchNumber(4e6)
{
  setNeedLighting(true);
  setUseDisplayList(true);
  connect(&m_sphereSlicesStacks, SIGNAL(valueChanged()), this, SLOT(invalidateOpenglRenderer()));
  connect(&m_useDynamicMaterial, SIGNAL(valueChanged()), this, SLOT(compile()));
  connect(&m_useDynamicMaterial, SIGNAL(valueChanged()), this, SLOT(invalidateOpenglRenderer()));
  addParameter(m_sphereSlicesStacks);
  addParameter(m_useDynamicMaterial);
}

Z3DTriangleRenderer::~Z3DTriangleRenderer()
{
}

void Z3DTriangleRenderer::setData(std::vector<glm::vec4> *pointAndRadiusInput,
                                std::vector<glm::vec4> *specularAndShininessInput)
{
  m_pointAndRadius.clear();
  m_specularAndShininess.clear();
  m_indexs.clear();
  int indices[6] = { 0, 1, 2, 2, 1, 3 };
  int quadIdx = 0;
  for (size_t i=0; i<pointAndRadiusInput->size(); i++) {
    m_pointAndRadius.push_back(pointAndRadiusInput->at(i));
    m_pointAndRadius.push_back(pointAndRadiusInput->at(i));
    m_pointAndRadius.push_back(pointAndRadiusInput->at(i));
    m_pointAndRadius.push_back(pointAndRadiusInput->at(i));
    for (int k=0; k<6; k++) {
      m_indexs.push_back(indices[k] + 4 * quadIdx);
    }
    quadIdx++;
  }
  if (specularAndShininessInput == NULL) {
    m_useDynamicMaterial.set(false);
  } else {
    for (size_t i=0; i<specularAndShininessInput->size(); i++) {
      m_specularAndShininess.push_back(specularAndShininessInput->at(i));
      m_specularAndShininess.push_back(specularAndShininessInput->at(i));
      m_specularAndShininess.push_back(specularAndShininessInput->at(i));
      m_specularAndShininess.push_back(specularAndShininessInput->at(i));
    }
  }
  size_t rightUpSize = m_allFlags.size();
  float cornerFlags[4] = {0 << 4 | 0,      // (-1, -1) left down
                          2 << 4 | 0,      // (1, -1) right down
                          0 << 4 | 2,      // (-1, 1) left up
                          2 << 4 | 2};     // (1, 1) right up

  if (rightUpSize > m_pointAndRadius.size()) {
    m_allFlags.resize(m_pointAndRadius.size());
  } else if (rightUpSize < m_pointAndRadius.size()) {
    m_allFlags.resize(m_pointAndRadius.size());
    for (size_t i=rightUpSize; i<m_allFlags.size(); i+=4) {
      m_allFlags[i] = cornerFlags[0];
      m_allFlags[i+1] = cornerFlags[1];
      m_allFlags[i+2] = cornerFlags[2];
      m_allFlags[i+3] = cornerFlags[3];
    }
  }
  invalidateOpenglRenderer();
  invalidateOpenglPickingRenderer();
  m_dataChanged = true;
  m_pickingDataChanged = true;
}

void Z3DTriangleRenderer::setDataColors(std::vector<glm::vec4> *pointColorsInput)
{
  m_pointColors.clear();
  for (size_t i=0; i<pointColorsInput->size(); i++) {
    m_pointColors.push_back(pointColorsInput->at(i));
    m_pointColors.push_back(pointColorsInput->at(i));
    m_pointColors.push_back(pointColorsInput->at(i));
    m_pointColors.push_back(pointColorsInput->at(i));
  }
  invalidateOpenglRenderer();
  m_dataChanged = true;
}

void Z3DTriangleRenderer::setDataPickingColors(std::vector<glm::vec4> *pointPickingColorsInput)
{
  m_pointPickingColors.clear();
  if (pointPickingColorsInput == NULL)
    return;
  for (size_t i=0; i<pointPickingColorsInput->size(); i++) {
    m_pointPickingColors.push_back(pointPickingColorsInput->at(i));
    m_pointPickingColors.push_back(pointPickingColorsInput->at(i));
    m_pointPickingColors.push_back(pointPickingColorsInput->at(i));
    m_pointPickingColors.push_back(pointPickingColorsInput->at(i));
  }
  invalidateOpenglPickingRenderer();
  m_pickingDataChanged = true;
}

void Z3DTriangleRenderer::compile()
{
  m_dataChanged = true;
  m_sphereShaderGrp.rebuild(generateHeader());
}

void Z3DTriangleRenderer::initialize()
{
  Z3DPrimitiveRenderer::initialize();
  QStringList allshaders;
  allshaders << "sphere.vert" << "sphere_func.frag" << "lighting.frag";
  m_sphereShaderGrp.init(allshaders, generateHeader(), m_rendererBase);
  m_sphereShaderGrp.addAllSupportedPostShaders();

  //glGenBuffers(m_VBOs.size(), &m_VBOs[0]);
  //glGenBuffers(m_pickingVBOs.size(), &m_pickingVBOs[0]);
}

void Z3DTriangleRenderer::deinitialize()
{
  //glDeleteBuffers(m_VBOs.size(), &m_VBOs[0]);
  //glDeleteBuffers(m_pickingVBOs.size(), &m_pickingVBOs[0]);
  if (!m_VAOs.empty()) {
    glDeleteVertexArrays(m_VAOs.size(), &m_VAOs[0]);
  }
  if (!m_pickingVAOs.empty()) {
    glDeleteVertexArrays(m_pickingVAOs.size(), &m_pickingVAOs[0]);
  }
  m_VAOs.clear();
  m_pickingVAOs.clear();
  for (size_t i=0; i<m_VBOs.size(); ++i) {
    glDeleteBuffers(m_VBOs[i].size(), &m_VBOs[i][0]);
  }
  m_VBOs.clear();
  for (size_t i=0; i<m_pickingVBOs.size(); ++i) {
    glDeleteBuffers(m_pickingVBOs[i].size(), &m_pickingVBOs[i][0]);
  }
  m_pickingVBOs.clear();

  m_sphereShaderGrp.removeAllShaders();
  CHECK_GL_ERROR;
  Z3DPrimitiveRenderer::deinitialize();
}

QString Z3DTriangleRenderer::generateHeader()
{
  QString headerSource = Z3DPrimitiveRenderer::generateHeader();
  if (m_useDynamicMaterial.get())
    headerSource += "#define DYNAMIC_MATERIAL_PROPERTY\n";
  return headerSource;
}

void Z3DTriangleRenderer::renderUsingOpengl()
{
  if (m_pointAndRadius.empty())
    return;
  appendDefaultColors();

  GLUquadricObj* quadric = gluNewQuadric();
  for (size_t i=0; i<m_pointAndRadius.size(); i+=4) {
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glTranslatef(getCoordScales().x * m_pointAndRadius[i].x,
                 getCoordScales().y * m_pointAndRadius[i].y,
                 getCoordScales().z * m_pointAndRadius[i].z);
    float diameter = m_pointAndRadius[i].w * getSizeScale() * 2;
    glScalef(diameter, diameter, diameter);
    // overwrite material property setted by z3drendererbase
    if (m_useDynamicMaterial.get() && !m_specularAndShininess.empty()) {
      glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, m_specularAndShininess[i].w);
      glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, glm::value_ptr(glm::vec4(m_specularAndShininess[i].xyz(), 1.f)));
    }
    glColor4fv(glm::value_ptr(glm::vec4(m_pointColors[i].rgb(), m_pointColors[i].a * getOpacity())));
    gluSphere(quadric, .5, m_sphereSlicesStacks.get(), m_sphereSlicesStacks.get());
    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();
  }
  gluDeleteQuadric(quadric);
}

void Z3DTriangleRenderer::renderPickingUsingOpengl()
{
  if (m_pointAndRadius.empty())
    return;
  if (m_pointPickingColors.empty() || m_pointAndRadius.size() != m_pointPickingColors.size())
    return;
  GLUquadricObj* quadric = gluNewQuadric();
  for (size_t i=0; i<m_pointAndRadius.size(); i+=4) {
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glTranslatef(getCoordScales().x * m_pointAndRadius[i].x,
                 getCoordScales().y * m_pointAndRadius[i].y,
                 getCoordScales().z * m_pointAndRadius[i].z);
    float radius = m_pointAndRadius[i].w * getSizeScale();
    glScalef(radius, radius, radius);
    glColor4fv(glm::value_ptr(m_pointPickingColors[i]));
    gluSphere(quadric, 1., 12, 12/*m_sphereSlicesStacks.get(), m_sphereSlicesStacks.get()*/);
    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();
  }
  gluDeleteQuadric(quadric);
}

void Z3DTriangleRenderer::render(Z3DEye eye)
{
  if (!m_initialized)
    return;

  if (m_pointAndRadius.empty())
    return;
  appendDefaultColors();


  m_sphereShaderGrp.bind();
  Z3DShaderProgram &shader = m_sphereShaderGrp.get();

  m_rendererBase->setGlobalShaderParameters(shader, eye);

  shader.setUniformValue("lighting_enabled", m_needLighting);
  shader.setUniformValue("pos_scale", getCoordScales());

  float fovy = glm::degrees(m_rendererBase->getCamera().getFieldOfView());
  float adj;
  if (fovy <= 90.f){
    adj = 1.0027+0.000111*fovy+0.000098*fovy*fovy;
  } else {
    adj = 2.02082 - 0.033935*fovy + 0.00037854*fovy*fovy;
  }
  shader.setUniformValue("box_correction", adj);

  size_t numBatch = std::ceil(m_pointAndRadius.size() * 1.0 / m_oneBatchNumber);

  if (m_hardwareSupportVAO) {
    if (m_dataChanged) {
      if (!m_VAOs.empty()) {
        glDeleteVertexArrays(m_VAOs.size(), &m_VAOs[0]);
      }
      m_VAOs.resize(numBatch);
      glGenVertexArrays(m_VAOs.size(), &m_VAOs[0]);

      for (size_t ivbo=0; ivbo<m_VBOs.size(); ++ivbo) {
        glDeleteBuffers(m_VBOs[ivbo].size(), &m_VBOs[ivbo][0]);
      }
      m_VBOs.resize(numBatch);
      for (size_t ivbo=0; ivbo<m_VBOs.size(); ++ivbo) {
        m_VBOs[ivbo].resize(5);
        glGenBuffers(m_VBOs[ivbo].size(), &m_VBOs[ivbo][0]);
      }

      //glBindVertexArray(m_VAO);
      // set vertex data
      GLint attr_a_vertex_radius = shader.attributeLocation("attr_vertex_radius");
      GLint attr_a_specular_shininess;
      if (m_useDynamicMaterial.get() && !m_specularAndShininess.empty()) {
        attr_a_specular_shininess = shader.attributeLocation("attr_specular_shininess");
      }
      GLint attr_color = shader.attributeLocation("attr_color");
      GLint attr_flags = shader.attributeLocation("attr_flags");

      for (size_t i=0; i<numBatch; ++i) {
        glBindVertexArray(m_VAOs[i]);
        size_t size = m_oneBatchNumber;
        if (i == numBatch-1)
          size = m_pointAndRadius.size() - (numBatch - 1) * m_oneBatchNumber;
        size_t start = m_oneBatchNumber * i;

        glEnableVertexAttribArray(attr_a_vertex_radius);
        glBindBuffer(GL_ARRAY_BUFFER, m_VBOs[i][0]);
        glBufferData(GL_ARRAY_BUFFER, size*4*sizeof(GLfloat), &(m_pointAndRadius[start]), GL_STATIC_DRAW);
        glVertexAttribPointer(attr_a_vertex_radius, 4, GL_FLOAT, GL_FALSE, 0, 0);

        if (m_useDynamicMaterial.get() && !m_specularAndShininess.empty()) {
          glEnableVertexAttribArray(attr_a_specular_shininess);
          glBindBuffer(GL_ARRAY_BUFFER, m_VBOs[i][3]);
          glBufferData(GL_ARRAY_BUFFER, size*4*sizeof(GLfloat), &(m_specularAndShininess[start]), GL_STATIC_DRAW);
          glVertexAttribPointer(attr_a_specular_shininess, 4, GL_FLOAT, GL_FALSE, 0, 0);
        }

        glEnableVertexAttribArray(attr_color);
        glBindBuffer(GL_ARRAY_BUFFER, m_VBOs[i][1]);
        glBufferData(GL_ARRAY_BUFFER, size*4*sizeof(GLfloat), &(m_pointColors[start]), GL_STATIC_DRAW);
        glVertexAttribPointer(attr_color, 4, GL_FLOAT, GL_FALSE, 0, 0);

        glEnableVertexAttribArray(attr_flags);
        glBindBuffer(GL_ARRAY_BUFFER, m_VBOs[i][2]);
        glBufferData(GL_ARRAY_BUFFER, size*sizeof(GLfloat), &(m_allFlags[start]), GL_STATIC_DRAW);
        glVertexAttribPointer(attr_flags, 1, GL_FLOAT, GL_FALSE, 0, 0);

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_VBOs[i][4]);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, size*6/4*sizeof(GLuint), &(m_indexs[0]), GL_STATIC_DRAW);

        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindVertexArray(0);
      }

      m_dataChanged = false;
    }

    for (size_t i=0; i<numBatch; ++i) {
      size_t size = m_oneBatchNumber;
      if (i == numBatch-1)
        size = m_pointAndRadius.size() - (numBatch - 1) * m_oneBatchNumber;
      glBindVertexArray(m_VAOs[i]);
      glDrawElements(GL_TRIANGLES, size * 6 / 4, GL_UNSIGNED_INT, 0);
      glBindVertexArray(0);
    }

  } else {
    if (m_dataChanged) {
      for (size_t ivbo=0; ivbo<m_VBOs.size(); ++ivbo) {
        glDeleteBuffers(m_VBOs[ivbo].size(), &m_VBOs[ivbo][0]);
      }
      m_VBOs.resize(numBatch);
      for (size_t ivbo=0; ivbo<m_VBOs.size(); ++ivbo) {
        m_VBOs[ivbo].resize(5);
        glGenBuffers(m_VBOs[ivbo].size(), &m_VBOs[ivbo][0]);
      }
    }
    // set vertex data
    GLint attr_a_vertex_radius = shader.attributeLocation("attr_vertex_radius");
    GLint attr_a_specular_shininess;
    if (m_useDynamicMaterial.get() && !m_specularAndShininess.empty()) {
      attr_a_specular_shininess = shader.attributeLocation("attr_specular_shininess");
    }
    GLint attr_color = shader.attributeLocation("attr_color");
    GLint attr_flags = shader.attributeLocation("attr_flags");

    for (size_t i=0; i<numBatch; ++i) {
      size_t size = m_oneBatchNumber;
      if (i == numBatch-1)
        size = m_pointAndRadius.size() - (numBatch - 1) * m_oneBatchNumber;
      size_t start = m_oneBatchNumber * i;

      glEnableVertexAttribArray(attr_a_vertex_radius);
      glBindBuffer(GL_ARRAY_BUFFER, m_VBOs[i][0]);
      if (m_dataChanged)
        glBufferData(GL_ARRAY_BUFFER, size*4*sizeof(GLfloat), &(m_pointAndRadius[start]), GL_STATIC_DRAW);
      glVertexAttribPointer(attr_a_vertex_radius, 4, GL_FLOAT, GL_FALSE, 0, 0);

      if (m_useDynamicMaterial.get() && !m_specularAndShininess.empty()) {
        glEnableVertexAttribArray(attr_a_specular_shininess);
        glBindBuffer(GL_ARRAY_BUFFER, m_VBOs[i][3]);
        if (m_dataChanged)
          glBufferData(GL_ARRAY_BUFFER, size*4*sizeof(GLfloat), &(m_specularAndShininess[start]), GL_STATIC_DRAW);
        glVertexAttribPointer(attr_a_specular_shininess, 4, GL_FLOAT, GL_FALSE, 0, 0);
      }

      glEnableVertexAttribArray(attr_color);
      glBindBuffer(GL_ARRAY_BUFFER, m_VBOs[i][1]);
      if (m_dataChanged)
        glBufferData(GL_ARRAY_BUFFER, size*4*sizeof(GLfloat), &(m_pointColors[start]), GL_STATIC_DRAW);
      glVertexAttribPointer(attr_color, 4, GL_FLOAT, GL_FALSE, 0, 0);

      glEnableVertexAttribArray(attr_flags);
      glBindBuffer(GL_ARRAY_BUFFER, m_VBOs[i][2]);
      if (m_dataChanged)
        glBufferData(GL_ARRAY_BUFFER, size*sizeof(GLfloat), &(m_allFlags[start]), GL_STATIC_DRAW);
      glVertexAttribPointer(attr_flags, 1, GL_FLOAT, GL_FALSE, 0, 0);

      glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_VBOs[i][4]);
      if (m_dataChanged)
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, size*6/4*sizeof(GLuint), &(m_indexs[0]), GL_STATIC_DRAW);

      glDrawElements(GL_TRIANGLES, size*6/4, GL_UNSIGNED_INT, 0);

      glBindBuffer(GL_ARRAY_BUFFER, 0);
      glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
      glDisableVertexAttribArray(attr_a_vertex_radius);
      if (m_useDynamicMaterial.get() && !m_specularAndShininess.empty())
        glDisableVertexAttribArray(attr_a_specular_shininess);
      glDisableVertexAttribArray(attr_color);
      glDisableVertexAttribArray(attr_flags);
    }

    m_dataChanged = false;
  }

  m_sphereShaderGrp.release();
}

void Z3DTriangleRenderer::renderPicking(Z3DEye eye)
{
  if (!m_initialized)
    return;

  if (m_pointAndRadius.empty())
    return;

  if (m_pointPickingColors.empty() || m_pointAndRadius.size() != m_pointPickingColors.size())
    return;

  m_sphereShaderGrp.bind();
  Z3DShaderProgram &shader = m_sphereShaderGrp.get();

  m_rendererBase->setGlobalShaderParameters(shader, eye);

  shader.setUniformValue("lighting_enabled", false);
  shader.setUniformValue("pos_scale", getCoordScales());

  float fovy = glm::degrees(m_rendererBase->getCamera().getFieldOfView());
  float adj;
  if (fovy <= 90.f){
    adj = 1.0027+0.000111*fovy+0.000098*fovy*fovy;
  } else {
    adj = 2.02082 - 0.033935*fovy + 0.00037854*fovy*fovy;
  }
  shader.setUniformValue("box_correction", adj);

  size_t numBatch = std::ceil(m_pointAndRadius.size() * 1.0 / m_oneBatchNumber);

  if (m_hardwareSupportVAO) {
    if (m_pickingDataChanged) {
      if (!m_pickingVAOs.empty()) {
        glDeleteVertexArrays(m_pickingVAOs.size(), &m_pickingVAOs[0]);
      }
      m_pickingVAOs.resize(numBatch);
      glGenVertexArrays(m_pickingVAOs.size(), &m_pickingVAOs[0]);

      for (size_t ivbo=0; ivbo<m_pickingVBOs.size(); ++ivbo) {
        glDeleteBuffers(m_pickingVBOs[ivbo].size(), &m_pickingVBOs[ivbo][0]);
      }
      m_pickingVBOs.resize(numBatch);
      for (size_t ivbo=0; ivbo<m_pickingVBOs.size(); ++ivbo) {
        m_pickingVBOs[ivbo].resize(4);
        glGenBuffers(m_pickingVBOs[ivbo].size(), &m_pickingVBOs[ivbo][0]);
      }

      //glBindVertexArray(m_pickingVAO);
      // set vertex data
      GLint attr_a_vertex_radius = shader.attributeLocation("attr_vertex_radius");
      GLint attr_color = shader.attributeLocation("attr_color");
      GLint attr_flags = shader.attributeLocation("attr_flags");

      for (size_t i=0; i<numBatch; ++i) {
        glBindVertexArray(m_pickingVAOs[i]);
        size_t size = m_oneBatchNumber;
        if (i == numBatch-1)
          size = m_pointAndRadius.size() - (numBatch - 1) * m_oneBatchNumber;
        size_t start = m_oneBatchNumber * i;

        glEnableVertexAttribArray(attr_a_vertex_radius);
        if (m_dataChanged) {
          glBindBuffer(GL_ARRAY_BUFFER, m_pickingVBOs[i][0]);
          glBufferData(GL_ARRAY_BUFFER, size*4*sizeof(GLfloat), &(m_pointAndRadius[start]), GL_STATIC_DRAW);
        } else {
          glBindBuffer(GL_ARRAY_BUFFER, m_VBOs[i][0]);
        }
        glVertexAttribPointer(attr_a_vertex_radius, 4, GL_FLOAT, GL_FALSE, 0, 0);

        glEnableVertexAttribArray(attr_color);
        glBindBuffer(GL_ARRAY_BUFFER, m_pickingVBOs[i][1]);
        glBufferData(GL_ARRAY_BUFFER, size*4*sizeof(GLfloat), &(m_pointPickingColors[start]), GL_STATIC_DRAW);
        glVertexAttribPointer(attr_color, 4, GL_FLOAT, GL_FALSE, 0, 0);

        glEnableVertexAttribArray(attr_flags);
        if (m_dataChanged) {
          glBindBuffer(GL_ARRAY_BUFFER, m_pickingVBOs[i][2]);
          glBufferData(GL_ARRAY_BUFFER, size*sizeof(GLfloat), &(m_allFlags[start]), GL_STATIC_DRAW);
        } else {
          glBindBuffer(GL_ARRAY_BUFFER, m_VBOs[i][2]);
        }
        glVertexAttribPointer(attr_flags, 1, GL_FLOAT, GL_FALSE, 0, 0);

        if (m_dataChanged) {
          glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_pickingVBOs[i][3]);
          glBufferData(GL_ELEMENT_ARRAY_BUFFER, size*6/4*sizeof(GLuint), &(m_indexs[0]), GL_STATIC_DRAW);
        } else {
          glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_VBOs[i][4]);
        }

        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindVertexArray(0);
      }

      m_pickingDataChanged = false;
    }

    for (size_t i=0; i<numBatch; ++i) {
      size_t size = m_oneBatchNumber;
      if (i == numBatch-1)
        size = m_pointAndRadius.size() - (numBatch - 1) * m_oneBatchNumber;
      glBindVertexArray(m_pickingVAOs[i]);
      glDrawElements(GL_TRIANGLES, size * 6 / 4, GL_UNSIGNED_INT, 0);
      glBindVertexArray(0);
    }

  } else {
    if (m_pickingDataChanged) {
      for (size_t ivbo=0; ivbo<m_pickingVBOs.size(); ++ivbo) {
        glDeleteBuffers(m_pickingVBOs[ivbo].size(), &m_pickingVBOs[ivbo][0]);
      }
      m_pickingVBOs.resize(numBatch);
      for (size_t ivbo=0; ivbo<m_pickingVBOs.size(); ++ivbo) {
        m_pickingVBOs[ivbo].resize(4);
        glGenBuffers(m_pickingVBOs[ivbo].size(), &m_pickingVBOs[ivbo][0]);
      }
    }
    // set vertex data
    GLint attr_a_vertex_radius = shader.attributeLocation("attr_vertex_radius");
    GLint attr_color = shader.attributeLocation("attr_color");
    GLint attr_flags = shader.attributeLocation("attr_flags");

    for (size_t i=0; i<numBatch; ++i) {
      size_t size = m_oneBatchNumber;
      if (i == numBatch-1)
        size = m_pointAndRadius.size() - (numBatch - 1) * m_oneBatchNumber;
      size_t start = m_oneBatchNumber * i;

      glEnableVertexAttribArray(attr_a_vertex_radius);
      if (m_dataChanged) {
        glBindBuffer(GL_ARRAY_BUFFER, m_pickingVBOs[i][0]);
        if (m_pickingDataChanged)
          glBufferData(GL_ARRAY_BUFFER, size*4*sizeof(GLfloat), &(m_pointAndRadius[start]), GL_STATIC_DRAW);
      } else {
        glBindBuffer(GL_ARRAY_BUFFER, m_VBOs[i][0]);
      }
      glVertexAttribPointer(attr_a_vertex_radius, 4, GL_FLOAT, GL_FALSE, 0, 0);

      glEnableVertexAttribArray(attr_color);
      glBindBuffer(GL_ARRAY_BUFFER, m_pickingVBOs[i][1]);
      if (m_pickingDataChanged)
        glBufferData(GL_ARRAY_BUFFER, size*4*sizeof(GLfloat), &(m_pointPickingColors[start]), GL_STATIC_DRAW);
      glVertexAttribPointer(attr_color, 4, GL_FLOAT, GL_FALSE, 0, 0);

      glEnableVertexAttribArray(attr_flags);
      if (m_dataChanged) {
        glBindBuffer(GL_ARRAY_BUFFER, m_pickingVBOs[i][2]);
        if (m_pickingDataChanged)
          glBufferData(GL_ARRAY_BUFFER, size*sizeof(GLfloat), &(m_allFlags[start]), GL_STATIC_DRAW);
      } else {
        glBindBuffer(GL_ARRAY_BUFFER, m_VBOs[i][2]);
      }
      glVertexAttribPointer(attr_flags, 1, GL_FLOAT, GL_FALSE, 0, 0);

      if (m_dataChanged) {
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_pickingVBOs[i][3]);
        if (m_pickingDataChanged)
          glBufferData(GL_ELEMENT_ARRAY_BUFFER, size*6/4*sizeof(GLuint), &(m_indexs[0]), GL_STATIC_DRAW);
      } else {
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_VBOs[i][4]);
      }

      glDrawElements(GL_TRIANGLES, size*6/4, GL_UNSIGNED_INT, 0);

      glBindBuffer(GL_ARRAY_BUFFER, 0);
      glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
      glDisableVertexAttribArray(attr_a_vertex_radius);
      glDisableVertexAttribArray(attr_color);
      glDisableVertexAttribArray(attr_flags);
    }

    m_pickingDataChanged = false;
  }

  m_sphereShaderGrp.release();
}

void Z3DTriangleRenderer::appendDefaultColors()
{
  if (m_pointColors.size() < m_pointAndRadius.size()) {
    for (size_t i=m_pointColors.size(); i<m_pointAndRadius.size(); i++)
      m_pointColors.push_back(glm::vec4(0.f, 0.f, 0.f, 1.f));
  }
}
