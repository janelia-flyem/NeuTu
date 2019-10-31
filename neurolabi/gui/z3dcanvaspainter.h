#ifndef Z3DCANVASPAINTER_H
#define Z3DCANVASPAINTER_H

#include <QString>

#include "z3dboundedfilter.h"
#include "z3dtexturecopyrenderer.h"
#include "z3drenderport.h"
#include "widgets/znumericparameter.h"
#include "qt/core/zexception.h"


class Z3DTexture;

class Z3DCanvas;

class Z3DCompositor;

class Z3DCanvasPainter : public Z3DBoundedFilter
{
Q_OBJECT
public:
  explicit Z3DCanvasPainter(Z3DGlobalParameters& globalParas, Z3DCanvas& canvas, QObject* parent = nullptr);

  virtual void invalidate(State inv = State::AllResultInvalid) override;

  Z3DCanvas& canvas()
  { return m_canvas; }

  const Z3DTexture* imageColorTexture(Z3DEye eye) const;

  const Z3DTexture* imageDepthTexture(Z3DEye eye) const;

  bool renderToImage(const QString& filename, Z3DScreenShotType sst);

  bool renderToImage(const QString& filename, int width, int height, Z3DScreenShotType sst, Z3DCompositor& compositor);

  QString renderToImageError() const;

  void setLock(bool v)
  { m_locked = v; }

protected:
  void onCanvasResized(size_t w, size_t h);

  virtual void process(Z3DEye eye) override;

  virtual bool isReady(Z3DEye /*eye*/) const override;

  virtual bool isValid(Z3DEye eye) const override;

  virtual void updateSize() override;

  void renderInportToImage(Z3DEye eye);

  void pasteQImage(QImage& dst, const QImage& src, int x, int y) const;

  QImage composeStereoViewQImage(const QImage& left, const QImage& right, bool half = false);

private:
  void setOutputSize(const glm::uvec2& size);

private:
  Z3DTextureCopyRenderer m_textureCopyRenderer;

  Z3DCanvas& m_canvas;
  Z3DRenderInputPort m_inport;
  Z3DRenderInputPort m_leftEyeInport;
  Z3DRenderInputPort m_rightEyeInport;

  bool m_renderToImage;
  QImage m_monoImg;
  QImage m_leftImg;
  QImage m_rightImg;
  bool m_tiledRendering;
  int m_tileStartX;
  int m_tileStartY;
  QString m_renderToImageError;
  Z3DScreenShotType m_renderToImageType;

  bool m_locked = false;
};

#endif // Z3DCANVASPAINTER_H
