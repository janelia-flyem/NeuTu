#include <QAction>
#include <QMessageBox>
#include "zsurfreconmodule.h"
#include "zsandbox.h"
#include "mainwindow.h"
/*
#undef ASCII
#undef BOOL
#undef TRUE
#undef FALSE
#include "imgproc/surfrecon.h"
*/
ZSurfReconModule::ZSurfReconModule(QObject *parent) :
  ZSandboxModule(parent)
{
  init();
}

void ZSurfReconModule::init()
{
  m_action = new QAction("SurfRecon", this);
  connect(m_action, SIGNAL(triggered()), this, SLOT(execute()));
}

void ZSurfReconModule::execute()
{
  /*
  ZStackDoc *doc =ZSandbox::GetCurrentDoc();
  if(!doc)return;
  ZStack  *src=doc->getStack();
  if(!src)return;
  QList<ZSwcTree*> trees=doc->getSwcList();

  Surf surface;
  VoxelSet in;

  for(QList<ZSwcTree*>::iterator it=trees.begin();it!=trees.end();++it)
  {
    ZSwcTree::DepthFirstIterator iter(*it);
    while(iter.hasNext())
    {
      Swc_Tree_Node* tn=iter.next();
      if(!SwcTreeNode::isVirtual(tn))
      {
        in.push_back(VoxelType(SwcTreeNode::x(tn),SwcTreeNode::y(tn),SwcTreeNode::z(tn)));
      }
    }
  }

  VoxelSet out;

  std::cout<<"input point cloud size:"<<in.size()<<std::endl;
  surface.surfrecon(in,out,26,1);
  std::cout<<"output point cloud size:"<<out.size()<<std::endl;

  int width=src->width(),height=src->height(),area=width*height;
  int ofx=src->getOffset().m_x,ofy=src->getOffset().m_y,ofz=src->getOffset().m_z;

  ZStack* clone=src->clone();
  uint8_t* p=clone->array8();

  size_t max_off=src->getVoxelNumber();
  for(uint i=0;i<out.size();++i)
  {
    const VoxelType& v=out[i];
    size_t offset=int(v.x-ofx)+int(v.y-ofy)*width+int(v.z-ofz)*area;
    if(offset<max_off && offset>0)
      p[offset]=0;
  }

  ZStackFrame *frame=ZSandbox::GetMainWindow()->createStackFrame(clone);
  ZSandbox::GetMainWindow()->addStackFrame(frame);
  ZSandbox::GetMainWindow()->presentStackFrame(frame);*/
}
