#ifndef Z3DSHADERGROUP_H
#define Z3DSHADERGROUP_H

#include "z3drendererbase.h"
#include "z3dshaderprogram.h"
#include <map>

class Z3DShaderGroup
{
public:
  explicit Z3DShaderGroup(Z3DRendererBase& rendererBase);

  void init(const QStringList& shaderFiles, const QString& header, const QString& geomHeader = "",
            const QStringList& normalShaderFiles = QStringList());

  void addAllSupportedPostShaders();

  void addDualDepthPeelingShaders();

  void addWeightedAverageShaders();

  void addWeightedBlendedShaders();

  void setGeometryInputType(GLenum inputType)
  { m_geometryInputType = inputType; }

  void setGeometryOutputType(GLenum outputType)
  { m_geometryOutputType = outputType; }

  void setGeometryOutputVertexCount(int count)
  { m_geometryOutputVertexCount = count; }

  void bind();

  void release();

  Z3DShaderProgram& get();

  void rebuild(const QString& header, const QString& geomHeader = "");

private:
  void buildNormalShader(Z3DShaderProgram* shader);

  void buildDualDepthPeelingInitShader(Z3DShaderProgram* shader);

  void buildDualDepthPeelingPeelShader(Z3DShaderProgram* shader);

  void buildWeightedAverageShader(Z3DShaderProgram* shader);

  void buildWeightedBlendedShader(Z3DShaderProgram* shader);

private:
  QStringList m_shaderFiles;
  QString m_header;
  QString m_geomHeader;
  Z3DRendererBase& m_base;
  QStringList m_normalShaderFiles;
  std::map<Z3DRendererBase::ShaderHookType, std::unique_ptr<Z3DShaderProgram>> m_shaders;

  GLenum m_geometryInputType;
  GLenum m_geometryOutputType;
  int m_geometryOutputVertexCount;
};

#endif // Z3DSHADERGROUP_H
