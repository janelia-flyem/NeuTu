#ifndef ZSTACKDOC3DHELPER_H
#define ZSTACKDOC3DHELPER_H

#include <QList>
#include <QMap>

#include "z3ddef.h"
#include "zuncopyable.h"

class ZSwcTree;
class Z3DView;
class ZStackObject;
class ZStackObjectInfoSet;
class ZFlyEmBody3dDoc;
class ZStackDoc;

/*!
 * \brief The class of helping managing data for 3D visualization
 *
 * ZStackDoc3dHelper is designed to facilitate adding a new layer to 3D
 * visualization. Data from \a ZStackDoc are partitioned to be rendered in
 * different layers exclusively, which is done in the function \a updateData.
 * Another function \a dataUpdateRequired decides if a layer needs to be updated
 * according to current information of object modification.
 */
class ZStackDoc3dHelper : private ZUncopyable
{
public:
  ZStackDoc3dHelper();
  ~ZStackDoc3dHelper();

  void processObjectModified(const ZStackObjectInfoSet &objInfo, Z3DView *view);

  bool dataUpdateRequired(neutube3d::ERendererLayer layer,
                          const ZStackObjectInfoSet &objInfo) const;

//  void attach(Z3DView *view) {
//    m_view = view;
//  }

  void updateGraphData(Z3DView *view);
  void updateSwcData(Z3DView *view);
  void updatePunctaData(Z3DView *view);
  void updateSurfaceData(Z3DView *view);
  void updateTodoData(Z3DView *view);
  void updateMeshData(Z3DView *view);
  void updateDecorationData(Z3DView *view);
  void updateRoiData(Z3DView *view);
  void updateSliceData(Z3DView *view);
  void updateData(Z3DView *view, neutube3d::ERendererLayer layer);

  void updateCustomCanvas(Z3DView *view);

  QList<ZStackObject*> getObjectList(neutube3d::ERendererLayer layer) const {
    return m_objectAdapter.contains(layer) ? m_objectAdapter[layer] :
                                             QList<ZStackObject*>();
  }

  static ZStackDoc3dHelper* GetDocHelper(ZStackDoc *doc);
  static void UpdateViewData(Z3DView *view, neutube3d::ERendererLayer layer);
//  static void Attach(ZStackDoc *doc, Z3DView *view);

  bool releaseObject(neutube3d::ERendererLayer layer, ZStackObject *obj);

private:
  void addObject(neutube3d::ERendererLayer layer, ZStackObject *obj);
//  void addObject(neutube3d::ERendererLayer layer, ZStackObjectPtr obj);
  void resetObjectAdapter(neutube3d::ERendererLayer layer);
  void updateCustomCanvas(Z3DView *view, ZFlyEmBody3dDoc *doc);

private:
//  Z3DView *m_view = NULL;
  QMap<neutube3d::ERendererLayer, QList<ZStackObject*> > m_objectAdapter;
};

#endif // ZSTACKDOC3DHELPER_H
