#include "z3dtextureblendrenderer.h"

#include "z3dtexture.h"

Z3DTextureBlendRenderer::Z3DTextureBlendRenderer(Z3DRendererBase& rendererBase, const QString& mode)
  : Z3DPrimitiveRenderer(rendererBase)
  , m_blendMode("Blend Mode")
  , m_VAO(1)
{
  m_blendMode.addOptionsWithData(qMakePair<QString, QString>("DepthTest", "DEPTH_TEST"),
                                 qMakePair<QString, QString>("FirstOnTop", "FIRST_ON_TOP"),
                                 qMakePair<QString, QString>("SecondOnTop", "SECOND_ON_TOP"),
                                 qMakePair<QString, QString>("DepthTestBlending", "DEPTH_TEST_BLENDING"),
                                 qMakePair<QString, QString>("FirstOnTopBlending", "FIRST_ON_TOP_BLENDING"),
                                 qMakePair<QString, QString>("SecondOnTopBlending", "SECOND_ON_TOP_BLENDING")
  );
  m_blendMode.select(mode);
  connect(&m_blendMode, &ZStringStringOptionParameter::valueChanged, this, &Z3DTextureBlendRenderer::compile);

  m_blendTextureShader.bindFragDataLocation(0, "FragData0");
  m_blendTextureShader.loadFromSourceFile("pass.vert", "compositor.frag",
                                          m_rendererBase.generateHeader() + generateHeader());
  CHECK_GL_ERROR
}

void Z3DTextureBlendRenderer::compile()
{
  m_blendTextureShader.setHeaderAndRebuild(m_rendererBase.generateHeader() + generateHeader());
}

QString Z3DTextureBlendRenderer::generateHeader()
{
  return QString("#define %1\n").arg(m_blendMode.associatedData());
}

void Z3DTextureBlendRenderer::render(Z3DEye eye)
{
  if (!m_colorTexture1 || !m_depthTexture1 ||
      !m_colorTexture2 || !m_depthTexture2)
    return;

  m_blendTextureShader.bind();
  m_rendererBase.setGlobalShaderParameters(m_blendTextureShader, eye);

  m_blendTextureShader.bindTexture("color_texture_0", m_colorTexture1);
  m_blendTextureShader.bindTexture("depth_texture_0", m_depthTexture1);

  m_blendTextureShader.bindTexture("color_texture_1", m_colorTexture2);
  m_blendTextureShader.bindTexture("depth_texture_1", m_depthTexture2);

  glDepthFunc(GL_ALWAYS);
  renderScreenQuad(m_VAO, m_blendTextureShader);
  glDepthFunc(GL_LESS);
  m_blendTextureShader.release();
}
