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

  virtual void configure(const ZJsonObject &obj);

  virtual ZJsonObject getConfigJson() const;

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
  // after deregister, m_pickingObjectsRegistered should be false;
  virtual void deregisterPickingObjects()
  {}

  // after register, m_pickingObjectsRegistered should be true and data picking color should be set
  // for renderers
  virtual void registerPickingObjects()
  {}

  const void* pickObject(int x, int y);

protected:
  Z3DFilterOutputPort<Z3DGeometryFilter> m_outPort;

  ZBoolParameter m_stayOnTop;
  bool m_pickingObjectsRegistered;
};

#endif // Z3DGEOMETRYFILTER_H
