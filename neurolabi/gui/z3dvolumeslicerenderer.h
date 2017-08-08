#ifndef Z3DVOLUMESLICERENDERER_H
#define Z3DVOLUMESLICERENDERER_H

#include "z3dprimitiverenderer.h"
#include "zmesh.h"
#include "zcolormap.h"
#include "z3dshaderprogram.h"

class Z3DVolume;

// render 2d slices of volume with colormap
// use colormap of each volume to composite final image
class Z3DVolumeSliceRenderer : public Z3DPrimitiveRenderer
{
  Q_OBJECT
public:
  explicit Z3DVolumeSliceRenderer(Z3DRendererBase& rendererBase);

  void setData(const std::vector<std::unique_ptr<Z3DVolume>>& vols,
               const std::vector<std::unique_ptr<ZColorMapParameter>>& colormaps);

  void setLayerTarget(Z3DRenderTarget* layerTarget)
  { m_layerTarget = layerTarget; }

  // a slice (quad) in 3D volume contains corner vertex and 3d texture coordinates
  // clear
  void clearQuads()
  { m_quads.clear(); }

  // add quad
  void addQuad(const ZMesh& quad);

protected:
  void bindVolumes(Z3DShaderProgram& shader) const;

  void bindVolume(Z3DShaderProgram& shader, size_t idx) const;

  virtual void compile() override;

  QString generateHeader();

  virtual void render(Z3DEye eye) override;

protected:
  //Z3DShaderProgram m_volumeSliceShader;
  Z3DShaderProgram m_scVolumeSliceShader;
  Z3DRenderTarget* m_layerTarget = nullptr;
  Z3DShaderProgram m_mergeChannelShader;

  const std::vector<std::unique_ptr<Z3DVolume>>* m_vols;
  const std::vector<std::unique_ptr<ZColorMapParameter>>* m_colormaps = nullptr;
  std::vector<QString> m_volumeUniformNames;
  std::vector<QString> m_colormapUniformNames;

private:
  std::vector<ZMesh> m_quads;
  ZVertexArrayObject m_VAO;
};

#endif // Z3DVOLUMESLICERENDERER_H
