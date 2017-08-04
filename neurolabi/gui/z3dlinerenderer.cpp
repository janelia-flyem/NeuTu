#include "z3dlinerenderer.h"

#include "z3dgl.h"
#include "z3dgpuinfo.h"
#include "z3dshaderprogram.h"
#include "z3dtexture.h"

Z3DLineRenderer::Z3DLineRenderer(Z3DRendererBase& rendererBase)
  : Z3DPrimitiveRenderer(rendererBase)
  , m_lineShaderGrp(rendererBase)
  , m_smoothLineShaderGrp(rendererBase)
  , m_smoothLineShaderGrp1(rendererBase)
  , m_linesPt(nullptr)
  , m_lineColorsPt(nullptr)
  , m_linePickingColorsPt(nullptr)
  , m_useSmoothLine(true)
  , m_srcLineWidth(1)
  , m_enableMultisample(true)
  , m_texture(nullptr)
  , m_VAO(1)
  , m_pickingVAO(1)
  , m_VBOs(2)
  , m_pickingVBOs(2)
  , m_dataChanged(false)
  , m_pickingDataChanged(false)
  , m_isLineStrip(false)
  , m_useTextureColor(false)
  , m_screenAligned(false)
  , m_roundCap(true)
  , m_VAOs(1)
  , m_pickingVAOs(1)
  , m_oneBatchNumber(4e6)
  , m_useGeomLineShader(false)
{
  updateLineWidth();
  connect(&m_rendererBase.geometriesMultisampleModePara(), &ZStringIntOptionParameter::valueChanged,
          this, &Z3DLineRenderer::updateLineWidth);
  setUseDisplayList(true);

  QStringList allshaders;
  allshaders << "line.vert" << "line_func.frag";
  QStringList normalShaders;
  normalShaders << "line.vert" << "line.frag";
  m_lineShaderGrp.init(allshaders, m_rendererBase.generateHeader() + generateHeader(), "",
                       normalShaders);
  m_lineShaderGrp.addAllSupportedPostShaders();

  if (m_useGeomLineShader) {
    // don't need in glsl 1.5
    m_smoothLineShaderGrp.setGeometryInputType(GL_LINES);
    m_smoothLineShaderGrp.setGeometryOutputType(GL_TRIANGLE_STRIP);
    m_smoothLineShaderGrp.setGeometryOutputVertexCount(4);
    allshaders.clear();
    allshaders << "wideline.vert" << "wideline.geom" << "wideline_func.frag";
    m_smoothLineShaderGrp.init(allshaders, m_rendererBase.generateHeader() + generateHeader(),
                               m_rendererBase.generateGeomHeader() + generateHeader());
    m_smoothLineShaderGrp.addAllSupportedPostShaders();
  } else {
    allshaders.clear();
    allshaders << "wideline1.vert" << "wideline_func1.frag";
    m_smoothLineShaderGrp1.init(allshaders, m_rendererBase.generateHeader() + generateHeader());
    m_smoothLineShaderGrp1.addAllSupportedPostShaders();
  }
  CHECK_GL_ERROR
}

void Z3DLineRenderer::setData(std::vector<glm::vec3>* linesInput)
{
  m_linesPt = linesInput;

  if (!m_useGeomLineShader) {
    m_smoothLineP0s.clear();
    m_smoothLineP1s.clear();
    m_indexs.clear();
    if (linesInput) {
      int indices[6] = {0, 1, 2, 2, 1, 3};
      int quadIdx = 0;
      if (m_isLineStrip) {
        for (size_t i = 1; i < linesInput->size(); ++i) {
          m_smoothLineP0s.push_back((*linesInput)[i - 1]);
          m_smoothLineP0s.push_back((*linesInput)[i - 1]);
          m_smoothLineP0s.push_back((*linesInput)[i - 1]);
          m_smoothLineP0s.push_back((*linesInput)[i - 1]);
          m_smoothLineP1s.push_back((*linesInput)[i]);
          m_smoothLineP1s.push_back((*linesInput)[i]);
          m_smoothLineP1s.push_back((*linesInput)[i]);
          m_smoothLineP1s.push_back((*linesInput)[i]);
          for (int k = 0; k < 6; ++k) {
            m_indexs.push_back(indices[k] + 4 * quadIdx);
          }
          quadIdx++;
        }
      } else {
        for (size_t i = 0; i + 1 < linesInput->size(); i += 2) {
          m_smoothLineP0s.push_back((*linesInput)[i]);
          m_smoothLineP0s.push_back((*linesInput)[i]);
          m_smoothLineP0s.push_back((*linesInput)[i]);
          m_smoothLineP0s.push_back((*linesInput)[i]);
          m_smoothLineP1s.push_back((*linesInput)[i + 1]);
          m_smoothLineP1s.push_back((*linesInput)[i + 1]);
          m_smoothLineP1s.push_back((*linesInput)[i + 1]);
          m_smoothLineP1s.push_back((*linesInput)[i + 1]);
          for (int k = 0; k < 6; ++k) {
            m_indexs.push_back(indices[k] + 4 * quadIdx);
          }
          quadIdx++;
        }
      }
      size_t rightUpSize = m_allFlags.size();
      float cornerFlags[4] = {0 << 4 | 0,      // (-1, -1) left down
                              2 << 4 | 0,      // (1, -1) right down
                              0 << 4 | 2,      // (-1, 1) left up
                              2 << 4 | 2};     // (1, 1) right up

      if (rightUpSize > m_smoothLineP0s.size()) {
        m_allFlags.resize(m_smoothLineP0s.size());
      } else if (rightUpSize < m_smoothLineP0s.size()) {
        m_allFlags.resize(m_smoothLineP0s.size());
        for (size_t i = rightUpSize; i < m_allFlags.size(); i += 4) {
          m_allFlags[i] = cornerFlags[0];
          m_allFlags[i + 1] = cornerFlags[1];
          m_allFlags[i + 2] = cornerFlags[2];
          m_allFlags[i + 3] = cornerFlags[3];
        }
      }
    }
  }

  invalidateOpenglRenderer();
  invalidateOpenglPickingRenderer();
  m_dataChanged = true;
  m_pickingDataChanged = true;
}

void Z3DLineRenderer::setDataColors(std::vector<glm::vec4>* lineColorsInput)
{
  if (m_useTextureColor) {
    m_useTextureColor = false;
    compile();
  }
  m_lineColorsPt = lineColorsInput;

  if (!m_useGeomLineShader) {
    m_smoothLineP0Colors.clear();
    m_smoothLineP1Colors.clear();
    if (lineColorsInput) {
      if (m_isLineStrip) {
        for (size_t i = 1; i < lineColorsInput->size(); ++i) {
          m_smoothLineP0Colors.push_back((*lineColorsInput)[i - 1]);
          m_smoothLineP0Colors.push_back((*lineColorsInput)[i - 1]);
          m_smoothLineP0Colors.push_back((*lineColorsInput)[i - 1]);
          m_smoothLineP0Colors.push_back((*lineColorsInput)[i - 1]);
          m_smoothLineP1Colors.push_back((*lineColorsInput)[i]);
          m_smoothLineP1Colors.push_back((*lineColorsInput)[i]);
          m_smoothLineP1Colors.push_back((*lineColorsInput)[i]);
          m_smoothLineP1Colors.push_back((*lineColorsInput)[i]);
        }
      } else {
        for (size_t i = 0; i + 1 < lineColorsInput->size(); i += 2) {
          m_smoothLineP0Colors.push_back((*lineColorsInput)[i]);
          m_smoothLineP0Colors.push_back((*lineColorsInput)[i]);
          m_smoothLineP0Colors.push_back((*lineColorsInput)[i]);
          m_smoothLineP0Colors.push_back((*lineColorsInput)[i]);
          m_smoothLineP1Colors.push_back((*lineColorsInput)[i + 1]);
          m_smoothLineP1Colors.push_back((*lineColorsInput)[i + 1]);
          m_smoothLineP1Colors.push_back((*lineColorsInput)[i + 1]);
          m_smoothLineP1Colors.push_back((*lineColorsInput)[i + 1]);
        }
      }
    }
  }
  invalidateOpenglRenderer();
  m_dataChanged = true;
}

void Z3DLineRenderer::setTexture(Z3DTexture* tex)
{
  if (m_useSmoothLine) {
    if (!m_useTextureColor) {
      m_useTextureColor = true;
      compile();
      m_dataChanged = true;
    }
    m_texture = tex;
    CHECK(m_texture->textureTarget() == GL_TEXTURE_1D);
  }
}

void Z3DLineRenderer::setDataPickingColors(std::vector<glm::vec4>* linePickingColorsInput)
{
  m_linePickingColorsPt = linePickingColorsInput;

  if (!m_useGeomLineShader) {
    m_smoothLinePickingColors.clear();
    if (linePickingColorsInput) {
      if (m_isLineStrip) {
        for (size_t i = 1; i < linePickingColorsInput->size(); ++i) {
          m_smoothLinePickingColors.push_back((*linePickingColorsInput)[i - 1]);
          m_smoothLinePickingColors.push_back((*linePickingColorsInput)[i - 1]);
          m_smoothLinePickingColors.push_back((*linePickingColorsInput)[i - 1]);
          m_smoothLinePickingColors.push_back((*linePickingColorsInput)[i - 1]);
        }
      } else {
        for (size_t i = 0; i + 1 < linePickingColorsInput->size(); i += 2) {
          m_smoothLinePickingColors.push_back((*linePickingColorsInput)[i]);
          m_smoothLinePickingColors.push_back((*linePickingColorsInput)[i]);
          m_smoothLinePickingColors.push_back((*linePickingColorsInput)[i]);
          m_smoothLinePickingColors.push_back((*linePickingColorsInput)[i]);
        }
      }
    }
  }
  invalidateOpenglPickingRenderer();
  m_pickingDataChanged = true;
}

void Z3DLineRenderer::setRoundCap(bool v)
{
  m_roundCap = v;
  if (m_roundCap)
    m_screenAligned = false;
  compile();
}

void Z3DLineRenderer::setScreenAlign(bool v)
{
  m_screenAligned = v;
  if (m_screenAligned)
    m_roundCap = false;
  compile();
}

void Z3DLineRenderer::compile()
{
  m_lineShaderGrp.rebuild(m_rendererBase.generateHeader() + generateHeader());
  if (m_useGeomLineShader)
    m_smoothLineShaderGrp.rebuild(m_rendererBase.generateHeader() + generateHeader(),
                                  m_rendererBase.generateGeomHeader() + generateHeader());
  else
    m_smoothLineShaderGrp1.rebuild(m_rendererBase.generateHeader() + generateHeader());
}

QString Z3DLineRenderer::generateHeader()
{
  QString headerSource;
  if (m_useTextureColor)
    headerSource += "#define USE_1DTEXTURE\n";
  if (m_screenAligned)
    headerSource += "#define LINE_SCREEN_ALIGNED\n";
  if (m_roundCap)
    headerSource += "#define ROUND_CAP\n";
  return headerSource;
}

float Z3DLineRenderer::lineWidth() const
{
  if (m_followSizeScale) {
    if (m_useSmoothLine)
      return std::max(1.f, m_lineWidth * m_rendererBase.sizeScale());
    else
      return std::min(Z3DGpuInfo::instance().maxAliasedLineWidth(),
                      std::max(m_rendererBase.sizeScale() * m_lineWidth,
                               Z3DGpuInfo::instance().minAliasedLineWidth()));
  } else {
    if (m_useSmoothLine)
      return m_lineWidth;
    else
      return std::min(Z3DGpuInfo::instance().maxAliasedLineWidth(),
                      std::max(m_lineWidth,
                               Z3DGpuInfo::instance().minAliasedLineWidth()));
  }
}

std::vector<glm::vec4>* Z3DLineRenderer::lineColors()
{
  if (!m_lineColorsPt) {
    m_lineColors.assign(m_linesPt->size(), glm::vec4(0.f, 0.f, 0.f, 1.f));
    return &m_lineColors;
  } else if (m_lineColorsPt->size() < m_linesPt->size()) {
    m_lineColors.clear();
    for (const auto& color : *m_lineColorsPt) {
      m_lineColors.push_back(color);
    }
    for (size_t i = m_lineColorsPt->size(); i < m_linesPt->size(); ++i) {
      m_lineColors.emplace_back(0.f, 0.f, 0.f, 1.f);
    }
    return &m_lineColors;
  }

  return m_lineColorsPt;
}

#ifndef ATLAS_USE_CORE_PROFILE
void Z3DLineRenderer::renderUsingOpengl()
{
  if (!m_linesPt || m_linesPt->empty())
    return;

  std::vector<glm::vec4> * colors = lineColors();

  if ((*colors)[0].a != opacity()) {
    for (size_t i=0; i<colors->size(); ++i)
      (*colors)[i].a = opacity();
  }

  glLineWidth(lineWidth());
  glPointSize(lineWidth());

  glMatrixMode(GL_MODELVIEW);
  glPushMatrix();
  //glScalef(getCoordTransform().x, getCoordTransform().y, getCoordTransform().z);
  glMultMatrixf(&coordTransform()[0][0]);   // not sure, todo check

  GLuint bufObjects[2];
  glGenBuffers(2, bufObjects);

  glEnableClientState(GL_VERTEX_ARRAY);
  glBindBuffer(GL_ARRAY_BUFFER, bufObjects[0]);
  glBufferData(GL_ARRAY_BUFFER, m_linesPt->size()*3*sizeof(GLfloat), m_linesPt->data(), GL_STATIC_DRAW);
  glVertexPointer(3, GL_FLOAT, 0, 0);

  glEnableClientState(GL_COLOR_ARRAY);
  glBindBuffer(GL_ARRAY_BUFFER, bufObjects[1]);
  glBufferData(GL_ARRAY_BUFFER, colors->size()*4*sizeof(GLfloat), colors->data(), GL_STATIC_DRAW);
  glColorPointer(4, GL_FLOAT, 0, 0);

  glDrawArrays(GL_LINES, 0, m_linesPt->size());
  glDrawArrays(GL_POINTS, 0, m_linesPt->size());

  glBindBuffer(GL_ARRAY_BUFFER, 0);
  glDeleteBuffers(2, bufObjects);
  glDisableClientState(GL_VERTEX_ARRAY);
  glDisableClientState(GL_COLOR_ARRAY);

  glMatrixMode(GL_MODELVIEW);
  glPopMatrix();

  glLineWidth(1.0);
  glPointSize(1.0);
}

void Z3DLineRenderer::renderPickingUsingOpengl()
{
  if (!m_linesPt || m_linesPt->empty())
    return;

  if (!m_linePickingColorsPt || m_linePickingColorsPt->empty()
      || m_linePickingColorsPt->size() != m_linesPt->size())
    return;

  glLineWidth(lineWidth());
  glPointSize(lineWidth());

  glMatrixMode(GL_MODELVIEW);
  glPushMatrix();
  //glScalef(getCoordTransform().x, getCoordTransform().y, getCoordTransform().z);
  glMultMatrixf(&coordTransform()[0][0]);   // not sure, todo check

  GLuint bufObjects[2];
  glGenBuffers(2, bufObjects);

  glEnableClientState(GL_VERTEX_ARRAY);
  glBindBuffer(GL_ARRAY_BUFFER, bufObjects[0]);
  glBufferData(GL_ARRAY_BUFFER, m_linesPt->size()*3*sizeof(GLfloat), m_linesPt->data(), GL_STATIC_DRAW);
  glVertexPointer(3, GL_FLOAT, 0, 0);

  glEnableClientState(GL_COLOR_ARRAY);
  glBindBuffer(GL_ARRAY_BUFFER, bufObjects[1]);
  glBufferData(GL_ARRAY_BUFFER, m_linePickingColorsPt->size()*4*sizeof(GLfloat), m_linePickingColorsPt->data(), GL_STATIC_DRAW);
  glColorPointer(4, GL_FLOAT, 0, 0);

  glDrawArrays(GL_LINES, 0, m_linesPt->size());
  glDrawArrays(GL_POINTS, 0, m_linesPt->size());

  glBindBuffer(GL_ARRAY_BUFFER, 0);
  glDeleteBuffers(2, bufObjects);
  glDisableClientState(GL_VERTEX_ARRAY);
  glDisableClientState(GL_COLOR_ARRAY);

  glMatrixMode(GL_MODELVIEW);
  glPopMatrix();

  glLineWidth(1.0);
  glPointSize(1.0);
}
#endif

void Z3DLineRenderer::render(Z3DEye eye)
{
  if (!m_linesPt || m_linesPt->empty())
    return;

  if (!m_useGeomLineShader && m_useSmoothLine) {
    renderSmooth(eye);
    return;
  }

  const std::vector<glm::vec4>* colors = lineColors();

  if (!m_useSmoothLine) {
    glLineWidth(lineWidth());
    glPointSize(lineWidth());
  }

  currentShaderGrp().bind();
  Z3DShaderProgram& shader = currentShaderGrp().get();
  m_rendererBase.setGlobalShaderParameters(shader, eye);
  setShaderParameters(shader);
  shader.setLineWidthUniform(m_lineWidth);
  if (m_useTextureColor)
    shader.bindTexture("texture", m_texture);

  if (m_hardwareSupportVAO) {
    if (m_dataChanged) {
      m_VAO.bind();
      GLint attr_vertex = shader.vertexAttributeLocation();

      glEnableVertexAttribArray(attr_vertex);
      m_VBOs.bind(GL_ARRAY_BUFFER, 0);
      glBufferData(GL_ARRAY_BUFFER, m_linesPt->size() * 3 * sizeof(GLfloat), m_linesPt->data(), GL_STATIC_DRAW);
      glVertexAttribPointer(attr_vertex, 3, GL_FLOAT, GL_FALSE, 0, 0);

      if (!m_useTextureColor) {
        GLint attr_color = shader.colorAttributeLocation();
        glEnableVertexAttribArray(attr_color);
        m_VBOs.bind(GL_ARRAY_BUFFER, 1);
        glBufferData(GL_ARRAY_BUFFER, colors->size() * 4 * sizeof(GLfloat), colors->data(), GL_STATIC_DRAW);
        glVertexAttribPointer(attr_color, 4, GL_FLOAT, GL_FALSE, 0, 0);
      }

      glBindBuffer(GL_ARRAY_BUFFER, 0);
      m_VAO.release();

      m_dataChanged = false;
    }

    m_VAO.bind();
    if (!m_lineWidthArray.empty()) {
      for (size_t i = 0; i < m_lineWidthArray.size(); ++i) {
        if (!m_useSmoothLine)
          glLineWidth(m_lineWidthArray[i]);
        if (m_isLineStrip)
          glDrawArrays(GL_LINE_STRIP, i * 2, 2);
        else
          glDrawArrays(GL_LINES, i * 2, 2);
      }
    } else {
      if (m_isLineStrip)
        glDrawArrays(GL_LINE_STRIP, 0, m_linesPt->size());
      else
        glDrawArrays(GL_LINES, 0, m_linesPt->size());
    }

#ifndef _FLYEM_
    if (!m_useSmoothLine)
      glDrawArrays(GL_POINTS, 0, m_linesPt->size());
#endif
    m_VAO.release();

  } else {
    GLint attr_vertex = shader.vertexAttributeLocation();
    glEnableVertexAttribArray(attr_vertex);
    m_VBOs.bind(GL_ARRAY_BUFFER, 0);
    if (m_dataChanged)
      glBufferData(GL_ARRAY_BUFFER, m_linesPt->size() * 3 * sizeof(GLfloat), m_linesPt->data(), GL_STATIC_DRAW);
    glVertexAttribPointer(attr_vertex, 3, GL_FLOAT, GL_FALSE, 0, 0);

    GLint attr_color = -1;
    if (!m_useTextureColor) {
      attr_color = shader.colorAttributeLocation();
      glEnableVertexAttribArray(attr_color);
      m_VBOs.bind(GL_ARRAY_BUFFER, 1);
      if (m_dataChanged)
        glBufferData(GL_ARRAY_BUFFER, colors->size() * 4 * sizeof(GLfloat), colors->data(), GL_STATIC_DRAW);
      glVertexAttribPointer(attr_color, 4, GL_FLOAT, GL_FALSE, 0, 0);
    }

    if (!m_lineWidthArray.empty()) {
      for (size_t i = 0; i < m_lineWidthArray.size(); ++i) {
        if (!m_useSmoothLine)
          glLineWidth(m_lineWidthArray[i]);
        if (m_isLineStrip)
          glDrawArrays(GL_LINE_STRIP, i * 2, 2);
        else
          glDrawArrays(GL_LINES, i * 2, 2);
      }
    } else {
      if (m_isLineStrip)
        glDrawArrays(GL_LINE_STRIP, 0, m_linesPt->size());
      else
        glDrawArrays(GL_LINES, 0, m_linesPt->size());
    }

#ifndef _FLYEM_
    if (!m_useSmoothLine)
      glDrawArrays(GL_POINTS, 0, m_linesPt->size());
#endif

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    if (!m_useTextureColor)
      glDisableVertexAttribArray(attr_color);
    glDisableVertexAttribArray(attr_vertex);

    m_dataChanged = false;
  }

  glLineWidth(1.0);
  glPointSize(1.0);

  currentShaderGrp().release();
}

void Z3DLineRenderer::renderPicking(Z3DEye eye)
{
  if (!m_linesPt || m_linesPt->empty())
    return;

  if (!m_linePickingColorsPt || m_linePickingColorsPt->empty()
      || m_linePickingColorsPt->size() != m_linesPt->size())
    return;

  if (!m_useGeomLineShader && m_useSmoothLine) {
    renderSmoothPicking(eye);
    return;
  }

  if (!m_useSmoothLine) {
    glLineWidth(lineWidth());
    glPointSize(lineWidth());
  }

  currentShaderGrp().bind();
  Z3DShaderProgram& shader = currentShaderGrp().get();
  m_rendererBase.setGlobalShaderParameters(shader, eye);
  setPickingShaderParameters(shader);
  shader.setLineWidthUniform(m_lineWidth);

  if (m_hardwareSupportVAO) {
    if (m_pickingDataChanged) {
      m_pickingVAO.bind();
      GLint attr_vertex = shader.vertexAttributeLocation();
      GLint attr_color = shader.colorAttributeLocation();

      glEnableVertexAttribArray(attr_vertex);
      if (m_dataChanged) {
        m_pickingVBOs.bind(GL_ARRAY_BUFFER, 0);
        glBufferData(GL_ARRAY_BUFFER, m_linesPt->size() * 3 * sizeof(GLfloat), m_linesPt->data(), GL_STATIC_DRAW);
      } else {
        m_VBOs.bind(GL_ARRAY_BUFFER, 0);
      }
      glVertexAttribPointer(attr_vertex, 3, GL_FLOAT, GL_FALSE, 0, 0);

      glEnableVertexAttribArray(attr_color);
      m_pickingVBOs.bind(GL_ARRAY_BUFFER, 1);
      glBufferData(GL_ARRAY_BUFFER, m_linePickingColorsPt->size() * 4 * sizeof(GLfloat), m_linePickingColorsPt->data(),
                   GL_STATIC_DRAW);
      glVertexAttribPointer(attr_color, 4, GL_FLOAT, GL_FALSE, 0, 0);

      glBindBuffer(GL_ARRAY_BUFFER, 0);
      m_pickingVAO.release();

      m_pickingDataChanged = false;
    }

    m_pickingVAO.bind();
    if (m_isLineStrip) {
      glDrawArrays(GL_LINE_STRIP, 0, m_linesPt->size());
    } else {
      glDrawArrays(GL_LINES, 0, m_linesPt->size());
    }
    m_pickingVAO.release();

  } else {
    GLint attr_vertex = shader.vertexAttributeLocation();
    GLint attr_color = shader.colorAttributeLocation();

    glEnableVertexAttribArray(attr_vertex);
    if (m_dataChanged) {
      m_pickingVBOs.bind(GL_ARRAY_BUFFER, 0);
      if (m_pickingDataChanged)
        glBufferData(GL_ARRAY_BUFFER, m_linesPt->size() * 3 * sizeof(GLfloat), m_linesPt->data(), GL_STATIC_DRAW);
    } else {
      m_VBOs.bind(GL_ARRAY_BUFFER, 0);
    }
    glVertexAttribPointer(attr_vertex, 3, GL_FLOAT, GL_FALSE, 0, 0);

    glEnableVertexAttribArray(attr_color);
    m_pickingVBOs.bind(GL_ARRAY_BUFFER, 1);
    if (m_pickingDataChanged)
      glBufferData(GL_ARRAY_BUFFER, m_linePickingColorsPt->size() * 4 * sizeof(GLfloat), m_linePickingColorsPt->data(),
                   GL_STATIC_DRAW);
    glVertexAttribPointer(attr_color, 4, GL_FLOAT, GL_FALSE, 0, 0);

    if (m_isLineStrip) {
      glDrawArrays(GL_LINE_STRIP, 0, m_linesPt->size());
    } else {
      glDrawArrays(GL_LINES, 0, m_linesPt->size());
    }

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glDisableVertexAttribArray(attr_color);
    glDisableVertexAttribArray(attr_vertex);

    m_pickingDataChanged = false;
  }

  glLineWidth(1.0);
  glPointSize(1.0);

  currentShaderGrp().release();
}

void Z3DLineRenderer::renderSmooth(Z3DEye eye)
{
  if (m_smoothLineP0Colors.size() < m_smoothLineP0s.size()) {
    for (size_t i = m_smoothLineP0Colors.size(); i < m_smoothLineP0s.size(); ++i) {
      m_smoothLineP0Colors.emplace_back(0.f, 0.f, 0.f, 1.f);
      m_smoothLineP1Colors.emplace_back(0.f, 0.f, 0.f, 1.f);
    }
  }

  m_smoothLineShaderGrp1.bind();
  Z3DShaderProgram& shader = m_smoothLineShaderGrp1.get();
  m_rendererBase.setGlobalShaderParameters(shader, eye);
  setShaderParameters(shader);
  shader.setLineWidthUniform(m_lineWidth);
  if (m_useTextureColor)
    shader.bindTexture("texture", m_texture);

  size_t numBatch = std::ceil(m_smoothLineP0s.size() * 1.0 / m_oneBatchNumber);

  if (m_hardwareSupportVAO) {
    if (m_dataChanged) {
      m_VAOs.resize(numBatch);

      m_batchVBOs.resize(numBatch);
      for (size_t ivbo = 0; ivbo < m_batchVBOs.size(); ++ivbo) {
        m_batchVBOs[ivbo].resize(6);
      }

      // set vertex data
      GLint attr_p0 = shader.p0AttributeLocation();
      GLint attr_p1 = shader.p1AttributeLocation();
      GLint attr_p0color;
      GLint attr_p1color;
      if (!m_useTextureColor) {
        attr_p0color = shader.p0ColorAttributeLocation();
        attr_p1color = shader.p1ColorAttributeLocation();
      }
      GLint attr_flags = shader.flagsAttributeLocation();

      for (size_t i = 0; i < numBatch; ++i) {
        m_VAOs.bind(i);
        size_t size = m_oneBatchNumber;
        if (i == numBatch - 1)
          size = m_smoothLineP0s.size() - (numBatch - 1) * m_oneBatchNumber;
        size_t start = m_oneBatchNumber * i;

        glEnableVertexAttribArray(attr_p0);
        m_batchVBOs[i].bind(GL_ARRAY_BUFFER, 0);
        glBufferData(GL_ARRAY_BUFFER, size * 3 * sizeof(GLfloat), &(m_smoothLineP0s[start]), GL_STATIC_DRAW);
        glVertexAttribPointer(attr_p0, 3, GL_FLOAT, GL_FALSE, 0, 0);

        glEnableVertexAttribArray(attr_p1);
        m_batchVBOs[i].bind(GL_ARRAY_BUFFER, 1);
        glBufferData(GL_ARRAY_BUFFER, size * 3 * sizeof(GLfloat), &(m_smoothLineP1s[start]), GL_STATIC_DRAW);
        glVertexAttribPointer(attr_p1, 3, GL_FLOAT, GL_FALSE, 0, 0);

        if (!m_useTextureColor) {
          glEnableVertexAttribArray(attr_p0color);
          m_batchVBOs[i].bind(GL_ARRAY_BUFFER, 3);
          glBufferData(GL_ARRAY_BUFFER, size * 4 * sizeof(GLfloat), &(m_smoothLineP0Colors[start]), GL_STATIC_DRAW);
          glVertexAttribPointer(attr_p0color, 4, GL_FLOAT, GL_FALSE, 0, 0);

          glEnableVertexAttribArray(attr_p1color);
          m_batchVBOs[i].bind(GL_ARRAY_BUFFER, 4);
          glBufferData(GL_ARRAY_BUFFER, size * 4 * sizeof(GLfloat), &(m_smoothLineP1Colors[start]), GL_STATIC_DRAW);
          glVertexAttribPointer(attr_p1color, 4, GL_FLOAT, GL_FALSE, 0, 0);
        }

        glEnableVertexAttribArray(attr_flags);
        m_batchVBOs[i].bind(GL_ARRAY_BUFFER, 2);
        glBufferData(GL_ARRAY_BUFFER, size * sizeof(GLfloat), &(m_allFlags[start]), GL_STATIC_DRAW);
        glVertexAttribPointer(attr_flags, 1, GL_FLOAT, GL_FALSE, 0, 0);

        m_batchVBOs[i].bind(GL_ELEMENT_ARRAY_BUFFER, 5);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, size * 6 / 4 * sizeof(GLuint), m_indexs.data(), GL_STATIC_DRAW);

        glBindBuffer(GL_ARRAY_BUFFER, 0);
        m_VAOs.release();
      }

      m_dataChanged = false;
    }

    for (size_t i = 0; i < numBatch; ++i) {
      size_t size = m_oneBatchNumber;
      if (i == numBatch - 1)
        size = m_smoothLineP0s.size() - (numBatch - 1) * m_oneBatchNumber;
      m_VAOs.bind(i);
      glDrawElements(GL_TRIANGLES, size * 6 / 4, GL_UNSIGNED_INT, 0);
      m_VAOs.release();
    }

  } else {
    if (m_dataChanged) {
      m_batchVBOs.resize(numBatch);
      for (size_t ivbo = 0; ivbo < m_batchVBOs.size(); ++ivbo) {
        m_batchVBOs[ivbo].resize(6);
      }
    }
    // set vertex data
    GLint attr_p0 = shader.p0AttributeLocation();
    GLint attr_p1 = shader.p1AttributeLocation();
    GLint attr_p0color = -1;
    GLint attr_p1color = -1;
    if (!m_useTextureColor) {
      attr_p0color = shader.p0ColorAttributeLocation();
      attr_p1color = shader.p1ColorAttributeLocation();
    }
    GLint attr_flags = shader.flagsAttributeLocation();

    for (size_t i = 0; i < numBatch; ++i) {
      size_t size = m_oneBatchNumber;
      if (i == numBatch - 1)
        size = m_smoothLineP0s.size() - (numBatch - 1) * m_oneBatchNumber;
      size_t start = m_oneBatchNumber * i;

      glEnableVertexAttribArray(attr_p0);
      m_batchVBOs[i].bind(GL_ARRAY_BUFFER, 0);
      if (m_dataChanged)
        glBufferData(GL_ARRAY_BUFFER, size * 3 * sizeof(GLfloat), &(m_smoothLineP0s[start]), GL_STATIC_DRAW);
      glVertexAttribPointer(attr_p0, 3, GL_FLOAT, GL_FALSE, 0, 0);

      glEnableVertexAttribArray(attr_p1);
      m_batchVBOs[i].bind(GL_ARRAY_BUFFER, 1);
      if (m_dataChanged)
        glBufferData(GL_ARRAY_BUFFER, size * 3 * sizeof(GLfloat), &(m_smoothLineP1s[start]), GL_STATIC_DRAW);
      glVertexAttribPointer(attr_p1, 3, GL_FLOAT, GL_FALSE, 0, 0);

      if (!m_useTextureColor) {
        glEnableVertexAttribArray(attr_p0color);
        m_batchVBOs[i].bind(GL_ARRAY_BUFFER, 3);
        if (m_dataChanged)
          glBufferData(GL_ARRAY_BUFFER, size * 4 * sizeof(GLfloat), &(m_smoothLineP0Colors[start]), GL_STATIC_DRAW);
        glVertexAttribPointer(attr_p0color, 4, GL_FLOAT, GL_FALSE, 0, 0);

        glEnableVertexAttribArray(attr_p1color);
        m_batchVBOs[i].bind(GL_ARRAY_BUFFER, 4);
        if (m_dataChanged)
          glBufferData(GL_ARRAY_BUFFER, size * 4 * sizeof(GLfloat), &(m_smoothLineP1Colors[start]), GL_STATIC_DRAW);
        glVertexAttribPointer(attr_p1color, 4, GL_FLOAT, GL_FALSE, 0, 0);
      }

      glEnableVertexAttribArray(attr_flags);
      m_batchVBOs[i].bind(GL_ARRAY_BUFFER, 2);
      if (m_dataChanged)
        glBufferData(GL_ARRAY_BUFFER, size * sizeof(GLfloat), &(m_allFlags[start]), GL_STATIC_DRAW);
      glVertexAttribPointer(attr_flags, 1, GL_FLOAT, GL_FALSE, 0, 0);

      m_batchVBOs[i].bind(GL_ELEMENT_ARRAY_BUFFER, 5);
      if (m_dataChanged)
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, size * 6 / 4 * sizeof(GLuint), m_indexs.data(), GL_STATIC_DRAW);

      glDrawElements(GL_TRIANGLES, size * 6 / 4, GL_UNSIGNED_INT, 0);

      glBindBuffer(GL_ARRAY_BUFFER, 0);
      glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
      glDisableVertexAttribArray(attr_p0);
      glDisableVertexAttribArray(attr_p1);
      if (!m_useTextureColor) {
        glDisableVertexAttribArray(attr_p0color);
        glDisableVertexAttribArray(attr_p1color);
      }
      glDisableVertexAttribArray(attr_flags);
    }

    m_dataChanged = false;
  }

  m_smoothLineShaderGrp1.release();
}

void Z3DLineRenderer::renderSmoothPicking(Z3DEye eye)
{
  m_smoothLineShaderGrp1.bind();
  Z3DShaderProgram& shader = m_smoothLineShaderGrp1.get();
  m_rendererBase.setGlobalShaderParameters(shader, eye);
  setPickingShaderParameters(shader);
  shader.setLineWidthUniform(m_lineWidth);

  size_t numBatch = std::ceil(m_smoothLineP0s.size() * 1.0 / m_oneBatchNumber);

  if (m_hardwareSupportVAO) {
    if (m_pickingDataChanged) {
      m_pickingVAOs.resize(numBatch);

      m_batchPickingVBOs.resize(numBatch);
      for (size_t ivbo = 0; ivbo < m_batchPickingVBOs.size(); ++ivbo) {
        m_batchPickingVBOs[ivbo].resize(5);
      }

      // set vertex data
      GLint attr_p0 = shader.p0AttributeLocation();
      GLint attr_p1 = shader.p1AttributeLocation();
      GLint attr_p0color = shader.p0ColorAttributeLocation();
      GLint attr_p1color = shader.p1ColorAttributeLocation();
      GLint attr_flags = shader.flagsAttributeLocation();

      for (size_t i = 0; i < numBatch; ++i) {
        m_pickingVAOs.bind(i);
        size_t size = m_oneBatchNumber;
        if (i == numBatch - 1)
          size = m_smoothLineP0s.size() - (numBatch - 1) * m_oneBatchNumber;
        size_t start = m_oneBatchNumber * i;

        glEnableVertexAttribArray(attr_p0);
        if (m_dataChanged) {
          m_batchPickingVBOs[i].bind(GL_ARRAY_BUFFER, 0);
          glBufferData(GL_ARRAY_BUFFER, size * 3 * sizeof(GLfloat), &(m_smoothLineP0s[start]), GL_STATIC_DRAW);
        } else {
          m_batchVBOs[i].bind(GL_ARRAY_BUFFER, 0);
        }
        glVertexAttribPointer(attr_p0, 3, GL_FLOAT, GL_FALSE, 0, 0);

        glEnableVertexAttribArray(attr_p1);
        if (m_dataChanged) {
          m_batchPickingVBOs[i].bind(GL_ARRAY_BUFFER, 1);
          glBufferData(GL_ARRAY_BUFFER, size * 3 * sizeof(GLfloat), &(m_smoothLineP1s[start]), GL_STATIC_DRAW);
        } else {
          m_batchVBOs[i].bind(GL_ARRAY_BUFFER, 1);
        }
        glVertexAttribPointer(attr_p1, 3, GL_FLOAT, GL_FALSE, 0, 0);

        glEnableVertexAttribArray(attr_p0color);
        m_batchPickingVBOs[i].bind(GL_ARRAY_BUFFER, 2);
        glBufferData(GL_ARRAY_BUFFER, size * 4 * sizeof(GLfloat), &(m_smoothLinePickingColors[start]), GL_STATIC_DRAW);
        glVertexAttribPointer(attr_p0color, 4, GL_FLOAT, GL_FALSE, 0, 0);

        glEnableVertexAttribArray(attr_p1color);
        glVertexAttribPointer(attr_p1color, 4, GL_FLOAT, GL_FALSE, 0, 0);

        glEnableVertexAttribArray(attr_flags);
        if (m_dataChanged) {
          m_batchPickingVBOs[i].bind(GL_ARRAY_BUFFER, 3);
          glBufferData(GL_ARRAY_BUFFER, size * sizeof(GLfloat), &(m_allFlags[start]), GL_STATIC_DRAW);
        } else {
          m_batchVBOs[i].bind(GL_ARRAY_BUFFER, 2);
        }
        glVertexAttribPointer(attr_flags, 1, GL_FLOAT, GL_FALSE, 0, 0);

        if (m_dataChanged) {
          m_batchPickingVBOs[i].bind(GL_ELEMENT_ARRAY_BUFFER, 4);
          glBufferData(GL_ELEMENT_ARRAY_BUFFER, size * 6 / 4 * sizeof(GLuint), m_indexs.data(), GL_STATIC_DRAW);
        } else {
          m_batchVBOs[i].bind(GL_ELEMENT_ARRAY_BUFFER, 5);
        }

        glBindBuffer(GL_ARRAY_BUFFER, 0);
        m_pickingVAOs.release();
      }

      m_pickingDataChanged = false;
    }

    for (size_t i = 0; i < numBatch; ++i) {
      size_t size = m_oneBatchNumber;
      if (i == numBatch - 1)
        size = m_smoothLineP0s.size() - (numBatch - 1) * m_oneBatchNumber;
      m_pickingVAOs.bind(i);
      glDrawElements(GL_TRIANGLES, size * 6 / 4, GL_UNSIGNED_INT, 0);
      m_pickingVAOs.release();
    }

  } else {
    if (m_pickingDataChanged) {
      m_batchPickingVBOs.resize(numBatch);
      for (size_t ivbo = 0; ivbo < m_batchPickingVBOs.size(); ++ivbo) {
        m_batchPickingVBOs[ivbo].resize(5);
      }
    }

    // set vertex data
    GLint attr_p0 = shader.p0AttributeLocation();
    GLint attr_p1 = shader.p1AttributeLocation();
    GLint attr_p0color = shader.p0ColorAttributeLocation();
    GLint attr_p1color = shader.p1ColorAttributeLocation();
    GLint attr_flags = shader.flagsAttributeLocation();

    for (size_t i = 0; i < numBatch; ++i) {
      size_t size = m_oneBatchNumber;
      if (i == numBatch - 1)
        size = m_smoothLineP0s.size() - (numBatch - 1) * m_oneBatchNumber;
      size_t start = m_oneBatchNumber * i;

      glEnableVertexAttribArray(attr_p0);
      if (m_dataChanged) {
        m_batchPickingVBOs[i].bind(GL_ARRAY_BUFFER, 0);
        if (m_pickingDataChanged)
          glBufferData(GL_ARRAY_BUFFER, size * 3 * sizeof(GLfloat), &(m_smoothLineP0s[start]), GL_STATIC_DRAW);
      } else {
        m_batchVBOs[i].bind(GL_ARRAY_BUFFER, 0);
      }
      glVertexAttribPointer(attr_p0, 3, GL_FLOAT, GL_FALSE, 0, 0);

      glEnableVertexAttribArray(attr_p1);
      if (m_dataChanged) {
        m_batchPickingVBOs[i].bind(GL_ARRAY_BUFFER, 1);
        if (m_pickingDataChanged)
          glBufferData(GL_ARRAY_BUFFER, size * 3 * sizeof(GLfloat), &(m_smoothLineP1s[start]), GL_STATIC_DRAW);
      } else {
        m_batchVBOs[i].bind(GL_ARRAY_BUFFER, 1);
      }
      glVertexAttribPointer(attr_p1, 3, GL_FLOAT, GL_FALSE, 0, 0);

      glEnableVertexAttribArray(attr_p0color);
      m_batchPickingVBOs[i].bind(GL_ARRAY_BUFFER, 2);
      glBufferData(GL_ARRAY_BUFFER, size * 4 * sizeof(GLfloat), &(m_smoothLinePickingColors[start]), GL_STATIC_DRAW);
      glVertexAttribPointer(attr_p0color, 4, GL_FLOAT, GL_FALSE, 0, 0);

      glEnableVertexAttribArray(attr_p1color);
      glVertexAttribPointer(attr_p1color, 4, GL_FLOAT, GL_FALSE, 0, 0);


      glEnableVertexAttribArray(attr_flags);
      if (m_dataChanged) {
        m_batchPickingVBOs[i].bind(GL_ARRAY_BUFFER, 3);
        if (m_pickingDataChanged)
          glBufferData(GL_ARRAY_BUFFER, size * sizeof(GLfloat), &(m_allFlags[start]), GL_STATIC_DRAW);
      } else {
        m_batchVBOs[i].bind(GL_ARRAY_BUFFER, 2);
      }
      glVertexAttribPointer(attr_flags, 1, GL_FLOAT, GL_FALSE, 0, 0);

      if (m_dataChanged) {
        m_batchPickingVBOs[i].bind(GL_ELEMENT_ARRAY_BUFFER, 4);
        if (m_pickingDataChanged)
          glBufferData(GL_ELEMENT_ARRAY_BUFFER, size * 6 / 4 * sizeof(GLuint), m_indexs.data(), GL_STATIC_DRAW);
      } else {
        m_batchVBOs[i].bind(GL_ELEMENT_ARRAY_BUFFER, 5);
      }

      glDrawElements(GL_TRIANGLES, size * 6 / 4, GL_UNSIGNED_INT, 0);

      glBindBuffer(GL_ARRAY_BUFFER, 0);
      glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
      glDisableVertexAttribArray(attr_p0);
      glDisableVertexAttribArray(attr_p1);
      glDisableVertexAttribArray(attr_p0color);
      glDisableVertexAttribArray(attr_p1color);
      glDisableVertexAttribArray(attr_flags);
    }

    m_pickingDataChanged = false;
  }

  m_smoothLineShaderGrp1.release();
}

//void Z3DLineRenderer::enableLineSmooth()
//{
//#if defined(_WIN32) || defined(_WIN64)
//  if (Z3DGpuInfoInstance.getGpuVendor() == Z3DGpuInfo::GPU_VENDOR_ATI) {
//    return;
//  }
//#endif
//  return;
//  if (m_rendererBase->getShaderHookType() == Z3DRendererBase::Normal) {
//    glPushAttrib(GL_ALL_ATTRIB_BITS);
//    glDisable(GL_MULTISAMPLE);
//    glEnable(GL_LINE_SMOOTH);
//    glEnable(GL_BLEND);
//    glBlendFunc(GL_ONE,GL_ONE_MINUS_SRC_ALPHA);
//  } else {
//    glPushAttrib(GL_LINE_BIT);
//    glDisable(GL_MULTISAMPLE);
//    glEnable(GL_LINE_SMOOTH);
//  }
//}

//void Z3DLineRenderer::disableLineSmooth()
//{
//#if defined(_WIN32) || defined(_WIN64)
//  if (Z3DGpuInfoInstance.getGpuVendor() == Z3DGpuInfo::GPU_VENDOR_ATI) {
//    return;
//  }
//#endif
//  return;
//  glPopAttrib();
//}
