#ifndef Z3D2DSLICEFILTER_H
#define Z3D2DSLICEFILTER_H


#include "z3dboundedfilter.h"

#include "z3dcameraparameter.h"
#include "widgets/znumericparameter.h"
#include "z3dvolume.h"
#include "z3dtransformparameter.h"
#include "zmesh.h"
#include "zwidgetsgroup.h"
#include <vector>
#include "z3d2dslicerenderer.h"
#include "z3dtexturecopyrenderer.h"
#include "z3drenderport.h"

class ZMesh;
class ZStackDoc;

class Z3D2DSliceFilter : public Z3DBoundedFilter
{
Q_OBJECT

  friend class Z3DCompositor;

public:
  explicit Z3D2DSliceFilter(Z3DGlobalParameters& globalParas, QObject* parent = nullptr);

  void addData(uint8_t *buffer, int width, int height,
               ZIntPoint centerloc, ZPoint dim1vec, ZPoint dim2vec,
               bool removeOtherData = false);

  virtual bool isStayOnTop() const
  { return m_stayOnTop.get(); }

  virtual void setStayOnTop(bool s)
  { m_stayOnTop.set(s); }

  bool isSliceDownsampled() const;

  std::shared_ptr<ZWidgetsGroup> widgetsGroup();

  bool isReady(Z3DEye eye) const override;

  virtual bool hasOpaque(Z3DEye eye) const override;

  virtual void renderOpaque(Z3DEye eye) override;

  virtual bool hasTransparent(Z3DEye eye) const override;

  virtual void renderTransparent(Z3DEye eye) override;

protected:

  void changeCoordTransform();

  virtual void setClipPlanes() override
  {}

  virtual void process(Z3DEye eye) override;

  virtual void updateNotTransformedBoundBoxImpl() override;

  virtual void expandCutRange() override
  {}

private:
  Z3D2DSliceRenderer m_2dSliceRenderer;
  Z3DTextureCopyRenderer m_textureCopyRenderer;

  ZStack* m_imgPack = nullptr;
  std::vector<std::unique_ptr<Z3DVolume>> m_volumes;
  std::vector<ZMesh> m_quads;
  ZBoolParameter m_stayOnTop;
  ZBoolParameter m_isSliceDownsampled;
  ZColorMapParameter m_sliceColormap;

  std::shared_ptr<ZWidgetsGroup> m_widgetsGroup;

  Z3DFilterOutputPort<Z3D2DSliceFilter> m_vPPort;
  Z3DRenderOutputPort m_opaqueOutport;
  Z3DRenderOutputPort m_opaqueLeftEyeOutport;
  Z3DRenderOutputPort m_opaqueRightEyeOutport;
};

#endif // Z3D2DSLICEFILTER_H
