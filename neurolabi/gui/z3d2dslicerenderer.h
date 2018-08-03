#ifndef Z3D2DSLICERENDERER_H
#define Z3D2DSLICERENDERER_H

#include "z3dprimitiverenderer.h"
#include "zmesh.h"
#include "zcolormap.h"
#include "z3dshaderprogram.h"

class Z3DVolume;

// render 2d slices of volume with a rotation angle
// one colormap for one slice
class Z3D2DSliceRenderer : public Z3DPrimitiveRenderer
{
  Q_OBJECT
public:
  explicit Z3D2DSliceRenderer(Z3DRendererBase& rendererBase);

  void setData(const std::vector<std::unique_ptr<Z3DVolume>>& vols,
               const std::vector<ZMesh>& quads,
               ZColorMapParameter& colormap);

protected:
  void bindVolumes(Z3DShaderProgram& shader) const;

  void bindVolume(Z3DShaderProgram& shader, size_t idx) const;

  virtual void compile() override;

  QString generateHeader();

  virtual void render(Z3DEye eye) override;

protected:
  Z3DShaderProgram m_sc2dImageShader;

  const std::vector<std::unique_ptr<Z3DVolume>>* m_vols = nullptr;
  ZColorMapParameter* m_colormap = nullptr;
  const std::vector<ZMesh>* m_quads = nullptr;
  std::vector<QString> m_volumeUniformNames;
  std::vector<QString> m_colormapUniformNames;

private:
  ZVertexArrayObject m_VAO;
};

#endif // Z3D2DSLICERENDERER_H
