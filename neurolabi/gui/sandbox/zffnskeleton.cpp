#include "zffnskeleton.h"
#include <map>
#include <algorithm>
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
#include "widgets/zpythonprocess.h"
#include "zobject3dfactory.h"
#include "swc/zswcpruner.h"

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
  m_src=NULL;
  m_resolution=6;
}

std::vector<ZIntPoint> ZFFNSkeletonModule::getSeedPos(ZStackDoc* doc)
{
  std::vector<ZIntPoint> points;
  ZIntPoint offset=m_src->getOffset();
  for(ZStroke2d* stroke:doc->getStrokeList()){
    ZPoint center_=stroke->getBoundBox().center();
    ZIntPoint center(center_.x(),center_.y(),center_.z());
    points.push_back(center-offset);
  }
  return points;
}


ZStack* ZFFNSkeletonModule::getFFNSegmentation(ZIntPoint start_pos,ZIntPoint end_pos,ZIntPoint seed_pos)
{
  const QString working_dir=QCoreApplication::applicationDirPath()+"/../python/service/ffn";
  ZPythonProcess python;
  python.setWorkDir(working_dir);
  python.setScript(working_dir+"/ffn_skeleton.py");

  //start_pos
  python.addArg(QString::number(start_pos.getX()));
  python.addArg(QString::number(start_pos.getY()));
  python.addArg(QString::number(start_pos.getZ()));

  //size
  python.addArg(QString::number(end_pos.getX()-start_pos.getX()));
  python.addArg(QString::number(end_pos.getY()-start_pos.getY()));
  python.addArg(QString::number(end_pos.getZ()-start_pos.getZ()));

  //seed
  python.addArg(QString::number(seed_pos.getX()-start_pos.getX()));
  python.addArg(QString::number(seed_pos.getY()-start_pos.getY()));
  python.addArg(QString::number(seed_pos.getZ()-start_pos.getZ()));
  python.addArg(working_dir+"/result.tif");
  ZStack* rv=new ZStack();
  python.run();
  rv->load(working_dir.toStdString()+"/result.tif");
  return rv;
}


void ZFFNSkeletonModule::getMask(std::set<ZIntPoint>& mask,ZStack* obj_mask)
{
  uint8_t* p=obj_mask->array8();
  int width=obj_mask->width(),height=obj_mask->height(),depth=obj_mask->depth();
  size_t area=width*height;
  ZIntPoint offset=obj_mask->getOffset();
  for(int k=0;k<depth;++k){
    for(int j=0;j<height;++j){
      for(int i=0;i<width;++i){
        if(p[i+j*width+k*area])
        {
          mask.insert((ZIntPoint(i,j,k)+offset)/m_resolution);
        }
      }
    }
  }
}


void ZFFNSkeletonModule::connectSwcTree(ZSwcTree* first,ZSwcTree* second){
  first->updateIterator();
  second->updateIterator();

  Swc_Tree_Node* a=NULL, *b=NULL;
  double min_dist=MAX_INT32;
  for(Swc_Tree_Node* na=first->begin();na!=first->end();na=first->next()){
    if(Swc_Tree_Node_Is_Regular(na)){
      double x1=na->node.x,y1=na->node.y,z1=na->node.z;
      for(Swc_Tree_Node* nb=second->begin();nb!=second->end();nb=second->next()){
        if(Swc_Tree_Node_Is_Regular(nb)){
          double x2=nb->node.x,y2=nb->node.y,z2=nb->node.z;
          double dist=(x1-x2)*(x1-x2)+(y1-y2)*(y1-y2)+(z1-z2)*(z1-z2);
          if( dist  <min_dist ){
            min_dist= dist;
            a=na;
            b=nb;
          }
        }
      }
    }
  }
  if(a && b){
    Swc_Tree_Node_Set_Root(b);
    Swc_Tree_Node_Add_Child(a,b);
    //Swc_Tree_Node_Set_Parent(b,a);
  }
}


void ZFFNSkeletonModule::trace(const ZIntPoint& seed_pos)
{
  int width=m_src->width(),height=m_src->height(),depth=m_src->depth();
  ZIntPoint start_pos(std::max(0,seed_pos.getX()-150),std::max(0,seed_pos.getY()-150),std::max(0,seed_pos.getZ()-150));
  ZIntPoint end_pos(std::min(width-1,seed_pos.getX()+150),std::min(height-1,seed_pos.getY()+150),std::min(depth-1,seed_pos.getZ()+150));
  ZStack* obj_mask=getFFNSegmentation(start_pos,end_pos,seed_pos);

  obj_mask->setOffset(m_src->getOffset()+start_pos);

  std::set<ZIntPoint>* mask=new std::set<ZIntPoint>();
  getMask(*mask,obj_mask);

  static ZStack zero_stack(GREY,m_resolution,m_resolution,m_resolution,1);
  std::vector<int> indices;
  for(int i=0;i<m_segs.size();++i){
    std::vector<ZIntPoint> common;
    std::set<ZIntPoint>* seg_mask=m_segs[i];
    if(seg_mask!=NULL)
    {
      std::set_intersection(mask->begin(),mask->end(),seg_mask->begin(),seg_mask->end(),std::inserter(common,common.begin()));
      if(common.size()>10){
        indices.push_back(i);
        for(auto pos:common){
          obj_mask->setBlockValue(pos.m_x*m_resolution,pos.m_y*m_resolution,pos.m_z*m_resolution,&zero_stack);
        }
        std::set_union(mask->begin(),mask->end(),seg_mask->begin(),seg_mask->end(),std::inserter(*mask,mask->begin()));
      }
    }
  }

  ZStackSkeletonizer skeletonizer;
  ZSwcTree* skeleton=skeletonizer.makeSkeleton(*obj_mask);

  //ZSwcPruner pruner;
  //pruner.setMinLength(50);
  //pruner.setRemovingOrphan(false);
  //pruner.prune(skeleton);

  if(indices.size()!=0){
    for(int i=0;i<indices.size();++i){
      int index=indices[i];
      connectSwcTree(skeleton,m_skeleton[index]);
      //Swc_Tree_Connect_Branch(skeleton->data(),m_skeleton[index]->begin());
      m_doc->removeObject(m_skeleton[index]);
      //delete m_skeleton[index];
      m_skeleton[index]=NULL;
      delete m_segs[index];
      m_segs[index]=NULL;
   }
  }
  m_skeleton.push_back(skeleton);
  m_segs.push_back(mask);
  m_doc->addSwcTree(skeleton,true,false);
  //clear seed
  m_doc->removeObject(ZStackObjectRole::ROLE_SEED,true);
  int cnt=0;
  for(int i=0;i<m_skeleton.size();++i){
    if(m_skeleton[i]!=NULL)
      cnt++;
  }
  std::cout<<"current skeleton cnt: "<<cnt<<std::endl;

  std::vector<ZObject3dScan*> objArray =ZObject3dFactory::MakeObject3dScanPointerArray(*obj_mask, 1, true);
  ZColorScheme colorScheme;
  colorScheme.setColorScheme(ZColorScheme::UNIQUE_COLOR);
  static int colorIndex = 0;

  for (std::vector<ZObject3dScan*>::iterator iter = objArray.begin();
       iter != objArray.end(); ++iter)
  {
    ZObject3dScan *obj = *iter;
    if (obj != NULL && !obj->isEmpty())
    {
      QColor color = colorScheme.getColor(colorIndex);
      color.setAlpha(164);
      obj->setColor(color);
      m_doc->getDataBuffer()->addUpdate(obj,ZStackDocObjectUpdate::ACTION_ADD_UNIQUE);
      m_doc->getDataBuffer()->deliver();
    }
  }
  delete obj_mask;
}


void ZFFNSkeletonModule::execute()
{
  ZStackDoc *doc =ZSandbox::GetCurrentDoc();
  if(!doc)return;
  ZStack  *src=doc->getStack();
  if(!src)return;

  if(src!=m_src){
    for(auto x:m_segs){
      if(x!=NULL)
      delete x;
    }
    /*for(auto x:m_skeleton){
      if(x!=NULL)
      delete x;
    }*/
    m_segs.clear();
    m_skeleton.clear();
    m_src=src;
    m_doc=doc;
  }
  //doc->removeObject(ZStackObjectRole::ROLE_SEGMENTATION);
  //substract offset
  std::vector<ZIntPoint> seed_poses=getSeedPos(doc);

  for(auto x:seed_poses){
    trace(x);
  }

  //
  /*ZStackFrame* frame=ZSandbox::GetMainWindow()->createStackFrame(rv);
  ZSandbox::GetMainWindow()->addStackFrame(frame);
  ZSandbox::GetMainWindow()->presentStackFrame(frame);*/
}

