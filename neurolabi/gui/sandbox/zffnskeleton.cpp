#include "zffnskeleton.h"
#include <map>
#include <QAction>
#include <QMessageBox>
#include "zsandbox.h"
#include "zstackdoc.h"
#include "mainwindow.h"
#include "zstroke2d.h"
#include "zobject3dscanarray.h"
#include "zstackdocdatabuffer.h"
#include "zstackskeletonizer.h"
#include "flyem/zstackwatershedcontainer.h"

ZFFNSkeletonModule::ZFFNSkeletonModule(QObject *parent) :
  ZSandboxModule(parent)
{
  init();
}

void ZFFNSkeletonModule::init()
{
  QAction *action = new QAction("FFNSkeleton", this);
  connect(action, SIGNAL(triggered()), this, SLOT(execute()));

  setAction(action);
}

void ZFFNSkeletonModule::execute()
{
  ZStackDoc *doc =ZSandbox::GetCurrentDoc();
  if(!doc)return;
  ZStack  *src=doc->getStack();
  if(!src)return;

  //doc->removeObject(ZStackObjectRole::ROLE_SEGMENTATION);

  //ZStack* src_cp=src->clone();

  /*for(auto x:m_segs){
    for(uint8_t *p=src_cp->array8(), *q=x->array8();
        p!=src_cp->array8()+src_cp->getVoxelNumber();
        ++p,++q){
      if(*q)*p=0;
    }
  }*/

  ZStackWatershedContainer container(src);

  for(ZStroke2d* stroke:doc->getStrokeList()){
      stroke->setLabel(1);
      container.addSeed(*stroke);
  }
  container.setScale(1);
  container.setAlgorithm("FFN");
  container.run();
  ZObject3dScanArray result;
  container.makeSplitResult(1, &result);
  for (ZObject3dScanArray::iterator iter = result.begin();
       iter != result.end(); ++iter) {
    ZObject3dScan *obj = *iter;
    doc->getDataBuffer()->addUpdate(
          obj, ZStackDocObjectUpdate::ACTION_ADD_NONUNIQUE);
  }
  doc->getDataBuffer()->deliver();
  result.shallowClear();

  ZStack* obj_mask=container.getResultStack()->clone();

  for(auto x:m_segs){
      for(uint8_t *p=obj_mask->array8(), *q=x->array8();
          p!=obj_mask->array8()+obj_mask->getVoxelNumber();
          ++p,++q){
        if(*q)*p=0;
      }
  }

  ZStackSkeletonizer skeletonizer;

  ZSwcTree* rv=skeletonizer.makeSkeleton(*obj_mask);
  //doc->addSwcTree(rv,true,false);
  if(rv!=NULL)
  {
    m_segs.push_back(obj_mask);
    if(m_skeleton){
      rv->updateIterator();
      Swc_Tree_Connect_Branch(m_skeleton->data(),rv->begin());
    }
    else{
      m_skeleton=rv;
      doc->addSwcTree(m_skeleton,true,false);
    }
  }
  std::cout<<m_skeleton->size();
  /*std::vector<Swc_Tree_Node*> node_list1,node_list2;
  std::vector<double> distance;

  rv->updateIterator();
  for(auto node=rv->begin();node!=rv->end();node=rv->next()){
    if(Swc_Tree_Node_Is_Leaf(node) || Swc_Tree_Node_Is_Regular_Root(node)){//leaf or regular root
    //if(Swc_Tree_Node_Is_Regular(node)){
      double x1=node->node.x,y1=node->node.y,z1=node->node.z,r1=node->node.id;
      m_skeleton->updateIterator();
      for(auto skeleton_node=skeleton->begin();skeleton_node!=skeleton->end();skeleton_node=skeleton->next()){
        if(Swc_Tree_Node_Is_Regular(node)){
          double x2=skeleton_node->node.x,y2=skeleton_node->node.y,z2=skeleton_node->node.z,r2=skeleton_node->node.id;
          double dis=(x1-x2)*(x1-x2)+(y1-y2)*(y1-y2)+(z1-z2)*(z1-z2);
          node_list1.push_back(node);
          node_list2.push_back(skeleton_node);
          distance.push_back(dis);
        }
      }
    }
  }
  double min=distance[0];
  int index=0;
  for(int i=1;i<distance.size();++i){
    if(distance[i]<min){
      min=distance[i];
      index=i;
    }
  }
  if(Swc_Tree_Node_Is_Leaf(node_list1[index])){
    Swc_Tree_Connect_Branch()
  }*/
  /*ZStackFrame* frame=ZSandbox::GetMainWindow()->createStackFrame(container.getResultStack()->clone());
  ZSandbox::GetMainWindow()->addStackFrame(frame);
  ZSandbox::GetMainWindow()->presentStackFrame(frame);*/
}

