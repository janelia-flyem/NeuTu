#include "zstackdoc3dhelper.h"
#include "zstackobject.h"
#include "zstackobjectinfo.h"
#include "z3dview.h"

#include "z3dvolumefilter.h"
#include "z3dpunctafilter.h"
#include "z3dswcfilter.h"
#include "z3dmeshfilter.h"
#include "z3dgraphfilter.h"
#include "z3dsurfacefilter.h"
#include "flyem/zflyemtodolistfilter.h"
#include "zfiletype.h"
#include "zswcnetwork.h"
#include "flyem/zflyembody3ddoc.h"
#include "zmeshfactory.h"
#include "zstackdocproxy.h"
#include "dvid/zdvidgrayslice.h"
#include "zobject3dscan.h"

ZStackDoc3dHelper::ZStackDoc3dHelper()
{
}

ZStackDoc3dHelper::~ZStackDoc3dHelper()
{
  for (auto &objList : m_objectAdapter) {
    for (ZStackObject *obj : objList) {
      delete obj;
    }
  }
}

void ZStackDoc3dHelper::processObjectModified(const ZStackObjectInfoSet &objInfo)
{
#ifdef _DEBUG_
  std::cout << "Processing object modification: " << std::endl;
  objInfo.print();
#endif
  const QList<neutube3d::ERendererLayer>& layerList = m_view->getLayerList();
  foreach (neutube3d::ERendererLayer layer, layerList) {
    if (dataUpdateRequired(layer, objInfo)) {
#ifdef _DEBUG_
      std::cout << "Updating layer " << layer << std::endl;
#endif
      m_view->updateDocData(layer);
    }
  }

  if (objInfo.contains(ZStackObject::TARGET_3D_CANVAS)) {
    updateCustomCanvas();
  }
}

bool ZStackDoc3dHelper::dataUpdateRequired(
    neutube3d::ERendererLayer layer, const ZStackObjectInfoSet &objInfo) const
{
  bool updating = false;

  switch (layer) {
  case neutube3d::LAYER_GRAPH:
    updating = objInfo.contains(ZStackObject::TYPE_3D_GRAPH) ||
        objInfo.contains(ZStackObjectRole::ROLE_3DGRAPH_DECORATOR);
    break;
  case neutube3d::LAYER_MESH:
    foreach (const ZStackObjectInfo &info, objInfo.keys()) {
      if ((info.getType() == ZStackObject::TYPE_MESH &&
           !info.getRole().hasRole(ZStackObjectRole::ROLE_ROI) &&
           !info.getRole().hasRole(ZStackObjectRole::ROLE_3DMESH_DECORATOR))) {
        updating = true;
        break;
      }
    }
    break;
  case neutube3d::LAYER_DECORATION:
    updating = objInfo.contains(ZStackObjectRole::ROLE_3DMESH_DECORATOR);
    break;
  case neutube3d::LAYER_PUNCTA:
    updating = objInfo.contains(ZStackObject::TYPE_PUNCTA) ||
        objInfo.contains(ZStackObject::TYPE_PUNCTUM);
    break;
  case neutube3d::LAYER_ROI:
    updating = objInfo.contains(ZStackObjectRole::ROLE_ROI);
    break;
  case neutube3d::LAYER_SURFACE:
    updating = objInfo.contains(ZStackObject::TYPE_3D_CUBE);
    break;
  case neutube3d::LAYER_SWC:
    updating = objInfo.contains(ZStackObject::TYPE_SWC);
    break;
  case neutube3d::LAYER_TODO:
    updating = objInfo.contains(ZStackObject::TYPE_FLYEM_TODO_ITEM) ||
        objInfo.contains(ZStackObject::TYPE_FLYEM_TODO_LIST);
    break;
  default:
    break;
  }

  return updating;
}

void ZStackDoc3dHelper::addObject(neutube3d::ERendererLayer layer, ZStackObject *obj)
{
  if (!m_objectAdapter.contains(layer)) {
    m_objectAdapter[layer] = QList<ZStackObject*>();
  }

  m_objectAdapter[layer].append(obj);
}

void ZStackDoc3dHelper::resetObjectAdapter(neutube3d::ERendererLayer layer)
{
  if (m_objectAdapter.contains(layer)) {
    auto &objList = m_objectAdapter[layer];
    for (ZStackObject *obj : objList) {
      delete obj;
    }
    objList.clear();
  }

  m_objectAdapter.remove(layer);
}

void ZStackDoc3dHelper::updateGraphData()
{
  Z3DGraphFilter *filter = m_view->getGraphFilter();
  if (filter != NULL) {
    filter->clear();

    ZStackDoc *doc = m_view->getDocument();
    if (doc->swcNetwork()) {
      ZPointNetwork *network = doc->swcNetwork()->toPointNetwork();
      filter->setData(*network, NULL);
      delete network;
    } else if (ZFileType::FileType(doc->additionalSource()) == ZFileType::FILE_JSON) {
      Z3DGraph graph;
      graph.importJsonFile(doc->additionalSource());
      filter->addData(graph);
    }

    filter->addData(
          doc->getPlayerList(ZStackObjectRole::ROLE_3DGRAPH_DECORATOR));

    TStackObjectList objList = doc->getObjectList(ZStackObject::TYPE_3D_GRAPH);
    for (TStackObjectList::const_iterator iter = objList.begin(); iter != objList.end(); ++iter) {
      Z3DGraph *graph = dynamic_cast<Z3DGraph*>(*iter);
      if (graph->isVisible()) {
        filter->addData(graph);
      }
    }
  }
}

void ZStackDoc3dHelper::updateTodoData()
{
#if defined(_FLYEM_)
  ZFlyEmBody3dDoc *doc = qobject_cast<ZFlyEmBody3dDoc*>(m_view->getDocument());
  if (doc) {
    ZFlyEmTodoListFilter *filter = m_view->getTodoFilter();
    if (filter != NULL) {
      QList<ZFlyEmToDoItem*> objList = doc->getObjectList<ZFlyEmToDoItem>();
      filter->setData(objList);
    }
  }
#endif
}

void ZStackDoc3dHelper::updateSwcData()
{
  Z3DSwcFilter *filter = m_view->getSwcFilter();
  if (filter) {
    filter->setData(m_view->getDocument()->getSwcList());
  }
}

void ZStackDoc3dHelper::updateDecorationData()
{
  Z3DMeshFilter *filter = m_view->getDecorationFilter();
  if (filter != NULL) {
    resetObjectAdapter(neutube3d::LAYER_DECORATION);
    QList<ZObject3dScan*> objList =
        m_view->getDocument()->getObjectList<ZObject3dScan>();
    QList<ZMesh*> meshList;
    foreach(ZObject3dScan *obj, objList) {
      if (obj->hasRole(ZStackObjectRole::ROLE_3DMESH_DECORATOR)) {
        ZMesh *mesh = ZMeshFactory::MakeMesh(*obj);
        if (mesh != NULL) {
          mesh->setLabel(obj->getLabel());
          if (obj->hasRole(ZStackObjectRole::ROLE_SEGMENTATION)) {
            mesh->addRole(ZStackObjectRole::ROLE_SEGMENTATION);
          }
          mesh->setColor(obj->getColor());
          mesh->pushObjectColor();
          mesh->setVisible(obj->isVisible());
          mesh->setSelectable(false);
          mesh->setObjectId(obj->getObjectId());
          addObject(neutube3d::LAYER_DECORATION, mesh);
          meshList.append(mesh);
        }
      }
    }

    filter->setData(meshList);
  }
}

void ZStackDoc3dHelper::updateMeshData()
{
  Z3DMeshFilter *filter = m_view->getMeshFilter();
  if (filter != NULL) {

    QList<ZMesh*> meshList =
        ZStackDocProxy::GetGeneralMeshList(m_view->getDocument());

    filter->setData(meshList);
  }
}

void ZStackDoc3dHelper::updateSliceData()
{
  //todo
}

void ZStackDoc3dHelper::updateRoiData()
{
  Z3DMeshFilter *filter = m_view->getRoiFilter();
  if (filter != NULL) {
//    QList<ZMesh*> meshList = m_view->getDocument()->getMeshList();

    QList<ZMesh*> filteredMeshList =
        ZStackDocProxy::GetRoiMeshList(m_view->getDocument());
//    foreach(ZMesh *mesh, meshList) {
//      if (mesh->hasRole(ZStackObjectRole::ROLE_ROI)) {
//        filteredMeshList.append(mesh);
//      }
//    }

    resetObjectAdapter(neutube3d::LAYER_ROI);
    QList<ZObject3dScan*> objList =
        m_view->getDocument()->getObjectList<ZObject3dScan>();
    foreach(ZObject3dScan *obj, objList) {
      if (obj->hasRole(ZStackObjectRole::ROLE_ROI)) {
        ZMesh *mesh = ZMeshFactory::MakeMesh(*obj);
        if (mesh != NULL) {
          mesh->setColor(obj->getColor());
          mesh->pushObjectColor();
          mesh->setVisible(obj->isVisible());
          addObject(neutube3d::LAYER_ROI, mesh);
          filteredMeshList.append(mesh);
        }
      }
    }

    filter->setData(filteredMeshList);
  }
}

void ZStackDoc3dHelper::updatePunctaData()
{
  Z3DPunctaFilter *filter = m_view->getPunctaFilter();
  if (filter != NULL) {
    filter->setData(m_view->getDocument()->getPunctumList());
  }
}

void ZStackDoc3dHelper::updateSurfaceData()
{
  Z3DSurfaceFilter *filter = m_view->getSurfaceFilter();
  if (filter != NULL) {
    std::vector<ZCubeArray*> all;
    TStackObjectList objList =
        m_view->getDocument()->getObjectList(ZStackObject::TYPE_3D_CUBE);
    for (TStackObjectList::const_iterator iter = objList.begin();
         iter != objList.end(); ++iter) {
      all.push_back(dynamic_cast<ZCubeArray*>(*iter));
    }
    filter->setData(all);
  }
}

void ZStackDoc3dHelper::updateData(neutube3d::ERendererLayer layer)
{
  switch (layer) {
  case neutube3d::LAYER_GRAPH:
    updateGraphData();
    break;
  case neutube3d::LAYER_SWC:
    updateSwcData();
    break;
  case neutube3d::LAYER_PUNCTA:
    updatePunctaData();
    break;
  case neutube3d::LAYER_SURFACE:
    updateSurfaceData();
    break;
  case neutube3d::LAYER_TODO:
    updateTodoData();
    break;
  case neutube3d::LAYER_MESH:
    updateMeshData();
    break;
  case neutube3d::LAYER_ROI:
    updateRoiData();
    break;
  case neutube3d::LAYER_DECORATION:
    updateDecorationData();
    break;
  case neutube3d::LAYER_SLICE:
    updateSliceData();
    break;
  default:
    break;
  }
}

void ZStackDoc3dHelper::updateCustomCanvas()
{
  ZFlyEmBody3dDoc *doc = qobject_cast<ZFlyEmBody3dDoc*>(m_view->getDocument());

  updateCustomCanvas(doc);
}

void ZStackDoc3dHelper::updateCustomCanvas(ZFlyEmBody3dDoc *doc)
{
  if (doc != NULL) {
    ZDvidGraySlice *slice = doc->getArbGraySlice();
    if (slice != NULL) {
      if (slice->isVisible()) {
        m_view->updateCustomCanvas(slice->getImage());
      }
    }
  }
}

bool ZStackDoc3dHelper::releaseObject(
    neutube3d::ERendererLayer layer, ZStackObject *obj)
{
  bool released = false;
  if (m_objectAdapter.contains(layer)) {
    auto &objList = m_objectAdapter[layer];
    released = objList.removeOne(obj);
  }

  return released;
}

ZStackDoc3dHelper* ZStackDoc3dHelper::GetDocHelper(ZStackDoc *doc)
{
  ZFlyEmBody3dDoc *cdoc = qobject_cast<ZFlyEmBody3dDoc*>(doc);
  if (cdoc != nullptr) {
    return cdoc->getHelper();
  }

  return nullptr;
}

void ZStackDoc3dHelper::Attach(ZStackDoc *doc, Z3DView *view)
{
  ZStackDoc3dHelper *helper = GetDocHelper(doc);
  if (helper) {
    helper->attach(view);
  }
}
