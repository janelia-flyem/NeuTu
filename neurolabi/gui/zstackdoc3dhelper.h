#ifndef ZSTACKDOC3DHELPER_H
#define ZSTACKDOC3DHELPER_H

#include <QList>
#include <QMap>

#include "z3ddef.h"
#include "zstackobjectptr.h"

class ZSwcTree;
class Z3DView;
class ZStackObject;
class ZStackObjectInfoSet;
class ZFlyEmBody3dDoc;

/*!
 * \brief The class of helping managing data for 3D visualization
 *
 * ZStackDoc3dHelper is designed to facilitate adding a new layer to 3D
 * visualization. Data from \a ZStackDoc are partitioned to be rendered in
 * different layers exclusively, which is done in the function \a updateData.
 * Another function \a dataUpdateRequired decides if a layer needs to be updated
 * according to current information of object modification.
 */
class ZStackDoc3dHelper
{
public:
  ZStackDoc3dHelper();

  void processObjectModified(const ZStackObjectInfoSet &objInfo);

  bool dataUpdateRequired(neutube3d::ERendererLayer layer,
                          const ZStackObjectInfoSet &objInfo) const;

  void attach(Z3DView *view) {
    m_view = view;
  }

  void updateGraphData();
  void updateSwcData();
  void updatePunctaData();
  void updateSurfaceData();
  void updateTodoData();
  void updateMeshData();
  void updateDecorationData();
  void updateRoiData();
  void updateData(neutube3d::ERendererLayer layer);

  void updateCustomCanvas(ZFlyEmBody3dDoc *doc);
  void updateCustomCanvas();

private:
  void addObject(neutube3d::ERendererLayer layer, ZStackObject *obj);
  void addObject(neutube3d::ERendererLayer layer, ZStackObjectPtr obj);
  void resetObjectAdapter(neutube3d::ERendererLayer layer);


private:
  Z3DView *m_view = NULL;
  QMap<neutube3d::ERendererLayer, QList<ZStackObjectPtr> > m_objectAdapter;
};

#endif // ZSTACKDOC3DHELPER_H
