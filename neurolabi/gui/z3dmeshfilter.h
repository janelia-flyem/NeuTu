#ifndef Z3DMESHFILTER_H
#define Z3DMESHFILTER_H

#include "z3dgeometryfilter.h"
#include "zoptionparameter.h"
#include "zwidgetsgroup.h"
#include "znumericparameter.h"
#include "z3dmeshrenderer.h"
#include "zeventlistenerparameter.h"
#include "zstringutils.h"
#include <map>
#include <vector>

class Z3DMeshFilter : public Z3DGeometryFilter
{
Q_OBJECT
public:
  explicit Z3DMeshFilter(Z3DGlobalParameters& globalParas, QObject* parent = nullptr);

  void setMeshColor(const glm::vec4& col)
  { m_singleColorForAllMesh.set(col); }

  bool isFixed() const
  { return m_meshList[0]->numVertices() == 96957; }

  virtual void process(Z3DEye eye) override;

  void setData(const std::vector<ZMesh*>& meshList);
  void setData(const QList<ZMesh*>& meshList);

  virtual bool isReady(Z3DEye eye) const override;

  std::shared_ptr<ZWidgetsGroup> widgetsGroup();

  virtual void renderOpaque(Z3DEye eye) override;

  virtual void renderTransparent(Z3DEye eye) override;

  void updateMeshVisibleState();

  ZBBox<glm::dvec3> meshBound(ZMesh* p);

signals:

  void meshSelected(ZMesh*, bool append);

protected:
  void prepareColor();

  void adjustWidgets();

  void selectMesh(QMouseEvent* e, int w, int h);

  void onApplyTransform();

  virtual void renderPicking(Z3DEye eye) override;

  void prepareData();

  virtual void registerPickingObjects() override;

  virtual void deregisterPickingObjects() override;

  //virtual void updateAxisAlignedBoundBoxImpl() override;
  virtual void updateNotTransformedBoundBoxImpl() override;

  virtual void addSelectionLines() override;

private:
  // get visible data from m_origMeshList put into m_meshList
  void getVisibleData();

private:
  Z3DMeshRenderer m_meshRenderer;

  ZStringIntOptionParameter m_colorMode;
  ZVec4Parameter m_singleColorForAllMesh;
  std::map<QString, std::unique_ptr<ZVec4Parameter>, QStringNaturalCompare> m_sourceColorMapper;

  // mesh list used for rendering, it is a subset of m_origMeshList. Some mesh are
  // hidden because they are unchecked from the object model. This allows us to control
  // the visibility of each single mesh.
  std::vector<ZMesh*> m_meshList;
  std::vector<ZMesh*> m_registeredMeshList;    // used for picking

  std::vector<glm::vec4> m_meshColors;
  std::vector<glm::vec4> m_meshPickingColors;

  ZEventListenerParameter m_selectMeshEvent;
  glm::ivec2 m_startCoord;
  ZMesh* m_pressedMesh;

  // generate and save to speed up bound box rendering for big mesh
  std::map<ZMesh*, ZBBox<glm::dvec3>> m_meshBoundboxMapper;

  std::shared_ptr<ZWidgetsGroup> m_widgetsGroup;
  bool m_dataIsInvalid;

  std::vector<ZMesh*> m_origMeshList;
};

#endif // Z3DMESHFILTER_H
