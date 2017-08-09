#ifndef Z3DVOLUMERAYCASTERRENDERER_H
#define Z3DVOLUMERAYCASTERRENDERER_H

#include "z3dprimitiverenderer.h"
#include "z3dtransferfunction.h"
#include "zmesh.h"
#include "z3dshaderprogram.h"
#include "z3drendertarget.h"

// use raycaster to render volume or 2D Image (stack with depth==1) with color
// transfer functions
class Z3DVolumeRaycasterRenderer : public Z3DPrimitiveRenderer
{
  Q_OBJECT
public:
  explicit Z3DVolumeRaycasterRenderer(Z3DRendererBase& rendererBase);

  void setChannels(const std::vector<std::unique_ptr<Z3DVolume>>& vols);

  void setLayerTarget(Z3DRenderTarget* layerTarget)
  { m_layerTarget = layerTarget; }

  // quad or entryexit texture should be set before rendering

  // For 2D Image rendering, once set, entry exit textures will be cleared and
  // renderer switch to 2D mode
  // To render a 2D image, quad should contains corner vertex and 2d texture coordinates
  // To render a slice in 3D volume, quad should contains corner vertex and 3d texture coordinates
  // DO NOT call this function for 3d Raycaster
  // clear
  void clearQuads()
  { m_quads.clear(); }

  // add quad
  void addQuad(const ZMesh& quad);

  // For 3D Raycasting rendering, once called, 2d quads will be cleared and renderer
  // switch to 3D mode
  void setEntryExitInfo(const Z3DTexture* entryTexCoordTexture,
                        const Z3DTexture* entryEyeCoordTexture,
                        const Z3DTexture* exitTexCoordTexture,
                        const Z3DTexture* exitEyeCoordTexture);

  void translate(double dx, double dy, double dz);

  // return true if something is rendered by this renderer
  bool hasVisibleRendering() const;

  QString compositeMode() const
  { return m_compositingMode.get(); }

  ZStringIntOptionParameter& compositingModePara()
  { return m_compositingMode; }

  ZFloatParameter& samplingRatePara()
  { return m_samplingRate; }

  ZFloatParameter& isoValuePara()
  { return m_isoValue; }

  ZFloatParameter& localMIPThresholdPara()
  { return m_localMIPThreshold; }

  const std::vector<std::unique_ptr<ZBoolParameter>>& channelVisibleParas() const
  { return m_channelVisibleParas; }

  const std::vector<std::unique_ptr<Z3DTransferFunctionParameter>>& transferFuncParas() const
  { return m_transferFuncParas; }

  const std::vector<std::unique_ptr<ZStringIntOptionParameter>>& texFilterModeParas() const
  { return m_texFilterModeParas; }

  //
  void setChannel1Visible(bool v) { if (m_channelVisibleParas.size() > 0) m_channelVisibleParas[0]->set(v); }
  void setChannel2Visible(bool v) { if (m_channelVisibleParas.size() > 1) m_channelVisibleParas[1]->set(v); }
  void setChannel3Visible(bool v) { if (m_channelVisibleParas.size() > 2) m_channelVisibleParas[2]->set(v); }
  void setChannel4Visible(bool v) { if (m_channelVisibleParas.size() > 3) m_channelVisibleParas[3]->set(v); }
  void setChannel5Visible(bool v) { if (m_channelVisibleParas.size() > 4) m_channelVisibleParas[4]->set(v); }

  inline void setOpaque(bool opaque) {
    m_opaque = opaque;
  }

  inline void setAlpha(double alpha) {
    m_alpha = alpha;
    resetTransferFunctions();
  }

  inline double getAlpha() {
    return m_alpha;
  }

  void setCompositeMode(const QString &option) { m_compositingMode.set(option); }
  void setTextureFilterMode(const QString &option)
  {
    for (const auto& p : m_texFilterModeParas) {
      p->set(option);
    }
  }

protected:
  void adjustWidgets();

  void bindVolumesAndTransferFuncs(Z3DShaderProgram& shader);

  void bindVolumeAndTransferFunc(Z3DShaderProgram& shader, size_t idx);

  virtual void compile() override;

  QString generateHeader();

  virtual void render(Z3DEye eye) override;

  virtual void renderPicking(Z3DEye) override;

  //  Z3DShaderProgram m_raycasterShader;
  //  Z3DShaderProgram m_2dImageShader;
  //  Z3DShaderProgram m_volumeSliceWithTransferfunShader;

  // single channel version
  Z3DShaderProgram m_scRaycasterShader;
  Z3DShaderProgram m_sc2dImageShader;
  Z3DShaderProgram m_scVolumeSliceWithTransferfunShader;
  Z3DRenderTarget* m_layerTarget = nullptr;
  Z3DShaderProgram m_mergeChannelShader;

  ZFloatParameter m_samplingRate;  // Sampling rate of the raycasting, specified relative to the size of one voxel
  ZFloatParameter m_isoValue;  // The used isovalue, when isosurface raycasting is enabled
  ZFloatParameter m_localMIPThreshold;

  ZStringIntOptionParameter m_compositingMode;

  std::vector<Z3DVolume*> m_volumes;
  std::vector<QString> m_volumeUniformNames;
  std::vector<QString> m_volumeDimensionNames;
  std::vector<QString> m_transferFuncUniformNames;
  std::vector<std::unique_ptr<ZBoolParameter>> m_channelVisibleParas;
  std::vector<std::unique_ptr<Z3DTransferFunctionParameter>> m_transferFuncParas;
  std::vector<std::unique_ptr<ZStringIntOptionParameter>> m_texFilterModeParas;

private:
  // this function is used to get proper default
  // transfer functions (grey or color depends on current number of channel)
  void resetTransferFunctions();

  bool m_is2DImage;

  std::vector<ZMesh> m_quads;
  const Z3DTexture* m_entryTexCoordTexture;
  const Z3DTexture* m_entryEyeCoordTexture;
  const Z3DTexture* m_exitTexCoordTexture;
  const Z3DTexture* m_exitEyeCoordTexture;

  bool m_opaque;
  double m_alpha; //only takes effect when m_opaque is true
  ZVertexArrayObject m_VAO;
};

#endif // Z3DVOLUMERAYCASTERRENDERER_H
