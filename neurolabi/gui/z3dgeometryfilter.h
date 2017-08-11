#ifndef Z3DGEOMETRYFILTER_H
#define Z3DGEOMETRYFILTER_H

#include "z3dboundedfilter.h"
#include "z3drendererbase.h"
#include "z3dport.h"
#include "z3dpickingmanager.h"

class Z3DGeometryFilter : public Z3DBoundedFilter
{
  friend class Z3DCompositor;

public:
  explicit Z3DGeometryFilter(Z3DGlobalParameters& globalPara, QObject* parent = nullptr);

  virtual void renderPicking(Z3DEye /*unused*/)
  {}

  bool isStayOnTop() const
  { return m_stayOnTop.get(); }

  void setStayOnTop(bool s)
  { m_stayOnTop.set(s); }

  void setVisible(bool v);
  bool isVisible() const;

  glm::mat4 coordTransform() const
  { return m_rendererBase.coordTransform(); }

  float opacity() const
  { return m_rendererBase.opacity(); }

  void setOpacity(float v)
  { m_rendererBase.setOpacity(v); }

  float sizeScale() const
  { return m_rendererBase.sizeScale(); }

  void setSizeScale(float s)
  { m_rendererBase.setSizeScale(s); }

  inline const Z3DCamera& getCamera() const {return m_rendererBase.getCamera();}

  glm::uvec3 getPickingTexSize() const;
  void updatePickingTexSize();

  inline Z3DPickingManager* getPickingManager() const {return m_pickingManager;}

  virtual void configure(const ZJsonObject &obj);

  virtual ZJsonObject getConfigJson() const;

  // output v1 is start point of ray, v2 is a point on the ray, v2-v1 is normalized
  // x and y are input screen point, width and height are input screen dimension
  void get3DRayUnderScreenPoint(
      glm::vec3 &v1, glm::vec3 &v2, int x, int y, int width, int height);
  void get3DRayUnderScreenPoint(
      glm::dvec3 &v1, glm::dvec3 &v2, int x, int y, int width, int height);

protected:
  virtual void process(Z3DEye /*eye*/) override
  {}

  // once processed, should be valid for both stereo view and mono view
  virtual void setValid(Z3DEye eye) override
  {
    Z3DBoundedFilter::setValid(eye);
    m_state = State::Valid;
  }

  // functions for picking, use these two function and m_pickingObjectsRegistered to control picking
  // note: input Z3DPickingManager might be nullptr
  // after deregister, m_pickingObjectsRegistered should be false;
  virtual void deregisterPickingObjects()
  {}

  // after register, m_pickingObjectsRegistered should be true and data picking color should be set
  // for renderers
  virtual void registerPickingObjects()
  {}

protected:
  Z3DFilterOutputPort<Z3DGeometryFilter> m_outPort;


  ZBoolParameter m_visible;
  ZBoolParameter m_stayOnTop;
  Z3DPickingManager* m_pickingManager;
  bool m_pickingObjectsRegistered;
  glm::ivec3 m_pickingTexSize;
};

#endif // Z3DGEOMETRYFILTER_H
