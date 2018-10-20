#include <QAction>
#include <QGridLayout>
#include <QMessageBox>
#include <QPushButton>
#include <QLabel>
#include <QFileDialog>
#include <QTreeView>
#include <QStandardItemModel>
#include <QStandardItem>
#include <QModelIndex>
#include <QTreeWidgetItem>
#include "flyem/zstackwatershedcontainer.h"
#include "zmultiscalesegmentationmanagement.h"
#include "zstack.hxx"
#include "zobject3dscanarray.h"
#include "zstackdoc.h"
#include "zstackframe.h"
#include "zsandbox.h"
#include "zstroke2d.h"
#include "zobject3dscan.h"
#include "zstackdocdatabuffer.h"
#include "zcolorscheme.h"
#include "zstackview.h"
#include "mainwindow.h"


//implementation for ZSegmentationScan

ZSegmentationScan::ZSegmentationScan()
{
  clearData();
}


void ZSegmentationScan::clearData()
{
  if(m_data.size() !=0 )
  {
    m_data.clear();
  }
  m_sz = MAX_INT32;
  m_ez = 0;
  m_sy = MAX_INT32;
  m_ey = 0;
  m_sx = MAX_INT32;
  m_ex = 0;
}


void ZSegmentationScan::unify(ZSegmentationScan *obj)
{

  int sz = std::min(minZ(), obj->minZ());
  int ez = std::max(maxZ(), obj->maxZ());
  int size_z = ez - sz + 1;

  int sy = std::min(minY(), obj->minY());
  int ey = std::max(maxY(), obj->maxY());
  int size_y = ey - sy + 1;

  int sx = std::min(minX(), obj->minX());
  int ex = std::max(maxX(), obj->maxX());

  std::cout<<sx<<" "<<ex<<" "<<sy<<" "<<ey<<" "<<sz<<" "<<ez<<endl;
  std::vector<std::vector<std::vector<int>>> new_data;
  new_data.resize(size_z);
  for(int z = 0; z < size_z; ++z)
  {
    new_data[z].resize(size_y);
  }

  for(int z = 0; z < size_z; ++z)
  {
    for(int y = 0; y < size_y; ++y)
    {
      new_data[z][y] = canonize(getStrip(z+sz,y+sy),obj->getStrip(z+sz,y+sy));
    }
  }

  m_data = new_data;
  m_sz = sz;
  m_ez = ez;
  m_sy = sy;
  m_ey = ey;
  m_sx = sx;
  m_ex = ex;
}


std::vector<int> ZSegmentationScan::canonize(std::vector<int> &a, std::vector<int> &b)
{
  if(a.size() == 0)
  {
    return b;
  }
  if(b.size() == 0)
  {
    return a;
  }

  std::vector<int> rv;
  std::vector<int>::iterator itas = a.begin();
  std::vector<int>::iterator itbs = b.begin();
  std::vector<int>::iterator itae = itas + 1;
  std::vector<int>::iterator itbe = itbs + 1;

  while(itae < a.end() && itbe < b.end())
  {
    int a = *itas;
    int b = *itae;
    if(*itas < *itbs)
    {
      itas += 2;
      itae += 2;
    }
    else
    {
      a = *itbs;
      b = *itbe;
      itbs += 2;
      itbe += 2;
    }
    if(rv.size() == 0)
    {
      rv.push_back(a);
      rv.push_back(b);
    }
    else
    {
      int d = rv.back();
      rv.pop_back();
      if(a <= d)
      {
        rv.push_back(b);
      }
      else
      {
        rv.push_back(d);
        rv.push_back(a);
        rv.push_back(b);
      }
    }
  }

  while(itae < a.end())
  {
    rv.push_back(*itas);
    rv.push_back(*itae);
    itas += 2;
    itae += 2;
  }
  while(itbe < b.end())
  {
    rv.push_back(*itbs);
    rv.push_back(*itbe);
    itbs += 2;
    itbe += 2;
  }
  return rv;
}


void ZSegmentationScan::preprareData(int depth, int height)
{
  //clearData();
  m_data.resize(depth);

  for(int i = 0; i < depth; ++i)
  {
    m_data[i].resize(height);
  }
}


ZIntCuboid ZSegmentationScan::getStackForegroundBoundBox(ZStack *stack)
{
  ZIntCuboid box(0,0,0,-1,-1,-1);
  if(!stack)
  {
    return box;
  }

  uint8_t *p = stack->array8();
  int width = stack->width();
  int height = stack->height();
  int depth = stack->depth();
  size_t area = width * height;

  int min_x = width, max_x = -1;
  int min_y = height, max_y = -1;
  int min_z = depth, max_z = -1;

  for(int z = 0 ; z < depth; ++z)
  {
    for(int y = 0; y < height; ++y)
    {
      uint8_t *q = p + z*area + y*width;
      for(int x = 0; x < width; ++x, ++q)
      {
        if(*q)
        {
          if(z < min_z) min_z = z;
          if(z > max_z) max_z = z;
          if(y < min_y) min_y = y;
          if(y > max_y) max_y = y;
          if(x < min_x) min_x = x;
          if(x > max_x) max_x = x;
        }
      }
    }
  }
  box.setFirstCorner(min_x,min_y,min_z);
  box.setLastCorner(max_x,max_y,max_z);
  box.translate(stack->getOffset());
  return box;
}

//int i = 1000;
void ZSegmentationScan::fromObject3DScan(ZObject3dScan *obj)
{
  ZIntCuboid box = obj->getBoundBox();
  ZStack * stack = new ZStack(GREY,box.getWidth(),box.getHeight(),box.getDepth(),1);
  stack->setOffset(obj->getBoundBox().getFirstCorner());
  obj->drawStack(stack,1);
  //stack->save((QString("/home/deli/")+QString::number(i++)+".tif").toStdString());
  fromStack(stack);
  delete stack;
}


ZObject3dScan* ZSegmentationScan::toObject3dScan()
{
  ZStack* stack = toStack();
  if(stack)
  {
    ZObject3dScan *rv = new ZObject3dScan();
    rv->loadStack(*stack);
    //stack->save("/home/deli/stack1.tif");
    delete stack;
    return rv;
  }
  return NULL;
}


ZStack* ZSegmentationScan::toStack()
{
  if(m_data.size() == 0)
  {
    return NULL;
  }
  ZIntCuboid box = getBoundBox();
  ZStack* rv = new ZStack(GREY, box.getWidth(), box.getHeight(), box.getDepth(), 1);
  rv->setOffset(box.getFirstCorner());
  labelStack(rv);
  return rv;
}


void ZSegmentationScan::fromStack(ZStack* stack)
{
  if(!stack)
  {
    return ;
  }
  clearData();
  int width = stack->width();
  int height = stack->height();
  size_t area  = width*height;

  ZIntCuboid box = getStackForegroundBoundBox(stack);

  if(box.getDepth() <= 0 || box.getHeight() <=0 || box.getWidth() <=0)
  {
    return ;
  }

  int ofx = stack->getOffset().m_x;
  int ofy = stack->getOffset().m_y;
  int ofz = stack->getOffset().m_z;
  m_offset = stack->getOffset();

  m_sz = box.getFirstCorner().m_z;
  m_sy = box.getFirstCorner().m_y;
  m_sx = box.getFirstCorner().m_x;
  m_ez = box.getLastCorner().m_z;
  m_ey = box.getLastCorner().m_y;
  m_ex = box.getLastCorner().m_x;

  preprareData(box.getDepth(), box.getHeight());

  uint8_t* p = stack->array8();

  for(int k = m_sz; k <= m_ez; ++k)
  {
    for(int j = m_sy; j <= m_ey; ++j)
    {
      uint8_t * q = p + (k-ofz)*area + (j-ofy)*width + (m_sx-ofx);
      int sx = -1, ex = -1;
      int flag = 0;
      for(int i = m_sx; i <= m_ex; ++i, ++q)
      {
        if(*q && (flag == 0))
        {
          sx = i;
          flag = 1;
        }
        else if((!*q) && (flag ==1))
        {
          ex = i -1;
          flag = 0;
          addSegment(k,j,sx,ex);
        }
      }
      if(flag ==1)
      {
        addSegment(k,j,sx,m_ex);
      }
    }
  }
}


void ZSegmentationScan::labelStack(ZStack *stack, int v)
{
  if(!stack)
  {
    return ;
  }

  uint8_t *p = stack->array8();
  int width = stack->width();
  int height = stack->height();
  int area = width * height;

  int ofx = stack->getOffset().m_x;//m_offset.m_x;
  int ofy = stack->getOffset().m_y;//m_offset.m_y;
  int ofz = stack->getOffset().m_z;//m_offset.m_z;

  for(uint z = 0; z < m_data.size(); ++z)
  {
    std::vector<std::vector<int>>& dy = m_data[z];
    for(uint y = 0; y < dy.size(); ++y)
    {
      std::vector<int>& dx = dy[y];
      uint8_t *q = p + (z + m_sz - ofz)*area + (y + m_sy - ofy)*width;
      for(uint x = 0; x < dx.size(); x += 2)
      {
        int sx = dx[x] - ofx;
        int ex = dx[x+1] - ofx;
        uint8_t* pd = q + sx;
        for(int i = sx; i <= ex; ++i)
        {
          *pd++ = v;
        }
      }
    }
  }
}


void ZSegmentationScan::maskStack(ZStack *stack)
{
  if(!stack)
  {
    return ;
  }

  int width = stack->width();
  int height = stack->height();
  int depth = stack->depth();

  ZStack* label = new ZStack(GREY, width, height, depth, 1);
  label->setOffset(stack->getOffset());
  labelStack(label);

  uint8_t *p = label->array8();
  uint8_t *pend = p + label->getVoxelNumber();
  uint8_t *q = stack->array8();

  for(; p != pend; ++p, ++q)
  {
    *q = (*q) * (*p);
  }

  delete label;
}


void ZSegmentationScan::addSegment(int z, int y, int sx, int ex)
{
  getStrip(z,y).push_back(sx);
  getStrip(z,y).push_back(ex);
}


void ZSegmentationScan::translate(ZIntPoint offset)
{
  translate(offset.getX(), offset.getY(), offset.getZ());
}


void ZSegmentationScan::translate(int ofx, int ofy, int ofz)
{
  for(uint z = 0; z < m_data.size(); ++z)
  {
    std::vector<std::vector<int>>& slice = m_data[z];
    for(uint y = 0; y < slice.size(); ++y)
    {
      std::vector<int>& strip = slice[y];
      for(uint x = 0; x < strip.size(); ++x)
      {
        strip[x] += ofx;
      }
    }
  }
  m_sx += ofx;
  m_ex += ofx;
  m_sy += ofy;
  m_ey += ofy;
  m_sz += ofz;
  m_ez += ofz;
}


//implementation for ZSegmentationNode
void ZSegmentationNode::updateChildrenLabel()
{
  int i=1;
  for(auto it = m_children.begin(); it!=m_children.end();++it)
  {
    ZSegmentationNode* child = *it;
    if(this->isRoot())
    {
      child->setLabel(QString::number(i));
    }
    else
    {
      child->setLabel(m_label+"."+QString::number(i));
    }
    child->updateChildrenLabel();
    ++i;
  }
}


void ZSegmentationNode::consumeSegmentations(std::vector<ZSegmentationScan*> &segmentations)
{
  if(segmentations.size() == 0)
  {
    return;
  }

  if(segmentations.size() == 1)
  {
    if(m_data)
    {
      delete m_data;
      m_data = NULL;
    }
    m_data = segmentations[0];
    return;
  }

  this->clearChildren();

  if(m_data)
  {
    delete m_data;
    m_data = NULL;
  }

  for (std::vector<ZSegmentationScan*>::iterator iter = segmentations.begin();iter != segmentations.end(); ++iter)
  {
    ZSegmentationScan *obj = *iter;
    ZSegmentationNode* child = new ZSegmentationNode();
    child->setParent(this);
    child->setData(obj);
    m_children.push_back(child);
  }
  this->updateChildrenLabel();
}


void ZSegmentationNode::removeChild(ZSegmentationNode *child, bool b_delete)
{
  for(auto it = m_children.begin(); it!=m_children.end();++it)
  {
    ZSegmentationNode* node = *it;
    if(child == node)
    {
      m_children.erase(it);
      child->setParent(NULL);
      if(b_delete)
      {
        node->destroy();
      }
      break;
    }
  }
}


void ZSegmentationNode::clearChildren()
{
  for(auto it = m_children.begin(); it!=m_children.end();++it)
  {
    ZSegmentationNode* node = *it;
    node->destroy();
  }
  m_children.clear();
}


int ZSegmentationNode::indexOf(ZSegmentationNode *node)
{
  for(uint i=0;i<m_children.size();++i)
  {
    if(m_children[i] == node)
    {
      return i;
    }
  }
  return -1;
}


ZSegmentationNode* ZSegmentationNode::find(QString label)
{
  if(m_label == label)
  {
    return this;
  }
  for(auto it = m_children.begin(); it != m_children.end(); ++it)
  {
    ZSegmentationNode* child = *it;
    ZSegmentationNode* rv = child->find(label);
    if(rv)
    {
      return rv;
    }
  }
  return NULL;
}


void ZSegmentationNode::makeMask(ZSegmentationScan* mask)
{
  if(this->isLeaf())
  {
    if(m_data)
    {
      //std::cout<<m_label.toStdString()<<std::endl;
      //std::cout<<m_data->minX()<<" "<<m_data->maxX()<<std::endl;
      std::cout<<"Begin unify"<<std::endl;
      mask->unify(m_data);
      std::cout<<"End unify"<<std::endl;
    }
    return;
  }

  for(auto it = m_children.begin(); it!=m_children.end(); ++it)
  {
    ZSegmentationNode* child = *it;
    child->makeMask(mask);
  }
}


void ZSegmentationNode::mergeNode(ZSegmentationNode *node)
{
  if(this == node)
  {
    return;
  }
  if(this->isLeaf())
  {
    ZSegmentationScan* data =new ZSegmentationScan();
    node->makeMask(data);
    if(this->m_data)
    {
      this->m_data->unify(data);
      delete data;
    }
    else
    {
      m_data = data;
    }
    node->destroy();
  }
  else
  {
    if(node->parent())
    {
      node->parent()->removeChild(node);
    }
    this->appendChild(node);
  }
  updateChildrenLabel();

}


void ZSegmentationNode::regularize()
{
  for(auto it = m_children.begin();it!=m_children.end();++it)
  {
    ZSegmentationNode* child = *it;
    child->regularize();
  }
  if(this->m_children.size() == 1)
  {
    if(!this->isRoot())
    {
      ZSegmentationNode* parent = this->parent();
      parent->children()[parent->indexOf(this)] = m_children[0];
      m_children[0]->setParent(parent);
      this->m_children.clear();
      this->m_parent = NULL;
      this->destroy();
    }
  }
}


void ZSegmentationNode::destroy()
{
  if(m_data)
  {
    delete m_data;
    m_data = NULL;
  }
  if(m_parent)
  {
    m_parent->removeChild(this);
  }
  while(m_children.size() > 0)
  {
    m_children[0]->destroy();
  }
  m_children.clear();
}


void ZSegmentationNode::splitNode(ZStack *stack, std::vector<ZStackObject *> &seeds)
{

  ZStack* stack_valid = NULL;
  if(this->isRoot())
  {
    stack_valid = stack;
  }
  else
  {
    ZSegmentationScan* mask = new ZSegmentationScan();
    this->makeMask(mask);
    ZIntCuboid box = mask->getBoundBox();
    stack_valid = stack->makeCrop(box);
    //stack_valid->save("/home/deli/stack_valid.tif");
    //mask->translate(-stack->getOffset());
    mask->maskStack(stack_valid);
    //stack_valid->save("/home/deli/stack_valid2.tif");
    delete mask;
  }

  ZStackWatershedContainer* container = new ZStackWatershedContainer(stack_valid);
  for(auto it= seeds.begin();it!=seeds.end();++it )
  {
    container->addSeed(*it);
  }
  container->setAlgorithm("watershed");
  container->setScale(estimateScale(stack_valid->getVoxelNumber()));
  container->setDsMethod("Min(ignore zero)");
  container->run();

  std::vector<ZSegmentationScan*> scan_array;

  ZObject3dScanArray result;
  container->makeSplitResult(1, &result);

  for(auto it = result.begin(); it != result.end(); ++it)// here
  {
    ZObject3dScan* obj = *it;
    ZSegmentationScan * scan = new ZSegmentationScan();
    scan->fromObject3DScan(obj);
    scan_array.push_back(scan);
  }

  this->consumeSegmentations(scan_array);

  result.shallowClear();

  if(!this->isRoot())
  {
    delete stack_valid;
  }
  delete container;
}


int ZSegmentationNode::estimateScale(size_t volume)
{
  int scale = 1;
  if(volume >= 1024*1024*1024)
  {
    scale = 4;
  }
  else if(volume >= 512*512*512)
  {
    scale = 2;
  }
  return scale;
}


void ZSegmentationNode::display(QStandardItem* tree)
{
  QStandardItem* current = new QStandardItem(m_label);

  current->setDragEnabled(true);
  current->setDropEnabled(true);

  tree->appendRow(current);

  for(auto it = m_children.begin(); it!=m_children.end(); ++it)
  {
    ZSegmentationNode* child = *it;
    child->display(current);
  }
}


//implementation for ZTreeView
void ZTreeView::dropEvent(QDropEvent *event)
{
  QStandardItemModel* tree = static_cast<QStandardItemModel*>(this->model());

  QStandardItem* selected = tree->itemFromIndex(this->currentIndex());
  QStandardItem* old_parent = selected->parent();
  QString text = selected->text();

  QTreeView::dropEvent(event);

  //remove it from old parent
  old_parent->removeRow(selected->row());

  ZMultiscaleSegmentationWindow* window=  static_cast<ZMultiscaleSegmentationWindow*>(this->parent());

  QStandardItem* new_parent = window->findItemByText(tree->invisibleRootItem(),text)->parent();

  window->moveNode(text, new_parent->text());
}


//implementation for ZMultiscaleSegmentationWindow
ZMultiscaleSegmentationWindow::ZMultiscaleSegmentationWindow(QWidget *parent) :
  QWidget(parent)
{
  init();
}


void ZMultiscaleSegmentationWindow::init()
{

  m_frame = NULL;
  m_stack = NULL;
  m_root = NULL;
  initWidgets();
}


ZMultiscaleSegmentationWindow::~ZMultiscaleSegmentationWindow()
{
  if(m_root)
  {
    delete m_root;
  }
}


void ZMultiscaleSegmentationWindow::initWidgets()
{
  this->setWindowTitle("Multiscale Segmentation Project");
  Qt::WindowFlags flags = this->windowFlags();
  flags |= Qt::WindowStaysOnTopHint;
  this->setWindowFlags(flags);

  //create widgets
  m_tree = new QStandardItemModel(this);
  m_tree_view = new ZTreeView(this);
  m_tree_view->setModel(m_tree);
  m_tree_view->setDragEnabled(true);
  m_tree_view->setAcceptDrops(true);
  m_tree->setHorizontalHeaderLabels(QStringList()<<"Multiscale Segmentations:");
  m_tree->invisibleRootItem()->setEditable(true);
  m_tree->invisibleRootItem()->setText("0");

  QPushButton* open_stack = new QPushButton("Open Stack");
  QPushButton* clear = new QPushButton("Clear");
  QPushButton* segment = new QPushButton("Segment");
  QPushButton* export_node = new QPushButton("Export");

  //setup layout
  QGridLayout* lay=new QGridLayout(this);
  lay->addWidget(m_tree_view,0,0,9,12);
  lay->addWidget(open_stack,9,0,1,3);
  lay->addWidget(clear,9,3,1,3);
  lay->addWidget(export_node,9,6,1,3);
  lay->addWidget(segment,9,9,1,3);


  //events
  connect(m_tree_view,SIGNAL(clicked(QModelIndex)),this,SLOT(onSelectNode(QModelIndex)));
  connect(open_stack,SIGNAL(clicked()),this,SLOT(onOpenStack()));
  connect(clear,SIGNAL(clicked()),this,SLOT(onClear()));
  connect(export_node,SIGNAL(clicked()),this,SLOT(onExport()));
  connect(segment,SIGNAL(clicked()),this,SLOT(onSegment()));

  this->setLayout(lay);
  this->setMinimumSize(400,600);
  this->move(300,200);
}


void ZMultiscaleSegmentationWindow::onSelectNode(QModelIndex index)
{
  if(!m_root || !m_frame)
  {
    return ;
  }
  m_frame->document()->removeObject(ZStackObjectRole::ROLE_SEGMENTATION,true);
  ZSegmentationNode* node = m_root->find(m_tree->itemFromIndex(index)->text());
  highLight(node);
}


void ZMultiscaleSegmentationWindow::onSegment()
{
  if(!m_frame || !m_stack)
  {
    return;
  }

  QStandardItem* selected_item = getSelectedNodeItem();
  QString text = selected_item->text();
  //mask segmentation stack
  ZSegmentationNode* selected_node = m_root->find(selected_item->text());

  std::vector<ZStackObject*> seeds = getSeeds();

  std::cout<<"Begin Splitting"<<std::endl;

  selected_node->splitNode(m_stack, seeds);

  std::cout<<"Splitting finished"<<std::endl;

  //display tree in UI
  clearTreeView();
  m_root->display(m_tree->invisibleRootItem());
  m_tree_view->setCurrentIndex(findItemByText(m_tree->invisibleRootItem(),text)->index());
  m_tree_view->expandAll();

  int i = 1;
  ZColorScheme scheme;
  scheme.setColorScheme(ZColorScheme::UNIQUE_COLOR);
  m_frame->document()->removeObject(ZStackObjectRole::ROLE_SEGMENTATION,true);

  std::cout<<"Begin hilighting"<<std::endl;
  highLight(selected_node,scheme.getColor(0));

  for(auto it = selected_node->children().begin(); it != selected_node->children().end(); ++it, ++i)
  {
    highLight(*it, scheme.getColor(i));
  }
  std::cout<<"Highliting finished"<<std::endl;

  removeSeeds();
}


void ZMultiscaleSegmentationWindow::onOpenStack()
{
  QString file_name = QFileDialog::getOpenFileName(this,"Open Stack","","tiff(*.tif)");
  if (file_name!="")
  {
    if(m_frame)
    {
      m_frame->close();
      m_frame = NULL;
    }
    if(m_root)
    {
      delete m_root;
      m_root = NULL;
    }

    m_stack = new ZStack();
    m_stack->load(file_name.toStdString());
    m_frame = ZSandbox::GetMainWindow()->createStackFrame(m_stack);
    ZSandbox::GetMainWindow()->addStackFrame(m_frame);
    ZSandbox::GetMainWindow()->presentStackFrame(m_frame);

    m_root = new ZSegmentationNode();
    m_root->setLabel("0");
    m_root->setParent(NULL);
    ZSegmentationScan* data = new ZSegmentationScan();
    data->fromStack(m_stack);
    //data->maskStack(m_stack);
    //m_stack->save("/home/deli/stack.tif");
    m_root->setData(data);
  }
}


void ZMultiscaleSegmentationWindow::onExport()
{
  if(!m_root || !m_stack)
  {
    return ;
  }
/*
  QStandardItem* item = this->getSelectedNodeItem();
  ZStack* result = new ZStack(m_stack->kind(),m_stack->width(),m_stack->height(),m_stack->depth(),m_stack->channelNumber());

  ZObject3dScan* mask = new ZObject3dScan();
  ZSegmentationNode* node = m_root->find(item->text());
  node->makeMask(mask);
  mask->translate(-m_stack->getOffset());
  mask->labelStack(result->c_stack(),1);
  result->save(QFileDialog::getSaveFileName(this,"Export Segmentation","","Tiff file(*.tif)").toStdString());
  //mask->getComplementObject().maskStack(m_stack);
  //m_frame->update();
  mask->labelStack(m_stack->c_stack(),0);
  //m_frame->scroll(1,1);
  node->destroy();
  m_root->regularize();
  m_root->updateChildrenLabel();
  clearTreeView();
  m_root->display(m_tree->invisibleRootItem());
  m_tree_view->expandAll();
  delete mask;
  delete result;*/
}


void ZMultiscaleSegmentationWindow::onClear()
{
  if(m_root)
  {
    m_root->destroy();
  }
  if(m_frame)
  {
    m_frame->close();
  }
  clearTreeView();

  m_frame = NULL;
  m_stack = NULL;
  m_root = NULL;
}


void ZMultiscaleSegmentationWindow::highLight(ZSegmentationNode *node, QColor color)
{
  ZSegmentationScan* scan = new ZSegmentationScan();

  std::cout<<"Begin masking"<<std::endl;
  node->makeMask(scan);
  std::cout<<"End masking"<<std::endl;

  std::cout<<"To Object3dScan"<<std::endl;
  ZObject3dScan *mask = scan->toObject3dScan();//here
  std::cout<<"To Object3dScan finished"<<std::endl;
  delete scan;
  mask->setColor(color);
  mask->addRole(ZStackObjectRole::ROLE_SEGMENTATION);
  m_frame->document()->getDataBuffer()->addUpdate(mask, ZStackDocObjectUpdate::ACTION_ADD_NONUNIQUE);
}


QStandardItem* ZMultiscaleSegmentationWindow::findItemByText(QStandardItem *root, QString text)
{
  if(root->text() == text)
  {
    return root;
  }
  for(int i=0;i<root->rowCount();++i)
  {
    QStandardItem* rv = findItemByText(root->child(i),text);
    if(rv)
    {
      return rv;
    }
  }
  return NULL;
}


void ZMultiscaleSegmentationWindow::moveNode(QString label, QString new_parent_label)
{

  ZSegmentationNode* node = m_root->find(label);
  ZSegmentationNode* new_parent = m_root->find(new_parent_label);

  new_parent->mergeNode(node);

  m_root->regularize();
  m_root->updateChildrenLabel();
  clearTreeView();
  m_root->display(m_tree->invisibleRootItem());

  m_tree_view->setCurrentIndex(findItemByText(m_tree->invisibleRootItem(),new_parent->label())->index());
  m_tree_view->expandAll();
}


QStandardItem* ZMultiscaleSegmentationWindow::getSelectedNodeItem()
{
  QModelIndex index = m_tree_view->currentIndex();
  return index.isValid() ? m_tree->itemFromIndex(index) : m_tree->invisibleRootItem();
}


void ZMultiscaleSegmentationWindow::clearTreeView()
{
  if(m_tree)
  {
    for(int i=0;i<m_tree->invisibleRootItem()->rowCount();++i)
    {
      m_tree->invisibleRootItem()->removeRow(i);
    }
  }
}


void ZMultiscaleSegmentationWindow::removeSeeds()
{
  if(!m_frame)
  {
    return ;
  }
  for(ZStroke2d* stroke:m_frame->document()->getStrokeList())
  {
    m_frame->document()->removeObject(stroke);
  }
}


std::vector<ZStackObject*> ZMultiscaleSegmentationWindow::getSeeds()
{
  if(!m_frame)
  {
    return std::vector<ZStackObject*>();
  }

  std::vector<ZStackObject*> seeds;
  std::map<QString,int> color_indices;
  int i = 0;

  for(ZStroke2d* stroke:m_frame->document()->getStrokeList())
  {
      std::map<QString,int>::iterator p_index = color_indices.find(stroke->getColor().name());
      if (p_index==color_indices.end())
      {
          color_indices.insert(std::make_pair(stroke->getColor().name(),++i));
          stroke->setLabel(i);
      }
      else
      {
          stroke->setLabel(p_index->second);
      }
      seeds.push_back(stroke);
  }
  return seeds;
}


//implementation for ZMultiscaleSegmentationManagementModule
ZMultiscaleSegManagementModule::ZMultiscaleSegManagementModule(QObject* parent):
ZSandboxModule(parent)
{
  init();
}


ZMultiscaleSegManagementModule::~ZMultiscaleSegManagementModule()
{
  delete m_window;
}


void ZMultiscaleSegManagementModule::init()
{
  m_action = new QAction("Multiscale Segmentation", this);
  connect(m_action, SIGNAL(triggered()), this, SLOT(execute()));
  m_window=new ZMultiscaleSegmentationWindow();
}


void ZMultiscaleSegManagementModule::execute()
{
  m_window->show();
}

