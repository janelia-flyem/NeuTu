#include "z3dmeshrenderer.h"

#include "z3dgl.h"
#include "z3dgpuinfo.h"
#include "z3dshaderprogram.h"
#include "z3dtexture.h"
#include "logging/zqslog.h"
#include "zmesh.h"
#include "neutubeconfig.h"

Z3DMeshRenderer::Z3DMeshRenderer(Z3DRendererBase& rendererBase)
  : Z3DPrimitiveRenderer(rendererBase)
  , m_meshShaderGrp(rendererBase)
  , m_meshPt(nullptr)
  , m_meshColorsPt(nullptr)
  , m_meshPickingColorsPt(nullptr)
  , m_origMeshPt(nullptr)
  , m_origMeshColorsPt(nullptr)
  , m_origMeshPickingColorsPt(nullptr)
  , m_texture(nullptr)
  , m_meshColorReady(false)
  , m_meshPickingColorReady(false)
  , m_colorSource("Color Source")
  , m_dataChanged(false)
  , m_pickingDataChanged(false)
  , m_wireframeMode("Wireframe Option")
  , m_wireframeColor("Wireframe Color", glm::vec4(1), glm::vec4(0), glm::vec4(1))
  , m_useTwoSidedLighting("Two-Sided Lighting", true)
{
  m_colorSource.addOptions("MeshColor", "Mesh1DTexture", "Mesh2DTexture", "Mesh3DTexture", "CustomColor");
  m_colorSource.select("MeshColor");
  connect(&m_colorSource, &ZStringIntOptionParameter::valueChanged, this, &Z3DMeshRenderer::compile);

  m_wireframeMode.addOptions("No Wireframe", "With Wireframe", "Only Wireframe");
  m_wireframeMode.select("No Wireframe");
  connect(&m_wireframeMode, &ZStringIntOptionParameter::valueChanged, this, &Z3DMeshRenderer::adjustWidgets);
  m_wireframeColor.setStyle("COLOR");

#if !defined(_USE_CORE_PROFILE_) && defined(_SUPPORT_FIXED_PIPELINE_)
  setUseDisplayList(true);
#endif

  QStringList allshaders;
  allshaders << "mesh.vert" << "mesh_func.frag" << "lighting2.frag";
  QStringList normalShaders;
  normalShaders << "mesh.vert" << "mesh.frag" << "lighting2.frag";
  m_meshShaderGrp.init(allshaders, m_rendererBase.generateHeader() + generateHeader(), "",
                       normalShaders);
  m_meshShaderGrp.addAllSupportedPostShaders();
}

void Z3DMeshRenderer::setData(std::vector<ZMesh*>* meshInput)
{
  m_origMeshPt = meshInput;
  m_meshPt = meshInput;
  prepareMesh();

#if !defined(_USE_CORE_PROFILE_) && defined(_SUPPORT_FIXED_PIPELINE_)
  invalidateOpenglRenderer();
  invalidateOpenglPickingRenderer();
#endif
  m_dataChanged = true;
  m_pickingDataChanged = true;
}

void Z3DMeshRenderer::setDataColors(std::vector<glm::vec4>* meshColorsInput)
{
  m_origMeshColorsPt = meshColorsInput;
  m_meshColorReady = false;
#if !defined(_USE_CORE_PROFILE_) && defined(_SUPPORT_FIXED_PIPELINE_)
  invalidateOpenglRenderer();
#endif
  m_dataChanged = true;
}

void Z3DMeshRenderer::setTexture(Z3DTexture* tex)
{
  m_texture = tex;
#if !defined(_USE_CORE_PROFILE_) && defined(_SUPPORT_FIXED_PIPELINE_)
  invalidateOpenglRenderer();
#endif
  m_dataChanged = true;
}

void Z3DMeshRenderer::setDataPickingColors(std::vector<glm::vec4>* meshPickingColorsInput)
{
  m_origMeshPickingColorsPt = meshPickingColorsInput;
  m_meshPickingColorReady = false;
#if !defined(_USE_CORE_PROFILE_) && defined(_SUPPORT_FIXED_PIPELINE_)
  invalidateOpenglPickingRenderer();
#endif
  m_pickingDataChanged = true;
}

void Z3DMeshRenderer::compile()
{
  m_dataChanged = true;
  m_meshShaderGrp.rebuild(m_rendererBase.generateHeader() + generateHeader());
}

QString Z3DMeshRenderer::generateHeader()
{
  QString headerSource;
  if (m_colorSource.isSelected("MeshColor"))
    headerSource += "#define USE_MESH_COLOR\n";
  else if (m_colorSource.isSelected("Mesh1DTexture"))
    headerSource += "#define USE_MESH_1DTEXTURE\n";
  else if (m_colorSource.isSelected("Mesh2DTexture"))
    headerSource += "#define USE_MESH_2DTEXTURE\n";
  else if (m_colorSource.isSelected("Mesh3DTexture"))
    headerSource += "#define USE_MESH_3DTEXTURE\n";
  return headerSource;
}

void Z3DMeshRenderer::prepareMesh()
{
  m_splitMeshes.clear();
  m_splitCount.clear();
  m_meshNeedSplit = false;

  if (!m_meshPt || m_meshPt->empty()) {
    return;
  }
  m_splitCount.resize(m_meshPt->size());
  size_t numTriThre = NeutubeConfig::GetMeshSplitThreshold();
  for (auto mesh : *m_meshPt) {
    if (mesh->numTriangles() > numTriThre) {
      LINFO() << mesh->numTriangles() << "triangles found. Split triggerd.";
      m_meshNeedSplit = true;
      break;
    }
  }
  if (m_meshNeedSplit) {
    LOG(INFO) << "Number of meshes before spliting " << m_meshPt->size();
    for (size_t i = 0; i < m_meshPt->size(); ++i) {
      if ((*m_meshPt)[i]->numTriangles() <= numTriThre) {
        m_splitMeshes.push_back(*((*m_meshPt)[i]));
        m_splitCount[i] = 1;
      } else {
        std::vector<std::shared_ptr<ZMesh>> res =
            (*m_meshPt)[i]->getSplitMeshList(numTriThre);
        m_splitCount[i] = res.size();
        if (!res.empty()) {
          size_t start = m_splitMeshes.size();
          m_splitMeshes.resize(start + res.size());
          for (auto splitMesh : res) {
            m_splitMeshes[start++] = *splitMesh;
          }
        }
        /*
        std::vector<ZMesh> res = (*m_meshPt)[i]->split(numTriThre);
        m_splitCount[i] = res.size();
        if (i == 0) {
          m_splitMeshes.swap(res);
        } else {
          size_t start = m_splitMeshes.size();
          m_splitMeshes.resize(start + res.size());
          for (size_t j = 0; j < res.size(); ++j)
            m_splitMeshes[j + start].swap(res[j]);
        }
        */
      }
    }
    m_splitMeshesWrapper.clear();
    for (size_t i = 0; i < m_splitMeshes.size(); ++i) {
      m_splitMeshesWrapper.push_back(&m_splitMeshes[i]);
    }
    m_meshPt = &m_splitMeshesWrapper;
    LOG(INFO) << "Number of meshes after spliting " << m_meshPt->size();
  }
}

void Z3DMeshRenderer::prepareMeshColor()
{
  m_meshColorReady = true;
  m_splitMeshesColors.clear();
  m_meshColorsPt = m_origMeshColorsPt;
  if (!m_origMeshColorsPt || m_origMeshColorsPt->size() < m_origMeshPt->size())
    return;
  if (m_meshNeedSplit) {
    for (size_t i = 0; i < m_splitCount.size(); ++i) {
      for (size_t j = 0; j < m_splitCount[i]; ++j)
        m_splitMeshesColors.push_back((*m_origMeshColorsPt)[i]);
    }
    m_meshColorsPt = &m_splitMeshesColors;
  }
}

void Z3DMeshRenderer::prepareMeshPickingColor()
{
  m_meshPickingColorReady = true;
  m_splitMeshesPickingColors.clear();
  m_meshPickingColorsPt = m_origMeshPickingColorsPt;
  if (!m_origMeshPickingColorsPt || m_origMeshPickingColorsPt->size() < m_origMeshPt->size())
    return;
  if (m_meshNeedSplit) {
    for (size_t i = 0; i < m_splitCount.size(); ++i) {
      for (size_t j = 0; j < m_splitCount[i]; ++j)
        m_splitMeshesPickingColors.push_back((*m_origMeshPickingColorsPt)[i]);
    }
    m_meshPickingColorsPt = &m_splitMeshesPickingColors;
  }
}

#if !defined(_USE_CORE_PROFILE_) && defined(_SUPPORT_FIXED_PIPELINE_)
void Z3DMeshRenderer::renderUsingOpengl()
{
  if (!m_meshPt || m_meshPt->empty())
    return;

  if (m_colorSource.isSelected("CustomColor") && !m_meshColorReady)
    prepareMeshColor();

  for (size_t i=0; i<m_meshPt->size(); ++i) {
    if (m_colorSource.isSelected("MeshColor") && (*m_meshPt)[i]->numColors() < (*m_meshPt)[i]->numVertices())
      return;
    if (m_colorSource.isSelected("Mesh1DTexture") &&
        ((*m_meshPt)[i]->num1DTextureCoordinates() < (*m_meshPt)[i]->numVertices() ||
         !m_texture || m_texture->textureTarget() != GL_TEXTURE_1D))
      return;
    if (m_colorSource.isSelected("Mesh2DTexture") &&
        ((*m_meshPt)[i]->num2DTextureCoordinates() < (*m_meshPt)[i]->numVertices() ||
         !m_texture || m_texture->textureTarget() != GL_TEXTURE_2D))
      return;
    if (m_colorSource.isSelected("Mesh3DTexture") &&
        ((*m_meshPt)[i]->num3DTextureCoordinates() < (*m_meshPt)[i]->numVertices() ||
         !m_texture || m_texture->textureTarget() != GL_TEXTURE_3D))
      return;
    if (m_colorSource.isSelected("CustomColor") &&
        (!m_meshColorsPt || m_meshColorsPt->size() < m_meshPt->size()))
      return;
    if ((*m_meshPt)[i]->numNormals() != (*m_meshPt)[i]->numVertices())
      (*m_meshPt)[i]->generateNormals();
  }

  glMatrixMode(GL_MODELVIEW);
  glPushMatrix();
  //glScalef(getCoordTransform().x, getCoordTransform().y, getCoordTransform().z);
  glMultMatrixf(&coordTransform()[0][0]);   // not sure, todo check

  if (m_colorSource.isSelected("Mesh2DTexture") || m_colorSource.isSelected("Mesh3DTexture")) {
    glActiveTexture(GL_TEXTURE0);
    m_texture->bind();
  }

  for (size_t i=0; i<m_meshPt->size(); ++i) {
    const std::vector<glm::vec3>& vertices = (*m_meshPt)[i]->vertices();
    const std::vector<glm::vec3>& normals = (*m_meshPt)[i]->normals();
    const std::vector<glm::vec4>& colors = (*m_meshPt)[i]->colors();
    const std::vector<float>& texture1DCoords = (*m_meshPt)[i]->textureCoordinates1D();
    const std::vector<glm::vec2>& texture2DCoords = (*m_meshPt)[i]->textureCoordinates2D();
    const std::vector<glm::vec3>& texture3DCoords = (*m_meshPt)[i]->textureCoordinates3D();
    const std::vector<GLuint>& triangleIndexes = (*m_meshPt)[i]->indices();
    GLenum type = (*m_meshPt)[i]->type();

    GLuint bufObjects[4];
    glGenBuffers(4, bufObjects);

    glEnableClientState(GL_VERTEX_ARRAY);
    glBindBuffer(GL_ARRAY_BUFFER, bufObjects[0]);
    glBufferData(GL_ARRAY_BUFFER, vertices.size()*3*sizeof(GLfloat), vertices.data(), GL_STATIC_DRAW);
    glVertexPointer(3, GL_FLOAT, 0, 0);

    if (m_colorSource.isSelected("MeshColor")) {
      glEnableClientState(GL_COLOR_ARRAY);
      glBindBuffer(GL_ARRAY_BUFFER, bufObjects[1]);
      glBufferData(GL_ARRAY_BUFFER, colors.size()*4*sizeof(GLfloat), colors.data(), GL_STATIC_DRAW);
      glColorPointer(4, GL_FLOAT, 0, 0);
    } else if (m_colorSource.isSelected("CustomColor")) {
      glColor4f((*m_meshColorsPt)[i].r, (*m_meshColorsPt)[i].g, (*m_meshColorsPt)[i].b,
                (*m_meshColorsPt)[i].a * opacity());
    } else if (m_colorSource.isSelected("Mesh1DTexture")) {
      glClientActiveTexture(GL_TEXTURE0);
      glEnableClientState(GL_TEXTURE_COORD_ARRAY);
      glBindBuffer(GL_ARRAY_BUFFER, bufObjects[1]);
      glBufferData(GL_ARRAY_BUFFER, texture1DCoords.size()*sizeof(GLfloat), texture1DCoords.data(), GL_STATIC_DRAW);
      glTexCoordPointer(1, GL_FLOAT, 0, 0);
    } else if (m_colorSource.isSelected("Mesh2DTexture")) {
      glClientActiveTexture(GL_TEXTURE0);
      glEnableClientState(GL_TEXTURE_COORD_ARRAY);
      glBindBuffer(GL_ARRAY_BUFFER, bufObjects[1]);
      glBufferData(GL_ARRAY_BUFFER, texture2DCoords.size()*2*sizeof(GLfloat), texture2DCoords.data(), GL_STATIC_DRAW);
      glTexCoordPointer(2, GL_FLOAT, 0, 0);
    } else if (m_colorSource.isSelected("Mesh3DTexture")) {
      glClientActiveTexture(GL_TEXTURE0);
      glEnableClientState(GL_TEXTURE_COORD_ARRAY);
      glBindBuffer(GL_ARRAY_BUFFER, bufObjects[1]);
      glBufferData(GL_ARRAY_BUFFER, texture3DCoords.size()*3*sizeof(GLfloat), texture3DCoords.data(), GL_STATIC_DRAW);
      glTexCoordPointer(3, GL_FLOAT, 0, 0);
    }

    glEnableClientState(GL_NORMAL_ARRAY);
    glBindBuffer(GL_ARRAY_BUFFER, bufObjects[2]);
    glBufferData(GL_ARRAY_BUFFER, normals.size()*3*sizeof(GLfloat), normals.data(), GL_STATIC_DRAW);
    glNormalPointer(GL_FLOAT, 0, 0);

    if (triangleIndexes.empty()) {
      glDrawArrays(type, 0, vertices.size());
    } else {
      glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, bufObjects[3]);
      glBufferData(GL_ELEMENT_ARRAY_BUFFER, triangleIndexes.size()*sizeof(GLuint), triangleIndexes.data(), GL_STATIC_DRAW);
      glDrawElements(type, triangleIndexes.size(), GL_UNSIGNED_INT, 0);
      glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    }

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glDeleteBuffers(4, bufObjects);
    glDisableClientState(GL_VERTEX_ARRAY);
    if (m_colorSource.isSelected("MeshColor")) {
      glDisableClientState(GL_COLOR_ARRAY);
    } else if (m_colorSource.isSelected("Mesh1DTexture")) {
      glDisableClientState(GL_TEXTURE_COORD_ARRAY);
    } else if (m_colorSource.isSelected("Mesh2DTexture")) {
      glDisableClientState(GL_TEXTURE_COORD_ARRAY);
    } else if (m_colorSource.isSelected("Mesh3DTexture")) {
      glDisableClientState(GL_TEXTURE_COORD_ARRAY);
    }
    glDisableClientState(GL_NORMAL_ARRAY);
  }

  glMatrixMode(GL_MODELVIEW);
  glPopMatrix();
}

void Z3DMeshRenderer::renderPickingUsingOpengl()
{
  if (!m_meshPt || m_meshPt->empty())
    return;

  if (!m_meshPickingColorReady)
    prepareMeshPickingColor();

  if (!m_meshPickingColorsPt || m_meshPickingColorsPt->empty()
      || m_meshPickingColorsPt->size() != m_meshPt->size())
    return;

  glMatrixMode(GL_MODELVIEW);
  glPushMatrix();
  //glScalef(getCoordTransform().x, getCoordTransform().y, getCoordTransform().z);
  glMultMatrixf(&coordTransform()[0][0]);   // not sure, todo check

  for (size_t i=0; i<m_meshPt->size(); ++i) {
    const std::vector<glm::vec3>& vertices = (*m_meshPt)[i]->vertices();
    //const std::vector<glm::vec3>& normals = (*m_meshPt)[i]->normals();
    const std::vector<GLuint>& triangleIndexes = (*m_meshPt)[i]->indices();
    GLenum type = (*m_meshPt)[i]->type();

    GLuint bufObjects[3];
    glGenBuffers(3, bufObjects);

    glEnableClientState(GL_VERTEX_ARRAY);
    glBindBuffer(GL_ARRAY_BUFFER, bufObjects[0]);
    glBufferData(GL_ARRAY_BUFFER, vertices.size()*3*sizeof(GLfloat), vertices.data(), GL_STATIC_DRAW);
    glVertexPointer(3, GL_FLOAT, 0, 0);

    glColor4f((*m_meshPickingColorsPt)[i].r, (*m_meshPickingColorsPt)[i].g,
              (*m_meshPickingColorsPt)[i].b, (*m_meshPickingColorsPt)[i].a);

    //glEnableClientState(GL_NORMAL_ARRAY);
    //glBindBuffer(GL_ARRAY_BUFFER, bufObjects[1]);
    //glBufferData(GL_ARRAY_BUFFER, normals.size()*3*sizeof(GLfloat), normals.data(), GL_STATIC_DRAW);
    //glNormalPointer(GL_FLOAT, 0, 0);

    if (triangleIndexes.empty()) {
      glDrawArrays(type, 0, vertices.size());
    } else {
      glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, bufObjects[2]);
      glBufferData(GL_ELEMENT_ARRAY_BUFFER, triangleIndexes.size()*sizeof(GLuint), triangleIndexes.data(), GL_STATIC_DRAW);
      glDrawElements(type, triangleIndexes.size(), GL_UNSIGNED_INT, 0);
      glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    }

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glDeleteBuffers(3, bufObjects);
    glDisableClientState(GL_VERTEX_ARRAY);
    glDisableClientState(GL_NORMAL_ARRAY);
  }

  glMatrixMode(GL_MODELVIEW);
  glPopMatrix();
}
#endif

void Z3DMeshRenderer::render(Z3DEye eye)
{
  if (!m_meshPt || m_meshPt->empty())
    return;

  if (m_colorSource.isSelected("CustomColor") && !m_meshColorReady)
    prepareMeshColor();

  for (auto mesh : *m_meshPt) {
    //if (m_colorSource.isSelected("MeshColor") && mesh->numColors() < mesh->numVertices())
      //return;
    if (m_colorSource.isSelected("Mesh1DTexture") &&
        (mesh->num1DTextureCoordinates() < mesh->numVertices() ||
          !m_texture || m_texture->textureTarget() != GL_TEXTURE_1D))
      return;
    if (m_colorSource.isSelected("Mesh2DTexture") &&
        (mesh->num2DTextureCoordinates() < mesh->numVertices() ||
          !m_texture || m_texture->textureTarget() != GL_TEXTURE_2D))
      return;
    if (m_colorSource.isSelected("Mesh3DTexture") &&
        (mesh->num3DTextureCoordinates() < mesh->numVertices() ||
          !m_texture || m_texture->textureTarget() != GL_TEXTURE_3D))
      return;
    if (m_colorSource.isSelected("CustomColor") &&
        (!m_meshColorsPt || m_meshColorsPt->size() < m_meshPt->size()))
      return;
    if (mesh->numNormals() != mesh->numVertices())
      mesh->generateNormals();
  }

  m_meshShaderGrp.bind();
  Z3DShaderProgram& shader = m_meshShaderGrp.get();
  m_rendererBase.setGlobalShaderParameters(shader, eye);
  setShaderParameters(shader);

  shader.setUseTwoSidedLightingUniform(m_useTwoSidedLighting.get());

  if (m_colorSource.isSelected("Mesh2DTexture") ||
      m_colorSource.isSelected("Mesh3DTexture") ||
      m_colorSource.isSelected("Mesh1DTexture")) {
    shader.bindTexture("texture", m_texture);
  }

  GLint attr_vertex = shader.vertexAttributeLocation();
  GLint attr_1dTexCoord0 = shader.tex1dCoord0AttributeLocation();
  GLint attr_2dTexCoord0 = shader.tex2dCoord0AttributeLocation();
  GLint attr_3dTexCoord0 = shader.tex3dCoord0AttributeLocation();
  GLint attr_normal = shader.normalAttributeLocation();
  GLint attr_color = shader.colorAttributeLocation();

  if (m_hardwareSupportVAO) {
    if (m_dataChanged) {
      m_VAOs.resize(m_meshPt->size());

      m_VBOs.resize(m_meshPt->size());
      for (size_t ivbo = 0; ivbo < m_VBOs.size(); ++ivbo) {
        m_VBOs[ivbo].resize(4);
      }

      for (size_t i = 0; i < m_meshPt->size(); ++i) {
        m_VAOs.bind(i);

        const std::vector<glm::vec3>& vertices = (*m_meshPt)[i]->vertices();
        const std::vector<float>& textureCoordinates1D = (*m_meshPt)[i]->textureCoordinates1D();
        const std::vector<glm::vec2>& textureCoordinates2D = (*m_meshPt)[i]->textureCoordinates2D();
        const std::vector<glm::vec3>& textureCoordinates3D = (*m_meshPt)[i]->textureCoordinates3D();
        const std::vector<glm::vec3>& normals = (*m_meshPt)[i]->normals();
        const std::vector<glm::vec4>& colors = (*m_meshPt)[i]->colors();
        const std::vector<GLuint>& triangleIndexes = (*m_meshPt)[i]->indices();

#ifdef _DEBUG_2
        std::cout << "Color count in renderer: " << colors.size() << std::endl;
#endif

        int bufIdx = 0;
        glEnableVertexAttribArray(attr_vertex);
        m_VBOs[i].bind(GL_ARRAY_BUFFER, bufIdx++);
        glBufferData(GL_ARRAY_BUFFER, vertices.size() * 3 * sizeof(GLfloat), vertices.data(), GL_STATIC_DRAW);
        glVertexAttribPointer(attr_vertex, 3, GL_FLOAT, GL_FALSE, 0, 0);

        if (attr_normal != -1) {
          glEnableVertexAttribArray(attr_normal);
          m_VBOs[i].bind(GL_ARRAY_BUFFER, bufIdx++);
          glBufferData(GL_ARRAY_BUFFER, normals.size() * 3 * sizeof(GLfloat), normals.data(), GL_STATIC_DRAW);
          glVertexAttribPointer(attr_normal, 3, GL_FLOAT, GL_FALSE, 0, 0);
        }

        if (!triangleIndexes.empty()) {
          m_VBOs[i].bind(GL_ELEMENT_ARRAY_BUFFER, bufIdx++);
          glBufferData(GL_ELEMENT_ARRAY_BUFFER, triangleIndexes.size() * sizeof(GLuint), triangleIndexes.data(),
                       GL_STATIC_DRAW);
        }

        if (m_colorSource.isSelected("Mesh1DTexture") && attr_1dTexCoord0 != -1 && !textureCoordinates1D.empty()) {
          glEnableVertexAttribArray(attr_1dTexCoord0);
          m_VBOs[i].bind(GL_ARRAY_BUFFER, bufIdx++);
          glBufferData(GL_ARRAY_BUFFER, textureCoordinates1D.size() * 1 * sizeof(GLfloat), textureCoordinates1D.data(),
                       GL_STATIC_DRAW);
          glVertexAttribPointer(attr_1dTexCoord0, 1, GL_FLOAT, GL_FALSE, 0, 0);
        }

        if (m_colorSource.isSelected("Mesh2DTexture") && attr_2dTexCoord0 != -1 && !textureCoordinates2D.empty()) {
          glEnableVertexAttribArray(attr_2dTexCoord0);
          m_VBOs[i].bind(GL_ARRAY_BUFFER, bufIdx++);
          glBufferData(GL_ARRAY_BUFFER, textureCoordinates2D.size() * 2 * sizeof(GLfloat), textureCoordinates2D.data(),
                       GL_STATIC_DRAW);
          glVertexAttribPointer(attr_2dTexCoord0, 2, GL_FLOAT, GL_FALSE, 0, 0);
        }

        if (m_colorSource.isSelected("Mesh3DTexture") && attr_3dTexCoord0 != -1 && !textureCoordinates3D.empty()) {
          glEnableVertexAttribArray(attr_3dTexCoord0);
          m_VBOs[i].bind(GL_ARRAY_BUFFER, bufIdx++);
          glBufferData(GL_ARRAY_BUFFER, textureCoordinates3D.size() * 3 * sizeof(GLfloat), textureCoordinates3D.data(),
                       GL_STATIC_DRAW);
          glVertexAttribPointer(attr_3dTexCoord0, 3, GL_FLOAT, GL_FALSE, 0, 0);
        }

        if (m_colorSource.isSelected("MeshColor") && attr_color != -1 && colors.size() >= vertices.size()) {
          glEnableVertexAttribArray(attr_color);
          m_VBOs[i].bind(GL_ARRAY_BUFFER, bufIdx++);
          glBufferData(GL_ARRAY_BUFFER, colors.size() * 4 * sizeof(GLfloat), colors.data(), GL_STATIC_DRAW);
          glVertexAttribPointer(attr_color, 4, GL_FLOAT, GL_FALSE, 0, 0);
        }

        glBindBuffer(GL_ARRAY_BUFFER, 0);

        m_VAOs.release();
      }

      m_dataChanged = false;
    }

    if (!m_wireframeMode.isSelected("Only Wireframe")) {
      for (size_t i = 0; i < m_meshPt->size(); ++i) {
        if (m_colorSource.isSelected("CustomColor")) {
          shader.setUseCustomColorUniform(true);
          shader.setCustomColorUniform((*m_meshColorsPt)[i]);
        } else if (m_colorSource.isSelected("MeshColor") && (*m_meshPt)[i]->numColors() < (*m_meshPt)[i]->numVertices()) {
          shader.setUseCustomColorUniform(true);
          shader.setCustomColorUniform(glm::vec4(0.f, 0.f, 0.f, 1.f));
        } else {
          shader.setUseCustomColorUniform(false);
        }

        GLenum type = (*m_meshPt)[i]->type();
        const std::vector<glm::vec3>& vertices = (*m_meshPt)[i]->vertices();
        const std::vector<GLuint>& triangleIndexes = (*m_meshPt)[i]->indices();
        m_VAOs.bind(i);
        if (triangleIndexes.empty()) {
          glDrawArrays(type, 0, vertices.size());
        } else {
          glDrawElements(type, triangleIndexes.size(), GL_UNSIGNED_INT, 0);
        }
        m_VAOs.release();
      }
    }

    if (!m_wireframeMode.isSelected("No Wireframe")) {
      // offset the wireframe
      glEnable(GL_POLYGON_OFFSET_LINE);
      glPolygonOffset(-1, -1);

      // draw the wireframe
      glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
      for (size_t i = 0; i < m_meshPt->size(); ++i) {
        shader.setUseCustomColorUniform(true);
        shader.setCustomColorUniform(m_wireframeColor.get());

        GLenum type = (*m_meshPt)[i]->type();
        const std::vector<glm::vec3>& vertices = (*m_meshPt)[i]->vertices();
        const std::vector<GLuint>& triangleIndexes = (*m_meshPt)[i]->indices();
        m_VAOs.bind(i);
        if (triangleIndexes.empty()) {
          glDrawArrays(type, 0, vertices.size());
        } else {
          glDrawElements(type, triangleIndexes.size(), GL_UNSIGNED_INT, 0);
        }
        m_VAOs.release();
      }
      // restore default polygon mode
      glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
      glDisable(GL_POLYGON_OFFSET_LINE);
    }

  } else {
    //    for (size_t i=0; i<m_meshPt->size(); ++i) {
    //      shader.setUniformValue("lighting_enabled", m_needLighting);
    //      if (m_colorSource.isSelected("CustomColor"))
    //        renderTriangleList(shader, *((*m_meshPt)[i]), (*m_meshColorsPt)[i]);
    //      else
    //        renderTriangleList(shader, *((*m_meshPt)[i]));
    //    }
    if (m_dataChanged) {
      m_VBOs.resize(m_meshPt->size());
      for (size_t ivbo = 0; ivbo < m_VBOs.size(); ++ivbo) {
        m_VBOs[ivbo].resize(4);
      }
    }

    for (size_t i = 0; i < m_meshPt->size(); ++i) {
      if (m_colorSource.isSelected("CustomColor")) {
        shader.setUseCustomColorUniform(true);
        shader.setCustomColorUniform((*m_meshColorsPt)[i]);
      } else if (m_colorSource.isSelected("MeshColor") && (*m_meshPt)[i]->numColors() < (*m_meshPt)[i]->numVertices()) {
        shader.setUseCustomColorUniform(true);
        shader.setCustomColorUniform(glm::vec4(0.f, 0.f, 0.f, 1.f));
      } else {
        shader.setUseCustomColorUniform(false);
      }

      const std::vector<glm::vec3>& vertices = (*m_meshPt)[i]->vertices();
      const std::vector<float>& textureCoordinates1D = (*m_meshPt)[i]->textureCoordinates1D();
      const std::vector<glm::vec2>& textureCoordinates2D = (*m_meshPt)[i]->textureCoordinates2D();
      const std::vector<glm::vec3>& textureCoordinates3D = (*m_meshPt)[i]->textureCoordinates3D();
      const std::vector<glm::vec3>& normals = (*m_meshPt)[i]->normals();
      const std::vector<glm::vec4>& colors = (*m_meshPt)[i]->colors();
      const std::vector<GLuint>& triangleIndexes = (*m_meshPt)[i]->indices();
      GLenum type = (*m_meshPt)[i]->type();

      int bufIdx = 0;
      glEnableVertexAttribArray(attr_vertex);
      m_VBOs[i].bind(GL_ARRAY_BUFFER, bufIdx++);
      if (m_dataChanged)
        glBufferData(GL_ARRAY_BUFFER, vertices.size() * 3 * sizeof(GLfloat), vertices.data(), GL_STATIC_DRAW);
      glVertexAttribPointer(attr_vertex, 3, GL_FLOAT, GL_FALSE, 0, 0);

      if (attr_normal != -1) {
        glEnableVertexAttribArray(attr_normal);
        m_VBOs[i].bind(GL_ARRAY_BUFFER, bufIdx++);
        if (m_dataChanged)
          glBufferData(GL_ARRAY_BUFFER, normals.size() * 3 * sizeof(GLfloat), normals.data(), GL_STATIC_DRAW);
        glVertexAttribPointer(attr_normal, 3, GL_FLOAT, GL_FALSE, 0, 0);
      }

      if (!triangleIndexes.empty()) {
        m_VBOs[i].bind(GL_ELEMENT_ARRAY_BUFFER, bufIdx++);
        if (m_dataChanged)
          glBufferData(GL_ELEMENT_ARRAY_BUFFER, triangleIndexes.size() * sizeof(GLuint), triangleIndexes.data(),
                       GL_STATIC_DRAW);
      }

      if (m_colorSource.isSelected("Mesh1DTexture") && attr_1dTexCoord0 != -1 && !textureCoordinates1D.empty()) {
        glEnableVertexAttribArray(attr_1dTexCoord0);
        m_VBOs[i].bind(GL_ARRAY_BUFFER, bufIdx++);
        if (m_dataChanged)
          glBufferData(GL_ARRAY_BUFFER, textureCoordinates1D.size() * 1 * sizeof(GLfloat), textureCoordinates1D.data(),
                       GL_STATIC_DRAW);
        glVertexAttribPointer(attr_1dTexCoord0, 1, GL_FLOAT, GL_FALSE, 0, 0);
      }

      if (m_colorSource.isSelected("Mesh2DTexture") && attr_2dTexCoord0 != -1 && !textureCoordinates2D.empty()) {
        glEnableVertexAttribArray(attr_2dTexCoord0);
        m_VBOs[i].bind(GL_ARRAY_BUFFER, bufIdx++);
        if (m_dataChanged)
          glBufferData(GL_ARRAY_BUFFER, textureCoordinates2D.size() * 2 * sizeof(GLfloat), textureCoordinates2D.data(),
                       GL_STATIC_DRAW);
        glVertexAttribPointer(attr_2dTexCoord0, 2, GL_FLOAT, GL_FALSE, 0, 0);
      }

      if (m_colorSource.isSelected("Mesh3DTexture") && attr_3dTexCoord0 != -1 && !textureCoordinates3D.empty()) {
        glEnableVertexAttribArray(attr_3dTexCoord0);
        m_VBOs[i].bind(GL_ARRAY_BUFFER, bufIdx++);
        if (m_dataChanged)
          glBufferData(GL_ARRAY_BUFFER, textureCoordinates3D.size() * 3 * sizeof(GLfloat), textureCoordinates3D.data(),
                       GL_STATIC_DRAW);
        glVertexAttribPointer(attr_3dTexCoord0, 3, GL_FLOAT, GL_FALSE, 0, 0);
      }

      if (m_colorSource.isSelected("MeshColor") && attr_color != -1 && colors.size() >= vertices.size()) {
        glEnableVertexAttribArray(attr_color);
        m_VBOs[i].bind(GL_ARRAY_BUFFER, bufIdx++);
        if (m_dataChanged)
          glBufferData(GL_ARRAY_BUFFER, colors.size() * 4 * sizeof(GLfloat), colors.data(), GL_STATIC_DRAW);
        glVertexAttribPointer(attr_color, 4, GL_FLOAT, GL_FALSE, 0, 0);
      }

      if (!m_wireframeMode.isSelected("Only Wireframe")) {
        if (triangleIndexes.empty()) {
          glDrawArrays(type, 0, vertices.size());
        } else {
          glDrawElements(type, triangleIndexes.size(), GL_UNSIGNED_INT, 0);
          glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
        }
      }
      if (!m_wireframeMode.isSelected("No Wireframe")) {
        shader.setUseCustomColorUniform(true);
        shader.setCustomColorUniform(m_wireframeColor.get());

        // offset the wireframe
        glEnable(GL_POLYGON_OFFSET_LINE);
        glPolygonOffset(-1, -1);

        // draw the wireframe
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        if (triangleIndexes.empty()) {
          glDrawArrays(type, 0, vertices.size());
        } else {
          glDrawElements(type, triangleIndexes.size(), GL_UNSIGNED_INT, 0);
          glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
        }
        // restore default polygon mode
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        glDisable(GL_POLYGON_OFFSET_LINE);
      }

      glBindBuffer(GL_ARRAY_BUFFER, 0);

      glDisableVertexAttribArray(attr_vertex);
      if (attr_normal != -1)
        glDisableVertexAttribArray(attr_normal);
      if (m_colorSource.isSelected("Mesh1DTexture") && attr_1dTexCoord0 != -1 && !textureCoordinates1D.empty())
        glDisableVertexAttribArray(attr_1dTexCoord0);
      if (m_colorSource.isSelected("Mesh2DTexture") && attr_2dTexCoord0 != -1 && !textureCoordinates2D.empty())
        glDisableVertexAttribArray(attr_2dTexCoord0);
      if (m_colorSource.isSelected("Mesh3DTexture") && attr_3dTexCoord0 != -1 && !textureCoordinates3D.empty())
        glDisableVertexAttribArray(attr_3dTexCoord0);
      if (m_colorSource.isSelected("MeshColor") && attr_color != -1 && colors.size() >= vertices.size())
        glDisableVertexAttribArray(attr_color);
    }

    m_dataChanged = false;
  }

  m_meshShaderGrp.release();
}

void Z3DMeshRenderer::renderPicking(Z3DEye eye)
{
  if (!m_meshPt || m_meshPt->empty())
    return;

  if (!m_meshPickingColorReady)
    prepareMeshPickingColor();

  if (!m_meshPickingColorsPt || m_meshPickingColorsPt->empty()
      || m_meshPickingColorsPt->size() != m_meshPt->size())
    return;

  m_meshShaderGrp.bind();
  Z3DShaderProgram& shader = m_meshShaderGrp.get();
  m_rendererBase.setGlobalShaderParameters(shader, eye);
  setPickingShaderParameters(shader);
  shader.setUseCustomColorUniform(true);

  GLint attr_vertex = shader.vertexAttributeLocation();
  GLint attr_normal = shader.normalAttributeLocation();

  if (m_hardwareSupportVAO) {
    if (m_pickingDataChanged) {
      m_pickingVAOs.resize(m_meshPt->size());

      m_pickingVBOs.resize(m_meshPt->size());
      for (size_t ivbo = 0; ivbo < m_pickingVBOs.size(); ++ivbo) {
        m_pickingVBOs[ivbo].resize(3);
      }

      for (size_t i = 0; i < m_meshPt->size(); ++i) {
        m_pickingVAOs.bind(i);

        const std::vector<glm::vec3>& vertices = (*m_meshPt)[i]->vertices();
        const std::vector<glm::vec3>& normals = (*m_meshPt)[i]->normals();
        const std::vector<GLuint>& triangleIndexes = (*m_meshPt)[i]->indices();

        int bufIdx = 0;
        glEnableVertexAttribArray(attr_vertex);
        if (m_dataChanged) {
          m_pickingVBOs[i].bind(GL_ARRAY_BUFFER, bufIdx++);
          glBufferData(GL_ARRAY_BUFFER, vertices.size() * 3 * sizeof(GLfloat), vertices.data(), GL_STATIC_DRAW);
        } else {
          m_VBOs[i].bind(GL_ARRAY_BUFFER, bufIdx++);
        }
        glVertexAttribPointer(attr_vertex, 3, GL_FLOAT, GL_FALSE, 0, 0);

        if (attr_normal != -1) {
          glEnableVertexAttribArray(attr_normal);
          if (m_dataChanged) {
            m_pickingVBOs[i].bind(GL_ARRAY_BUFFER, bufIdx++);
            glBufferData(GL_ARRAY_BUFFER, normals.size() * 3 * sizeof(GLfloat), normals.data(), GL_STATIC_DRAW);
          } else {
            m_VBOs[i].bind(GL_ARRAY_BUFFER, bufIdx++);
          }
          glVertexAttribPointer(attr_normal, 3, GL_FLOAT, GL_FALSE, 0, 0);
        }

        if (!triangleIndexes.empty()) {
          if (m_dataChanged) {
            m_pickingVBOs[i].bind(GL_ELEMENT_ARRAY_BUFFER, bufIdx++);
            glBufferData(GL_ELEMENT_ARRAY_BUFFER, triangleIndexes.size() * sizeof(GLuint), triangleIndexes.data(),
                         GL_STATIC_DRAW);
          } else {
            m_VBOs[i].bind(GL_ELEMENT_ARRAY_BUFFER, bufIdx++);
          }
        }

        glBindBuffer(GL_ARRAY_BUFFER, 0);

        m_pickingVAOs.release();
      }

      m_pickingDataChanged = false;
    }

    if (m_wireframeMode.isSelected("Only Wireframe")) {
      // offset the wireframe
      glEnable(GL_POLYGON_OFFSET_LINE);
      glPolygonOffset(-1, -1);

      // draw the wireframe
      glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    }

    for (size_t i = 0; i < m_meshPt->size(); ++i) {
      shader.setCustomColorUniform((*m_meshPickingColorsPt)[i]);

      GLenum type = (*m_meshPt)[i]->type();
      const std::vector<glm::vec3>& vertices = (*m_meshPt)[i]->vertices();
      const std::vector<GLuint>& triangleIndexes = (*m_meshPt)[i]->indices();

      m_pickingVAOs.bind(i);
      if (triangleIndexes.empty()) {
        glDrawArrays(type, 0, vertices.size());
      } else {
        glDrawElements(type, triangleIndexes.size(), GL_UNSIGNED_INT, 0);
      }
      m_pickingVAOs.release();
    }

    if (m_wireframeMode.isSelected("Only Wireframe")) {
      // restore default polygon mode
      glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
      glDisable(GL_POLYGON_OFFSET_LINE);
    }

  } else {
    //for (size_t i=0; i<m_meshPt->size(); ++i) {
    //renderTriangleList(shader, *((*m_meshPt)[i]), (*m_meshPickingColorsPt)[i]);
    //}
    if (m_pickingDataChanged) {
      m_pickingVBOs.resize(m_meshPt->size());
      for (size_t ivbo = 0; ivbo < m_pickingVBOs.size(); ++ivbo) {
        m_pickingVBOs[ivbo].resize(3);
      }
    }

    if (m_wireframeMode.isSelected("Only Wireframe")) {
      // offset the wireframe
      glEnable(GL_POLYGON_OFFSET_LINE);
      glPolygonOffset(-1, -1);

      // draw the wireframe
      glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    }

    for (size_t i = 0; i < m_meshPt->size(); ++i) {
      shader.setCustomColorUniform((*m_meshPickingColorsPt)[i]);

      const std::vector<glm::vec3>& vertices = (*m_meshPt)[i]->vertices();
      const std::vector<glm::vec3>& normals = (*m_meshPt)[i]->normals();
      const std::vector<GLuint>& triangleIndexes = (*m_meshPt)[i]->indices();
      GLenum type = (*m_meshPt)[i]->type();

      int bufIdx = 0;
      glEnableVertexAttribArray(attr_vertex);
      if (m_dataChanged) {
        m_pickingVBOs[i].bind(GL_ARRAY_BUFFER, bufIdx++);
        if (m_pickingDataChanged)
          glBufferData(GL_ARRAY_BUFFER, vertices.size() * 3 * sizeof(GLfloat), vertices.data(), GL_STATIC_DRAW);
      } else {
        m_VBOs[i].bind(GL_ARRAY_BUFFER, bufIdx++);
      }
      glVertexAttribPointer(attr_vertex, 3, GL_FLOAT, GL_FALSE, 0, 0);

      if (attr_normal != -1) {
        glEnableVertexAttribArray(attr_normal);
        if (m_dataChanged) {
          m_pickingVBOs[i].bind(GL_ARRAY_BUFFER, bufIdx++);
          if (m_pickingDataChanged)
            glBufferData(GL_ARRAY_BUFFER, normals.size() * 3 * sizeof(GLfloat), normals.data(), GL_STATIC_DRAW);
        } else {
          m_VBOs[i].bind(GL_ARRAY_BUFFER, bufIdx++);
        }
        glVertexAttribPointer(attr_normal, 3, GL_FLOAT, GL_FALSE, 0, 0);
      }

      if (!triangleIndexes.empty()) {
        if (m_dataChanged) {
          m_pickingVBOs[i].bind(GL_ELEMENT_ARRAY_BUFFER, bufIdx++);
          if (m_pickingDataChanged)
            glBufferData(GL_ELEMENT_ARRAY_BUFFER, triangleIndexes.size() * sizeof(GLuint), triangleIndexes.data(),
                         GL_STATIC_DRAW);
        } else {
          m_VBOs[i].bind(GL_ELEMENT_ARRAY_BUFFER, bufIdx++);
        }
        glDrawElements(type, triangleIndexes.size(), GL_UNSIGNED_INT, 0);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
      } else {
        glDrawArrays(type, 0, vertices.size());
      }

      glBindBuffer(GL_ARRAY_BUFFER, 0);

      glDisableVertexAttribArray(attr_vertex);
      if (attr_normal != -1)
        glDisableVertexAttribArray(attr_normal);
    }

    if (m_wireframeMode.isSelected("Only Wireframe")) {
      // restore default polygon mode
      glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
      glDisable(GL_POLYGON_OFFSET_LINE);
    }

    m_pickingDataChanged = false;
  }

  m_meshShaderGrp.release();
}

void Z3DMeshRenderer::adjustWidgets()
{
  m_wireframeColor.setVisible(!m_wireframeMode.isSelected("No Wireframe"));
}
