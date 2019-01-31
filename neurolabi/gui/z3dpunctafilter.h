#ifndef Z3DPUNCTAFILTER_H
#define Z3DPUNCTAFILTER_H

#include "z3dgeometryfilter.h"
#include "widgets/zoptionparameter.h"
#include "zwidgetsgroup.h"
#include "zcolormap.h"
#include "widgets/znumericparameter.h"
#include "z3dsphererenderer.h"
#include "zeventlistenerparameter.h"
#include "zpunctum.h"
#include "z3drenderport.h"
#include "z3dtexturecopyrenderer.h"
#include "zstringutils.h"
#include <QString>
#include <QPoint>
#include <map>
#include <vector>

class Z3DPunctaFilter : public Z3DGeometryFilter
{
  Q_OBJECT
public:
  explicit Z3DPunctaFilter(Z3DGlobalParameters& globalParas, QObject* parent = nullptr);

  void setData(const std::vector<ZPunctum*>& punctaList);
  void setData(const QList<ZPunctum*>& punctaList);

  virtual bool isReady(Z3DEye eye) const override;

  std::shared_ptr<ZWidgetsGroup> widgetsGroup();

  inline void setColorMode(const std::string& mode)
  {
    m_colorMode.select(mode.c_str());
  }

  //virtual bool hasOpaque(Z3DEye eye) const override { return Z3DGeometryFilter::hasOpaque(eye) && !m_randomGlow.get(); }
  virtual void renderOpaque(Z3DEye eye) override;

  //virtual bool hasTransparent(Z3DEye eye) const override { return Z3DGeometryFilter::hasTransparent(eye) || m_randomGlow.get(); }
  virtual void renderTransparent(Z3DEye eye) override;

  void configure(const ZJsonObject &obj) override;

  void punctumBound(const ZPunctum& p, ZBBox<glm::dvec3>& result) const;

  void updatePunctumVisibleState();

signals:
  void punctumSelected(ZPunctum*, bool append);

protected:
  void prepareColor();
  void adjustWidgets();
  void changePunctaSize();
  void selectPuncta(QMouseEvent *e, int w, int h);
  void updateData();

  virtual void process(Z3DEye eye) override;

  virtual void renderPicking(Z3DEye eye) override;

  virtual void registerPickingObjects() override;

  virtual void deregisterPickingObjects() override;

  void prepareData();

  void notTransformedPunctumBound(const ZPunctum& p, ZBBox<glm::dvec3>& result) const;

  //virtual void updateAxisAlignedBoundBoxImpl() override;
  virtual void updateNotTransformedBoundBoxImpl() override;

  virtual void addSelectionLines() override;

private:
  // get visible data from origPunctaList put into punctaList
  void getVisibleData();

private:
  Z3DRenderOutputPort m_monoEyeOutport;
  Z3DRenderOutputPort m_leftEyeOutport;
  Z3DRenderOutputPort m_rightEyeOutport;
  Z3DRenderOutputPort m_monoEyeOutport2;
  Z3DRenderOutputPort m_leftEyeOutport2;
  Z3DRenderOutputPort m_rightEyeOutport2;

  Z3DSphereRenderer m_sphereRenderer;

  ZStringIntOptionParameter m_colorMode;
  ZVec4Parameter m_singleColorForAllPuncta;
  std::map<QString, std::unique_ptr<ZVec4Parameter>, QStringNaturalCompare>
  m_sourceColorMapper;
  ZColorMapParameter m_colorMapScore;
  ZColorMapParameter m_colorMapMeanIntensity;
  ZColorMapParameter m_colorMapMaxIntensity;
  ZBoolParameter m_useSameSizeForAllPuncta;

  //  Z3DSphereRenderer m_glowSphereRenderer;
  //  Z3DTextureGlowRenderer m_textureGlowRenderer;
  //  ZBoolParameter m_randomGlow;
  //  ZFloatParameter m_glowPercentage;
  //  Z3DTextureCopyRenderer m_textureCopyRenderer;

  // puncta list used for rendering, it is a subset of m_origPunctaList. Some puncta are
  // hidden because they are unchecked from the object model. This allows us to control
  // the visibility of each single punctum.
  std::vector<ZPunctum*> m_punctaList;
  std::vector<ZPunctum*> m_registeredPunctaList;    // used for picking

  ZEventListenerParameter m_selectPunctumEvent;
  glm::ivec2 m_startCoord;
  ZPunctum* m_pressedPunctum = nullptr;

  std::vector<glm::vec4> m_pointAndRadius;
  std::vector<glm::vec4> m_specularAndShininess;
  std::vector<glm::vec4> m_pointColors;
  std::vector<glm::vec4> m_pointPickingColors;

  std::shared_ptr<ZWidgetsGroup> m_widgetsGroup;
  std::vector<ZWidgetsGroup*> m_colorsForDifferentSourceWidgetsGroup;
  bool m_dataIsInvalid = false;

  std::vector<ZPunctum*> m_origPunctaList;
};

#endif // Z3DPUNCTAFILTER_H
