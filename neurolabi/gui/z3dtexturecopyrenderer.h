#ifndef Z3DTEXTURECOPYRENDERER_H
#define Z3DTEXTURECOPYRENDERER_H

#include "z3dprimitiverenderer.h"

class Z3DTextureCopyRenderer : public Z3DPrimitiveRenderer
{
Q_OBJECT
public:
  // Multiply_Alpha : output color will be multiplied by alpha value (convert to premultiplied format)
  // None (default): just copy
  // Divide_By_Alpha : output color will be divided by alpha value (input is premultiplied format)
  enum class OutputColorOption
  {
    NoChange, DivideByAlpha, MultiplyAlpha
  };

  explicit Z3DTextureCopyRenderer(Z3DRendererBase& rendererBase, OutputColorOption mode = OutputColorOption::NoChange);

  void setColorTexture(const Z3DTexture* colorTex)
  { m_colorTexture = colorTex; }

  void setDepthTexture(const Z3DTexture* depthTex)
  { m_depthTexture = depthTex; }

  // if true, color with zero alpha value should be discarded, which might save many depth texture lookup. default is false
  // Make sure your color and depth buffer are cleared before if set to true
  // glClear + discard transparent  is usually faster than   not discard transparent if many pixels are empty
  void setDiscardTransparent(bool v)
  { m_discardTransparent = v; }

protected:
  virtual void compile() override;

  QString generateHeader() const;

  virtual void render(Z3DEye eye) override;

protected:
  const Z3DTexture* m_colorTexture;
  const Z3DTexture* m_depthTexture;

  Z3DShaderGroup m_copyTextureShaderGrp;
  bool m_discardTransparent;

  OutputColorOption m_mode;
  ZVertexArrayObject m_VAO;
};


#endif // Z3DTEXTURECOPYRENDERER_H
