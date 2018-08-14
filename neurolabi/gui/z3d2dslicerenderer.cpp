#include "z3d2dslicerenderer.h"

#include "z3dtexture.h"
#include "z3dvolume.h"

Z3D2DSliceRenderer::Z3D2DSliceRenderer(Z3DRendererBase& rendererBase)
  : Z3DPrimitiveRenderer(rendererBase)
  , m_VAO(1)
{
  m_sc2dImageShader.bindFragDataLocation(0, "FragData0");
  m_sc2dImageShader.loadFromSourceFile("transform_with_2dtexture.vert", "image2d_with_colormap_single_channel.frag",
                                       m_rendererBase.generateHeader() + generateHeader());
}

void Z3D2DSliceRenderer::setData(const std::vector<std::unique_ptr<Z3DVolume> >& vols,
                                 const std::vector<ZMesh>& quads,
                                 ZColorMapParameter& colormap)
{
  CHECK(quads.size() >= vols.size() && !vols.empty() && vols[0]->is2DData());

  for (const auto& quad : quads) {
    if (quad.empty() ||
        (quad.numVertices() != 4 && quad.numVertices() != 6) ||
        quad.numVertices() != quad.num2DTextureCoordinates()) {
      LOG(FATAL) << "Input quad should be 2D slice with 2D texture coordinates";
      return;
    }
  }

  m_vols = &vols;
  m_colormap = &colormap;
  m_quads = &quads;

  if (m_vols->size() != m_volumeUniformNames.size()) {
    compile();
    m_volumeUniformNames.resize(m_vols->size());
    m_colormapUniformNames.resize(m_vols->size());
    for (size_t i = 0; i < m_vols->size(); ++i) {
      m_volumeUniformNames[i] = QString("volume_%1").arg(i + 1);
      m_colormapUniformNames[i] = QString("colormap_%1").arg(i + 1);
    }
  }
}

void Z3D2DSliceRenderer::bindVolumes(Z3DShaderProgram& shader) const
{
  size_t idx = 0;
  for (size_t i = 0; i < m_vols->size(); ++i) {
    // volumes
    shader.bindTexture(m_volumeUniformNames[idx], m_vols->at(i)->texture(),
                       GLint(GL_NEAREST), GLint(GL_NEAREST));

    // colormap
    shader.bindTexture(m_colormapUniformNames[idx++], m_colormap->get().texture1D());
  }
}

void Z3D2DSliceRenderer::bindVolume(Z3DShaderProgram& shader, size_t idx) const
{
  // volumes
  shader.bindTexture(m_volumeUniformNames[0], m_vols->at(idx)->texture(),
                     GLint(GL_NEAREST), GLint(GL_NEAREST));

  // colormap
  shader.bindTexture(m_colormapUniformNames[0], m_colormap->get().texture1D());
}

void Z3D2DSliceRenderer::compile()
{
  m_sc2dImageShader.setHeaderAndRebuild(m_rendererBase.generateHeader() + generateHeader());
}

QString Z3D2DSliceRenderer::generateHeader()
{
  QString headerSource;

  headerSource += QString("#define NUM_VOLUMES 0\n");
  headerSource += "#define DISABLE_TEXTURE_COORD_OUTPUT\n";

  return headerSource;
}

void Z3D2DSliceRenderer::render(Z3DEye eye)
{
  bool needRender = m_vols && m_vols->size() > 0 && !m_quads->empty();
  if (!needRender)
    return;

  m_sc2dImageShader.bind();
  m_rendererBase.setGlobalShaderParameters(m_sc2dImageShader, eye);

  for (size_t i = 0; i < m_vols->size(); ++i) {
    bindVolume(m_sc2dImageShader, i);
    renderTriangleList(m_VAO, m_sc2dImageShader, m_quads->at(i));
  }

  m_sc2dImageShader.release();
}
