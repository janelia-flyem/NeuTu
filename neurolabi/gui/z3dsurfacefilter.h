#ifndef Z3DSURFACEFILTER_H
#define Z3DSURFACEFILTER_H


#include "z3dgeometryfilter.h"
#include "widgets/zoptionparameter.h"
#include "widgets/zwidgetsgroup.h"
#include "widgets/znumericparameter.h"
#include "z3dmeshrenderer.h"
#include "zeventlistenerparameter.h"
#include "zstringutils.h"
#include "zcubearray.h"
#include <map>
#include <vector>

class Z3DSurfaceFilter : public Z3DGeometryFilter
{
Q_OBJECT
public:
  explicit Z3DSurfaceFilter(Z3DGlobalParameters& globalParas, QObject* parent = nullptr);

  void setSurfaceColor(const glm::vec4& col)
  { m_singleColorForAllSurface.set(col); }

  virtual void process(Z3DEye eye) override;

  void setData(const std::vector<ZCubeArray*>& cubeList);
  void setData(const QList<ZCubeArray*>& cubeList);

  virtual bool isReady(Z3DEye eye) const override;

  std::shared_ptr<ZWidgetsGroup> widgetsGroup();

  virtual void renderOpaque(Z3DEye eye) override;

  virtual void renderTransparent(Z3DEye eye) override;

  void updateSurfaceVisibleState();

  ZBBox<glm::dvec3> cubeBound(const ZCubeArray& p);

signals:
  void opacityValueChanged();

protected:
  void prepareColor();

  void adjustWidgets();

  void prepareData();

  //virtual void updateAxisAlignedBoundBoxImpl() override;
  virtual void updateNotTransformedBoundBoxImpl() override;

  virtual void addSelectionLines() override;

private:
  // get visible data from m_origMeshList put into m_meshList
  void getVisibleData();

private:
  Z3DMeshRenderer m_meshRenderer;

  ZStringIntOptionParameter m_colorMode;
  ZVec4Parameter m_singleColorForAllSurface;
  std::map<QString, std::unique_ptr<ZVec4Parameter>, QStringNaturalCompare> m_sourceColorMapper;

  // cubeArray list used for rendering, it is a subset of m_origCubeArrayList. Some cubes are
  // hidden because they are unchecked from the object model. This allows us to control
  // the visibility of each single cubeArray.
  std::vector<ZCubeArray*> m_cubeArrayList;

  std::vector<glm::vec4> m_cubeArrayColors;

  std::shared_ptr<ZWidgetsGroup> m_widgetsGroup;
  bool m_dataIsInvalid;

  std::vector<ZCubeArray*> m_origCubeArrayList;

  std::vector<ZMesh> m_meshCache;
  std::vector<ZMesh*> m_meshPtCache;
};

#endif // Z3DSURFACEFILTER_H
