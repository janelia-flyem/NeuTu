#include "z3dtexturecopyrenderer.h"

#include "z3dshaderprogram.h"
#include "z3dtexture.h"

Z3DTextureCopyRenderer::Z3DTextureCopyRenderer(Z3DRendererBase& rendererBase, OutputColorOption mode)
  : Z3DPrimitiveRenderer(rendererBase)
  , m_copyTextureShaderGrp(rendererBase)
  , m_discardTransparent(false)
  , m_mode(mode)
  , m_VAO(1)
{
  QStringList allshaders;
  allshaders << "pass.vert" << "copyimage_func.frag";
  QStringList normalShaders;
  normalShaders << "pass.vert" << "copyimage.frag";
  m_copyTextureShaderGrp.init(allshaders, m_rendererBase.generateHeader() + generateHeader(), "", normalShaders);
  m_copyTextureShaderGrp.addAllSupportedPostShaders();
  CHECK_GL_ERROR
}

void Z3DTextureCopyRenderer::compile()
{
  m_copyTextureShaderGrp.rebuild(m_rendererBase.generateHeader() + generateHeader());
}

QString Z3DTextureCopyRenderer::generateHeader() const
{
  QString headerSource;
  if (m_mode == OutputColorOption::MultiplyAlpha) {
    headerSource += "#define Multiply_Alpha\n";
  } else if (m_mode == OutputColorOption::DivideByAlpha) {
    headerSource += "#define Divide_By_Alpha\n";
  }
  return headerSource;
}

void Z3DTextureCopyRenderer::render(Z3DEye eye)
{
  if (!m_colorTexture || !m_depthTexture)
    return;

  m_copyTextureShaderGrp.bind();
  Z3DShaderProgram& shader = m_copyTextureShaderGrp.get();
  m_rendererBase.setGlobalShaderParameters(shader, eye);
  shader.setUniform("discard_transparent", m_discardTransparent);

  // pass texture parameters to the shader
  shader.bindTexture("color_texture", m_colorTexture);
  shader.bindTexture("depth_texture", m_depthTexture);

  renderScreenQuad(m_VAO, shader);
  m_copyTextureShaderGrp.release();
}
