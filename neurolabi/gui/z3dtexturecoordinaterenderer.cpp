#include "z3dtexturecoordinaterenderer.h"

#include "z3dgl.h"
#include "zmesh.h"

Z3DTextureCoordinateRenderer::Z3DTextureCoordinateRenderer(Z3DRendererBase& rendererBase)
  : Z3DPrimitiveRenderer(rendererBase)
  , m_mesh(nullptr)
  , m_VBOs(3)
  , m_VAO(1)
  , m_dataChanged(false)
{
  m_renderTextureCoordinateShader.bindFragDataLocation(0, "FragData0");
  m_renderTextureCoordinateShader.loadFromSourceFile("transform_with_3dtexture.vert",
                                                     "render_3dtexture_coordinate.frag",
                                                     m_rendererBase.generateHeader());
}

void Z3DTextureCoordinateRenderer::compile()
{
  m_renderTextureCoordinateShader.setHeaderAndRebuild(m_rendererBase.generateHeader());
}

void Z3DTextureCoordinateRenderer::render(Z3DEye eye)
{
  if (!m_mesh || m_mesh->vertices().empty() ||
      m_mesh->numVertices() != m_mesh->num3DTextureCoordinates())
    return;

  const std::vector<glm::vec3>& vertices = m_mesh->vertices();
  const std::vector<glm::vec3>& texCoords = m_mesh->textureCoordinates3D();
  const std::vector<GLuint>& triangleIndexes = m_mesh->indices();

  m_renderTextureCoordinateShader.bind();
  m_rendererBase.setGlobalShaderParameters(m_renderTextureCoordinateShader, eye);

  if (m_hardwareSupportVAO) {
    if (m_dataChanged) {
      m_VAO.bind();
      GLint attr_vertex = m_renderTextureCoordinateShader.vertexAttributeLocation();
      GLint attr_3dTexCoord0 = m_renderTextureCoordinateShader.tex3dCoord0AttributeLocation();

      int bufIdx = 0;
      glEnableVertexAttribArray(attr_vertex);
      m_VBOs.bind(GL_ARRAY_BUFFER, bufIdx++);
      glBufferData(GL_ARRAY_BUFFER, vertices.size() * 3 * sizeof(GLfloat), vertices.data(), GL_STATIC_DRAW);
      glVertexAttribPointer(attr_vertex, 3, GL_FLOAT, GL_FALSE, 0, nullptr);

      glEnableVertexAttribArray(attr_3dTexCoord0);
      m_VBOs.bind(GL_ARRAY_BUFFER, bufIdx++);
      glBufferData(GL_ARRAY_BUFFER, texCoords.size() * 3 * sizeof(GLfloat), texCoords.data(), GL_STATIC_DRAW);
      glVertexAttribPointer(attr_3dTexCoord0, 3, GL_FLOAT, GL_FALSE, 0, nullptr);

      if (!triangleIndexes.empty()) {
        m_VBOs.bind(GL_ELEMENT_ARRAY_BUFFER, bufIdx++);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, triangleIndexes.size() * sizeof(GLuint), triangleIndexes.data(),
                     GL_STATIC_DRAW);
      }

      glBindBuffer(GL_ARRAY_BUFFER, 0);
      m_VAO.release();

      m_dataChanged = false;
    }

    m_VAO.bind();
    if (triangleIndexes.empty()) {
      glDrawArrays(m_mesh->type(), 0, vertices.size());
    } else {
      glDrawElements(m_mesh->type(), triangleIndexes.size(), GL_UNSIGNED_INT, 0);
    }
    m_VAO.release();

  } else {
    GLint attr_vertex = m_renderTextureCoordinateShader.vertexAttributeLocation();
    GLint attr_3dTexCoord0 = m_renderTextureCoordinateShader.tex3dCoord0AttributeLocation();

    int bufIdx = 0;
    glEnableVertexAttribArray(attr_vertex);
    m_VBOs.bind(GL_ARRAY_BUFFER, bufIdx++);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * 3 * sizeof(GLfloat), vertices.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(attr_vertex, 3, GL_FLOAT, GL_FALSE, 0, nullptr);

    glEnableVertexAttribArray(attr_3dTexCoord0);
    m_VBOs.bind(GL_ARRAY_BUFFER, bufIdx++);
    glBufferData(GL_ARRAY_BUFFER, texCoords.size() * 3 * sizeof(GLfloat), texCoords.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(attr_3dTexCoord0, 3, GL_FLOAT, GL_FALSE, 0, nullptr);

    if (triangleIndexes.empty()) {
      glDrawArrays(m_mesh->type(), 0, vertices.size());
    } else {
      m_VBOs.bind(GL_ELEMENT_ARRAY_BUFFER, bufIdx++);
      glBufferData(GL_ELEMENT_ARRAY_BUFFER, triangleIndexes.size() * sizeof(GLuint), triangleIndexes.data(),
                   GL_STATIC_DRAW);
      glDrawElements(m_mesh->type(), triangleIndexes.size(), GL_UNSIGNED_INT, 0);
      glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    }

    glBindBuffer(GL_ARRAY_BUFFER, 0);

    glDisableVertexAttribArray(attr_vertex);
    glDisableVertexAttribArray(attr_3dTexCoord0);
  }

  m_renderTextureCoordinateShader.release();
}
