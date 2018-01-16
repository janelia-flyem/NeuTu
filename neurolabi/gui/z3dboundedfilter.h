#ifndef Z3DBOUNDEDFILTER_H
#define Z3DBOUNDEDFILTER_H

#include "z3dfilter.h"
#include "z3dtransferfunction.h"
#include "z3dlinerenderer.h"
#include "z3dmeshrenderer.h"
#include "z3dsphererenderer.h"
#include "z3darrowrenderer.h"

class ZLineSegment;

// child class should implement two pure virtual function and call updateBoundBox whenever its
// own parameter affect bound box
class Z3DBoundedFilter : public Z3DFilter
{
Q_OBJECT
public:
  explicit Z3DBoundedFilter(Z3DGlobalParameters& globalPara, QObject* parent = nullptr);

  bool isVisible() const
  { return m_visible.get(); }

  void setVisible(bool v)
  { m_visible.set(v); }

  bool isTransformEnabled() const
  { return m_transformEnabled; }

  virtual void setViewport(glm::uvec2 viewport)
  { m_rendererBase.setViewport(viewport); }

  virtual void setViewport(glm::uvec4 viewport)
  { m_rendererBase.setViewport(viewport); }

  inline Z3DCamera& camera()
  { return m_rendererBase.camera(); }

  inline Z3DCamera& globalCamera()
  { return m_rendererBase.globalCamera(); }

  inline Z3DCameraParameter& globalCameraPara()
  { return m_rendererBase.globalCameraPara(); }

  inline Z3DPickingManager& pickingManager()
  { return m_rendererBase.globalParas().pickingManager; }

  inline const Z3DPickingManager& pickingManager() const
  { return m_rendererBase.globalParas().pickingManager; }

  inline Z3DTrackballInteractionHandler& interactionHandler()
  { return m_rendererBase.globalParas().interactionHandler; }

  virtual void setShaderHookType(Z3DRendererBase::ShaderHookType t)
  { m_rendererBase.setShaderHookType(t); }

  virtual void setShaderHookParaDDPDepthBlenderTexture(const Z3DTexture* t)
  { m_rendererBase.setShaderHookParaDDPDepthBlenderTexture(t); }

  virtual void setShaderHookParaDDPFrontBlenderTexture(const Z3DTexture* t)
  { m_rendererBase.setShaderHookParaDDPFrontBlenderTexture(t); }

  inline Z3DRendererBase::ShaderHookParameter& shaderHookPara()
  { return m_rendererBase.shaderHookPara(); }

  const ZBBox<glm::dvec3>& axisAlignedBoundBox() const
  { return m_axisAlignedBoundBox; }

  const ZBBox<glm::dvec3>& notTransformedBoundBox() const
  { return m_notTransformedBoundBox; }

  // Useful coordinate L->Left U->Up F->Front R->Right D->Down B->Back
  glm::vec3 physicalLUF() const
  { return glm::vec3(m_notTransformedBoundBox.minCorner()); }

  glm::vec3 physicalRDB() const
  { return glm::vec3(m_notTransformedBoundBox.maxCorner()); }

  glm::vec3 physicalLDF() const
  { return glm::vec3(physicalLUF().x, physicalRDB().y, physicalLUF().z); }

  glm::vec3 physicalRDF() const
  { return glm::vec3(physicalRDB().x, physicalRDB().y, physicalLUF().z); }

  glm::vec3 physicalRUF() const
  { return glm::vec3(physicalRDB().x, physicalLUF().y, physicalLUF().z); }

  glm::vec3 physicalLUB() const
  { return glm::vec3(physicalLUF().x, physicalLUF().y, physicalRDB().z); }

  glm::vec3 physicalLDB() const
  { return glm::vec3(physicalLUF().x, physicalRDB().y, physicalRDB().z); }

  glm::vec3 physicalRUB() const
  { return glm::vec3(physicalRDB().x, physicalLUF().y, physicalRDB().z); }

  // bound voxels in world coordinate
  glm::vec3 worldLUF() const
  { return glm::applyMatrix(m_rendererBase.coordTransform(), physicalLUF()); }

  glm::vec3 worldRDB() const
  { return glm::applyMatrix(m_rendererBase.coordTransform(), physicalRDB()); }

  glm::vec3 worldLDF() const
  { return glm::applyMatrix(m_rendererBase.coordTransform(), physicalLDF()); }

  glm::vec3 worldRDF() const
  { return glm::applyMatrix(m_rendererBase.coordTransform(), physicalRDF()); }

  glm::vec3 worldRUF() const
  { return glm::applyMatrix(m_rendererBase.coordTransform(), physicalRUF()); }

  glm::vec3 worldLUB() const
  { return glm::applyMatrix(m_rendererBase.coordTransform(), physicalLUB()); }

  glm::vec3 worldLDB() const
  { return glm::applyMatrix(m_rendererBase.coordTransform(), physicalLDB()); }

  glm::vec3 worldRUB() const
  { return glm::applyMatrix(m_rendererBase.coordTransform(), physicalRUB()); }

  virtual bool hasOpaque(Z3DEye) const
  { return m_rendererBase.opacity() == 1.f; }

  virtual void renderOpaque(Z3DEye)
  {}

  virtual bool hasTransparent(Z3DEye) const
  { return m_rendererBase.opacity() < 1.f; }

  virtual void renderTransparent(Z3DEye)
  {}

  void renderSelectionBox(Z3DEye eye);

  void setXScale(float s)
  { m_rendererBase.setXScale(s); }

  void setYScale(float s)
  { m_rendererBase.setYScale(s); }

  void setZScale(float s)
  { m_rendererBase.setZScale(s); }

  float sizeScale() const
  { return m_rendererBase.sizeScale(); }

  void setSizeScale(float s)
  { m_rendererBase.setSizeScale(s); }

  float opacity() const
  { return m_rendererBase.opacity(); }

  void setOpacity(float o);
  void setOpacityQuitely(float o);

  void setMaterialSpecular(const glm::vec4 &v) {
    m_rendererBase.setMaterialSpecular(v);
  }

  std::string getControlName() const {
    return m_controlName;
  }

  void setControlName(const std::string &name) {
    m_controlName = name;
  }

  glm::mat4 coordTransform() const
  { return m_rendererBase.coordTransform(); }

  glm::vec3 getViewCoord(double x, double y, double z, double w, double h);

  void setOffset(float sx, float sy, float sz)
  { m_rendererBase.setOffset(sx, sy, sz); }

  void setScale(float sx, float sy, float sz)
  { m_rendererBase.setScale(sx, sy, sz); }

  void hideBoundBox()
  { m_boundBoxMode.select("No Bound Box"); }

  inline int xCutLowerValue() const { return m_xCut.lowerValue(); }
  inline int xCutUpperValue() const { return m_xCut.upperValue(); }
  inline int yCutLowerValue() const { return m_yCut.lowerValue(); }
  inline int yCutUpperValue() const { return m_yCut.upperValue(); }
  inline int zCutLowerValue() const { return m_zCut.lowerValue(); }
  inline int zCutUpperValue() const { return m_zCut.upperValue(); }

  inline int xCutMin() const { return m_xCut.minimum(); }
  inline int xCutMax() const { return m_xCut.maximum(); }
  inline int yCutMin() const { return m_yCut.minimum(); }
  inline int yCutMax() const { return m_yCut.maximum(); }
  inline int zCutMin() const { return m_zCut.minimum(); }
  inline int zCutMax() const { return m_zCut.maximum(); }


  inline void setXCutLower(int v) { m_xCut.setLowerValue(v); }
  inline void setXCutUpper(int v) { m_xCut.setUpperValue(v); }
  inline void setYCutLower(int v) { m_yCut.setLowerValue(v); }
  inline void setYCutUpper(int v) { m_yCut.setUpperValue(v); }
  inline void setZCutLower(int v) { m_zCut.setLowerValue(v); }
  inline void setZCutUpper(int v) { m_zCut.setUpperValue(v); }

  void resetCut();
  ZIntCuboid cutBox();
  void setCutBox(const ZIntCuboid &box);

  // output v1 is start point of ray, v2 is a point on the ray, v2-v1 is normalized
  // x and y are input screen point, width and height are input screen dimension
  void rayUnderScreenPoint(glm::vec3& v1, glm::vec3& v2, int x, int y, int width, int height);

  void rayUnderScreenPoint(glm::dvec3& v1, glm::dvec3& v2, int x, int y, int width, int height);

  ZLineSegment getScreenRay(int x, int y, int width, int height);

signals:

  void boundBoxChanged();

  void objVisibleChanged(bool v);
  void opacityChanged(double v);

protected:
  void updateBoundBox();

  // implement this to empty function if clip planes are not needed
  virtual void setClipPlanes();

  void initializeCutRange();

  void initializeRotationCenter();

  void setTransformEnabled(bool v)
  { m_transformEnabled = v; }

  void renderBoundBox(Z3DEye eye);

  void appendBoundboxLines(const ZBBox<glm::dvec3>& bound, std::vector<glm::vec3>& lines);

  // implement these to support bound box
  virtual void updateAxisAlignedBoundBoxImpl();

  virtual void updateNotTransformedBoundBoxImpl()
  {}

  // besides the big selection box, other additional lines can be added through this function
  virtual void addSelectionLines()
  {}

  // reimplement this if cut range has different behavior
  virtual void expandCutRange();

private:
  void updateAxisAlignedBoundBox();

  void updateNotTransformedBoundBox();

  void onBoundBoxModeChanged();

  void updateBoundBoxLineColors();

  void updateSelectionLineColors();

  void onBoundBoxLineWidthChanged();

  void onSelectionBoundBoxLineWidthChanged();

protected:
  Z3DRendererBase m_rendererBase;

  Z3DLineRenderer m_baseBoundBoxRenderer;
  Z3DLineRenderer m_selectionBoundBoxRenderer;

  ZBoolParameter m_visible;
  ZFloatSpanParameter m_xCut;
  ZFloatSpanParameter m_yCut;
  ZFloatSpanParameter m_zCut;
  ZStringIntOptionParameter m_boundBoxMode;
  ZIntParameter m_boundBoxLineWidth;
  ZVec4Parameter m_boundBoxLineColor;
  //Z3DTransferFunctionParameter m_boundBoxLineColor;
  ZIntParameter m_selectionLineWidth;
  ZVec4Parameter m_selectionLineColor;

  ZBBox<glm::dvec3> m_axisAlignedBoundBox;
  glm::vec3 m_center;
  ZBBox<glm::dvec3> m_notTransformedBoundBox;

  std::vector<glm::vec3> m_normalBoundBoxLines;
  std::vector<glm::vec3> m_axisAlignedBoundBoxLines;
  std::vector<glm::vec4> m_boundBoxLineColors;

  std::vector<glm::vec3> m_selectionLines;
  std::vector<glm::vec4> m_selectionLineColors;

  bool m_canUpdateClipPlane;

  bool m_isSelected;
  std::string m_controlName;

private:
  glm::ivec2 m_lastMousePosition;
  glm::vec3 m_startMouseWorldPos;
  glm::vec3 m_startTrans;
  float m_startDepth;

  bool m_transformEnabled;
};

#endif // Z3DBOUNDEDFILTER_H
