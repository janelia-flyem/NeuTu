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
#include "mainwindow.h"


//implementation for ZSegmentationNode
std::map<QString,int> ZSegmentationNode::m_color_table = std::map<QString,int>();

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


void ZSegmentationNode::consumeSegmentations(ZObject3dScanArray &segmentations)
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

  for (ZObject3dScanArray::iterator iter = segmentations.begin();iter != segmentations.end(); ++iter)
  {
    ZObject3dScan *obj = *iter;
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
  for(int i=0;i<m_children.size();++i)
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


void ZSegmentationNode::makeMask(ZObject3dScan* mask)
{
  if(this->isLeaf())
  {
    if(m_data)
    {
      mask->unify(*m_data);
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
    ZObject3dScan* data =new ZObject3dScan();
    node->makeMask(data);
    if(this->m_data)
    {
      this->m_data->unify(*data);
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
  for(auto it = m_children.begin(); it != m_children.end(); ++it)
  {
    ZSegmentationNode* child = *it;
    child->destroy();
  }
  m_children.clear();
  std::map<QString,int>::iterator it = m_color_table.find(m_label);
  if(it != m_color_table.end())
  {
    m_color_table.erase(it);
  }
}


int ZSegmentationNode::getColorIndex()
{
  bool index[256] = {false};
  std::map<QString,int>::iterator it = m_color_table.find(m_label);
  if(it == m_color_table.end())
  {
    for(auto p = m_color_table.begin(); p!=m_color_table.end();++p)
    {
      index[p->second] = true;
    }
    for(int i=0;i<256;++i)
    {
      if(!index[i])
      {
        m_color_table.insert(std::pair<QString,int>(m_label,i));
        return i;
      }
    }
    return -1;
  }
  return it->second;
}


void ZSegmentationNode::splitNode(ZStack *stack, std::vector<ZStackObject *> &seeds)
{
  ZObject3dScan* mask = new ZObject3dScan();
  this->makeMask(mask);

  ZIntCuboid box = mask->getBoundBox();
  ZStack* stack_valid = stack->makeCrop(box);
  //stack_valid->save("/home/deli/1.tif");

  //mask->translate(-stack->getOffset());
  mask->translate(-box.getFirstCorner());

  mask->maskStack(stack_valid);
  //stack_valid->save("/home/deli/2.tif");

  ZStackWatershedContainer* container = new ZStackWatershedContainer(stack_valid);
  for(auto it= seeds.begin();it!=seeds.end();++it )
  {
    container->addSeed(*it);
  }
  container->setAlgorithm("watershed");

  size_t volume = mask->getBoundBox().getVolume();

  int scale = 1;
  if(volume >= 1000*1000*1000)
  {
    scale = 4;
  }
  else if(volume >= 500*500*500)
  {
    scale = 2;
  }
  container->setScale(scale);
  container->setDsMethod("Min(ignore zero)");
  container->run();

  ZObject3dScanArray result;
  container->makeSplitResult(1, &result);
  this->consumeSegmentations(result);

  result.shallowClear();
  delete mask;
  delete stack_valid;
  delete container;
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
  //clear other masks
  m_frame->document()->removeObject(ZStackObjectRole::ROLE_SEGMENTATION,true);

  ZObject3dScan* mask = new ZObject3dScan();

  ZSegmentationNode* node = m_root->find(m_tree->itemFromIndex(index)->text());

  node->makeMask(mask);

  ZColorScheme scheme = ZColorScheme();
  scheme.setColorScheme(ZColorScheme::LABEL_COLOR);

  mask->setColor(scheme.getColor(node->getColorIndex()));
  mask->addRole(ZStackObjectRole::ROLE_SEGMENTATION);

  m_frame->document()->getDataBuffer()->addUpdate(mask, ZStackDocObjectUpdate::ACTION_ADD_NONUNIQUE);
}


void ZMultiscaleSegmentationWindow::onSegment()
{
  if(!m_frame || !m_stack){
    return;
  }

  QStandardItem* selected_item = getSelectedNodeItem();
  QString text = selected_item->text();
  //mask segmentation stack
  ZSegmentationNode* selected_node = m_root->find(selected_item->text());

  std::vector<ZStackObject*> seeds = getSeeds();
  selected_node->splitNode(m_stack, seeds);

  //display tree in UI
  for(int i=0;i<m_tree->invisibleRootItem()->rowCount();++i)
  {
    m_tree->invisibleRootItem()->removeRow(i);
  }
  m_root->display(m_tree->invisibleRootItem());
  m_tree_view->setCurrentIndex(findItemByText(m_tree->invisibleRootItem(),text)->index());

  removeSeeds();
}


void ZMultiscaleSegmentationWindow::onOpenStack()
{
  QString file_name = QFileDialog::getOpenFileName(this,"Open Stack","","tiff(*.tif)");
  if (file_name!="")
  {
    m_stack = new ZStack();
    m_stack->load(file_name.toStdString());
    m_frame = ZSandbox::GetMainWindow()->createStackFrame(m_stack);
    ZSandbox::GetMainWindow()->addStackFrame(m_frame);
    ZSandbox::GetMainWindow()->presentStackFrame(m_frame);

    if(m_root)
    {
      delete m_root;
      m_root = NULL;
    }
    m_root = new ZSegmentationNode();
    m_root->setLabel("0");
    m_root->setParent(NULL);
    ZObject3dScan* data = new ZObject3dScan();
    data->loadStack(*m_stack);
    m_root->setData(data);
  }
}


void ZMultiscaleSegmentationWindow::onExport()
{
  QStandardItem* item = this->getSelectedNodeItem();
  ZStack* result = new ZStack(m_stack->kind(),m_stack->width(),m_stack->height(),m_stack->depth(),m_stack->channelNumber());

  ZObject3dScan* mask = new ZObject3dScan();
  m_root->find(item->text())->makeMask(mask);
  mask->translate(-m_stack->getOffset());
  mask->labelStack(result->c_stack(),1);
  result->save(QFileDialog::getSaveFileName(this,"Export Segmentation","","Tiff file(*.tif)").toStdString());

  delete mask;
  delete result;
}


void ZMultiscaleSegmentationWindow::onClear()
{
  m_frame->close();
  for(int i=0;i<m_tree->invisibleRootItem()->rowCount();++i)
  {
    m_tree->invisibleRootItem()->removeRow(i);
  }
  m_frame = NULL;
  m_stack = NULL;
  if(m_root)
  {
    m_root->destroy();
  }
  m_root = NULL;
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

  for(int i=0;i<m_tree->invisibleRootItem()->rowCount();++i)
  {
    m_tree->invisibleRootItem()->removeRow(i);
  }
  m_root->display(m_tree->invisibleRootItem());

  m_tree_view->setCurrentIndex(findItemByText(m_tree->invisibleRootItem(),new_parent->label())->index());
}


QStandardItem* ZMultiscaleSegmentationWindow::getSelectedNodeItem()
{
  QModelIndex index = m_tree_view->currentIndex();
  return index.isValid() ? m_tree->itemFromIndex(index) : m_tree->invisibleRootItem();
}


void ZMultiscaleSegmentationWindow::removeSeeds()
{
  for(ZStroke2d* stroke:m_frame->document()->getStrokeList())
  {
    m_frame->document()->removeObject(stroke);
  }
}


std::vector<ZStackObject*> ZMultiscaleSegmentationWindow::getSeeds()
{
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

