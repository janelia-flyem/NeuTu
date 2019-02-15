#include "zobjsmanagerwidget.h"

#include <map>
#include <set>

#include <QSortFilterProxyModel>
#include <QHBoxLayout>
#include <QKeyEvent>

#include "mvc/zstackdoc.h"
#include "zswcobjsmodel.h"
#include "zswcnodeobjsmodel.h"
#include "zpunctaobjsmodel.h"
#include "zdocplayerobjsmodel.h"
#include "zgraphobjsmodel.h"
#include "zsurfaceobjsmodel.h"
#include "zmeshobjsmodel.h"
#include "zroiobjsmodel.h"
#include "QsLog/QsLog.h"
#include "neutubeconfig.h"
#include "zswctree.h"
#include "zobjsmodelmanager.h"
#include "zmesh.h"
#include "zpunctum.h"



ZObjsManagerWidget::ZObjsManagerWidget(ZStackDoc *doc, QWidget *parent) :
  QWidget(parent), m_doc(doc)
{
  setFocusPolicy(Qt::StrongFocus);
  createWidget();
}

ZObjsManagerWidget::~ZObjsManagerWidget()
{
}

ZSwcObjsModel* ZObjsManagerWidget::getSwcObjsModel()
{
  return m_doc->getModelManager()->getObjsModel<ZSwcObjsModel>(
        ZStackObject::EType::SWC);
}

ZPunctaObjsModel* ZObjsManagerWidget::getPunctaObjsModel()
{
  return m_doc->getModelManager()->getObjsModel<ZPunctaObjsModel>(
        ZStackObject::EType::PUNCTA);
}

ZMeshObjsModel* ZObjsManagerWidget::getMeshObjsModel()
{
  return m_doc->getModelManager()->getObjsModel<ZMeshObjsModel>(
        ZStackObject::EType::MESH);
}

ZGraphObjsModel* ZObjsManagerWidget::getGraphObjsModel()
{
  return m_doc->getModelManager()->getObjsModel<ZGraphObjsModel>(
        ZStackObjectRole::ROLE_SEED);
}

ZDocPlayerObjsModel* ZObjsManagerWidget::getSeedObjsModel()
{
  return m_doc->getModelManager()->getObjsModel<ZDocPlayerObjsModel>(
        ZStackObjectRole::ROLE_SEED);
}

void ZObjsManagerWidget::swcItemDoubleClicked(QModelIndex index)
{
  ZSwcTree *p2 =getSwcObjsModel()->getSwcTree(index);
  if (p2 != NULL) {
    emit swcDoubleClicked(p2);
  } else {
    Swc_Tree_Node *p3 = getSwcObjsModel()->getSwcTreeNode(index);
    if (p3 != NULL) {
      emit swcNodeDoubleClicked(p3);
    }
  }
}
#if 0
void ZObjsManagerWidget::processDoubleClickOnCategorizedSwcNode(
    QModelIndex index)
{
  Swc_Tree_Node *p3 = m_doc->swcNodeObjsModel()->getSwcTreeNode(index);
  if (p3 != NULL) {
    emit swcNodeDoubleClicked(p3);
  }
}
#endif

void ZObjsManagerWidget::swcSelectionChangedFromTreeView(
    QItemSelection selected, QItemSelection deselected)
{
  ZSwcObjsModel *model = getSwcObjsModel();
  QModelIndexList indexes = deselected.indexes();
  for (int i=0; i<indexes.size(); i++) {
    ZSwcTree *p2 = model->getSwcTree(indexes[i]);
    if (p2 != NULL) {
      m_doc->setSwcSelected(p2, false);
    } else {
      Swc_Tree_Node *p3 = model->getSwcTreeNode(indexes[i]);
      if (p3 != NULL) {
        m_doc->deselectSwcTreeNode(p3);
        //m_doc->setSwcTreeNodeSelected(p3, false);
      }
    }
  }
  indexes = selected.indexes();
  for (int i=0; i<indexes.size(); i++) {
    ZSwcTree *p2 = model->getSwcTree(indexes[i]);
    if (p2 != NULL) {
      m_doc->setSwcSelected(p2, true);
    } else {
      Swc_Tree_Node *p3 = model->getSwcTreeNode(indexes[i]);
      if (p3 != NULL) {
        //m_doc->setSwcTreeNodeSelected(p3, true);
        m_doc->selectSwcTreeNode(p3, true);
      }
    }
  }
}

#if 0
void ZObjsManagerWidget::updateSelectionFromCategorizedSwcNode(
    QItemSelection selected, QItemSelection deselected)
{
  QModelIndexList indexes = deselected.indexes();
  for (int i=0; i<indexes.size(); i++) {
    Swc_Tree_Node *p3 = m_doc->swcNodeObjsModel()->getSwcTreeNode(indexes[i]);
    if (p3 != NULL) {
      m_doc->deselectSwcTreeNode(p3);
    } else {
      std::set<Swc_Tree_Node*> nodeSet =
          m_doc->swcNodeObjsModel()->getSwcTreeNodeSet(indexes[i]);
      for (std::set<Swc_Tree_Node*>::const_iterator iter = nodeSet.begin();
           iter != nodeSet.end(); ++iter) {
        m_doc->deselectSwcTreeNode(*iter);
      }
    }
  }
  indexes = selected.indexes();
  for (int i=0; i<indexes.size(); i++) {
    Swc_Tree_Node *p3 = m_doc->swcNodeObjsModel()->getSwcTreeNode(indexes[i]);
    if (p3 != NULL) {
      m_doc->selectSwcTreeNode(p3, true);
    } else {
      std::set<Swc_Tree_Node*> nodeSet =
          m_doc->swcNodeObjsModel()->getSwcTreeNodeSet(indexes[i]);
      for (std::set<Swc_Tree_Node*>::const_iterator iter = nodeSet.begin();
           iter != nodeSet.end(); ++iter) {
        m_doc->selectSwcTreeNode(*iter, true);
      }
    }
  }
}
#endif

void ZObjsManagerWidget::punctaItemDoubleClicked(QModelIndex index)
{
  ZPunctum *p = getPunctaObjsModel()->getPunctum(
        m_punctaProxyModel->mapToSource(index));
  if (p != NULL) {
    emit punctaDoubleClicked(p);
  }
}

void ZObjsManagerWidget::punctaSelectionChangedFromTreeView(
    QItemSelection selected, QItemSelection deselected)
{
  QModelIndexList indexes = deselected.indexes();
  std::vector<ZPunctum*> coll;

  ZPunctaObjsModel *model = getPunctaObjsModel();
  for (int i=0; i<indexes.size(); i++) {
    if (indexes[i].column() > 0)
        continue;
    ZPunctum *p = model->getPunctum(m_punctaProxyModel->mapToSource(indexes[i]));
    if (p != NULL) {
      coll.push_back(p);
    } else {
      const std::vector<ZPunctum*>* ps = model->getPuncta(
            m_punctaProxyModel->mapToSource(indexes[i]));
      if (ps != NULL) {
        std::copy(ps->begin(), ps->end(), std::back_inserter(coll));
      }
    }
  }
  if (!coll.empty()) {
    m_doc->setObjectSelected(coll, false); //to be tested
//    m_doc->setPunctumSelected(coll.begin(), coll.end(), false);
    coll.clear();
  }

  indexes = selected.indexes();
  for (int i=0; i<indexes.size(); i++) {
    if (indexes[i].column() > 0)
        continue;
    ZPunctaObjsModel *model = getPunctaObjsModel();
    ZPunctum *p = model->getPunctum(m_punctaProxyModel->mapToSource(indexes[i]));
    if (p != NULL) {
      coll.push_back(p);
    } else {
      const std::vector<ZPunctum*>* ps = model->getPuncta(
            m_punctaProxyModel->mapToSource(indexes[i]));
      if (ps != NULL) {
        std::copy(ps->begin(), ps->end(), std::back_inserter(coll));
      }
    }
  }
  if (!coll.empty()) {
    m_doc->setObjectSelected(coll, true);
//    m_doc->setPunctumSelected(coll.begin(), coll.end(), true);
  }
}

void ZObjsManagerWidget::meshItemDoubleClicked(QModelIndex index)
{
  ZMesh *p2 = getMeshObjsModel()->getMesh(index);
  if (p2 != NULL) {
    emit meshDoubleClicked(p2);
  }
}

void ZObjsManagerWidget::meshSelectionChangedFromTreeView(
    QItemSelection selected, QItemSelection deselected)
{
  QModelIndexList indexes = deselected.indexes();
  ZMeshObjsModel *model = getMeshObjsModel();
  for (int i=0; i<indexes.size(); i++) {
    ZMesh *p2 = model->getMesh(indexes[i]);
    if (p2 != NULL) {
      m_doc->setMeshSelected(p2, false);
    }
  }
  indexes = selected.indexes();
  for (int i=0; i<indexes.size(); i++) {
    ZMesh *p2 = model->getMesh(indexes[i]);
    if (p2 != NULL) {
      m_doc->setMeshSelected(p2, true);
    }
  }
}

void ZObjsManagerWidget::punctaSelectionChanged(QList<ZPunctum *> selected, QList<ZPunctum *> deselected)
{
  if (!selected.empty() && m_punctaObjsTreeView != NULL) {
    QItemSelection is;
    buildItemSelectionFromList(selected, is);
    m_punctaObjsTreeView->selectionModel()->select(
          is, QItemSelectionModel::Select | QItemSelectionModel::Rows);
    // scroll to first one if necessary
    ZPunctaObjsModel *model = getPunctaObjsModel();
    QModelIndex index = model->getIndex(selected[0]);
    if (m_punctaObjsTreeView->isExpanded(
          m_punctaProxyModel->mapFromSource(model->parent(index)))) {
      m_punctaObjsTreeView->scrollTo(m_punctaProxyModel->mapFromSource(index));
    }
  }

  if (!deselected.empty()) {
    QItemSelection is;
    buildItemSelectionFromList(deselected, is);
    m_punctaObjsTreeView->selectionModel()->select(
          is, QItemSelectionModel::Deselect | QItemSelectionModel::Rows);
  }
}

void ZObjsManagerWidget::meshSelectionChanged(
    QList<ZMesh*> selected, QList<ZMesh*> deselected)
{
  if (!selected.empty() && m_meshObjsTreeView != NULL) {
    QItemSelection is;
    buildItemSelectionFromList(selected, is);
    m_meshObjsTreeView->selectionModel()->select(
          is, QItemSelectionModel::Select | QItemSelectionModel::Rows);
    // scroll to first one if necessary
    ZMeshObjsModel *model = getMeshObjsModel();
    QModelIndex index = model->getIndex(selected[0]);
    if (m_meshObjsTreeView->isExpanded(model->parent(index))) {
      m_meshObjsTreeView->scrollTo(index);
    }
  }

  if (!deselected.empty()) {
    QItemSelection is;
    buildItemSelectionFromList(deselected, is);
    m_meshObjsTreeView->selectionModel()->select(
          is, QItemSelectionModel::Deselect | QItemSelectionModel::Rows);
  }
}

void ZObjsManagerWidget::swcSelectionChanged(
    QList<ZSwcTree *> selected, QList<ZSwcTree *> deselected)
{
  if (!selected.empty()) {
    QItemSelection is;
    buildItemSelectionFromList(selected, is);
    m_swcObjsTreeView->selectionModel()->select(
          is, QItemSelectionModel::Select | QItemSelectionModel::Rows);
    // scroll to first one if necessary
    QModelIndex index = getSwcObjsModel()->getIndex(selected[0]);
    //if (m_swcObjsTreeView->isExpanded(m_doc->swcObjsModel()->parent(index))) {
    m_swcObjsTreeView->scrollTo(index);
    //}
  }

  if (!deselected.empty()) {
    QItemSelection is;
    buildItemSelectionFromList(deselected, is);
    m_swcObjsTreeView->selectionModel()->select(
          is, QItemSelectionModel::Deselect | QItemSelectionModel::Rows);
  }
}
#if 0
void ZObjsManagerWidget::swcTreeNodeSelectionChanged(
    QList<Swc_Tree_Node *> selected, QList<Swc_Tree_Node *> deselected)
{
  if (!selected.empty()) {
    QItemSelection is;
    buildItemSelectionFromList(selected, is);
    m_swcObjsTreeView->selectionModel()->select(
          is, QItemSelectionModel::Select | QItemSelectionModel::Rows);
    // scroll to first one if necessary
    ZSwcObjsModel *model = getSwcObjsModel();
    QModelIndex index = model->getIndex(selected[0]);
    if (m_swcObjsTreeView->isExpanded(model->parent(index))) {
      m_swcObjsTreeView->scrollTo(index);
    }
  }

  if (!deselected.empty()) {
    QItemSelection is;
    buildItemSelectionFromList(deselected, is);
    m_swcObjsTreeView->selectionModel()->select(
          is, QItemSelectionModel::Deselect | QItemSelectionModel::Rows);
  }
}
#endif


void ZObjsManagerWidget::createWidget()
{
  QTabWidget *tabs = new QTabWidget(this);
  tabs->setElideMode(Qt::ElideNone);
  tabs->setUsesScrollButtons(true);

  m_swcObjsTreeView = new QTreeView(this);
  m_swcObjsTreeView->setTextElideMode(Qt::ElideLeft);
  m_swcObjsTreeView->setExpandsOnDoubleClick(false);
  m_swcObjsTreeView->setModel(getSwcObjsModel());
  m_swcObjsTreeView->setSelectionMode(QAbstractItemView::ExtendedSelection);
  m_swcObjsTreeView->setContextMenuPolicy(Qt::CustomContextMenu);
  connect(m_swcObjsTreeView, SIGNAL(doubleClicked(QModelIndex)),
          this, SLOT(swcItemDoubleClicked(QModelIndex)));
  connect(m_swcObjsTreeView->selectionModel(),
          SIGNAL(selectionChanged(QItemSelection,QItemSelection)),
          this, SLOT(swcSelectionChangedFromTreeView(QItemSelection,QItemSelection)));
  tabs->addTab(m_swcObjsTreeView, "Neurons");

#if 0
  if (NeutubeConfig::getInstance().getObjManagerConfig().isCategorizedSwcNodeOn()) {
    m_swcNodeObjsTreeView = new QTreeView(this);
    m_swcNodeObjsTreeView->setExpandsOnDoubleClick(false);
    m_swcNodeObjsTreeView->setModel(m_doc->swcNodeObjsModel());
    m_swcNodeObjsTreeView->setSelectionMode(QAbstractItemView::ExtendedSelection);
    m_swcNodeObjsTreeView->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(m_swcNodeObjsTreeView, SIGNAL(doubleClicked(QModelIndex)),
            this, SLOT(processDoubleClickOnCategorizedSwcNode(QModelIndex)));
    connect(m_swcNodeObjsTreeView->selectionModel(),
            SIGNAL(selectionChanged(QItemSelection,QItemSelection)),
            this, SLOT(updateSelectionFromCategorizedSwcNode(QItemSelection,QItemSelection)));
    tabs->addTab(m_swcNodeObjsTreeView, "Neuron Nodes");
  }
#endif

  if (NeutubeConfig::getInstance().getObjManagerConfig().isMeshOn()) {
    m_meshObjsTreeView = new QTreeView(this);
    m_meshObjsTreeView->setTextElideMode(Qt::ElideLeft);
    m_meshObjsTreeView->setExpandsOnDoubleClick(false);
    m_meshObjsTreeView->setModel(getMeshObjsModel());
    m_meshObjsTreeView->setSelectionMode(QAbstractItemView::ExtendedSelection);
    m_meshObjsTreeView->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(m_meshObjsTreeView, SIGNAL(doubleClicked(QModelIndex)),
            this, SLOT(meshItemDoubleClicked(QModelIndex)));
    connect(m_meshObjsTreeView->selectionModel(),
            SIGNAL(selectionChanged(QItemSelection,QItemSelection)),
            this, SLOT(meshSelectionChangedFromTreeView(QItemSelection,QItemSelection)));
    tabs->addTab(m_meshObjsTreeView, "Meshes");
  }

  if (NeutubeConfig::getInstance().getMainWindowConfig().isMarkPunctaOn()) {
    m_punctaObjsTreeView = new QTreeView(this);
    m_punctaObjsTreeView->setSortingEnabled(true);
    m_punctaObjsTreeView->setExpandsOnDoubleClick(false);
    m_punctaProxyModel = new QSortFilterProxyModel(this);
    m_punctaProxyModel->setSourceModel(getPunctaObjsModel());
    //m_punctaObjsTreeView->setModel(m_doc->punctaObjsModel());
    m_punctaObjsTreeView->setModel(m_punctaProxyModel);
    m_punctaObjsTreeView->setSelectionMode(QAbstractItemView::ExtendedSelection);
    m_punctaObjsTreeView->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(m_punctaObjsTreeView, SIGNAL(doubleClicked(QModelIndex)),
            this, SLOT(punctaItemDoubleClicked(QModelIndex)));
    connect(m_punctaObjsTreeView->selectionModel(), SIGNAL(selectionChanged(QItemSelection,QItemSelection)),
            this, SLOT(punctaSelectionChangedFromTreeView(QItemSelection,QItemSelection)));
    m_punctaObjsTreeView->sortByColumn(0, Qt::AscendingOrder);
    tabs->addTab(m_punctaObjsTreeView, "Puncta");
  }

  if (GET_APPLICATION_NAME == "FlyEM") {
    m_seedObjsTreeView = new QTreeView(this);
    m_seedObjsTreeView->setSortingEnabled(false);
    m_seedObjsTreeView->setExpandsOnDoubleClick(false);
    m_seedObjsTreeView->setModel(getSeedObjsModel());
    m_seedObjsTreeView->setSelectionMode(QAbstractItemView::ExtendedSelection);
    m_seedObjsTreeView->setContextMenuPolicy(Qt::CustomContextMenu);
    tabs->addTab(m_seedObjsTreeView, "Seeds");
#if 0
    m_roiObjsTreeView = new QTreeView(this);
    m_roiObjsTreeView->setSortingEnabled(false);
    m_roiObjsTreeView->setExpandsOnDoubleClick(false);
    m_roiObjsTreeView->setModel(m_doc->roiObjsModel());
    m_roiObjsTreeView->setSelectionMode(QAbstractItemView::ExtendedSelection);
    m_roiObjsTreeView->setContextMenuPolicy(Qt::CustomContextMenu);
    tabs->addTab(m_roiObjsTreeView, "ROI");
#endif
  }

  m_graphObjsTreeView = new QTreeView(this);
  m_graphObjsTreeView->setSortingEnabled(false);
  m_graphObjsTreeView->setExpandsOnDoubleClick(false);
  m_graphObjsTreeView->setModel(getGraphObjsModel());
  m_graphObjsTreeView->setSelectionMode(QAbstractItemView::ExtendedSelection);
  m_graphObjsTreeView->setContextMenuPolicy(Qt::CustomContextMenu);
  tabs->addTab(m_graphObjsTreeView, "Graph");

  /*
  m_surfaceObjsTreeView = new QTreeView(this);
  m_surfaceObjsTreeView->setSortingEnabled(false);
  m_surfaceObjsTreeView->setExpandsOnDoubleClick(false);
  m_surfaceObjsTreeView->setModel(m_doc->surfaceObjsModel());
  m_surfaceObjsTreeView->setSelectionMode(QAbstractItemView::ExtendedSelection);
  m_surfaceObjsTreeView->setContextMenuPolicy(Qt::CustomContextMenu);
  tabs->addTab(m_surfaceObjsTreeView, "Surface");
*/

  QHBoxLayout *layout = new QHBoxLayout;
  layout->addWidget(tabs);
  setLayout(layout);

  if (m_doc->hasSelectedPuncta()) {
    //std::set<ZPunctum*> *selectedPuncta = m_doc->selectedPuncta();
    QList<ZPunctum*> selected =
        m_doc->getSelectedObjectList<ZPunctum>(ZStackObject::EType::PUNCTUM);
    QList<ZPunctum*> deselected;
    //std::copy(selectedPuncta->begin(), selectedPuncta->end(), std::back_inserter(selected));
    punctaSelectionChanged(selected, deselected);
  }
  if (m_doc->hasSelectedMeshes()) {
    QList<ZMesh*> selected =
        m_doc->getSelectedObjectList<ZMesh>(ZStackObject::EType::PUNCTUM);
    QList<ZMesh*> deselected;
    meshSelectionChanged(selected, deselected);
  }
  if (!m_doc->hasSelectedSwc()) {
    //std::set<ZSwcTree*> *selectedSwcs = m_doc->selectedSwcs();
    QList<ZSwcTree*> selected =
        m_doc->getSelectedObjectList<ZSwcTree>(ZStackObject::EType::SWC);
    QList<ZSwcTree*> deselected;
    //std::copy(selectedSwcs->begin(), selectedSwcs->end(), std::back_inserter(selected));
    swcSelectionChanged(selected, deselected);
  }
  /*
  if (m_doc->hasSelectedSwcNode()) {
    std::set<Swc_Tree_Node*> nodeSet = m_doc->getSelectedSwcNodeSet();
    QList<Swc_Tree_Node*> selected;
    QList<Swc_Tree_Node*> deselected;
    std::copy(nodeSet.begin(), nodeSet.end(),
              std::back_inserter(selected));
    swcTreeNodeSelectionChanged(selected, deselected);
  }
  */

  connect(m_doc,
          SIGNAL(punctaSelectionChanged(QList<ZPunctum*>,QList<ZPunctum*>)),
          this, SLOT(punctaSelectionChanged(QList<ZPunctum*>,QList<ZPunctum*>)));
  connect(m_doc,
          SIGNAL(meshSelectionChanged(QList<ZMesh*>,QList<ZMesh*>)),
          this, SLOT(meshSelectionChanged(QList<ZMesh*>,QList<ZMesh*>)));
  connect(m_doc,
          SIGNAL(swcSelectionChanged(QList<ZSwcTree*>,QList<ZSwcTree*>)),
          this, SLOT(swcSelectionChanged(QList<ZSwcTree*>,QList<ZSwcTree*>)));
//  connect(m_doc,
//          SIGNAL(swcTreeNodeSelectionChanged(QList<Swc_Tree_Node*>,QList<Swc_Tree_Node*>)),
//          this, SLOT(swcTreeNodeSelectionChanged(QList<Swc_Tree_Node*>,QList<Swc_Tree_Node*>)));
}

void ZObjsManagerWidget::keyPressEvent(QKeyEvent *event)
{
  switch(event->key())
  {
  case Qt::Key_Backspace:
  case Qt::Key_Delete:
  {
    m_doc->executeRemoveSelectedObjectCommand();
#if 0
    if (m_doc->selectedChains()->empty() && m_doc->selectedPuncta()->empty() && m_doc->selectedSwcs()->empty())
      return;
    QUndoCommand *removeSelectedObjectsCommand =
        new ZStackDocRemoveSelectedObjectCommand(m_doc);
    m_doc->undoStack()->push(removeSelectedObjectsCommand);
#endif
  }
    break;
  default:
    break;
  }
}

namespace {

struct ModelIndexCompareRow {
  bool operator() (const QModelIndex& lhs, const QModelIndex& rhs) const
  {return lhs.row() < rhs.row();}
};

void buildItemSelection(
    const std::map<QModelIndex, std::set<QModelIndex, ModelIndexCompareRow> > &allIndex,
    QItemSelection &is)
{
  int numRange = 0;
  for (std::map<QModelIndex, std::set<QModelIndex, ModelIndexCompareRow> >::const_iterator ait = allIndex.begin();
       ait != allIndex.end(); ++ait) {
    const std::set<QModelIndex, ModelIndexCompareRow> &indexes = ait->second;
    std::set<QModelIndex, ModelIndexCompareRow>::iterator it = indexes.begin();
    QModelIndex start = *it;
    QModelIndex end = start;
    int prevRow = start.row();
    ++it;
    for (; it != indexes.end(); ++it) {
      if (it->row() - prevRow == 1) {
        end = *it;
        prevRow = end.row();
      } else if (it->row() - prevRow > 1) {
        is.select(start, end);
        numRange++;
        start = *it;
        end = start;
        prevRow = start.row();
      } else {
        LDEBUG() << "impossible";
      }
    }
    is.select(start, end);
    numRange++;
  }
  //LDEBUG() << "number of range: " << numRange;
}

}

// build continues range to speed up selection speed in qt tree view
void ZObjsManagerWidget::buildItemSelectionFromList(const QList<ZPunctum *> &list, QItemSelection &is)
{
  if (list.empty())
    return;

  std::map<QModelIndex, std::set<QModelIndex, ModelIndexCompareRow> > allIndex;

  for (int i=0; i<list.size(); i++) {
    QModelIndex index = m_punctaProxyModel->mapFromSource(
          getPunctaObjsModel()->getIndex(list[i]));
    if (index.isValid()) {
      allIndex[index.parent()].insert(index);
    } else {
      LERROR() << "puncta don't exist in widget, something is wrong";
    }
  }

  buildItemSelection(allIndex, is);
}

void ZObjsManagerWidget::buildItemSelectionFromList(const QList<ZSwcTree *> &list, QItemSelection &is)
{
  if (list.empty())
    return;

  std::map<QModelIndex, std::set<QModelIndex, ModelIndexCompareRow> > allIndex;

  for (int i=0; i<list.size(); i++) {
    QModelIndex index = getSwcObjsModel()->getIndex(list[i]);
    if (index.isValid()) {
      allIndex[index.parent()].insert(index);
    } else {
      LERROR() << "swc tree does not exist in widget, something is wrong";
    }
  }

  buildItemSelection(allIndex, is);
}

#if 0
void ZObjsManagerWidget::buildItemSelectionFromList(
    const QList<Swc_Tree_Node *> &list, QItemSelection &is)
{
  if (list.empty())
    return;

  std::map<QModelIndex, std::set<QModelIndex, ModelIndexCompareRow> > allIndex;

  ZSwcO
  for (int i=0; i<list.size(); i++) {
    QModelIndex index = getSwcNodeObjsModel()->getIndex(list[i]);
    if (index.isValid()) {
      allIndex[index.parent()].insert(index);
    }
  }

  buildItemSelection(allIndex, is);
}
#endif

void ZObjsManagerWidget::buildItemSelectionFromList(const QList<ZMesh*>& list, QItemSelection& is)
{
  if (list.empty())
    return;

  std::map<QModelIndex, std::set<QModelIndex, ModelIndexCompareRow> > allIndex;

  ZMeshObjsModel *model = getMeshObjsModel();
  for (int i=0; i<list.size(); i++) {
    QModelIndex index = model->getIndex(list[i]);
    if (index.isValid()) {
      allIndex[index.parent()].insert(index);
    } else {
      LERROR() << "mesh does not exist in widget, something is wrong";
    }
  }

  buildItemSelection(allIndex, is);
}
