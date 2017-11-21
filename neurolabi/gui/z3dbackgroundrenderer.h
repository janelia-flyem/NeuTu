#ifndef Z3DBACKGROUNDRENDERER_H
#define Z3DBACKGROUNDRENDERER_H

#include "z3dprimitiverenderer.h"

class Z3DBackgroundRenderer : public Z3DPrimitiveRenderer
{
Q_OBJECT
public:
  explicit Z3DBackgroundRenderer(Z3DRendererBase& rendererBase);

  ZStringIntOptionParameter& modePara()
  { return m_mode; }

  ZVec4Parameter& firstColorPara()
  { return m_firstColor; }

  ZVec4Parameter& secondColorPara()
  { return m_secondColor; }

  ZStringIntOptionParameter& gradientOrientationPara()
  { return m_gradientOrientation; }

  void setRenderingRegion(double left = 0., double right = 1., double bottom = 0., double top = 1.);

protected:
  void adjustWidgets();

  virtual void compile() override;

  QString generateHeader();

#if !defined(_USE_CORE_PROFILE_) && defined(_SUPPORT_FIXED_PIPELINE_)
  virtual void renderUsingOpengl() override;
  virtual void renderPickingUsingOpengl() override;
#endif

  virtual void render(Z3DEye eye) override;

  virtual void renderPicking(Z3DEye /*unused*/) override;

protected:
  Z3DShaderGroup m_backgroundShaderGrp;

  ZVec4Parameter m_firstColor;
  ZVec4Parameter m_secondColor;
  ZStringIntOptionParameter m_gradientOrientation;
  ZStringIntOptionParameter m_mode;

  ZVertexArrayObject m_VAO;
  ZVertexBufferObject m_VBO;

  glm::vec4 m_region;
};

#endif // Z3DBACKGROUNDRENDERER_H
