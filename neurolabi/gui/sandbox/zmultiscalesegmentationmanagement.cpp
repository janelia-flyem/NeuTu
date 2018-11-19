#include <QStandardItem>
#include <QPushButton>
#include <QLabel>
#include <QAction>
#include <QString>
#include <QLayout>
#include <QCheckBox>
#include <QSpinBox>
#include "zmultiscalesegmentationmanagement.h"
#include "zstackobject.h"
#include "flyem/zstackwatershedcontainer.h"
#include "zobject3dscanarray.h"
#include "zstackframe.h"
#include "zsandbox.h"
#include "zstackdocdatabuffer.h"
#include "imgproc/zstackprocessor.h"
#include "zstackdoc.h"
#include "mainwindow.h"


//implementation for ZSegmentationNode
int ZSegmentationNode::s_id = 0;


void ZSegmentationNode::consumeSegmentations(std::vector<ZSegmentationScan*> &segmentations)
{
  if(segmentations.size() <= 1)
  {
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
  //for(auto it = m_children.begin(); it!=m_children.end();++it)
  while(m_children.size() > 0)
  {
    ZSegmentationNode* node = m_children[0];
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


std::vector<ZSegmentationNode*> ZSegmentationNode::getLeaves()
{
  std::vector<ZSegmentationNode*> rv;
  collectLeaves(rv);
  return rv;
}


void ZSegmentationNode::collectLeaves(std::vector<ZSegmentationNode*>& rv)
{
  if(this->isLeaf())
  {
    rv.push_back(this);
    return ;
  }
  for(auto child: children())
  {
    child->collectLeaves(rv);
  }
}


ZSegmentationNode* ZSegmentationNode::find(int id)
{
  if(m_id == id)
  {
    return this;
  }
  for(auto it = m_children.begin(); it != m_children.end(); ++it)
  {
    ZSegmentationNode* child = *it;
    ZSegmentationNode* rv = child->find(id);
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
      mask->unify(m_data);
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


template<typename T>
void ZSegmentationNode::splitNode(ZStack *stack, std::vector<T*> &seeds, QString algorithm, double alpha, double beta)
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
    mask->maskStack(stack_valid);
    delete mask;
  }

  ZStackWatershedContainer* container = new ZStackWatershedContainer(stack_valid);
  addSeed(*container, seeds);

  container->setAlgorithm(algorithm);
  container->setScale(estimateScale(stack_valid->getVoxelNumber()));

  container->setDsMethod("Min(ignore zero)");
  container->run(alpha,beta);

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
  QStandardItem* current = new QStandardItem(QString::number(m_id));

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
  QStandardItem* clone = deepCopy(selected);
  QStandardItem* old_parent = selected->parent();

  QString text = selected->text();

  int row = selected->row();

  QTreeView::dropEvent(event);

  //remove it from old parent
  old_parent->removeRow(selected->row());
  QStandardItem* tmp = ZMultiscaleSegmentationWindow::instance()->findItemById(tree->invisibleRootItem(),text.toInt());

  if(tmp == NULL)//drop on self
  {
    old_parent->insertRow(row,clone);
  }
  else
  {
    QStandardItem* new_parent = tmp->parent();
    if(new_parent == NULL)//drop at blank
    {
      ZMultiscaleSegmentationWindow::instance()->clearTreeView();
      ZMultiscaleSegmentationWindow::instance()->m_root->display(tree->invisibleRootItem());
      ZMultiscaleSegmentationWindow::instance()->m_tree_view->expandAll();
    }
    else
    {
      ZMultiscaleSegmentationWindow::instance()->moveNode(text, new_parent->text());
    }
    delete clone;
  }

}


QStandardItem* ZTreeView::deepCopy(QStandardItem *item)
{
  QStandardItem* rv = item->clone();
  for(int i = 0 ;i < item->rowCount(); ++i)
  {
    rv->insertRow(i, deepCopy(item->child(i)));
  }
  return rv;
}


//implementation for ZMultiscaleSegmentationWindow
ZMultiscaleSegmentationWindow* ZMultiscaleSegmentationWindow::s_window = nullptr;


ZMultiscaleSegmentationWindow::ZMultiscaleSegmentationWindow()
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
  m_tree->invisibleRootItem()->setText("0");

  QPushButton* open_stack = new QPushButton("Open");
  QPushButton* clear = new QPushButton("Clear");
  QPushButton* export_node = new QPushButton("Export");
  QPushButton* segment = new QPushButton("Segment");
  QPushButton* auto_segment = new QPushButton("AutoSegment");
  QPushButton* merge = new QPushButton("Merge");
  /*QPushButton* flood = new QPushButton("Flood");*/
  QPushButton* promote = new QPushButton("Promote");

  m_show_leaf = new QCheckBox("Show Leaf");
  m_show_leaf->setChecked(true);

  m_leaky_boundary = new QCheckBox("Boudary Leak");
  m_leaky_boundary->setChecked(false);

  m_alpha = new QSpinBox();
  m_alpha->setRange(-5,5);

  m_beta = new QSpinBox();
  m_beta->setRange(-5,5);

  //setup layout
  QGridLayout* lay=new QGridLayout(this);
  lay->addWidget(m_tree_view,0,0,8,15);
  lay->addWidget(m_show_leaf,8,0,1,3);
  lay->addWidget(m_leaky_boundary,8,3,1,3);
  lay->addWidget(new QLabel("alpha:"),8,6,1,2);
  lay->addWidget(m_alpha,8,8,1,3);
  lay->addWidget(new QLabel("beta:"),8,11,1,2);
  lay->addWidget(m_beta,8,13,1,3);
  lay->addWidget(open_stack,9,0,1,5);
  lay->addWidget(clear,9,5,1,5);
  lay->addWidget(export_node,9,10,1,5);
  lay->addWidget(segment,10,0,1,4);
  lay->addWidget(auto_segment,10,4,1,4);
  lay->addWidget(merge,10,8,1,4);
  //lay->addWidget(flood,10,12,1,3);
  lay->addWidget(promote,10,12,1,3);



  //events
  connect(m_tree_view,SIGNAL(clicked(QModelIndex)),this,SLOT(onSelectNode(QModelIndex)));
  connect(open_stack,SIGNAL(clicked()),this,SLOT(onOpenStack()));
  connect(clear,SIGNAL(clicked()),this,SLOT(onClear()));
  connect(export_node,SIGNAL(clicked()),this,SLOT(onExport()));
  connect(segment,SIGNAL(clicked()),this,SLOT(onSegment()));
  connect(auto_segment,SIGNAL(clicked()),this,SLOT(onAutoSegment()));
  connect(merge,SIGNAL(clicked()),this,SLOT(onMerge()));
  //connect(flood,SIGNAL(clicked()),this,SLOT(onFlood()));
  connect(promote,SIGNAL(clicked()),this,SLOT(onPromote()));

  this->setLayout(lay);
  this->setMinimumSize(400,600);
  this->move(300,200);
}


/*
void ZMultiscaleSegmentationWindow::onFlood()
{
  if(!m_frame || !m_root)
  {
    return ;
  }

  QList<ZObject3dScan*> selected = m_frame->document()->getSelectedObjectList<ZObject3dScan>(ZStackObject::TYPE_OBJECT3D_SCAN);

  if(selected.size() < 1)
  {
    return ;
  }

  ZSegmentationNode* node = m_root->find(selected.first()->getLabel());

  if(!node || !node->isLeaf())
  {
    return ;
  }

  for(ZSegmentationNode* child: m_root->getLeaves())
  {
    if(child != node)
    {
      if(node->data()->getBoundBox().contains(child->data()->getBoundBox()))
      {
        node->mergeNode(child);
      }
    }
  }
  m_root->regularize();
  clearTreeView();
  m_root->display(m_tree->invisibleRootItem());
  m_tree_view->expandAll();
  highLight(m_root);
}
*/


void ZMultiscaleSegmentationWindow::onPromote()
{
  QStandardItem* item =  getSelectedNodeItem();
  if(item && m_root)
  {
    m_root->mergeNode(m_root->find(item->text().toInt()));
    m_root->regularize();
    clearTreeView();
    m_root->display(m_tree->invisibleRootItem());
    m_tree_view->expandAll();
  }
}


void ZMultiscaleSegmentationWindow::onMerge()
{
  if(!m_frame || !m_root)
  {
    return ;
  }

  QList<ZObject3dScan*> selected = m_frame->document()->getSelectedObjectList<ZObject3dScan>(ZStackObject::TYPE_OBJECT3D_SCAN);

  if(selected.size() <= 1)
  {
    return ;
  }

  QList<ZObject3dScan*>::iterator it = selected.begin();
  ZSegmentationNode *node = m_root->find((*it)->getLabel());

  if(!node)
  {
    return ;
  }

  for(++it; it != selected.end(); ++it)
  {
    ZSegmentationNode* p = m_root->find((*it)->getLabel());
    if(!p)
    {
      continue;
    }
    node->mergeNode(p);
  }

  m_root->regularize();
  clearTreeView();
  //m_frame->document()->removeObject(ZStackObjectRole::ROLE_SEGMENTATION,true);
  m_root->display(m_tree->invisibleRootItem());
  m_tree_view->expandAll();
  highLight(m_root);
}


void ZMultiscaleSegmentationWindow::onSelectNode(QModelIndex index)
{
  if(!m_root || !m_frame)
  {
    return ;
  }
  //m_frame->document()->removeObject(ZStackObjectRole::ROLE_SEGMENTATION,true);
  ZSegmentationNode* node = m_root->find(m_tree->itemFromIndex(index)->text().toInt());
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
  ZSegmentationNode* selected_node = m_root->find(text.toInt());

  if(!selected_node)
  {
    return ;
  }

  std::vector<ZStackObject*> seeds = getSeeds();

  if(m_leaky_boundary->isChecked())
  {
    double alpha = std::pow(10.0,m_alpha->value());
    double beta = std::pow(10.0,m_beta->value());
    selected_node->splitNode<ZStackObject>(m_stack, seeds,"watershedmst",alpha,beta);
  }
  else
  {
    selected_node->splitNode<ZStackObject>(m_stack, seeds,"watershed");
  }

  //display tree in UI
  clearTreeView();
  m_root->display(m_tree->invisibleRootItem());
  m_tree_view->expandAll();
  highLight(m_root);

  QStandardItem* item = findItemById(m_tree->invisibleRootItem(),text.toInt());
  if(item && item->index().isValid())
  {
    m_tree_view->setCurrentIndex(item->index());
  }
  removeSeeds();
}


void ZMultiscaleSegmentationWindow::onAutoSegment()
{
  if(!m_frame || !m_stack)
  {
    return;
  }

  QStandardItem* selected_item = getSelectedNodeItem();
  QString text = selected_item->text();
  //mask segmentation stack
  ZSegmentationNode* selected_node = m_root->find(text.toInt());

  if(!selected_node)
  {
    return ;
  }
  ZSegmentationScan* scan = new ZSegmentationScan();
  selected_node->makeMask(scan);
  ZStack* tmp = m_stack->makeCrop(scan->getBoundBox());
  scan->maskStack(tmp);
  std::vector<ZStack*> seeds = seedsFromMaximum(tmp);

  selected_node->splitNode<ZStack>(m_stack, seeds);

  //display tree in UI
  clearTreeView();
  m_root->display(m_tree->invisibleRootItem());
  m_tree_view->setCurrentIndex(findItemById(m_tree->invisibleRootItem(),text.toInt())->index());
  m_tree_view->expandAll();

  highLight(m_root);
  delete scan;
  for(auto seed: seeds)
  {
    delete seed;
  }
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

    ZSegmentationNode::s_id = 0;
    m_root = new ZSegmentationNode();
    m_root->setParent(NULL);
    ZSegmentationScan* data = new ZSegmentationScan();
    data->fromStack(m_stack);
    m_root->setData(data);
  }
}


void ZMultiscaleSegmentationWindow::onExport()
{
  if(!m_root || !m_stack)
  {
    return ;
  }

  QStandardItem* item = this->getSelectedNodeItem();
  //ZStack* result = new ZStack(m_stack->kind(),m_stack->width(),m_stack->height(),m_stack->depth(),m_stack->channelNumber());

  //ZObject3dScan* mask = new ZObject3dScan();
  ZSegmentationScan* mask = new ZSegmentationScan();
  ZSegmentationNode* node = m_root->find(item->text().toInt());
  node->makeMask(mask);
  //mask->translate(-m_stack->getOffset());
  //mask->labelStack(result->c_stack(),1);
  //result->save(QFileDialog::getSaveFileName(this,"Export Segmentation","","Tiff file(*.tif)").toStdString());
  //mask->getComplementObject().maskStack(m_stack);
  //m_frame->update();
  mask->labelStack(m_stack,0);
  //m_frame->scroll(1,1);
  node->destroy();
  m_root->regularize();
  clearTreeView();
  m_root->display(m_tree->invisibleRootItem());
  m_tree_view->expandAll();
  delete mask;
  //delete result;
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


void onSelectMask(ZStackObject* obj)
{
  int id = obj->getLabel();
  ZMultiscaleSegmentationWindow::instance()->selectNode(id);
}


void onDeselectMask(ZStackObject* obj)
{
  int id = obj->getLabel();
  ZMultiscaleSegmentationWindow::instance()->deselectNode(id);
}


void ZMultiscaleSegmentationWindow::selectNode(int id)
{
  QStandardItem* item = findItemById(m_tree->invisibleRootItem(),id);
  if(item)
  {
    //item->setCheckState(Qt::Checked);
    m_tree_view->setCurrentIndex(item->index());
    std::cout<< id <<" is selected"<<std::endl;
  }
}


void ZMultiscaleSegmentationWindow::deselectNode(int /*id*/)
{
/*
  QStandardItem* item = findItemById(m_tree->invisibleRootItem(),id);
  {
    item->setCheckState(Qt::Checked);
    std::cout<< id <<" is deselected"<<std::endl;
  }
  */
}


void ZMultiscaleSegmentationWindow::highLight(ZSegmentationNode *node)
{
  if(!node)
  {
    return ;
  }

  m_frame->document()->removeObject(ZStackObjectRole::ROLE_SEGMENTATION,true);

  if(m_show_leaf->isChecked())
  {
    ZColorScheme scheme;
    int i = 1;
    scheme.setColorScheme(ZColorScheme::UNIQUE_COLOR);
    for(auto it: node->getLeaves())
    {
      ZSegmentationScan* scan = new ZSegmentationScan();
      it->makeMask(scan);
      ZObject3dScan *mask = scan->toObject3dScan();//here
      delete scan;
      mask->setLabel(it->id());
      mask->setColor(scheme.getColor(i++));
      showMask(mask);
    }
  }
  else
  {
    ZSegmentationScan* scan = new ZSegmentationScan();
    node->makeMask(scan);
    ZObject3dScan *mask = scan->toObject3dScan();//here
    delete scan;
    mask->setColor(QColor(255,0,0));
    mask->setLabel(node->id());
    showMask(mask);
  }
}


void ZMultiscaleSegmentationWindow::showMask(ZObject3dScan *mask)
{
  mask->addRole(ZStackObjectRole::ROLE_SEGMENTATION);
  mask->setSelectable(true);
  mask->addCallBackOnSelection(onSelectMask);
  mask->addCallBackOnDeselection(onDeselectMask);
  m_frame->document()->getDataBuffer()->addUpdate(mask, ZStackDocObjectUpdate::ACTION_ADD_NONUNIQUE);
}


QStandardItem* ZMultiscaleSegmentationWindow::findItemById(QStandardItem *root, int id)
{
  if(root->text().toInt() == id)
  {
    return root;
  }
  for(int i=0;i<root->rowCount();++i)
  {
    QStandardItem* rv = findItemById(root->child(i),id);
    if(rv)
    {
      return rv;
    }
  }
  return NULL;
}


void ZMultiscaleSegmentationWindow::moveNode(QString label, QString new_parent_label)
{

  ZSegmentationNode* node = m_root->find(label.toInt());
  ZSegmentationNode* new_parent = m_root->find(new_parent_label.toInt());

  new_parent->mergeNode(node);

  m_root->regularize();
  clearTreeView();
  m_root->display(m_tree->invisibleRootItem());
  highLight(m_root);
  m_tree_view->expandAll();
}


QStandardItem* ZMultiscaleSegmentationWindow::getSelectedNodeItem()
{
  if(!m_tree_view || !m_tree)
  {
    return NULL;
  }

  QModelIndex index = m_tree_view->currentIndex();
  return index.isValid() ? m_tree->itemFromIndex(index) : m_tree->invisibleRootItem();
}


void ZMultiscaleSegmentationWindow::clearTreeView()
{
  if(m_tree)
  {
    m_tree->clear();
    m_tree->invisibleRootItem()->setText("0");
    m_tree->setHorizontalHeaderLabels(QStringList()<<"Multiscale Segmentations:");
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


std::vector<ZStack*> ZMultiscaleSegmentationWindow::seedsFromMaximum(ZStack *stack)
{
  std::vector<ZStack*> rv;
  if(!stack)
  {
    return rv;
  }

  //ZStackProcessor::GaussianSmooth(stack->c_stack(),5,5);
  ZStackProcessor processor;
  processor.medianFilter(stack);
  int width = stack->width();
  int height = stack->height();
  int depth = stack->depth();
  size_t area = width * height;

  ZStack* seeds = new ZStack(stack->kind(), width, height, depth,stack->channelNumber());
  seeds->setOffset(stack->getOffset());

  uint8_t *p = stack->array8();
  uint8_t* q = seeds->array8();

  int label = 1;
  int step_x = std::max(5, width/20);
  int step_y = std::max(5, height/20);
  int step_z = std::max(5, depth/20);
  for(int k = 1; k < depth-1 && label <256; k+= step_z)
  {
    for(int j = 1; j < height-1 && label < 256; j+= step_y)
    {
      for(int i = 1; i < width-1 && label < 256; i+= step_x)
      {
        bool flag  = true;
        for(int z = -1; z <= 1 && flag; ++z)
        {
          for(int y = -1; y <= 1 && flag; ++y)
          {
            for(int x = -1; x <=1 && flag; ++x)
            {
              if( x==0 && y==0 && z==0)
              {
                continue;
              }
              else if(p[area*(k+z) + width*(j+y) + i+x] >= p[area*k + width*j + i])
              {
                flag = false;
              }
            }
          }
        }
        if(flag)
        {
          //std::cout<<k<<" "<<j<<" "<<i<<std::endl;
          q[k*area + j*width + i] = label++;
        }
        else
        {

        }
      }
    }
  }

  rv.push_back(seeds);
  return rv;
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


ZMultiscaleSegmentationWindow* ZMultiscaleSegmentationWindow::instance()
{
  if(!s_window)
  {
    s_window = new ZMultiscaleSegmentationWindow();
  }

  return s_window;
}


//implementation for ZMultiscaleSegmentationManagementModule
ZMultiscaleSegManagementModule::ZMultiscaleSegManagementModule(QObject* parent):
ZSandboxModule(parent)
{
  init();
}


ZMultiscaleSegManagementModule::~ZMultiscaleSegManagementModule()
{
  delete ZMultiscaleSegmentationWindow::instance();
}


void ZMultiscaleSegManagementModule::init()
{
  m_action = new QAction("Multiscale Segmentation", this);
  connect(m_action, SIGNAL(triggered()), this, SLOT(execute()));
}


void ZMultiscaleSegManagementModule::execute()
{
  ZMultiscaleSegmentationWindow::instance()->show();
}

