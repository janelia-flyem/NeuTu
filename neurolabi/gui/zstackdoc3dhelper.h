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

/*!
 * \brief The class of helping managing data for 3D visualization
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
  void updateRoiData();
  void updateData(neutube3d::ERendererLayer layer);

private:
  void addObject(neutube3d::ERendererLayer layer, ZStackObject *obj);
  void addObject(neutube3d::ERendererLayer layer, ZStackObjectPtr obj);
  void resetObjectAdapter(neutube3d::ERendererLayer layer);


private:
  Z3DView *m_view = NULL;
  QMap<neutube3d::ERendererLayer, QList<ZStackObjectPtr> > m_objectAdapter;
};

#endif // ZSTACKDOC3DHELPER_H
