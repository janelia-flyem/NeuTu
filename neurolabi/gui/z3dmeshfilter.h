#ifndef Z3DMESHFILTER_H
#define Z3DMESHFILTER_H

#include "z3dgeometryfilter.h"
#include "zoptionparameter.h"
#include "zwidgetsgroup.h"
#include "znumericparameter.h"
#include "z3dmeshrenderer.h"
#include "zeventlistenerparameter.h"
#include "z3drenderport.h"
#include "z3dtexturecopyrenderer.h"
#include <QObject>
#include <QString>
#include <QPoint>
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

  void setGlow(bool v)
  { m_glow.set(v); }

  virtual void process(Z3DEye eye) override;

  void setData(std::vector<ZMesh*>* meshList);

  void setData(QList<ZMesh*>* meshList);

  void setSelectedMeshes(std::set<ZMesh*>* list)
  { m_selectedMeshes = list; }

  virtual bool isReady(Z3DEye eye) const override;

  std::shared_ptr<ZWidgetsGroup> widgetsGroup();

  std::shared_ptr<ZWidgetsGroup> widgetsGroupForAnnotationFilter();

  virtual bool hasOpaque(Z3DEye eye) const override
  { return Z3DGeometryFilter::hasOpaque(eye) && !m_glow.get(); }

  virtual void renderOpaque(Z3DEye eye) override;

  virtual bool hasTransparent(Z3DEye eye) const override
  { return Z3DGeometryFilter::hasTransparent(eye) || m_glow.get(); }

  virtual void renderTransparent(Z3DEye eye) override;

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

  ZBBox<glm::dvec3> meshBound(ZMesh* p);

  //virtual void updateAxisAlignedBoundBoxImpl() override;
  virtual void updateNotTransformedBoundBoxImpl() override;

  void updateMeshVisibleState();

private:
  // get visible data from m_origMeshList put into m_meshList
  void getVisibleData();

private:
  Z3DRenderOutputPort m_monoEyeOutport;
  Z3DRenderOutputPort m_leftEyeOutport;
  Z3DRenderOutputPort m_rightEyeOutport;
  Z3DRenderOutputPort m_monoEyeOutport2;
  Z3DRenderOutputPort m_leftEyeOutport2;
  Z3DRenderOutputPort m_rightEyeOutport2;

  Z3DMeshRenderer m_triangleListRenderer;

  ZStringIntOptionParameter m_colorMode;
  ZVec4Parameter m_singleColorForAllMesh;

  //Z3DTextureGlowRenderer m_textureGlowRenderer;
  ZBoolParameter m_glow;
  Z3DTextureCopyRenderer m_textureCopyRenderer;

  //std::map<QString, size_t> m_sourceColorMapper;   // should use unordered_map
  // mesh list used for rendering, it is a subset of m_origMeshList. Some mesh are
  // hidden because they are unchecked from the object model. This allows us to control
  // the visibility of each single punctum.
  std::vector<ZMesh*> m_meshList;
  std::vector<ZMesh*> m_registeredMeshList;    // used for picking

  std::vector<glm::vec4> m_meshColors;
  std::vector<glm::vec4> m_meshPickingColors;

  ZEventListenerParameter m_selectMeshEvent;
  glm::ivec2 m_startCoord;
  ZMesh* m_pressedMesh;
  std::set<ZMesh*>* m_selectedMeshes;   //point to all selected meshes, managed by other class

  // generate and save to speed up bound box rendering for big mesh
  std::map<ZMesh*, ZBBox<glm::dvec3>> m_meshBoundboxMapper;

  std::shared_ptr<ZWidgetsGroup> m_widgetsGroup;
  bool m_dataIsInvalid;

  std::vector<ZMesh*> m_origMeshList;
};

#endif // Z3DMESHFILTER_H
