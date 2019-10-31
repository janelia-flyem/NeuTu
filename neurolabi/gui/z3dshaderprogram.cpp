#include "z3dshaderprogram.h"

#include <QFile>

#include "z3dgl.h"
#include "zsysteminfo.h"
#include "logging/zqslog.h"
#include "z3dshadermanager.h"
#include "qt/core/zexception.h"


Z3DShaderProgram::Z3DShaderProgram()
{
  m_id = glCreateProgram();
  if (!m_id) {
    throw ZGLException("Z3DShaderProgram: Could not create shader program");
  }
}

Z3DShaderProgram::~Z3DShaderProgram()
{
  glDeleteProgram(m_id);
}

void Z3DShaderProgram::addShader(Z3DShader& shader)
{
  if (std::find(m_shaders.begin(), m_shaders.end(), &shader) != m_shaders.end())
    return;
  if (m_context != shader.context()) {
    throw ZGLException(
      "Z3DShaderProgram: Add shader failed as program and shader are not associated with same context");
  }
  glAttachShader(m_id, shader.shaderId());
  m_linked = false;
  m_shaders.push_back(&shader);
}

void Z3DShaderProgram::addShaderFromSourceCode(Z3DShader::Type type, const char* source)
{
  m_anonShaders.emplace_back(std::make_unique<Z3DShader>(type));
  m_anonShaders[m_anonShaders.size() - 1]->compileSourceCode(source);
  addShader(*m_anonShaders[m_anonShaders.size() - 1].get());
}

void Z3DShaderProgram::removeAllShaders()
{
  for (auto const& shader : m_shaders) {
    glDetachShader(m_id, shader->shaderId());
  }
  for (auto const& shader : m_anonShaders) {
    glDetachShader(m_id, shader->shaderId());
  }
  m_shaders.clear();
  m_anonShaders.clear();
  m_linked = false;
}

void Z3DShaderProgram::link()
{
  GLint value;
  if (m_shaders.empty()) {
    // If there are no explicit shaders, then it is possible that the
    // application added a program binary with glProgramBinaryOES(),
    // or otherwise populated the shaders itself. Check to see if the
    // program is already linked and bail out if so.
    value = 0;
    glGetProgramiv(m_id, GL_LINK_STATUS, &value);
    m_linked = (value != 0);
    if (m_linked) {
      storeUniformLocations();
      storeAttributeLocations();
    }
  }
  glLinkProgram(m_id);
  value = 0;
  glGetProgramiv(m_id, GL_LINK_STATUS, &value);
  m_linked = (value != 0);
  if (m_linked) {
    storeUniformLocations();
    storeAttributeLocations();
  } else {
    value = 0;
    glGetProgramiv(m_id, GL_INFO_LOG_LENGTH, &value);
    QString log;
    if (value > 1) {
      std::vector<char> logbuf(value);
      GLint len;
      glGetProgramInfoLog(m_id, value, &len, logbuf.data());
      log = QString::fromUtf8(logbuf.data());
    } else {
      log = "failed";
    }
    throw ZGLException(QString("Z3DShaderProgram::Link: %1").arg(log));
  }
}

void Z3DShaderProgram::bind()
{
  if (!m_linked)
    link();
  m_textureUnitManager.reset();
  m_locToTextureUnit.clear();
  glUseProgram(m_id);
}

void Z3DShaderProgram::release()
{
  glUseProgram(0);
}

void Z3DShaderProgram::bindFragDataLocation(GLuint colorNumber, const QString& name)
{
  if (GLVersionGE(3, 0)) {
    glBindFragDataLocation(programId(), colorNumber, qUtf8Printable(name));
  }
}

void Z3DShaderProgram::bindTexture(const QString& name, const Z3DTexture* texture)
{
  if (!texture)
    return;

  int loc = uniformLocation(name);
  if (loc != -1) {
    GLenum textureEnum;
    GLint textureNumber;
    auto it = m_locToTextureUnit.find(loc);
    if (it == m_locToTextureUnit.end()) {
      m_textureUnitManager.nextAvailableUnit();
      textureEnum = m_textureUnitManager.currentUnitEnum();
      textureNumber = m_textureUnitManager.currentUnitNumber();
      m_locToTextureUnit[loc] = std::make_pair(textureEnum, textureNumber);
    } else {
      textureEnum = it->second.first;
      textureNumber = it->second.second;
    }
    glActiveTexture(textureEnum);
    texture->bind();
    setUniform(name, textureNumber);
    glActiveTexture(GL_TEXTURE0);
  }
}

void Z3DShaderProgram::bindTexture(const QString& name, const Z3DTexture* texture, GLint minFilter, GLint magFilter)
{
  if (!texture)
    return;

  int loc = uniformLocation(name);
  if (loc != -1) {
    GLenum textureEnum;
    GLint textureNumber;
    auto it = m_locToTextureUnit.find(loc);
    if (it == m_locToTextureUnit.end()) {
      m_textureUnitManager.nextAvailableUnit();
      textureEnum = m_textureUnitManager.currentUnitEnum();
      textureNumber = m_textureUnitManager.currentUnitNumber();
      m_locToTextureUnit[loc] = std::make_pair(textureEnum, textureNumber);
    } else {
      textureEnum = it->second.first;
      textureNumber = it->second.second;
    }
    glActiveTexture(textureEnum);
    texture->bind();
    texture->setFilter(minFilter, magFilter);
    setUniform(name, textureNumber);
    glActiveTexture(GL_TEXTURE0);
  }
}

void Z3DShaderProgram::bindTexture(const QString& name, GLenum target, GLuint textureId)
{
  int loc = uniformLocation(name);
  if (loc != -1) {
    GLenum textureEnum;
    GLint textureNumber;
    auto it = m_locToTextureUnit.find(loc);
    if (it == m_locToTextureUnit.end()) {
      m_textureUnitManager.nextAvailableUnit();
      textureEnum = m_textureUnitManager.currentUnitEnum();
      textureNumber = m_textureUnitManager.currentUnitNumber();
      m_locToTextureUnit[loc] = std::make_pair(textureEnum, textureNumber);
    } else {
      textureEnum = it->second.first;
      textureNumber = it->second.second;
    }
    glActiveTexture(textureEnum);
    glBindTexture(target, textureId);
    setUniform(name, textureNumber);
    glActiveTexture(GL_TEXTURE0);
  }
}

void Z3DShaderProgram::loadFromSourceFile(const QString& vertFilename, const QString& geomFilename,
                                          const QString& fragFilename, const QString& header, const QString& geomHeader)
{
  removeAllShaders();
  addShader(Z3DShaderManager::instance().shader(vertFilename, header, m_context));
  addShader(Z3DShaderManager::instance().shader(fragFilename, header, m_context));
  if (!geomFilename.isEmpty()) {
    addShader(Z3DShaderManager::instance().shader(geomFilename, geomHeader, m_context));
  }
  link();
  m_shaderFiles.clear();
  m_shaderFiles << vertFilename << fragFilename << geomFilename;
}

void Z3DShaderProgram::loadFromSourceFile(const QString& vertFilename, const QString& fragFilename,
                                          const QString& header, const QString& geomHeader)
{
  loadFromSourceFile(vertFilename, "", fragFilename, header, geomHeader);
}

void Z3DShaderProgram::loadFromSourceFile(const QStringList& shaderFilenames, const QString& header,
                                          const QString& geomHeader)
{
  removeAllShaders();
  for (int i = 0; i < shaderFilenames.size(); ++i) {
    if (shaderFilenames[i].isEmpty())
      continue;
    if (shaderFilenames[i].endsWith(".geom", Qt::CaseInsensitive)) {
      addShader(Z3DShaderManager::instance().shader(shaderFilenames[i], geomHeader, m_context));
    } else {
      addShader(Z3DShaderManager::instance().shader(shaderFilenames[i], header, m_context));
    }
  }
  link();
  m_shaderFiles = shaderFilenames;
}

void Z3DShaderProgram::loadFromSourceCode(const QStringList& vertSrcs, const QStringList& geomSrcs,
                                          const QStringList& fragSrcs, const QString& header, const QString& geomHeader)
{
  removeAllShaders();
  for (int i = 0; i < vertSrcs.size(); ++i) {
    QString vertSrc = header + vertSrcs[i];
    addShaderFromSourceCode(Z3DShader::Type::Vertex, vertSrc);
  }

  for (int i = 0; i < geomSrcs.size(); ++i) {
    QString geomSrc = geomHeader + geomSrcs[i];
    addShaderFromSourceCode(Z3DShader::Type::Geometry, geomSrc);
  }

  for (int i = 0; i < fragSrcs.size(); ++i) {
    QString fragSrc = header + fragSrcs[i];
    addShaderFromSourceCode(Z3DShader::Type::Fragment, fragSrc);
  }

  link();
}

void Z3DShaderProgram::loadFromSourceCode(const QStringList& vertSrcs, const QStringList& fragSrcs,
                                          const QString& header, const QString& geomHeader)
{
  loadFromSourceCode(vertSrcs, QStringList(), fragSrcs, header, geomHeader);
}

void Z3DShaderProgram::setHeaderAndRebuild(const QString& header, const QString& geomHeader)
{
  loadFromSourceFile(m_shaderFiles, header, geomHeader);
}

int Z3DShaderProgram::uniformLocation(const QString& name) const
{
  std::map<QString, Uniform>::const_iterator it = m_uniforms.find(name);
  if (it != m_uniforms.end()) {
    return it->second.location;
  }
  if (logUniformLocationError()) {
    LOG(WARNING) << "Failed to locate uniform: " << name;
  }
  return -1;
}

int Z3DShaderProgram::attributeLocation(const QString& name) const
{
  std::map<QString, Attribute>::const_iterator it = m_attributes.find(name);
  if (it != m_attributes.end()) {
    return it->second.location;
  }
  if (logUniformLocationError()) {
    LOG(WARNING) << "Failed to locate attribute: " << name;
  }
  return -1;
}

//void Z3DShaderProgram::setUniformValue(GLint loc, bool value)
//{
//  setUniformValue(loc, static_cast<GLint>(value));
//}

//void Z3DShaderProgram::setUniformValue(GLint loc, bool v1, bool v2)
//{
//  setUniformValue(loc, static_cast<GLint>(v1), static_cast<GLint>(v2));
//}

//void Z3DShaderProgram::setUniformValue(GLint loc, bool v1, bool v2, bool v3)
//{
//  setUniformValue(loc, static_cast<GLint>(v1), static_cast<GLint>(v2), static_cast<GLint>(v3));
//}

//void Z3DShaderProgram::setUniformValue(GLint loc, bool v1, bool v2, bool v3, bool v4)
//{
//  setUniformValue(loc, static_cast<GLint>(v1), static_cast<GLint>(v2), static_cast<GLint>(v3), static_cast<GLint>(v4));
//}

//void Z3DShaderProgram::setUniformValue(const QString &name, bool value)
//{
//  setUniformValue(name, static_cast<GLint>(value));
//}

//void Z3DShaderProgram::setUniformValue(const QString &name, bool v1, bool v2)
//{
//  setUniformValue(name, static_cast<GLint>(v1), static_cast<GLint>(v2));
//}

//void Z3DShaderProgram::setUniformValue(const QString &name, bool v1, bool v2, bool v3)
//{
//  setUniformValue(name, static_cast<GLint>(v1), static_cast<GLint>(v2), static_cast<GLint>(v3));
//}

//void Z3DShaderProgram::setUniformValue(const QString &name, bool v1, bool v2, bool v3, bool v4)
//{
//  setUniformValue(name, static_cast<GLint>(v1), static_cast<GLint>(v2), static_cast<GLint>(v3), static_cast<GLint>(v4));
//}

void Z3DShaderProgram::storeUniformLocations()
{
  m_uniforms.clear();
  GLint count;
  GLint maxLength;
  glGetProgramiv(programId(), GL_ACTIVE_UNIFORMS, &count);
  glGetProgramiv(programId(), GL_ACTIVE_UNIFORM_MAX_LENGTH, &maxLength);
  std::vector<char> name(maxLength);
  for (GLint i = 0; i < count; ++i) {
    Uniform u;
    glGetActiveUniform(programId(), i, maxLength, nullptr, &u.size, &u.type, name.data());
    u.location = glGetUniformLocation(programId(), name.data());
    QString nm(name.data());
    if (nm.endsWith("[0]"))
      nm.chop(3);
    efficientAddOrUpdate(m_uniforms, nm, u);
  }

  std::map<QString, Uniform>::const_iterator it;

  it = m_uniforms.find("screen_dim");
  m_screenDimUniform = (it == m_uniforms.end()) ? nullptr : &(it->second);
  it = m_uniforms.find("screen_dim_RCP");
  m_screenDimRCPUniform = (it == m_uniforms.end()) ? nullptr : &(it->second);
  it = m_uniforms.find("camera_position");
  m_cameraPositionUniform = (it == m_uniforms.end()) ? nullptr : &(it->second);
  it = m_uniforms.find("view_matrix");
  m_viewMatrixUniform = (it == m_uniforms.end()) ? nullptr : &(it->second);
  it = m_uniforms.find("view_matrix_inverse");
  m_viewMatrixInverseUniform = (it == m_uniforms.end()) ? nullptr : &(it->second);
  it = m_uniforms.find("projection_matrix");
  m_projectionMatrixUniform = (it == m_uniforms.end()) ? nullptr : &(it->second);
  it = m_uniforms.find("projection_matrix_inverse");
  m_projectionMatrixInverseUniform = (it == m_uniforms.end()) ? nullptr : &(it->second);
  it = m_uniforms.find("normal_matrix");
  m_normalMatrixUniform = (it == m_uniforms.end()) ? nullptr : &(it->second);
  it = m_uniforms.find("viewport_matrix");
  m_viewportMatrixUniform = (it == m_uniforms.end()) ? nullptr : &(it->second);
  it = m_uniforms.find("viewport_matrix_inverse");
  m_viewportMatrixInverseUniform = (it == m_uniforms.end()) ? nullptr : &(it->second);
  it = m_uniforms.find("projection_view_matrix");
  m_projectionViewMatrixUniform = (it == m_uniforms.end()) ? nullptr : &(it->second);
  it = m_uniforms.find("gamma");
  m_gammaUniform = (it == m_uniforms.end()) ? nullptr : &(it->second);
  it = m_uniforms.find("size_scale");
  m_sizeScaleUniform = (it == m_uniforms.end()) ? nullptr : &(it->second);
  it = m_uniforms.find("pos_transform");
  m_posTransformUniform = (it == m_uniforms.end()) ? nullptr : &(it->second);
  it = m_uniforms.find("pos_transform_normal_matrix");
  m_posTransformNormalMatrixUniform = (it == m_uniforms.end()) ? nullptr : &(it->second);
  it = m_uniforms.find("lights_position");
  m_lightsPositionUniform = (it == m_uniforms.end()) ? nullptr : &(it->second);
  it = m_uniforms.find("lights_ambient");
  m_lightsAmbientUniform = (it == m_uniforms.end()) ? nullptr : &(it->second);
  it = m_uniforms.find("lights_diffuse");
  m_lightsDiffuseUniform = (it == m_uniforms.end()) ? nullptr : &(it->second);
  it = m_uniforms.find("lights_specular");
  m_lightsSpecularUniform = (it == m_uniforms.end()) ? nullptr : &(it->second);
  it = m_uniforms.find("lights_spotCutoff");
  m_lightsSpotCutoffUniform = (it == m_uniforms.end()) ? nullptr : &(it->second);
  it = m_uniforms.find("lights_attenuation");
  m_lightsAttenuationUniform = (it == m_uniforms.end()) ? nullptr : &(it->second);
  it = m_uniforms.find("lights_spotExponent");
  m_lightsSpotExponentUniform = (it == m_uniforms.end()) ? nullptr : &(it->second);
  it = m_uniforms.find("lights_spotDirection");
  m_lightsSpotDirectionUniform = (it == m_uniforms.end()) ? nullptr : &(it->second);
  it = m_uniforms.find("material_specular");
  m_materialSpecularUniform = (it == m_uniforms.end()) ? nullptr : &(it->second);
  it = m_uniforms.find("material_shininess");
  m_materialShininessUniform = (it == m_uniforms.end()) ? nullptr : &(it->second);
  it = m_uniforms.find("material_ambient");
  m_materialAmbientUniform = (it == m_uniforms.end()) ? nullptr : &(it->second);
  it = m_uniforms.find("ortho");
  m_orthoUniform = (it == m_uniforms.end()) ? nullptr : &(it->second);
  it = m_uniforms.find("scene_ambient");
  m_sceneAmbientUniform = (it == m_uniforms.end()) ? nullptr : &(it->second);
  it = m_uniforms.find("alpha");
  m_alphaUniform = (it == m_uniforms.end()) ? nullptr : &(it->second);
  it = m_uniforms.find("fog_color_top");
  m_fogColorTopUniform = (it == m_uniforms.end()) ? nullptr : &(it->second);
  it = m_uniforms.find("fog_color_bottom");
  m_fogColorBottomUniform = (it == m_uniforms.end()) ? nullptr : &(it->second);
  it = m_uniforms.find("fog_end");
  m_fogEndUniform = (it == m_uniforms.end()) ? nullptr : &(it->second);
  it = m_uniforms.find("fog_scale");
  m_fogScaleUniform = (it == m_uniforms.end()) ? nullptr : &(it->second);
  it = m_uniforms.find("fog_density_log2e");
  m_fogDensityLog2eUniform = (it == m_uniforms.end()) ? nullptr : &(it->second);
  it = m_uniforms.find("fog_density_density_log2e");
  m_fogDensityDensityLog2eUniform = (it == m_uniforms.end()) ? nullptr : &(it->second);
  it = m_uniforms.find("clip_planes");
  m_clipPlanesUniform = (it == m_uniforms.end()) ? nullptr : &(it->second);
  it = m_uniforms.find("lighting_enabled");
  m_lightingEnabledUniform = (it == m_uniforms.end()) ? nullptr : &(it->second);
  it = m_uniforms.find("color1");
  m_color1Uniform = (it == m_uniforms.end()) ? nullptr : &(it->second);
  it = m_uniforms.find("color2");
  m_color2Uniform = (it == m_uniforms.end()) ? nullptr : &(it->second);
  it = m_uniforms.find("line_width");
  m_lineWidthUniform = (it == m_uniforms.end()) ? nullptr : &(it->second);
  it = m_uniforms.find("box_correction");
  m_boxCorrectionUniform = (it == m_uniforms.end()) ? nullptr : &(it->second);
  it = m_uniforms.find("custom_color");
  m_customColorUniform = (it == m_uniforms.end()) ? nullptr : &(it->second);
  it = m_uniforms.find("use_custom_color");
  m_useCustomColorUniform = (it == m_uniforms.end()) ? nullptr : &(it->second);
  it = m_uniforms.find("region");
  m_regionUniform = (it == m_uniforms.end()) ? nullptr : &(it->second);
  it = m_uniforms.find("use_two_sided_lighting");
  m_useTwoSidedLightingUniform = (it == m_uniforms.end()) ? nullptr : &(it->second);
}

void Z3DShaderProgram::storeAttributeLocations()
{
  m_attributes.clear();
  GLint count;
  GLint maxLength;
  glGetProgramiv(programId(), GL_ACTIVE_ATTRIBUTES, &count);
  glGetProgramiv(programId(), GL_ACTIVE_ATTRIBUTE_MAX_LENGTH, &maxLength);
  std::vector<char> name(maxLength);
  for (GLint i = 0; i < count; ++i) {
    Attribute u;
    glGetActiveAttrib(programId(), i, maxLength, nullptr, &u.size, &u.type, name.data());
    u.location = glGetAttribLocation(programId(), name.data());
    efficientAddOrUpdate(m_attributes, name.data(), u);
  }

  std::map<QString, Attribute>::const_iterator it;

  it = m_attributes.find("attr_vertex");
  m_vertexAttribute = (it == m_attributes.end()) ? nullptr : &(it->second);
  it = m_attributes.find("attr_normal");
  m_normalAttribute = (it == m_attributes.end()) ? nullptr : &(it->second);
  it = m_attributes.find("attr_origin");
  m_originAttribute = (it == m_attributes.end()) ? nullptr : &(it->second);
  it = m_attributes.find("attr_axis");
  m_axisAttribute = (it == m_attributes.end()) ? nullptr : &(it->second);
  it = m_attributes.find("attr_flags");
  m_flagsAttribute = (it == m_attributes.end()) ? nullptr : &(it->second);
  it = m_attributes.find("attr_color");
  m_colorAttribute = (it == m_attributes.end()) ? nullptr : &(it->second);
  it = m_attributes.find("attr_color2");
  m_color2Attribute = (it == m_attributes.end()) ? nullptr : &(it->second);
  it = m_attributes.find("attr_specular_shininess");
  m_specularShininessAttribute = (it == m_attributes.end()) ? nullptr : &(it->second);
  it = m_attributes.find("attr_1dTexCoord0");
  m_1dTexCoord0Attribute = (it == m_attributes.end()) ? nullptr : &(it->second);
  it = m_attributes.find("attr_2dTexCoord0");
  m_2dTexCoord0Attribute = (it == m_attributes.end()) ? nullptr : &(it->second);
  it = m_attributes.find("attr_3dTexCoord0");
  m_3dTexCoord0Attribute = (it == m_attributes.end()) ? nullptr : &(it->second);
  it = m_attributes.find("attr_p0");
  m_p0Attribute = (it == m_attributes.end()) ? nullptr : &(it->second);
  it = m_attributes.find("attr_p1");
  m_p1Attribute = (it == m_attributes.end()) ? nullptr : &(it->second);
  it = m_attributes.find("attr_p0color");
  m_p0ColorAttribute = (it == m_attributes.end()) ? nullptr : &(it->second);
  it = m_attributes.find("attr_p1color");
  m_p1ColorAttribute = (it == m_attributes.end()) ? nullptr : &(it->second);
  it = m_attributes.find("attr_T");
  m_TAttribute = (it == m_attributes.end()) ? nullptr : &(it->second);
}
