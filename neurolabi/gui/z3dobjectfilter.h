#ifndef Z3DOBJECTFILTER_H
#define Z3DOBJECTFILTER_H

#include "z3dgeometryfilter.h"
#include "z3dlinerenderer.h"

class ZStackObject;

class Z3DObjectFilter : public Z3DGeometryFilter
{
  Q_OBJECT
public:
  Z3DObjectFilter(Z3DGlobalParameters& globalParas, QObject* parent = nullptr);
  ~Z3DObjectFilter();

  void clear();

  virtual void renderOpaque(Z3DEye eye) override;
  virtual void renderTransparent(Z3DEye eye) override;

  virtual bool isReady(Z3DEye eye) const override;

protected:
  void prepareData();
  void prepareColor();
  std::vector<double> boundBox();
  virtual void process(Z3DEye eye) override;
  virtual void updateNotTransformedBoundBoxImpl() override;

private:
  QList<ZStackObject*> m_objList;

  Z3DLineRenderer m_lineRenderer;
  std::vector<glm::vec3> m_lines;
  std::vector<glm::vec4> m_lineColors;

  bool m_dataIsInvalid = false;

};

#endif // Z3DOBJECTFILTER_H
