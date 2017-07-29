#include "z3dshadergroup.h"

#include "z3dgl.h"
#include "z3dgpuinfo.h"
#include "z3dshaderprogram.h"

Z3DShaderGroup::Z3DShaderGroup(Z3DRendererBase& rendererBase)
  : m_base(rendererBase)
  , m_geometryInputType(GL_LINES_ADJACENCY)
  , m_geometryOutputType(GL_TRIANGLE_STRIP)
  , m_geometryOutputVertexCount(24)
{
}

void Z3DShaderGroup::init(const QStringList& shaderFiles, const QString& header, const QString& geomHeader,
                          const QStringList& normalShaderFiles)
{
  m_shaderFiles = shaderFiles;
  m_header = header;
  m_geomHeader = geomHeader;
  m_normalShaderFiles = normalShaderFiles;
  m_shaders[Z3DRendererBase::ShaderHookType::Normal].reset(new Z3DShaderProgram());
  if (!GLVersionGE(3, 2)) {
    m_shaders[Z3DRendererBase::ShaderHookType::Normal]->setGeometryInputType(m_geometryInputType);
    m_shaders[Z3DRendererBase::ShaderHookType::Normal]->setGeometryOutputType(m_geometryOutputType);
    m_shaders[Z3DRendererBase::ShaderHookType::Normal]->setGeometryOutputVertexCount(m_geometryOutputVertexCount);
  }
  buildNormalShader(m_shaders[Z3DRendererBase::ShaderHookType::Normal].get());
}

void Z3DShaderGroup::addAllSupportedPostShaders()
{
  addDualDepthPeelingShaders();
  addWeightedAverageShaders();
}

void Z3DShaderGroup::addDualDepthPeelingShaders()
{
  if (Z3DGpuInfo::instance().isDualDepthPeelingSupported()) {
    m_shaders[Z3DRendererBase::ShaderHookType::DualDepthPeelingInit].reset(new Z3DShaderProgram());
    if (!GLVersionGE(3, 2)) {
      m_shaders[Z3DRendererBase::ShaderHookType::DualDepthPeelingInit]->setGeometryInputType(m_geometryInputType);
      m_shaders[Z3DRendererBase::ShaderHookType::DualDepthPeelingInit]->setGeometryOutputType(m_geometryOutputType);
      m_shaders[Z3DRendererBase::ShaderHookType::DualDepthPeelingInit]->setGeometryOutputVertexCount(
        m_geometryOutputVertexCount);
    }
    m_shaders[Z3DRendererBase::ShaderHookType::DualDepthPeelingPeel].reset(new Z3DShaderProgram());
    if (!GLVersionGE(3, 2)) {
      m_shaders[Z3DRendererBase::ShaderHookType::DualDepthPeelingPeel]->setGeometryInputType(m_geometryInputType);
      m_shaders[Z3DRendererBase::ShaderHookType::DualDepthPeelingPeel]->setGeometryOutputType(m_geometryOutputType);
      m_shaders[Z3DRendererBase::ShaderHookType::DualDepthPeelingPeel]->setGeometryOutputVertexCount(
        m_geometryOutputVertexCount);
    }
    buildDualDepthPeelingInitShader(m_shaders[Z3DRendererBase::ShaderHookType::DualDepthPeelingInit].get());
    buildDualDepthPeelingPeelShader(m_shaders[Z3DRendererBase::ShaderHookType::DualDepthPeelingPeel].get());
  }
}

void Z3DShaderGroup::addWeightedAverageShaders()
{
  if (Z3DGpuInfo::instance().isWeightedAverageSupported()) {
    m_shaders[Z3DRendererBase::ShaderHookType::WeightedAverageInit].reset(new Z3DShaderProgram());
    if (!GLVersionGE(3, 2)) {
      m_shaders[Z3DRendererBase::ShaderHookType::WeightedAverageInit]->setGeometryInputType(m_geometryInputType);
      m_shaders[Z3DRendererBase::ShaderHookType::WeightedAverageInit]->setGeometryOutputType(m_geometryOutputType);
      m_shaders[Z3DRendererBase::ShaderHookType::WeightedAverageInit]->setGeometryOutputVertexCount(
        m_geometryOutputVertexCount);
    }
    buildWeightedAverageShader(m_shaders[Z3DRendererBase::ShaderHookType::WeightedAverageInit].get());
  }
}

void Z3DShaderGroup::addWeightedBlendedShaders()
{
    m_shaders[Z3DRendererBase::WeightedBlendedInit] = new Z3DShaderProgram();
    buildWeightedBlendedShader(m_shaders[Z3DRendererBase::WeightedBlendedInit]);
}

void Z3DShaderGroup::bind()
{
  get().bind();
  if (m_base.shaderHookType() == Z3DRendererBase::ShaderHookType::DualDepthPeelingPeel) {
    get().bindTexture("DepthBlenderTex", m_base.shaderHookPara().dualDepthPeelingDepthBlenderTexture);
    get().bindTexture("FrontBlenderTex", m_base.shaderHookPara().dualDepthPeelingFrontBlenderTexture);
  }
}

void Z3DShaderGroup::release()
{
  // if bind is ok, this should be fine
  get().release();
}

//Z3DShaderProgram& Z3DShaderGroup::get(Z3DRendererBase::ShaderHookType sht)
//{
//  return *m_shaders[sht];
//}

void Z3DShaderGroup::rebuild(const QString& header, const QString& geomHeader)
{
  m_header = header;
  m_geomHeader = geomHeader;
  auto i = m_shaders.begin();
  while (i != m_shaders.end()) {
    i->second->removeAllShaders();
    switch (i->first) {
      case Z3DRendererBase::ShaderHookType::Normal:
        buildNormalShader(i->second.get());
        break;
      case Z3DRendererBase::ShaderHookType::DualDepthPeelingInit:
        buildDualDepthPeelingInitShader(i->second.get());
        break;
      case Z3DRendererBase::ShaderHookType::DualDepthPeelingPeel:
        buildDualDepthPeelingPeelShader(i->second.get());
        break;
      case Z3DRendererBase::ShaderHookType::WeightedAverageInit:
        buildWeightedAverageShader(i->second.get());
        break;
      case Z3DRendererBase::ShaderHookType::WeightedBlendedInit;
        buildWeightedBlendedShader(i->second.get());
      default:
        break;
    }
    ++i;
  }
}

void Z3DShaderGroup::buildNormalShader(Z3DShaderProgram* shader)
{
  if (m_normalShaderFiles.empty()) {
    QStringList allshaders(m_shaderFiles);
    allshaders << "common.frag";
    shader->bindFragDataLocation(0, "FragData0");
    shader->loadFromSourceFile(allshaders, m_header, m_geomHeader);
  } else {
    shader->bindFragDataLocation(0, "FragData0");
    shader->loadFromSourceFile(m_normalShaderFiles, m_header, m_geomHeader);
  }
}

void Z3DShaderGroup::buildDualDepthPeelingInitShader(Z3DShaderProgram* shader)
{
  QStringList allshaders(m_shaderFiles);
  allshaders << "dual_peeling_init.frag";
  shader->bindFragDataLocation(0, "FragData0");
  shader->bindFragDataLocation(1, "FragData1");
  shader->loadFromSourceFile(allshaders, m_header, m_geomHeader);
}

//#define USE_RECT_TEX

void Z3DShaderGroup::buildDualDepthPeelingPeelShader(Z3DShaderProgram* shader)
{
  QStringList allshaders(m_shaderFiles);
  allshaders << "dual_peeling_peel.frag";
#ifdef USE_RECT_TEX
  QString header = m_header;
  header += "#define USE_RECT_TEX\n";
  shader->bindFragDataLocation(0, "FragData0");
  shader->bindFragDataLocation(1, "FragData1");
  shader->bindFragDataLocation(2, "FragData2");
  shader->loadFromSourceFile(allshaders, header, m_geomHeader);
#else
  shader->bindFragDataLocation(0, "FragData0");
  shader->bindFragDataLocation(1, "FragData1");
  shader->bindFragDataLocation(2, "FragData2");
  shader->loadFromSourceFile(allshaders, m_header, m_geomHeader);
#endif
}

void Z3DShaderGroup::buildWeightedAverageShader(Z3DShaderProgram* shader)
{
  QStringList allshaders(m_shaderFiles);
  allshaders << "wavg_init.frag";
  shader->bindFragDataLocation(0, "FragData0");
  shader->bindFragDataLocation(1, "FragData1");
  shader->loadFromSourceFile(allshaders, m_header, m_geomHeader);
}

void Z3DShaderGroup::buildWeightedBlendedShader(Z3DShaderProgram *shader)
{
  QStringList allshaders(m_shaderFiles);
  allshaders << "cube_wboit_compose.vert" << "cube_wboit_compose.frag";

  //ZOUT(LTRACE(), 5) <<"buildWeightedBlendedShader header ... "<<m_header;

  shader->bindFragDataLocation(0, "FragData0");
  shader->bindFragDataLocation(1, "FragData1");
  shader->loadFromSourceFile(allshaders, m_header);
}
