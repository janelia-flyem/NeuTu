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
  addWeightedBlendedShaders();
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
  if (Z3DGpuInfo::instance().isWeightedBlendedSupported()) {
    m_shaders[Z3DRendererBase::ShaderHookType::WeightedBlendedInit].reset(new Z3DShaderProgram());
    if (!GLVersionGE(3, 2)) {
      m_shaders[Z3DRendererBase::ShaderHookType::WeightedBlendedInit]->setGeometryInputType(m_geometryInputType);
      m_shaders[Z3DRendererBase::ShaderHookType::WeightedBlendedInit]->setGeometryOutputType(m_geometryOutputType);
      m_shaders[Z3DRendererBase::ShaderHookType::WeightedBlendedInit]->setGeometryOutputVertexCount(
        m_geometryOutputVertexCount);
    }
    buildWeightedBlendedShader(m_shaders[Z3DRendererBase::ShaderHookType::WeightedBlendedInit].get());
  }
}

void Z3DShaderGroup::bind()
{
  get().bind();
  if (m_base.shaderHookType() == Z3DRendererBase::ShaderHookType::DualDepthPeelingPeel) {
    get().bindTexture("DepthBlenderTex", m_base.shaderHookPara().dualDepthPeelingDepthBlenderTexture);
    get().bindTexture("FrontBlenderTex", m_base.shaderHookPara().dualDepthPeelingFrontBlenderTexture);
  } else if (m_base.shaderHookType() == Z3DRendererBase::ShaderHookType::WeightedBlendedInit) {
    float n = m_base.camera().nearDist();
    float f = m_base.camera().farDist();
    //http://www.opengl.org/archives/resources/faq/technical/depthbuffer.htm
    // zw = a/ze + b;  ze = a/(zw - b);  a = f*n/(f-n);  b = 0.5*(f+n)/(f-n) + 0.5;
    float a = f * n / (f - n);
    float b = 0.5f * (f + n) / (f - n) + 0.5f;
    get().setUniform("ze_to_zw_b", b);
    get().setUniform("ze_to_zw_a", a);
    get().setUniform("weighted_blended_depth_scale", m_base.globalParas().weightedBlendedDepthScale.get());
  }
}

void Z3DShaderGroup::release()
{
  // if bind is ok, this should be fine
  get().release();
}

Z3DShaderProgram& Z3DShaderGroup::get()
{
  return *m_shaders[m_base.shaderHookType()];
}

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
      case Z3DRendererBase::ShaderHookType::WeightedBlendedInit:
        buildWeightedBlendedShader(i->second.get());
		break;
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
  allshaders << "wblended_init.frag";
  shader->bindFragDataLocation(0, "FragData0");
  shader->bindFragDataLocation(1, "FragData1");
  shader->loadFromSourceFile(allshaders, m_header, m_geomHeader);
}
