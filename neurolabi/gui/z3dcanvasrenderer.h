#ifndef Z3DCANVASRENDERER_H
#define Z3DCANVASRENDERER_H

#include "z3drenderprocessor.h"
#include "znumericparameter.h"

#include <QString>

class Z3DTexture;
class Z3DCanvas;
class Z3DTextureCopyRenderer;

class Z3DCanvasRenderer : public Z3DRenderProcessor
{
  Q_OBJECT
public:
  Z3DCanvasRenderer();
  ~Z3DCanvasRenderer();

  virtual void invalidate(InvalidationState inv = InvalidAllResult);

  void setCanvas(Z3DCanvas* canvas);

  Z3DCanvas *getCanvas() const;

  const Z3DTexture* getImageColorTexture(Z3DEye eye) const;
  const Z3DTexture* getImageDepthTexture(Z3DEye eye) const;

  bool renderToImage(const QString &filename, Z3DScreenShotType sst);
  bool renderToImage(const QString &filename, int width, int height, Z3DScreenShotType sst);

  QString getRenderToImageError() const;

protected slots:
  void onCanvasResized(int w, int h);

protected:
  virtual void process(Z3DEye eye);
  virtual void initialize();
  virtual void deinitialize();

  virtual bool isReady(Z3DEye) const;
  virtual bool isValid(Z3DEye eye) const;

  void renderInportToImage(const QString& filename, Z3DEye eye);

  Z3DTextureCopyRenderer *m_textureCopyRenderer;

  Z3DCanvas* m_canvas;
  Z3DRenderInputPort m_inport;
  Z3DRenderInputPort m_leftEyeInport;
  Z3DRenderInputPort m_rightEyeInport;
  bool m_renderToImage;
  QString m_renderToImageFilename;
  QString m_renderToImageError;
  Z3DScreenShotType m_renderToImageType;
};

#endif // Z3DCANVASRENDERER_H
