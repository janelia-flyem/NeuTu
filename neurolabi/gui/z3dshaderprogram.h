#ifndef Z3DSHADERPROGRAM_H
#define Z3DSHADERPROGRAM_H

#include "z3dtexture.h"
#include "zglmutils.h"
#include "z3dshader.h"
#include "z3dcontext.h"
#include <map>
#include <vector>

// throw ZGLException if error
class Z3DShaderProgram
{
public:
  Z3DShaderProgram();

  ~Z3DShaderProgram();

  Z3DShaderProgram(const Z3DShaderProgram&) = delete;

  Z3DShaderProgram& operator=(const Z3DShaderProgram&) = delete;

  void addShader(Z3DShader& shader);

  void addShaderFromSourceCode(Z3DShader::Type type, const char* source);

  void addShaderFromSourceCode(Z3DShader::Type type, const QString& source)
  { addShaderFromSourceCode(type, source.toUtf8().constData()); }

  void removeAllShaders();

  void link();

  bool isLinked() const
  { return m_linked; }

  // overriding bind() to reset texture unit manager
  void bind();

  void release();

  unsigned int programId() const
  { return m_id; }

  Z3DContextGroup context() const
  { return m_context; }

  void setGeometryInputType(GLenum /*unused*/)
  {}

  void setGeometryOutputType(GLenum /*unused*/)
  {}

  void setGeometryOutputVertexCount(int /*unused*/)
  {}

  void bindFragDataLocation(GLuint colorNumber, const QString& name);

  // bind samplers
  void bindTexture(const QString& name, const Z3DTexture* texture);

  void bindTexture(const QString& name, const Z3DTexture* texture, GLint minFilter, GLint magFilter);

  void bindTexture(const QString& name, GLenum target, GLuint textureId);

  // load functions will load shaders and link, throw Exception if error
  // input filenames should not contain path, shader paths are managed by Z3DApplication
  void loadFromSourceFile(const QString& vertFilename, const QString& geomFilename,
                          const QString& fragFilename, const QString& header, const QString& geomHeader = "");

  void loadFromSourceFile(const QString& vertFilename, const QString& fragFilename,
                          const QString& header, const QString& geomHeader = "");

  void loadFromSourceFile(const QStringList& shaderFilenames, const QString& header, const QString& geomHeader = "");

  // header will be prepended to all srcs
  void loadFromSourceCode(const QStringList& vertSrcs, const QStringList& geomSrcs,
                          const QStringList& fragSrcs, const QString& header = "", const QString& geomHeader = "");

  void loadFromSourceCode(const QStringList& vertSrcs, const QStringList& fragSrcs,
                          const QString& header = "", const QString& geomHeader = "");

  // set new header for current readed src and rebuild
  void setHeaderAndRebuild(const QString& header, const QString& geomHeader = "");

  // log error if uniform can not be found, many typos can be found this way.
  // Logging can be turn off by setLogUniformLocationError(false)
  int uniformLocation(const QString& name) const;

  int uniformLocation(const char* name) const
  { return uniformLocation(QString(name)); }

  int uniformLocation(const QByteArray& name) const
  { return uniformLocation(QString(name)); }

  int attributeLocation(const QString& name) const;

  int attributeLocation(const char* name) const
  { return attributeLocation(QString(name)); }

  int attributeLocation(const QByteArray& name) const
  { return attributeLocation(QString(name)); }

  void setLogUniformLocationError(bool logError)
  { m_logUniformLocationError = logError; }

  bool logUniformLocationError() const
  { return m_logUniformLocationError; }

  void setUniform(GLint loc, GLfloat x)
  { if (loc != -1) { glUniform1f(loc, x); }}

  void setUniform(GLint loc, GLfloat x, GLfloat y)
  { if (loc != -1) { glUniform2f(loc, x, y); }}

  void setUniform(GLint loc, GLfloat x, GLfloat y, GLfloat z)
  { if (loc != -1) { glUniform3f(loc, x, y, z); }}

  void setUniform(GLint loc, GLfloat x, GLfloat y, GLfloat z, GLfloat w)
  { if (loc != -1) { glUniform4f(loc, x, y, z, w); }}

  void setUniform(GLint loc, GLint v1)
  { if (loc != -1) { glUniform1i(loc, v1); }}

  void setUniform(GLint loc, GLint v1, GLint v2)
  { if (loc != -1) { glUniform2i(loc, v1, v2); }}

  void setUniform(GLint loc, GLint v1, GLint v2, GLint v3)
  { if (loc != -1) { glUniform3i(loc, v1, v2, v3); }}

  void setUniform(GLint loc, GLint v1, GLint v2, GLint v3, GLint v4)
  { if (loc != -1) { glUniform4i(loc, v1, v2, v3, v4); }}

  void setUniform(GLint loc, GLuint v1)
  { if (loc != -1) { glUniform1ui(loc, v1); }}

  void setUniform(GLint loc, GLuint v1, GLuint v2)
  { if (loc != -1) { glUniform2ui(loc, v1, v2); }}

  void setUniform(GLint loc, GLuint v1, GLuint v2, GLuint v3)
  { if (loc != -1) { glUniform3ui(loc, v1, v2, v3); }}

  void setUniform(GLint loc, GLuint v1, GLuint v2, GLuint v3, GLuint v4)
  { if (loc != -1) { glUniform4ui(loc, v1, v2, v3, v4); }}

  void setUniform(const QString& name, GLfloat value)
  { setUniform(uniformLocation(name), value); }

  void setUniform(const QString& name, GLint value)
  { setUniform(uniformLocation(name), value); }

  void setUniform(const QString& name, GLuint value)
  { setUniform(uniformLocation(name), value); }

  void setUniform(const QString& name, GLfloat x, GLfloat y)
  { setUniform(uniformLocation(name), x, y); }

  void setUniform(const QString& name, GLfloat x, GLfloat y, GLfloat z)
  { setUniform(uniformLocation(name), x, y, z); }

  void setUniform(const QString& name, GLfloat x, GLfloat y, GLfloat z, GLfloat w)
  { setUniform(uniformLocation(name), x, y, z, w); }

  void setUniform(const QString& name, GLint v1, GLint v2)
  { setUniform(uniformLocation(name), v1, v2); }

  void setUniform(const QString& name, GLint v1, GLint v2, GLint v3)
  { setUniform(uniformLocation(name), v1, v2, v3); }

  void setUniform(const QString& name, GLint v1, GLint v2, GLint v3, GLint v4)
  { setUniform(uniformLocation(name), v1, v2, v3, v4); }

  void setUniform(const QString& name, GLuint v1, GLuint v2)
  { setUniform(uniformLocation(name), v1, v2); }

  void setUniform(const QString& name, GLuint v1, GLuint v2, GLuint v3)
  { setUniform(uniformLocation(name), v1, v2, v3); }

  void setUniform(const QString& name, GLuint v1, GLuint v2, GLuint v3, GLuint v4)
  { setUniform(uniformLocation(name), v1, v2, v3, v4); }

  // Booleans
  //  void setUniformValue(GLint loc, bool value);
  //  void setUniformValue(GLint loc, bool v1, bool v2);
  //  void setUniformValue(GLint loc, bool v1, bool v2, bool v3);
  //  void setUniformValue(GLint loc, bool v1, bool v2, bool v3, bool v4);
  //  void setUniformValue(const QString& name, bool value);
  //  void setUniformValue(const QString& name, bool v1, bool v2);
  //  void setUniformValue(const QString& name, bool v1, bool v2, bool v3);
  //  void setUniformValue(const QString& name, bool v1, bool v2, bool v3, bool v4);

  // Vectors
  void setUniform(GLint loc, const glm::vec2& value)
  { if (loc != -1) { glUniform2fv(loc, 1, &value[0]); }}

  void setUniform(GLint loc, const glm::vec3& value)
  { if (loc != -1) { glUniform3fv(loc, 1, &value[0]); }}

  void setUniform(GLint loc, const glm::vec4& value)
  { if (loc != -1) { glUniform4fv(loc, 1, &value[0]); }}

  void setUniform(GLint loc, const glm::ivec2& value)
  { if (loc != -1) { glUniform2iv(loc, 1, &value[0]); }}

  void setUniform(GLint loc, const glm::ivec3& value)
  { if (loc != -1) { glUniform3iv(loc, 1, &value[0]); }}

  void setUniform(GLint loc, const glm::ivec4& value)
  { if (loc != -1) { glUniform4iv(loc, 1, &value[0]); }}

  void setUniform(GLint loc, const glm::uvec2& value)
  { if (loc != -1) { glUniform2uiv(loc, 1, &value[0]); }}

  void setUniform(GLint loc, const glm::uvec3& value)
  { if (loc != -1) { glUniform3uiv(loc, 1, &value[0]); }}

  void setUniform(GLint loc, const glm::uvec4& value)
  { if (loc != -1) { glUniform4uiv(loc, 1, &value[0]); }}

  void setUniform(const QString& name, const glm::vec2& value)
  { setUniform(uniformLocation(name), value); }

  void setUniform(const QString& name, const glm::vec3& value)
  { setUniform(uniformLocation(name), value); }

  void setUniform(const QString& name, const glm::vec4& value)
  { setUniform(uniformLocation(name), value); }

  void setUniform(const QString& name, const glm::ivec2& value)
  { setUniform(uniformLocation(name), value); }

  void setUniform(const QString& name, const glm::ivec3& value)
  { setUniform(uniformLocation(name), value); }

  void setUniform(const QString& name, const glm::ivec4& value)
  { setUniform(uniformLocation(name), value); }

  void setUniform(const QString& name, const glm::uvec2& value)
  { setUniform(uniformLocation(name), value); }

  void setUniform(const QString& name, const glm::uvec3& value)
  { setUniform(uniformLocation(name), value); }

  void setUniform(const QString& name, const glm::uvec4& value)
  { setUniform(uniformLocation(name), value); }

  void setUniformArray(GLint loc, const GLfloat* values, int count)
  { if (loc != -1) { glUniform1fv(loc, count, values); }}

  void setUniformArray(GLint loc, const glm::vec2* values, int count)
  { if (loc != -1) { glUniform2fv(loc, count, &(values[0][0])); }}

  void setUniformArray(GLint loc, const glm::vec3* values, int count)
  { if (loc != -1) { glUniform3fv(loc, count, &(values[0][0])); }}

  void setUniformArray(GLint loc, const glm::vec4* values, int count)
  { if (loc != -1) { glUniform4fv(loc, count, &(values[0][0])); }}

  void setUniformArray(GLint loc, const glm::ivec2* values, int count)
  { if (loc != -1) { glUniform2iv(loc, count, &(values[0][0])); }}

  void setUniformArray(GLint loc, const glm::ivec3* values, int count)
  { if (loc != -1) { glUniform3iv(loc, count, &(values[0][0])); }}

  void setUniformArray(GLint loc, const glm::ivec4* values, int count)
  { if (loc != -1) { glUniform4iv(loc, count, &(values[0][0])); }}

  void setUniformArray(GLint loc, const glm::uvec2* values, int count)
  { if (loc != -1) { glUniform2uiv(loc, count, &(values[0][0])); }}

  void setUniformArray(GLint loc, const glm::uvec3* values, int count)
  { if (loc != -1) { glUniform3uiv(loc, count, &(values[0][0])); }}

  void setUniformArray(GLint loc, const glm::uvec4* values, int count)
  { if (loc != -1) { glUniform4uiv(loc, count, &(values[0][0])); }}

  void setUniformArray(const QString& name, const GLfloat* values, int count)
  { setUniformArray(uniformLocation(name), values, count); }

  void setUniformArray(const QString& name, const glm::vec2* values, int count)
  { setUniformArray(uniformLocation(name), values, count); }

  void setUniformArray(const QString& name, const glm::vec3* values, int count)
  { setUniformArray(uniformLocation(name), values, count); }

  void setUniformArray(const QString& name, const glm::vec4* values, int count)
  { setUniformArray(uniformLocation(name), values, count); }

  void setUniformArray(const QString& name, const glm::ivec2* values, int count)
  { setUniformArray(uniformLocation(name), values, count); }

  void setUniformArray(const QString& name, const glm::ivec3* values, int count)
  { setUniformArray(uniformLocation(name), values, count); }

  void setUniformArray(const QString& name, const glm::ivec4* values, int count)
  { setUniformArray(uniformLocation(name), values, count); }

  void setUniformArray(const QString& name, const glm::uvec2* values, int count)
  { setUniformArray(uniformLocation(name), values, count); }

  void setUniformArray(const QString& name, const glm::uvec3* values, int count)
  { setUniformArray(uniformLocation(name), values, count); }

  void setUniformArray(const QString& name, const glm::uvec4* values, int count)
  { setUniformArray(uniformLocation(name), values, count); }

  void setUniform(GLint loc, const glm::mat2& value, bool transpose = false)
  { if (loc != -1) { glUniformMatrix2fv(loc, 1, GLboolean(transpose), &value[0][0]); }}

  void setUniform(GLint loc, const glm::mat3& value, bool transpose = false)
  { if (loc != -1) { glUniformMatrix3fv(loc, 1, GLboolean(transpose), &value[0][0]); }}

  void setUniform(GLint loc, const glm::mat4& value, bool transpose = false)
  { if (loc != -1) { glUniformMatrix4fv(loc, 1, GLboolean(transpose), &value[0][0]); }}

  void setUniform(const QString& name, const glm::mat2& value, bool transpose = false)
  { setUniform(uniformLocation(name), value, transpose); }

  void setUniform(const QString& name, const glm::mat3& value, bool transpose = false)
  { setUniform(uniformLocation(name), value, transpose); }

  void setUniform(const QString& name, const glm::mat4& value, bool transpose = false)
  { setUniform(uniformLocation(name), value, transpose); }

  void setUniformArray(GLint loc, const glm::mat2* values, int count, bool transpose = false)
  { if (loc != -1) { glUniformMatrix2fv(loc, count, GLboolean(transpose), &(values[0][0][0])); }}

  void setUniformArray(GLint loc, const glm::mat3* values, int count, bool transpose = false)
  { if (loc != -1) { glUniformMatrix3fv(loc, count, GLboolean(transpose), &(values[0][0][0])); }}

  void setUniformArray(GLint loc, const glm::mat4* values, int count, bool transpose = false)
  { if (loc != -1) { glUniformMatrix4fv(loc, count, GLboolean(transpose), &(values[0][0][0])); }}

  void setUniformArray(const QString& name, const glm::mat2* values, int count, bool transpose = false)
  { setUniformArray(uniformLocation(name), values, count, transpose); }

  void setUniformArray(const QString& name, const glm::mat3* values, int count, bool transpose = false)
  { setUniformArray(uniformLocation(name), values, count, transpose); }

  void setUniformArray(const QString& name, const glm::mat4* values, int count, bool transpose = false)
  { setUniformArray(uniformLocation(name), values, count, transpose); }

  void setScreenDimUniform(const glm::vec2& value)
  { if (m_screenDimUniform) { setUniform(m_screenDimUniform->location, value); }}

  void setScreenDimRCPUniform(const glm::vec2& value)
  { if (m_screenDimRCPUniform) { setUniform(m_screenDimRCPUniform->location, value); }}

  void setCameraPositionUniform(const glm::vec3& value)
  { if (m_cameraPositionUniform) { setUniform(m_cameraPositionUniform->location, value); }}

  void setViewMatrixUniform(const glm::mat4& value)
  { if (m_viewMatrixUniform) { setUniform(m_viewMatrixUniform->location, value); }}

  void setViewMatrixInverseUniform(const glm::mat4& value)
  { if (m_viewMatrixInverseUniform) { setUniform(m_viewMatrixInverseUniform->location, value); }}

  void setProjectionMatrixUniform(const glm::mat4& value)
  { if (m_projectionMatrixUniform) { setUniform(m_projectionMatrixUniform->location, value); }}

  void setProjectionMatrixInverseUniform(const glm::mat4& value)
  { if (m_projectionMatrixInverseUniform) { setUniform(m_projectionMatrixInverseUniform->location, value); }}

  void setNormalMatrixUniform(const glm::mat3& value)
  { if (m_normalMatrixUniform) { setUniform(m_normalMatrixUniform->location, value); }}

  void setViewportMatrixUniform(const glm::mat4& value)
  { if (m_viewportMatrixUniform) { setUniform(m_viewportMatrixUniform->location, value); }}

  void setViewportMatrixInverseUniform(const glm::mat4& value)
  { if (m_viewportMatrixInverseUniform) { setUniform(m_viewportMatrixInverseUniform->location, value); }}

  void setProjectionViewMatrixUniform(const glm::mat4& value)
  { if (m_projectionViewMatrixUniform) { setUniform(m_projectionViewMatrixUniform->location, value); }}

  void setGammaUniform(float value)
  { if (m_gammaUniform) { setUniform(m_gammaUniform->location, value); }}

  void setSizeScaleUniform(float value)
  { if (m_sizeScaleUniform) { setUniform(m_sizeScaleUniform->location, value); }}

  void setPosTransformUniform(const glm::mat4& value)
  { if (m_posTransformUniform) { setUniform(m_posTransformUniform->location, value); }}

  void setPosTransformNormalMatrixUniform(const glm::mat3& value)
  { if (m_posTransformNormalMatrixUniform) { setUniform(m_posTransformNormalMatrixUniform->location, value); }}

  void setLightsPositionUniform(const glm::vec4* value, int count)
  { if (m_lightsPositionUniform) { setUniformArray(m_lightsPositionUniform->location, value, count); }}

  void setLightsAmbientUniform(const glm::vec4* value, int count)
  { if (m_lightsAmbientUniform) { setUniformArray(m_lightsAmbientUniform->location, value, count); }}

  void setLightsDiffuseUniform(const glm::vec4* value, int count)
  { if (m_lightsDiffuseUniform) { setUniformArray(m_lightsDiffuseUniform->location, value, count); }}

  void setLightsSpecularUniform(const glm::vec4* value, int count)
  { if (m_lightsSpecularUniform) { setUniformArray(m_lightsSpecularUniform->location, value, count); }}

  void setLightsSpotCutoffUniform(const float* value, int count)
  { if (m_lightsSpotCutoffUniform) { setUniformArray(m_lightsSpotCutoffUniform->location, value, count); }}

  void setLightsAttenuationUniform(const glm::vec3* value, int count)
  { if (m_lightsAttenuationUniform) { setUniformArray(m_lightsAttenuationUniform->location, value, count); }}

  void setLightsSpotExponentUniform(const float* value, int count)
  { if (m_lightsSpotExponentUniform) { setUniformArray(m_lightsSpotExponentUniform->location, value, count); }}

  void setLightsSpotDirectionUniform(const glm::vec3* value, int count)
  { if (m_lightsSpotDirectionUniform) { setUniformArray(m_lightsSpotDirectionUniform->location, value, count); }}

  void setMaterialSpecularUniform(const glm::vec4& value)
  { if (m_materialSpecularUniform) { setUniform(m_materialSpecularUniform->location, value); }}

  void setMaterialShininessUniform(float value)
  { if (m_materialShininessUniform) { setUniform(m_materialShininessUniform->location, value); }}

  void setMaterialAmbientUniform(const glm::vec4& value)
  { if (m_materialAmbientUniform) { setUniform(m_materialAmbientUniform->location, value); }}

  void setOrthoUniform(float value)
  { if (m_orthoUniform) { setUniform(m_orthoUniform->location, value); }}

  void setSceneAmbientUniform(const glm::vec4& value)
  { if (m_sceneAmbientUniform) { setUniform(m_sceneAmbientUniform->location, value); }}

  void setAlphaUniform(float value)
  { if (m_alphaUniform) { setUniform(m_alphaUniform->location, value); }}

  void setFogColorTopUniform(const glm::vec3& value)
  { if (m_fogColorTopUniform) { setUniform(m_fogColorTopUniform->location, value); }}

  void setFogColorBottomUniform(const glm::vec3& value)
  { if (m_fogColorBottomUniform) { setUniform(m_fogColorBottomUniform->location, value); }}

  void setFogEndUniform(float value)
  { if (m_fogEndUniform) { setUniform(m_fogEndUniform->location, value); }}

  void setFogScaleUniform(float value)
  { if (m_fogScaleUniform) { setUniform(m_fogScaleUniform->location, value); }}

  void setFogDensityLog2eUniform(float value)
  { if (m_fogDensityLog2eUniform) { setUniform(m_fogDensityLog2eUniform->location, value); }}

  void setFogDensityDensityLog2eUniform(float value)
  { if (m_fogDensityDensityLog2eUniform) { setUniform(m_fogDensityDensityLog2eUniform->location, value); }}

  void setClipPlanesUniform(const glm::vec4* value, int count)
  { if (m_clipPlanesUniform) { setUniformArray(m_clipPlanesUniform->location, value, count); }}

  void setLightingEnabledUniform(bool value)
  { if (m_lightingEnabledUniform) { setUniform(m_lightingEnabledUniform->location, value); }}

  void setColor1Uniform(const glm::vec4& value)
  { if (m_color1Uniform) { setUniform(m_color1Uniform->location, value); }}

  void setColor2Uniform(const glm::vec4& value)
  { if (m_color2Uniform) { setUniform(m_color2Uniform->location, value); }}

  void setLineWidthUniform(float value)
  { if (m_lineWidthUniform) { setUniform(m_lineWidthUniform->location, value); }}

  void setBoxCorrectionUniform(float value)
  { if (m_boxCorrectionUniform) { setUniform(m_boxCorrectionUniform->location, value); }}

  void setCustomColorUniform(const glm::vec4& value)
  { if (m_customColorUniform) { setUniform(m_customColorUniform->location, value); }}

  void setUseCustomColorUniform(bool value)
  { if (m_useCustomColorUniform) { setUniform(m_useCustomColorUniform->location, value); }}

  void setUseTwoSidedLightingUniform(bool value)
  { if (m_useTwoSidedLightingUniform) { setUniform(m_useTwoSidedLightingUniform->location, value); }}

  void setRegionUniform(const glm::vec4& value)
  { if (m_regionUniform) { setUniform(m_regionUniform->location, value); }}

  GLint vertexAttributeLocation() const
  { return m_vertexAttribute ? m_vertexAttribute->location : -1; }

  GLint normalAttributeLocation() const
  { return m_normalAttribute ? m_normalAttribute->location : -1; }

  GLint originAttributeLocation() const
  { return m_originAttribute ? m_originAttribute->location : -1; }

  GLint axisAttributeLocation() const
  { return m_axisAttribute ? m_axisAttribute->location : -1; }

  GLint flagsAttributeLocation() const
  { return m_flagsAttribute ? m_flagsAttribute->location : -1; }

  GLint colorAttributeLocation() const
  { return m_colorAttribute ? m_colorAttribute->location : -1; }

  GLint color2AttributeLocation() const
  { return m_color2Attribute ? m_color2Attribute->location : -1; }

  GLint specularShininessAttributeLocation() const
  { return m_specularShininessAttribute ? m_specularShininessAttribute->location : -1; }

  GLint tex1dCoord0AttributeLocation() const
  { return m_1dTexCoord0Attribute ? m_1dTexCoord0Attribute->location : -1; }

  GLint tex2dCoord0AttributeLocation() const
  { return m_2dTexCoord0Attribute ? m_2dTexCoord0Attribute->location : -1; }

  GLint tex3dCoord0AttributeLocation() const
  { return m_3dTexCoord0Attribute ? m_3dTexCoord0Attribute->location : -1; }

  GLint p0AttributeLocation() const
  { return m_p0Attribute ? m_p0Attribute->location : -1; }

  GLint p1AttributeLocation() const
  { return m_p1Attribute ? m_p1Attribute->location : -1; }

  GLint p0ColorAttributeLocation() const
  { return m_p0ColorAttribute ? m_p0ColorAttribute->location : -1; }

  GLint p1ColorAttributeLocation() const
  { return m_p1ColorAttribute ? m_p1ColorAttribute->location : -1; }

  GLint TAttributeLocation() const
  { return m_TAttribute ? m_TAttribute->location : -1; }

protected:
  struct Uniform
  {
    GLenum type;
    GLint size;
    GLint location;
  };
  struct Attribute
  {
    GLenum type;
    GLint size;
    GLint location;
  };

  void storeUniformLocations();

  void storeAttributeLocations();

protected:
  bool m_logUniformLocationError = true;
  // srcs read from file, withour header
  QStringList m_vertSrcs;
  QStringList m_geomSrcs;
  QStringList m_fragSrcs;
  // shader filenames
  QStringList m_shaderFiles;

  Z3DTextureUnitManager m_textureUnitManager;
  std::map<GLint, std::pair<GLenum, GLint>> m_locToTextureUnit;

  std::map<QString, Uniform> m_uniforms;
  std::map<QString, Attribute> m_attributes;
  const Uniform* m_screenDimUniform;
  const Uniform* m_screenDimRCPUniform;
  const Uniform* m_cameraPositionUniform;
  const Uniform* m_viewMatrixUniform;
  const Uniform* m_viewMatrixInverseUniform;
  const Uniform* m_projectionMatrixUniform;
  const Uniform* m_projectionMatrixInverseUniform;
  const Uniform* m_normalMatrixUniform;
  const Uniform* m_viewportMatrixUniform;
  const Uniform* m_viewportMatrixInverseUniform;
  const Uniform* m_projectionViewMatrixUniform;
  const Uniform* m_gammaUniform;
  const Uniform* m_sizeScaleUniform;
  const Uniform* m_posTransformUniform;
  const Uniform* m_posTransformNormalMatrixUniform;
  const Uniform* m_lightsPositionUniform;
  const Uniform* m_lightsAmbientUniform;
  const Uniform* m_lightsDiffuseUniform;
  const Uniform* m_lightsSpecularUniform;
  const Uniform* m_lightsSpotCutoffUniform;
  const Uniform* m_lightsAttenuationUniform;
  const Uniform* m_lightsSpotExponentUniform;
  const Uniform* m_lightsSpotDirectionUniform;
  const Uniform* m_materialSpecularUniform;
  const Uniform* m_materialShininessUniform;
  const Uniform* m_materialAmbientUniform;
  const Uniform* m_orthoUniform;
  const Uniform* m_sceneAmbientUniform;
  const Uniform* m_alphaUniform;
  const Uniform* m_fogColorTopUniform;
  const Uniform* m_fogColorBottomUniform;
  const Uniform* m_fogEndUniform;
  const Uniform* m_fogScaleUniform;
  const Uniform* m_fogDensityLog2eUniform;
  const Uniform* m_fogDensityDensityLog2eUniform;
  const Uniform* m_clipPlanesUniform;
  const Uniform* m_lightingEnabledUniform;
  const Uniform* m_color1Uniform;
  const Uniform* m_color2Uniform;
  const Uniform* m_lineWidthUniform;
  const Uniform* m_boxCorrectionUniform;
  const Uniform* m_customColorUniform;
  const Uniform* m_useCustomColorUniform;
  const Uniform* m_regionUniform;
  const Uniform* m_useTwoSidedLightingUniform;

  const Attribute* m_vertexAttribute;
  const Attribute* m_normalAttribute;
  const Attribute* m_originAttribute;
  const Attribute* m_axisAttribute;
  const Attribute* m_flagsAttribute;
  const Attribute* m_colorAttribute;
  const Attribute* m_color2Attribute;
  const Attribute* m_specularShininessAttribute;
  const Attribute* m_1dTexCoord0Attribute;
  const Attribute* m_2dTexCoord0Attribute;
  const Attribute* m_3dTexCoord0Attribute;
  const Attribute* m_p0Attribute;
  const Attribute* m_p1Attribute;
  const Attribute* m_p0ColorAttribute;
  const Attribute* m_p1ColorAttribute;
  const Attribute* m_TAttribute;

private:
  bool m_linked = false;
  GLuint m_id = 0;
  std::vector<Z3DShader*> m_shaders;
  std::vector<std::unique_ptr<Z3DShader>> m_anonShaders;
  Z3DContextGroup m_context;
};

#endif // Z3DSHADERPROGRAM_H
