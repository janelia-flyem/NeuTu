#ifndef Z3DIMAGE2DRENDERER_H
#define Z3DIMAGE2DRENDERER_H

#include "z3dprimitiverenderer.h"
#include "z3dshaderprogram.h"
#include "zcolormap.h"
#include "zmesh.h"

class Z3DVolume;

// render 2d image with colormap
// use colormap of each volume to composite final image
class Z3DImage2DRenderer : public Z3DPrimitiveRenderer
{
Q_OBJECT
public:
  explicit Z3DImage2DRenderer(Z3DRendererBase& rendererBase);

  // input vols can not be nullptr
  void setChannels(const std::vector<std::unique_ptr<Z3DVolume>>& vols,
                   const std::vector<std::unique_ptr<ZColorMapParameter>>& colormaps);

  void setLayerTarget(Z3DRenderTarget* layerTarget)
  { m_layerTarget = layerTarget; }

  // quad contains corner vertex and 2d texture coordinates
  // clear
  void clearQuads()
  { m_quads.clear(); }

  // add quad
  void addQuad(const ZMesh& quad);

protected:
  void bindVolumes(Z3DShaderProgram& shader) const;

  void bindVolume(Z3DShaderProgram& shader, size_t idx) const;

  bool hasVolume() const;

  virtual void compile() override;

  QString generateHeader();

  virtual void render(Z3DEye eye) override;

protected:
  //Z3DShaderProgram m_image2DShader;
  Z3DShaderProgram m_scImage2DShader;
  Z3DRenderTarget* m_layerTarget = nullptr;
  Z3DShaderProgram m_mergeChannelShader;

  std::vector<Z3DVolume*> m_volumes;
  std::vector<ZColorMapParameter*> m_colormaps;
  std::vector<QString> m_volumeUniformNames;
  std::vector<QString> m_colormapUniformNames;

private:
  std::vector<ZMesh> m_quads;
  ZVertexArrayObject m_VAO;
};

#endif // Z3DIMAGE2DRENDERER_H
