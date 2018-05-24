#include "z3dprimitiverenderer.h"

#include "z3dgl.h"
#include "zmesh.h"
#include "z3dgpuinfo.h"
#include "z3dshaderprogram.h"

Z3DPrimitiveRenderer::Z3DPrimitiveRenderer(Z3DRendererBase& rendererBase)
  : m_rendererBase(rendererBase)
  , m_needLighting(true)
  #if !defined(_USE_CORE_PROFILE_) && defined(_SUPPORT_FIXED_PIPELINE_)
  , m_useDisplayList(false)
  #endif
  , m_followCoordTransform(true)
  , m_followOpacity(true)
  , m_followSizeScale(true)
  , m_hardwareSupportVAO(Z3DGpuInfo::instance().isVAOSupported())
{
  m_rendererBase.registerRenderer(this);
}

Z3DPrimitiveRenderer::~Z3DPrimitiveRenderer()
{
  m_rendererBase.unregisterRenderer(this);
}

void Z3DPrimitiveRenderer::renderScreenQuad(const ZVertexArrayObject& vao, const Z3DShaderProgram& shader)
{
  if (!shader.isLinked())
    return;

  vao.bind();

  const GLfloat vertices[] = {-1.f, 1.f, 0.f, //top left corner
                              -1.f, -1.f, 0.f, //bottom left corner
                              1.f, 1.f, 0.f, //top right corner
                              1.f, -1.f, 0.f}; // bottom right rocner
  GLint attr_vertex = shader.vertexAttributeLocation();

  GLuint bufObjects[1];
  glGenBuffers(1, bufObjects);

  glEnableVertexAttribArray(attr_vertex);
  glBindBuffer(GL_ARRAY_BUFFER, bufObjects[0]);
  glBufferData(GL_ARRAY_BUFFER, 3 * 4 * sizeof(GLfloat), vertices, GL_STATIC_DRAW);
  glVertexAttribPointer(attr_vertex, 3, GL_FLOAT, GL_FALSE, 0, nullptr);

  glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

  glBindBuffer(GL_ARRAY_BUFFER, 0);
  glDeleteBuffers(1, bufObjects);

  glDisableVertexAttribArray(attr_vertex);

  vao.release();
}

void Z3DPrimitiveRenderer::renderTriangleList(
    const ZVertexArrayObject& vao, const Z3DShaderProgram& shader,
    const ZMesh& mesh)
{
  if (mesh.empty() || !shader.isLinked())
    return;

  const std::vector<glm::vec3>& vertices = mesh.vertices();
  const std::vector<float>& textureCoordinates1D = mesh.textureCoordinates1D();
  const std::vector<glm::vec2>& textureCoordinates2D = mesh.textureCoordinates2D();
  const std::vector<glm::vec3>& textureCoordinates3D = mesh.textureCoordinates3D();
  const std::vector<glm::vec3>& normals = mesh.normals();
  const std::vector<glm::vec4>& colors = mesh.colors();
  const std::vector<GLuint>& triangleIndexes = mesh.indices();
  GLenum type = mesh.type();

  vao.bind();

  GLint attr_vertex = shader.vertexAttributeLocation();
  GLint attr_1dTexCoord0 = shader.tex1dCoord0AttributeLocation();
  GLint attr_2dTexCoord0 = shader.tex2dCoord0AttributeLocation();
  GLint attr_3dTexCoord0 = shader.tex3dCoord0AttributeLocation();
  GLint attr_normal = shader.normalAttributeLocation();
  GLint attr_color = shader.colorAttributeLocation();

  GLsizei bufObjectsSize = 1;  // vertex
  if (attr_1dTexCoord0 != -1 && !textureCoordinates1D.empty())
    bufObjectsSize++;
  if (attr_2dTexCoord0 != -1 && !textureCoordinates2D.empty())
    bufObjectsSize++;
  if (attr_3dTexCoord0 != -1 && !textureCoordinates3D.empty())
    bufObjectsSize++;
  if (attr_normal != -1 && !normals.empty())
    bufObjectsSize++;
  if (attr_color != -1 && !colors.empty())
    bufObjectsSize++;
  if (!triangleIndexes.empty())
    bufObjectsSize++;

  std::vector<GLuint> bufObjects(bufObjectsSize);
  glGenBuffers(bufObjectsSize, bufObjects.data());

  int bufIdx = 0;
  glEnableVertexAttribArray(attr_vertex);
  glBindBuffer(GL_ARRAY_BUFFER, bufObjects[bufIdx++]);
  glBufferData(GL_ARRAY_BUFFER, vertices.size() * 3 * sizeof(GLfloat), vertices.data(), GL_STATIC_DRAW);
  glVertexAttribPointer(attr_vertex, 3, GL_FLOAT, GL_FALSE, 0, nullptr);

  if (attr_1dTexCoord0 != -1 && !textureCoordinates1D.empty()) {
    glEnableVertexAttribArray(attr_1dTexCoord0);
    glBindBuffer(GL_ARRAY_BUFFER, bufObjects[bufIdx++]);
    glBufferData(GL_ARRAY_BUFFER, textureCoordinates1D.size() * 1 * sizeof(GLfloat), textureCoordinates1D.data(),
                 GL_STATIC_DRAW);
    glVertexAttribPointer(attr_1dTexCoord0, 1, GL_FLOAT, GL_FALSE, 0, nullptr);
  }

  if (attr_2dTexCoord0 != -1 && !textureCoordinates2D.empty()) {
    glEnableVertexAttribArray(attr_2dTexCoord0);
    glBindBuffer(GL_ARRAY_BUFFER, bufObjects[bufIdx++]);
    glBufferData(GL_ARRAY_BUFFER, textureCoordinates2D.size() * 2 * sizeof(GLfloat), textureCoordinates2D.data(),
                 GL_STATIC_DRAW);
    glVertexAttribPointer(attr_2dTexCoord0, 2, GL_FLOAT, GL_FALSE, 0, nullptr);
  }

  if (attr_3dTexCoord0 != -1 && !textureCoordinates3D.empty()) {
    glEnableVertexAttribArray(attr_3dTexCoord0);
    glBindBuffer(GL_ARRAY_BUFFER, bufObjects[bufIdx++]);
    glBufferData(GL_ARRAY_BUFFER, textureCoordinates3D.size() * 3 * sizeof(GLfloat), textureCoordinates3D.data(),
                 GL_STATIC_DRAW);
    glVertexAttribPointer(attr_3dTexCoord0, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
  }

  if (attr_normal != -1 && !normals.empty()) {
    glEnableVertexAttribArray(attr_normal);
    glBindBuffer(GL_ARRAY_BUFFER, bufObjects[bufIdx++]);
    glBufferData(GL_ARRAY_BUFFER, normals.size() * 3 * sizeof(GLfloat), normals.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(attr_normal, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
  }

  if (attr_color != -1 && !colors.empty()) {
    glEnableVertexAttribArray(attr_color);
    glBindBuffer(GL_ARRAY_BUFFER, bufObjects[bufIdx++]);
    glBufferData(GL_ARRAY_BUFFER, colors.size() * 4 * sizeof(GLfloat), colors.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(attr_color, 4, GL_FLOAT, GL_FALSE, 0, nullptr);
  }

  if (triangleIndexes.empty()) {
    glDrawArrays(type, 0, vertices.size());
  } else {
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, bufObjects[bufIdx++]);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, triangleIndexes.size() * sizeof(GLuint), triangleIndexes.data(),
                 GL_STATIC_DRAW);
    glDrawElements(type, triangleIndexes.size(), GL_UNSIGNED_INT, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
  }

  glBindBuffer(GL_ARRAY_BUFFER, 0);
  glDeleteBuffers(bufObjectsSize, bufObjects.data());

  glDisableVertexAttribArray(attr_vertex);
  if (attr_1dTexCoord0 != -1 && !textureCoordinates1D.empty())
    glDisableVertexAttribArray(attr_1dTexCoord0);
  if (attr_2dTexCoord0 != -1 && !textureCoordinates2D.empty())
    glDisableVertexAttribArray(attr_2dTexCoord0);
  if (attr_3dTexCoord0 != -1 && !textureCoordinates3D.empty())
    glDisableVertexAttribArray(attr_3dTexCoord0);
  if (attr_normal != -1 && !normals.empty())
    glDisableVertexAttribArray(attr_normal);
  if (attr_color != -1 && !colors.empty())
    glDisableVertexAttribArray(attr_color);

  vao.release();
}

#if !defined(_USE_CORE_PROFILE_) && defined(_SUPPORT_FIXED_PIPELINE_)
void Z3DPrimitiveRenderer::invalidateOpenglRenderer()
{
  if (m_useDisplayList)
    emit openglRendererInvalid();
}

void Z3DPrimitiveRenderer::invalidateOpenglPickingRenderer()
{
  if (m_useDisplayList)
    emit openglPickingRendererInvalid();
}
#endif

void Z3DPrimitiveRenderer::setShaderParameters(Z3DShaderProgram& shader)
{
  shader.setLightingEnabledUniform(m_needLighting);
  if (!m_followCoordTransform)
    shader.setPosTransformUniform(glm::mat4(1.f));
  if (!m_followOpacity)
    shader.setAlphaUniform(1.f);
  if (!m_followSizeScale)
    shader.setSizeScaleUniform(1.f);
}

void Z3DPrimitiveRenderer::setPickingShaderParameters(Z3DShaderProgram& shader)
{
  shader.setLightingEnabledUniform(false);
  if (!m_followCoordTransform)
    shader.setPosTransformUniform(glm::mat4(1.f));
  if (!m_followSizeScale)
    shader.setSizeScaleUniform(1.f);
}
