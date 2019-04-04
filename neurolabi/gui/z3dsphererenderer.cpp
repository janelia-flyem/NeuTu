#include "z3dsphererenderer.h"

#include "z3dgl.h"
#include "z3dgpuinfo.h"
#include "z3dshaderprogram.h"

Z3DSphereRenderer::Z3DSphereRenderer(Z3DRendererBase& rendererBase)
  : Z3DPrimitiveRenderer(rendererBase)
  , m_sphereShaderGrp(rendererBase)
  , m_sphereSlicesStacks("Sphere Slices Stacks", 36, 20, 100)
  , m_useDynamicMaterial("Calculate Material Property From Intensity", true)
  //  , m_VBOs(5)
  //  , m_pickingVBOs(4)
  , m_VAOs(1)
  , m_pickingVAOs(1)
  , m_dataChanged(false)
  , m_pickingDataChanged(false)
  , m_oneBatchNumber(4e6)
{
#if !defined(_USE_CORE_PROFILE_) && defined(_SUPPORT_FIXED_PIPELINE_)
  setUseDisplayList(true);
  connect(&m_sphereSlicesStacks, &ZIntParameter::valueChanged, this, &Z3DSphereRenderer::invalidateOpenglRenderer);
  connect(&m_useDynamicMaterial, &ZBoolParameter::valueChanged, this, &Z3DSphereRenderer::invalidateOpenglRenderer);
#endif

  connect(&m_useDynamicMaterial, &ZBoolParameter::valueChanged, this, &Z3DSphereRenderer::compile);
  //addParameter(m_sphereSlicesStacks);
  //addParameter(m_useDynamicMaterial);

  QStringList allshaders;
  allshaders << "sphere.vert" << "sphere_func.frag" << "lighting2.frag";
  m_sphereShaderGrp.init(allshaders, m_rendererBase.generateHeader() + generateHeader());
  m_sphereShaderGrp.addAllSupportedPostShaders();
}

void Z3DSphereRenderer::setData(std::vector<glm::vec4>* pointAndRadiusInput,
                                std::vector<glm::vec4>* specularAndShininessInput)
{
  m_pointAndRadius.clear();
  m_specularAndShininess.clear();
  m_indexs.clear();
  int indices[6] = {0, 1, 2, 2, 1, 3};
  int quadIdx = 0;
  for (auto pr : *pointAndRadiusInput) {
    m_pointAndRadius.push_back(pr);
    m_pointAndRadius.push_back(pr);
    m_pointAndRadius.push_back(pr);
    m_pointAndRadius.push_back(pr);
    for (int k = 0; k < 6; ++k) {
      m_indexs.push_back(indices[k] + 4 * quadIdx);
    }
    quadIdx++;
  }
  if (!specularAndShininessInput) {
    m_useDynamicMaterial.set(false);
  } else {
    for (auto ss : *specularAndShininessInput) {
      m_specularAndShininess.push_back(ss);
      m_specularAndShininess.push_back(ss);
      m_specularAndShininess.push_back(ss);
      m_specularAndShininess.push_back(ss);
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
    for (size_t i = rightUpSize; i < m_allFlags.size(); i += 4) {
      m_allFlags[i] = cornerFlags[0];
      m_allFlags[i + 1] = cornerFlags[1];
      m_allFlags[i + 2] = cornerFlags[2];
      m_allFlags[i + 3] = cornerFlags[3];
    }
  }
#if !defined(_USE_CORE_PROFILE_) && defined(_SUPPORT_FIXED_PIPELINE_)
  invalidateOpenglRenderer();
  invalidateOpenglPickingRenderer();
#endif
  m_dataChanged = true;
  m_pickingDataChanged = true;
}

void Z3DSphereRenderer::setDataColors(std::vector<glm::vec4>* pointColorsInput)
{
  m_pointColors.clear();
  for (auto color : *pointColorsInput) {
    m_pointColors.push_back(color);
    m_pointColors.push_back(color);
    m_pointColors.push_back(color);
    m_pointColors.push_back(color);
  }
#if !defined(_USE_CORE_PROFILE_) && defined(_SUPPORT_FIXED_PIPELINE_)
  invalidateOpenglRenderer();
#endif
  m_dataChanged = true;
}

void Z3DSphereRenderer::setDataPickingColors(std::vector<glm::vec4>* pointPickingColorsInput)
{
  m_pointPickingColors.clear();
  if (!pointPickingColorsInput)
    return;
  m_pointPickingColors.resize(pointPickingColorsInput->size() * 4);
  for (size_t i = 0; i < pointPickingColorsInput->size(); ++i) {
    const glm::vec4& color = (*pointPickingColorsInput)[i];
    size_t index = i * 4;
    m_pointPickingColors[index] = color;
    m_pointPickingColors[index + 1] = color;
    m_pointPickingColors[index + 2] = color;
    m_pointPickingColors[index + 3] = color;
  }
  /*
  for (auto color : *pointPickingColorsInput) {
    m_pointPickingColors.push_back(color);
    m_pointPickingColors.push_back(color);
    m_pointPickingColors.push_back(color);
    m_pointPickingColors.push_back(color);
  }
  */

#if !defined(_USE_CORE_PROFILE_) && defined(_SUPPORT_FIXED_PIPELINE_)
  invalidateOpenglPickingRenderer();
#endif
  m_pickingDataChanged = true;
}

void Z3DSphereRenderer::compile()
{
  m_dataChanged = true;
  m_sphereShaderGrp.rebuild(m_rendererBase.generateHeader() + generateHeader());
}

QString Z3DSphereRenderer::generateHeader()
{
  QString headerSource;
  if (m_useDynamicMaterial.get())
    headerSource += "#define DYNAMIC_MATERIAL_PROPERTY\n";
  return headerSource;
}

#if !defined(_USE_CORE_PROFILE_) && defined(_SUPPORT_FIXED_PIPELINE_)
void Z3DSphereRenderer::renderUsingOpengl()
{
  if (m_pointAndRadius.empty())
    return;
  appendDefaultColors();

  GLUquadricObj* quadric = gluNewQuadric();
  for (size_t i=0; i<m_pointAndRadius.size(); i+=4) {
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glm::vec3 loc = glm::applyMatrix(coordTransform(), glm::vec3(m_pointAndRadius[i].xyz()));
    glTranslatef(loc.x, loc.y, loc.z);
    float diameter = m_pointAndRadius[i].w * sizeScale() * 2;
    glScalef(diameter, diameter, diameter);
    // overwrite material property setted by z3drendererbase
    if (m_useDynamicMaterial.get() && !m_specularAndShininess.empty()) {
      glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, m_specularAndShininess[i].w);
      glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, glm::value_ptr(glm::vec4(m_specularAndShininess[i].xyz(), 1.f)));
    }
    glColor4fv(glm::value_ptr(glm::vec4(m_pointColors[i].rgb(), m_pointColors[i].a * opacity())));
    gluSphere(quadric, .5, m_sphereSlicesStacks.get(), m_sphereSlicesStacks.get());
    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();
  }
  gluDeleteQuadric(quadric);
}

void Z3DSphereRenderer::renderPickingUsingOpengl()
{
  if (m_pointAndRadius.empty())
    return;
  if (m_pointPickingColors.empty() || m_pointAndRadius.size() != m_pointPickingColors.size())
    return;
  GLUquadricObj* quadric = gluNewQuadric();
  for (size_t i=0; i<m_pointAndRadius.size(); i+=4) {
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glm::vec3 loc = glm::applyMatrix(coordTransform(), glm::vec3(m_pointAndRadius[i].xyz()));
    glTranslatef(loc.x, loc.y, loc.z);
    float radius = m_pointAndRadius[i].w * sizeScale();
    glScalef(radius, radius, radius);
    glColor4fv(glm::value_ptr(m_pointPickingColors[i]));
    gluSphere(quadric, 1., 12, 12/*m_sphereSlicesStacks.get(), m_sphereSlicesStacks.get()*/);
    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();
  }
  gluDeleteQuadric(quadric);
}
#endif

void Z3DSphereRenderer::render(Z3DEye eye)
{
  /*
  if (renderingPickingOnly()) {
    return;
  }
  */

  if (m_pointAndRadius.empty())
    return;
  appendDefaultColors();


  m_sphereShaderGrp.bind();
  Z3DShaderProgram& shader = m_sphereShaderGrp.get();

  m_rendererBase.setGlobalShaderParameters(shader, eye);
  setShaderParameters(shader);

  float fovy = glm::degrees(m_rendererBase.camera().fieldOfView());
  float adj;
  if (fovy <= 90.f) {
    adj = 1.0027 + 0.000111 * fovy + 0.000098 * fovy * fovy;
  } else {
    adj = 2.02082 - 0.033935 * fovy + 0.00037854 * fovy * fovy;
  }
  shader.setBoxCorrectionUniform(adj);

  size_t numBatch = std::ceil(m_pointAndRadius.size() * 1.0 / m_oneBatchNumber);

  if (m_hardwareSupportVAO) {
    if (m_dataChanged) {
      m_VAOs.resize(numBatch);

      m_VBOs.resize(numBatch);
      for (size_t ivbo = 0; ivbo < m_VBOs.size(); ++ivbo) {
        m_VBOs[ivbo].resize(5);
      }

      // set vertex data
      GLint attr_a_vertex_radius = shader.vertexAttributeLocation();
      GLint attr_a_specular_shininess;
      if (m_useDynamicMaterial.get() && !m_specularAndShininess.empty()) {
        attr_a_specular_shininess = shader.specularShininessAttributeLocation();
      }
      GLint attr_color = shader.colorAttributeLocation();
      GLint attr_flags = shader.flagsAttributeLocation();

      for (size_t i = 0; i < numBatch; ++i) {
        m_VAOs.bind(i);
        size_t size = m_oneBatchNumber;
        if (i == numBatch - 1)
          size = m_pointAndRadius.size() - (numBatch - 1) * m_oneBatchNumber;
        size_t start = m_oneBatchNumber * i;

        glEnableVertexAttribArray(attr_a_vertex_radius);
        m_VBOs[i].bind(GL_ARRAY_BUFFER, 0);
        glBufferData(GL_ARRAY_BUFFER, size * 4 * sizeof(GLfloat), &(m_pointAndRadius[start]), GL_STATIC_DRAW);
        glVertexAttribPointer(attr_a_vertex_radius, 4, GL_FLOAT, GL_FALSE, 0, 0);

        if (m_useDynamicMaterial.get() && !m_specularAndShininess.empty()) {
          glEnableVertexAttribArray(attr_a_specular_shininess);
          m_VBOs[i].bind(GL_ARRAY_BUFFER, 3);
          glBufferData(GL_ARRAY_BUFFER, size * 4 * sizeof(GLfloat), &(m_specularAndShininess[start]), GL_STATIC_DRAW);
          glVertexAttribPointer(attr_a_specular_shininess, 4, GL_FLOAT, GL_FALSE, 0, 0);
        }

        glEnableVertexAttribArray(attr_color);
        m_VBOs[i].bind(GL_ARRAY_BUFFER, 1);
        glBufferData(GL_ARRAY_BUFFER, size * 4 * sizeof(GLfloat), &(m_pointColors[start]), GL_STATIC_DRAW);
        glVertexAttribPointer(attr_color, 4, GL_FLOAT, GL_FALSE, 0, 0);

        glEnableVertexAttribArray(attr_flags);
        m_VBOs[i].bind(GL_ARRAY_BUFFER, 2);
        glBufferData(GL_ARRAY_BUFFER, size * sizeof(GLfloat), &(m_allFlags[start]), GL_STATIC_DRAW);
        glVertexAttribPointer(attr_flags, 1, GL_FLOAT, GL_FALSE, 0, 0);

        m_VBOs[i].bind(GL_ELEMENT_ARRAY_BUFFER, 4);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, size * 6 / 4 * sizeof(GLuint), m_indexs.data(), GL_STATIC_DRAW);

        glBindBuffer(GL_ARRAY_BUFFER, 0);
        m_VAOs.release();
      }

      m_dataChanged = false;
    }

    for (size_t i = 0; i < numBatch; ++i) {
      size_t size = m_oneBatchNumber;
      if (i == numBatch - 1)
        size = m_pointAndRadius.size() - (numBatch - 1) * m_oneBatchNumber;
      m_VAOs.bind(i);
      glDrawElements(GL_TRIANGLES, size * 6 / 4, GL_UNSIGNED_INT, 0);
      m_VAOs.release();
    }

  } else {
    if (m_dataChanged) {
      m_VBOs.resize(numBatch);
      for (size_t ivbo = 0; ivbo < m_VBOs.size(); ++ivbo) {
        m_VBOs[ivbo].resize(5);
      }
    }
    // set vertex data
    GLint attr_a_vertex_radius = shader.vertexAttributeLocation();
    GLint attr_a_specular_shininess = -1;
    if (m_useDynamicMaterial.get() && !m_specularAndShininess.empty()) {
      attr_a_specular_shininess = shader.specularShininessAttributeLocation();
    }
    GLint attr_color = shader.colorAttributeLocation();
    GLint attr_flags = shader.flagsAttributeLocation();

    for (size_t i = 0; i < numBatch; ++i) {
      size_t size = m_oneBatchNumber;
      if (i == numBatch - 1)
        size = m_pointAndRadius.size() - (numBatch - 1) * m_oneBatchNumber;
      size_t start = m_oneBatchNumber * i;

      glEnableVertexAttribArray(attr_a_vertex_radius);
      m_VBOs[i].bind(GL_ARRAY_BUFFER, 0);
      if (m_dataChanged)
        glBufferData(GL_ARRAY_BUFFER, size * 4 * sizeof(GLfloat), &(m_pointAndRadius[start]), GL_STATIC_DRAW);
      glVertexAttribPointer(attr_a_vertex_radius, 4, GL_FLOAT, GL_FALSE, 0, 0);

      if (m_useDynamicMaterial.get() && !m_specularAndShininess.empty()) {
        glEnableVertexAttribArray(attr_a_specular_shininess);
        m_VBOs[i].bind(GL_ARRAY_BUFFER, 3);
        if (m_dataChanged)
          glBufferData(GL_ARRAY_BUFFER, size * 4 * sizeof(GLfloat), &(m_specularAndShininess[start]), GL_STATIC_DRAW);
        glVertexAttribPointer(attr_a_specular_shininess, 4, GL_FLOAT, GL_FALSE, 0, 0);
      }

      glEnableVertexAttribArray(attr_color);
      m_VBOs[i].bind(GL_ARRAY_BUFFER, 1);
      if (m_dataChanged)
        glBufferData(GL_ARRAY_BUFFER, size * 4 * sizeof(GLfloat), &(m_pointColors[start]), GL_STATIC_DRAW);
      glVertexAttribPointer(attr_color, 4, GL_FLOAT, GL_FALSE, 0, 0);

      glEnableVertexAttribArray(attr_flags);
      m_VBOs[i].bind(GL_ARRAY_BUFFER, 2);
      if (m_dataChanged)
        glBufferData(GL_ARRAY_BUFFER, size * sizeof(GLfloat), &(m_allFlags[start]), GL_STATIC_DRAW);
      glVertexAttribPointer(attr_flags, 1, GL_FLOAT, GL_FALSE, 0, 0);

      m_VBOs[i].bind(GL_ELEMENT_ARRAY_BUFFER, 4);
      if (m_dataChanged)
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, size * 6 / 4 * sizeof(GLuint), m_indexs.data(), GL_STATIC_DRAW);

      glDrawElements(GL_TRIANGLES, size * 6 / 4, GL_UNSIGNED_INT, 0);

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

void Z3DSphereRenderer::renderPicking(Z3DEye eye)
{
  if (m_pointAndRadius.empty())
    return;

  if (m_pointPickingColors.empty() || m_pointAndRadius.size() != m_pointPickingColors.size())
    return;

  m_sphereShaderGrp.bind();
  Z3DShaderProgram& shader = m_sphereShaderGrp.get();

  m_rendererBase.setGlobalShaderParameters(shader, eye);
  setPickingShaderParameters(shader);

  float fovy = glm::degrees(m_rendererBase.camera().fieldOfView());
  float adj;
  if (fovy <= 90.f) {
    adj = 1.0027 + 0.000111 * fovy + 0.000098 * fovy * fovy;
  } else {
    adj = 2.02082 - 0.033935 * fovy + 0.00037854 * fovy * fovy;
  }
  shader.setBoxCorrectionUniform(adj);

  size_t numBatch = std::ceil(m_pointAndRadius.size() * 1.0 / m_oneBatchNumber);

  if (m_hardwareSupportVAO) {
    if (m_pickingDataChanged) {
      m_pickingVAOs.resize(numBatch);

      m_pickingVBOs.resize(numBatch);
      for (size_t ivbo = 0; ivbo < m_pickingVBOs.size(); ++ivbo) {
        m_pickingVBOs[ivbo].resize(4);
      }

      // set vertex data
      GLint attr_a_vertex_radius = shader.vertexAttributeLocation();
      GLint attr_color = shader.colorAttributeLocation();
      GLint attr_flags = shader.flagsAttributeLocation();

      for (size_t i = 0; i < numBatch; ++i) {
        m_pickingVAOs.bind(i);
        size_t size = m_oneBatchNumber;
        if (i == numBatch - 1)
          size = m_pointAndRadius.size() - (numBatch - 1) * m_oneBatchNumber;
        size_t start = m_oneBatchNumber * i;

        glEnableVertexAttribArray(attr_a_vertex_radius);
        if (m_dataChanged) {
          m_pickingVBOs[i].bind(GL_ARRAY_BUFFER, 0);
          glBufferData(GL_ARRAY_BUFFER, size * 4 * sizeof(GLfloat), &(m_pointAndRadius[start]), GL_STATIC_DRAW);
        } else {
          m_VBOs[i].bind(GL_ARRAY_BUFFER, 0);
        }
        glVertexAttribPointer(attr_a_vertex_radius, 4, GL_FLOAT, GL_FALSE, 0, 0);

        glEnableVertexAttribArray(attr_color);
        m_pickingVBOs[i].bind(GL_ARRAY_BUFFER, 1);
        glBufferData(GL_ARRAY_BUFFER, size * 4 * sizeof(GLfloat), &(m_pointPickingColors[start]), GL_STATIC_DRAW);
        glVertexAttribPointer(attr_color, 4, GL_FLOAT, GL_FALSE, 0, 0);

        glEnableVertexAttribArray(attr_flags);
        if (m_dataChanged) {
          m_pickingVBOs[i].bind(GL_ARRAY_BUFFER, 2);
          glBufferData(GL_ARRAY_BUFFER, size * sizeof(GLfloat), &(m_allFlags[start]), GL_STATIC_DRAW);
        } else {
          m_VBOs[i].bind(GL_ARRAY_BUFFER, 2);
        }
        glVertexAttribPointer(attr_flags, 1, GL_FLOAT, GL_FALSE, 0, 0);

        if (m_dataChanged) {
          m_pickingVBOs[i].bind(GL_ELEMENT_ARRAY_BUFFER, 3);
          glBufferData(GL_ELEMENT_ARRAY_BUFFER, size * 6 / 4 * sizeof(GLuint), m_indexs.data(), GL_STATIC_DRAW);
        } else {
          m_VBOs[i].bind(GL_ELEMENT_ARRAY_BUFFER, 4);
        }

        glBindBuffer(GL_ARRAY_BUFFER, 0);

        m_pickingVAOs.release();
      }

      m_pickingDataChanged = false;
    }

    for (size_t i = 0; i < numBatch; ++i) {
      size_t size = m_oneBatchNumber;
      if (i == numBatch - 1)
        size = m_pointAndRadius.size() - (numBatch - 1) * m_oneBatchNumber;
      m_pickingVAOs.bind(i);
      glDrawElements(GL_TRIANGLES, size * 6 / 4, GL_UNSIGNED_INT, 0);
      m_pickingVAOs.release();
    }

  } else {
    if (m_pickingDataChanged) {
      m_pickingVBOs.resize(numBatch);
      for (size_t ivbo = 0; ivbo < m_pickingVBOs.size(); ++ivbo) {
        m_pickingVBOs[ivbo].resize(4);
      }
    }
    // set vertex data
    GLint attr_a_vertex_radius = shader.vertexAttributeLocation();
    GLint attr_color = shader.colorAttributeLocation();
    GLint attr_flags = shader.flagsAttributeLocation();

    for (size_t i = 0; i < numBatch; ++i) {
      size_t size = m_oneBatchNumber;
      if (i == numBatch - 1)
        size = m_pointAndRadius.size() - (numBatch - 1) * m_oneBatchNumber;
      size_t start = m_oneBatchNumber * i;

      glEnableVertexAttribArray(attr_a_vertex_radius);
      if (m_dataChanged) {
        m_pickingVBOs[i].bind(GL_ARRAY_BUFFER, 0);
        if (m_pickingDataChanged)
          glBufferData(GL_ARRAY_BUFFER, size * 4 * sizeof(GLfloat), &(m_pointAndRadius[start]), GL_STATIC_DRAW);
      } else {
        m_VBOs[i].bind(GL_ARRAY_BUFFER, 0);
      }
      glVertexAttribPointer(attr_a_vertex_radius, 4, GL_FLOAT, GL_FALSE, 0, 0);

      glEnableVertexAttribArray(attr_color);
      m_pickingVBOs[i].bind(GL_ARRAY_BUFFER, 1);
      if (m_pickingDataChanged)
        glBufferData(GL_ARRAY_BUFFER, size * 4 * sizeof(GLfloat), &(m_pointPickingColors[start]), GL_STATIC_DRAW);
      glVertexAttribPointer(attr_color, 4, GL_FLOAT, GL_FALSE, 0, 0);

      glEnableVertexAttribArray(attr_flags);
      if (m_dataChanged) {
        m_pickingVBOs[i].bind(GL_ARRAY_BUFFER, 2);
        if (m_pickingDataChanged)
          glBufferData(GL_ARRAY_BUFFER, size * sizeof(GLfloat), &(m_allFlags[start]), GL_STATIC_DRAW);
      } else {
        m_VBOs[i].bind(GL_ARRAY_BUFFER, 2);
      }
      glVertexAttribPointer(attr_flags, 1, GL_FLOAT, GL_FALSE, 0, 0);

      if (m_dataChanged) {
        m_pickingVBOs[i].bind(GL_ELEMENT_ARRAY_BUFFER, 3);
        if (m_pickingDataChanged)
          glBufferData(GL_ELEMENT_ARRAY_BUFFER, size * 6 / 4 * sizeof(GLuint), m_indexs.data(), GL_STATIC_DRAW);
      } else {
        m_VBOs[i].bind(GL_ELEMENT_ARRAY_BUFFER, 4);
      }

      glDrawElements(GL_TRIANGLES, size * 6 / 4, GL_UNSIGNED_INT, 0);

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

void Z3DSphereRenderer::appendDefaultColors()
{
  if (m_pointColors.size() < m_pointAndRadius.size()) {
    for (size_t i = m_pointColors.size(); i < m_pointAndRadius.size(); ++i)
      m_pointColors.emplace_back(0.f, 0.f, 0.f, 1.f);
  }
}
