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

void Mesh::setPositions(glm::vec3 a, glm::vec3 b)
{
    ePositions.push_back(a);
    ePositions.push_back(b);
}

void Mesh::setPositions(glm::vec3 a, glm::vec3 b, glm::vec3 c)
{
    positions.push_back(a);
    positions.push_back(b);
    positions.push_back(c);
}

void Mesh::setPositions(glm::vec3 a, glm::vec3 b, glm::vec3 c, glm::vec3 d)
{
    setPositions(a,b,c);
    setPositions(c,d,a);
}

void Mesh::setColor(std::vector<glm::vec4> &color, glm::vec4 c, int n)
{
    for(int i=0; i<n; i++)
    {
        color.push_back(c);
    }
}

//
Triangle::Triangle() : Mesh()
{

}

Triangle::~Triangle()
{

}

void Triangle::init(glm::vec3 a, glm::vec3 b, glm::vec3 c)
{
    glm::vec3 norm = glm::normalize( glm::cross(b - a, c - b) );

    setPositions(a,b,c);

    for(int i=0; i<3; i++)
    {
        normals.push_back(norm);
    }
}

void Triangle::setColor(glm::vec4 c)
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

void Quadrilateral::init(glm::vec3 a, glm::vec3 b, glm::vec3 c, glm::vec3 d)
{
    glm::vec3 norm = glm::normalize( glm::cross(b - a, c - b) );

    setPositions(a,b,c,d);

    for(int i=0; i<4; i++)
    {
        normals.push_back(norm);
    }
}

void Quadrilateral::setColor(glm::vec4 c)
{
    Mesh::setColor(colors,c,6);
}

//
Cube::Cube() : Quadrilateral()
{
    norm.push_back( glm::vec3(1.0f, 0.0f, 0.0f) );  // +x
    norm.push_back( glm::vec3(-1.0f, 0.0f, 0.0f) ); // -x
    norm.push_back( glm::vec3(0.0f, 1.0f, 0.0f) );  // +y
    norm.push_back( glm::vec3(0.0f, -1.0f, 0.0f) ); // -y
    norm.push_back( glm::vec3(0.0f, 0.0f, 1.0f) );  // +z
    norm.push_back( glm::vec3(0.0f, 0.0f, -1.0f) ); // -z

    // a,b,c
    texc.push_back( glm::vec2(0, 0) );
    texc.push_back( glm::vec2(1, 0) );
    texc.push_back( glm::vec2(1, 1) );
    // c,d,a
    texc.push_back( glm::vec2(1, 1) );
    texc.push_back( glm::vec2(0, 1) );
    texc.push_back( glm::vec2(0, 0) );

    for (int i=0; i<6; i++)
    {
        for (int j=0; j<6; j++)
        {
            normals.push_back( norm[i] );
            texCoords.push_back( texc[j] );
        }
    }

    //
    for(int i=0; i<6; i++)
        b_visible[i] = true;

    //
    nVertices = 0;
}

Cube::~Cube()
{
}

void Cube::init(double sx, double sy, double sz, double tx, double ty, double tz)
{
    double cx = sx/2.0;
    double cy = sy/2.0;
    double cz = sz/2.0;

    double l = -cx - tx;
    double r = cx - tx;
    double u = cy - ty;
    double d = -cy - ty;
    double f = -cz - tz;
    double b = cz - tz;

    x = tx;
    y = ty;
    z = tz;

    //
    points.push_back( glm::vec3( l, d, f ) ); // 0
    points.push_back( glm::vec3( l, u, f ) ); // 1
    points.push_back( glm::vec3( r, u, f ) ); // 2
    points.push_back( glm::vec3( r, d, f ) ); // 3
    points.push_back( glm::vec3( l, d, b ) ); // 4
    points.push_back( glm::vec3( l, u, b ) ); // 5
    points.push_back( glm::vec3( r, u, b ) ); // 6
    points.push_back( glm::vec3( r, d, b ) ); // 7

    //
    faces();

    //
    edges();

    //
    for (int i=0; i<6; i++)
    {
        if(b_visible[i])
        {
            for (int j=0; j<6; j++)
            {
                normals.push_back( norm[i] );
                texCoords.push_back( texc[j] );
            }
        }
    }
}

void Cube::initIdentity()
{
    init(1,1,1,0,0,0);
}

void Cube::faces()
{
    // 12 triangles: 36 vertices and 36 colors

    //
    if(b_visible[0])
    {
        setPositions(points[7], points[3], points[2], points[6]); // GL_TEXTURE_CUBE_MAP_POSITIVE_X 	0 +x Right
        nVertices += 6;
    }
    if(b_visible[1])
    {
        setPositions(points[0], points[4], points[5], points[1]); // GL_TEXTURE_CUBE_MAP_NEGATIVE_X 	1 -x Left
        nVertices += 6;
    }
    if(b_visible[2])
    {
        setPositions(points[1], points[5], points[6], points[2]); // GL_TEXTURE_CUBE_MAP_POSITIVE_Y 	2 +y Up (Top)
        nVertices += 6;
    }
    if(b_visible[3])
    {
        setPositions(points[4], points[0], points[3], points[7]); // GL_TEXTURE_CUBE_MAP_NEGATIVE_Y 	3 -y Down (Bottom)
        nVertices += 6;
    }
    if(b_visible[4])
    {
        setPositions(points[6], points[5], points[4], points[7]); // GL_TEXTURE_CUBE_MAP_POSITIVE_Z 	4 +z Back
        nVertices += 6;
    }
    if(b_visible[5])
    {
        setPositions(points[3], points[0], points[1], points[2]); // GL_TEXTURE_CUBE_MAP_NEGATIVE_Z 	5 -z Front
        nVertices += 6;
    }

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

void Cube::setFaceColor(glm::vec4 c)
{
    Mesh::setColor(colors, c, nVertices);
}

void Cube::setEdgeColor(glm::vec4 c)
{
    Mesh::setColor(eColors, c, 24);
}

void Cube::setVisible(bool v[6])
{
    for(int i=0; i<6; i++)
        b_visible[i] = v[i];
}

//
Z3DCubeRenderer::Z3DCubeRenderer(QObject *parent)
  : Z3DPrimitiveRenderer(parent)
  , m_cubeShaderGrp()
  , m_dataChanged(false)
  , m_pickingDataChanged(false)
  , m_oneBatchNumber(4e6)
{
  setNeedLighting(true);
  setUseDisplayList(true);

  //
  oit2DComposeProgram = NULL;
  m_vao = 0;
  m_fbo = 0;
  m_renderbuffer = 0;
  m_texture = 0;

  //
  nCubes = 0;

  //
  m_cubes.clear();

  //
  m_screen << QVector3D(-1.0f, -1.0f, 0.0f) // (a-b-c)
           << QVector3D( 1.0f, -1.0f, 0.0f)
           << QVector3D( 1.0f,  1.0f, 0.0f)
           << QVector3D( 1.0f,  1.0f, 0.0f) // (c-d-a)
           << QVector3D(-1.0f,  1.0f, 0.0f)
           << QVector3D(-1.0f, -1.0f, 0.0f);
}

Z3DCubeRenderer::~Z3DCubeRenderer()
{
}

void Z3DCubeRenderer::addCube(double l, double x, double y, double z, glm::vec4 color, bool v[6])
{
    addCube(l,l,l,x,y,z,color,v);
}

void Z3DCubeRenderer::addCube(double sx, double sy, double sz, double tx, double ty, double tz, glm::vec4 color, bool v[6])
{
    Cube cube;

    //
    cube.setVisible(v);
    cube.init(sx,sy,sz,tx,ty,tz);
    cube.setFaceColor(color);

    //
    m_cubes.push_back(cube);

    //
    m_dataChanged = true;
}

void Z3DCubeRenderer::compile()
{
  m_dataChanged = true;
  m_cubeShaderGrp.rebuild(generateHeader());
}

void Z3DCubeRenderer::initialize()
{
  Z3DPrimitiveRenderer::initialize();
  QStringList cubeShaders, screenShaders;
  cubeShaders << "cube_wboit.vert" << "lighting.frag" << "cube_wboit.frag";
  m_cubeShaderGrp.init(QStringList(), generateHeader(), m_rendererBase, cubeShaders);
  m_cubeShaderGrp.addAllSupportedPostShaders();

  //
  oit2DComposeProgram = new QGLShaderProgram;
  oit2DComposeProgram->addShaderFromSourceFile(QGLShader::Vertex, ":/Resources/shader/cube_wboit_compose.vert");
  oit2DComposeProgram->addShaderFromSourceFile(QGLShader::Fragment, ":/Resources/shader/cube_wboit_compose.frag");
  if (!oit2DComposeProgram->link())
  {
      qWarning() << oit2DComposeProgram->log() << endl;
  }

  //
  glGenFramebuffers(1, &m_fbo);
  glBindFramebuffer(GL_FRAMEBUFFER, m_fbo);

  glGenTextures(1, &m_texture);
  glBindTexture(GL_TEXTURE_2D, m_texture);

  glGenRenderbuffers(1, &m_renderbuffer);
  glBindRenderbuffer(GL_RENDERBUFFER, m_renderbuffer);

  glBindFramebuffer(GL_FRAMEBUFFER, 0);

  //
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

  //
  if (oit2DComposeProgram)
  {
      oit2DComposeProgram->release();
      delete oit2DComposeProgram;
      oit2DComposeProgram = NULL;
  }

  if(m_vao)
  {
    glDeleteBuffers(1, &m_vao);
  }

  if(m_fbo)
  {
    glDeleteBuffers(1, &m_fbo);
  }

  if(m_renderbuffer)
  {
    glDeleteBuffers(1, &m_renderbuffer);
  }

  if(m_texture)
  {
    glDeleteBuffers(1, &m_texture);
  }

  m_cubeShaderGrp.removeAllShaders();
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
  Z3DShaderProgram &oit3DTransparentizeShader = m_cubeShaderGrp.get();
  m_rendererBase->setGlobalShaderParameters(oit3DTransparentizeShader, eye);
  oit3DTransparentizeShader.setUniformValue("lighting_enabled", m_needLighting);
  oit3DTransparentizeShader.setUniformValue("pos_scale", getCoordScales());

  nCubes = m_cubes.size();

  // size of view
//  double theta, neardist, w, h;
//  theta = glm::degrees(m_rendererBase->getCamera().getFieldOfView());
//  neardist = m_rendererBase->getCamera().getNearDist();
//  h = neardist * glm::tan(theta);
//  w = m_rendererBase->getCamera().getAspectRatio() * h;

  double w, h;
  w = m_rendererBase->getViewport().z;
  h = m_rendererBase->getViewport().w;

  //
  if (m_hardwareSupportVAO) {
    if (m_dataChanged) {
      if (!m_VAOs.empty()) {
        glDeleteVertexArrays(m_VAOs.size(), &m_VAOs[0]);
      }
      m_VAOs.resize(nCubes);
      glGenVertexArrays(m_VAOs.size(), &m_VAOs[0]);

      if (!m_VBOs.empty()) {
        glDeleteVertexArrays(m_VBOs.size(), &m_VBOs[0]);
      }
      m_VBOs.resize(nCubes);
      glGenBuffers( m_VBOs.size(), &m_VBOs[0]);

      //glBindVertexArray(m_VAO);
      // oit pass
      GLint loc_position = oit3DTransparentizeShader.attributeLocation("vPosition");
      GLint loc_normal = oit3DTransparentizeShader.attributeLocation("vNormal");
      GLint loc_color = oit3DTransparentizeShader.attributeLocation("vColor");

      for (size_t i=0; i<nCubes; ++i)
      {
          glBindVertexArray(m_VAOs[i]);
          glBindBuffer( GL_ARRAY_BUFFER, m_VBOs[i] );

          size_t size_position = sizeof(glm::vec3)*m_cubes[i].positions.size();
          size_t size_normal = sizeof(glm::vec3)*m_cubes[i].normals.size();
          size_t size_color = sizeof(glm::vec4)*m_cubes[i].colors.size();

          glBufferData( GL_ARRAY_BUFFER, size_position + size_normal + size_color, NULL, GL_STATIC_DRAW );
          glBufferSubData( GL_ARRAY_BUFFER, 0, size_position, &(m_cubes[i].positions[0]) );
          glBufferSubData( GL_ARRAY_BUFFER, size_position, size_normal, &(m_cubes[i].normals[0]) );
          glBufferSubData( GL_ARRAY_BUFFER, size_position + size_normal, size_color, &(m_cubes[i].colors[0]) );

          //
          glEnableVertexAttribArray( loc_position );
          glVertexAttribPointer( loc_position, 3, GL_FLOAT, GL_FALSE, 0, (GLvoid*)(0) );

          glEnableVertexAttribArray( loc_normal );
          glVertexAttribPointer( loc_normal, 3, GL_FLOAT, GL_FALSE, 0, (GLvoid*)(size_position) );

          glEnableVertexAttribArray( loc_color );
          glVertexAttribPointer( loc_color, 4, GL_FLOAT, GL_FALSE, 0, (GLvoid*)(size_position + size_normal));

          glBindBuffer( GL_ARRAY_BUFFER, 0);
          glBindVertexArray(0);
      }

      m_dataChanged = false;
    }

    // compose pass
    // vao
    glGenVertexArrays(1, &m_vao);
    glBindVertexArray( m_vao );
    oit2DComposeProgram->setAttributeArray("vPosition", m_screen.constData());
    oit2DComposeProgram->enableAttributeArray("vPosition");
    glBindVertexArray(0);

    // fbo
    glGetIntegerv(GL_DRAW_FRAMEBUFFER_BINDING, &m_preFBO);

    glBindFramebuffer(GL_FRAMEBUFFER, m_fbo);

    glBindTexture(GL_TEXTURE_2D, m_texture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB32F, w, h, 0, GL_RGB, GL_FLOAT, 0);

    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_texture, 0);
    glBindTexture(GL_TEXTURE_2D, 0);

    glBindRenderbuffer(GL_RENDERBUFFER, m_renderbuffer);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, w, h);

    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, m_renderbuffer);
    glBindRenderbuffer(GL_RENDERBUFFER, 0);

    glBindFramebuffer(GL_FRAMEBUFFER, m_preFBO);

    // render
    // oit pass
    glDepthMask(GL_FALSE);
    glEnable(GL_BLEND);

    glBlendFuncSeparate(GL_ONE, GL_ONE, GL_ZERO, GL_ONE_MINUS_SRC_ALPHA);

    for(size_t i=0; i<nCubes; i++)
    {
        glBindVertexArray( m_VAOs[i] );
        glDrawArrays( GL_TRIANGLES, 0, m_cubes[i].positions.size() );
        glBindVertexArray(0);
    }

    glDepthMask(GL_TRUE);
    glDisable(GL_BLEND);

    // compose pass
    if (!oit2DComposeProgram->bind())
    {
        qWarning() << oit2DComposeProgram->log() << endl;
    }

    //
    glBlendFunc(GL_ONE_MINUS_SRC_ALPHA, GL_SRC_ALPHA);

    glBindFramebuffer(GL_FRAMEBUFFER, m_fbo);

    //
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, m_texture);

    GLuint cloc_accum = glGetUniformLocation(oit2DComposeProgram->programId(), "accumTexture");
    glUniform1i(cloc_accum, 0);

    GLuint cloc_revealage = glGetUniformLocation(oit2DComposeProgram->programId(), "revealageTexture");
    glUniform1i(cloc_revealage, 1);

//    GLenum DrawBuffers[1] = {GL_COLOR_ATTACHMENT0};
//    glDrawBuffers(1, DrawBuffers); // "1" is the size of DrawBuffers

    glBindVertexArray( m_vao );
    glDrawArrays( GL_TRIANGLES, 0, m_screen.size());
    glBindVertexArray(0);

    glBindFramebuffer(GL_FRAMEBUFFER, m_preFBO);

    glDisable(GL_BLEND);
  } else {
    // w/o vao defined
  }

  m_cubeShaderGrp.release();
}

void Z3DCubeRenderer::renderPicking(Z3DEye eye)
{
}

bool Z3DCubeRenderer::isEmpty()
{
    return m_cubes.empty();
}
