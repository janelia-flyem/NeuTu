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
    std::vector<glm::vec3> norm;
    std::vector<glm::vec2> texc;

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

void Cube::setFaceColor(glm::vec4 c)
{
    Mesh::setColor(colors, c, 36);
}

void Cube::setEdgeColor(glm::vec4 c)
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
  nCubes = 0;

  //
  m_screen.init(glm::vec3(-1.0f, -1.0f, 0.0f), glm::vec3(1.0f, -1.0f, 0.0f), glm::vec3(1.0f, 1.0f, 0.0f), glm::vec3(-1.0f, 1.0f, 0.0f));
}

Z3DCubeRenderer::~Z3DCubeRenderer()
{
}

void Z3DCubeRenderer::addCube(int sx, int sy, int sz, int tx, int ty, int tz, glm::vec4 color)
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
  Z3DShaderProgram &oit3DTransparentizeShader = m_cubeShaderGrp.get();

  m_rendererBase->setGlobalShaderParameters(oit3DTransparentizeShader, eye);
  oit3DTransparentizeShader.setUniformValue("lighting_enabled", m_needLighting);
  oit3DTransparentizeShader.setUniformValue("pos_scale", getCoordScales());

  nCubes = m_cubes.size();

  m_screenShaderGrp.bind();
  Z3DShaderProgram &oit2DComposeShader = m_screenShaderGrp.get();
  oit2DComposeShader.setUniformValue("pos_scale", getCoordScales());

  //
  float h = glm::degrees(m_rendererBase->getCamera().getFieldOfView());
  float w = m_rendererBase->getCamera().getAspectRatio() * h;

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

          glBindBuffer(GL_ARRAY_BUFFER, 0);
          glBindVertexArray(0);
      }

      // compose pass
      GLint cloc_position = oit2DComposeShader.attributeLocation("vPosition");
      GLint cloc_accum = oit2DComposeShader.attributeLocation("accumTexture");
      GLint cloc_revealage = oit2DComposeShader.attributeLocation("revealageTexture");

      glGenVertexArrays(1, &m_vao);
      glBindVertexArray( m_vao );

      glGenBuffers(1, &m_vbo);
      glBindBuffer( GL_ARRAY_BUFFER, m_vbo );
      glBufferData( GL_ARRAY_BUFFER, m_screen.positions.size()*sizeof(glm::vec3), &(m_screen.positions[0]), GL_STATIC_DRAW );

      glEnableVertexAttribArray( cloc_position );
      glVertexAttribPointer( cloc_position, 3, GL_FLOAT, GL_FALSE, 0, (GLvoid*)(0) );

      //
      glBindVertexArray(0);

      // frame buffer
      glGenFramebuffers(1, &m_fbo);
      glBindFramebuffer(GL_FRAMEBUFFER, m_fbo);

      glGenTextures(1, &m_rt);
      glBindTexture(GL_TEXTURE_2D, m_rt);

      glGenRenderbuffers(1, &m_db);
      glBindRenderbuffer(GL_RENDERBUFFER, m_db);

      glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB32F, w, h, 0, GL_RGB, GL_FLOAT, 0);

      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

      glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, w, h);
      glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, m_db);

      glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_rt, 0);

      glActiveTexture(GL_TEXTURE0);
      glBindTexture(GL_TEXTURE_2D, m_rt);
      glUniform1i(cloc_accum, 0);
      glUniform1i(cloc_revealage, 1);

      glBindFramebuffer(GL_FRAMEBUFFER, 0);

      m_dataChanged = false;
    }

    // render
    // oit pass
    glDepthMask (GL_FALSE);
    glEnable(GL_BLEND);

    glBlendFuncSeparate(GL_ONE, GL_ONE, GL_ZERO, GL_ONE_MINUS_SRC_ALPHA);

    for(int i=0; i<nCubes; i++)
    {
        glBindVertexArray( m_VAOs[i] );
        glDrawArrays( GL_TRIANGLES, 0, m_cubes[i].positions.size() );
        glBindVertexArray(0);
    }

    glDepthMask (GL_TRUE);
    glDisable (GL_BLEND);

    // compose pass
    glEnable(GL_BLEND);
    glBlendFunc(GL_ONE_MINUS_SRC_ALPHA, GL_SRC_ALPHA);

    glBindVertexArray( m_vao );
    glDrawArrays( GL_TRIANGLES, 0, m_screen.positions.size());
    glBindVertexArray(0);

    glDisable (GL_BLEND);

  } else {
    // w/o vao defined
  }

  m_cubeShaderGrp.release();
  m_screenShaderGrp.release();
}

void Z3DCubeRenderer::renderPicking(Z3DEye eye)
{
}
