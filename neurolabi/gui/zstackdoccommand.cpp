#include "zstackdoccommand.h"

#include <iostream>
#include <QMutableListIterator>
#include <QMessageBox>
#include <QApplication>

#include "zswctree.h"
#include "tz_error.h"
#include "zlocsegchainconn.h"
#include "zlocsegchain.h"
#include "zstack.hxx"
#include "zstroke2d.h"
#include "zneurontracer.h"
#include "zstackdoc.h"
#include "zswcconnector.h"
#include "zgraph.h"
#include "zdocumentable.h"
#include "zdocplayer.h"
#include "neutubeconfig.h"
#include "zstring.h"
#include "zfiletype.h"

using namespace std;

#define INIT_ZUNDOCOMMAND m_isSwcSaved(false), m_loggingCommand(true)

ZUndoCommand::ZUndoCommand(QUndoCommand *parent) : QUndoCommand(parent),
  INIT_ZUNDOCOMMAND
{

}

ZUndoCommand::ZUndoCommand(const QString &text, QUndoCommand *parent) :
  QUndoCommand(text, parent), INIT_ZUNDOCOMMAND
{

}

void ZUndoCommand::enableLog(bool on)
{
  m_loggingCommand = on;
}

bool ZUndoCommand::loggingCommand() const
{
  return m_loggingCommand;
}

void ZUndoCommand::logCommand(const QString &msg) const
{
  if (loggingCommand() && !msg.isEmpty()) {
    LINFO() << msg;
  }
}

void ZUndoCommand::setLogMessage(const QString &msg)
{
  m_logMessage = msg;
}

void ZUndoCommand::setLogMessage(const std::string &msg)
{
  m_logMessage = msg.c_str();
}

void ZUndoCommand::setLogMessage(const char *msg)
{
  m_logMessage = msg;
}

void ZUndoCommand::logCommand() const
{
  logCommand(m_logMessage);
}

void ZUndoCommand::logUndoCommand() const
{
  if (!m_logMessage.isEmpty()) {
    logCommand("Undo: " + m_logMessage);
  }
}

void ZUndoCommand::startUndo()
{
  logUndoCommand();
}

void ZUndoCommand::setSaved(NeuTube::EDocumentableType type, bool state)
{
  switch (type) {
  case NeuTube::Documentable_SWC:
    m_isSwcSaved = state;
    break;
  default:
    break;
  }
}

bool ZUndoCommand::isSaved(NeuTube::EDocumentableType type) const
{
  switch (type) {
  case NeuTube::Documentable_SWC:
    return m_isSwcSaved;
  default:
    return false;
  }

  return false;
}

ZStackDocCommand::SwcEdit::ChangeSwcCommand::ChangeSwcCommand(
    ZStackDoc *doc, QUndoCommand *parent) :
  ZUndoCommand(parent), m_doc(doc), m_isSwcModified(false)
{
}

ZStackDocCommand::SwcEdit::ChangeSwcCommand::~ChangeSwcCommand()
{
  for (std::set<Swc_Tree_Node*>::const_iterator iter = m_garbageSet.begin();
       iter != m_garbageSet.end(); ++iter) {
    delete *iter;
  }

  for (std::set<Swc_Tree_Node*>::const_iterator iter = m_removedNodeSet.begin();
       iter != m_removedNodeSet.end(); ++iter) {
    delete *iter;
  }
}

void ZStackDocCommand::SwcEdit::ChangeSwcCommand::redo()
{
  ZUndoCommand::redo();
}

void ZStackDocCommand::SwcEdit::ChangeSwcCommand::recover()
{
  m_garbageSet.insert(m_newNodeSet.begin(), m_newNodeSet.end());
  m_newNodeSet.clear();
  m_removedNodeSet.clear();
  for (std::map<Swc_Tree_Node*, Swc_Tree_Node>::iterator
       iter = m_backupSet.begin(); iter != m_backupSet.end(); ++iter) {
    *(iter->first) = iter->second;
  }
  m_backupSet.clear();

  m_doc->deprecateTraceMask();

  m_doc->processSwcModified();
  m_doc->notifyObjectModified();
}

void ZStackDocCommand::SwcEdit::ChangeSwcCommand::undo()
{
  startUndo();
  recover();
}

void ZStackDocCommand::SwcEdit::ChangeSwcCommand::backup(Swc_Tree_Node *tn)
{
  if (tn != NULL) {
    if (m_backupSet.count(tn) == 0) {
      m_backupSet[tn] = *tn;
    }
  }
}

void ZStackDocCommand::SwcEdit::ChangeSwcCommand::backupChildren(
    Swc_Tree_Node *tn)
{
  if (tn != NULL) {
    Swc_Tree_Node *child = SwcTreeNode::firstChild(tn);
    while (child != NULL) {
      backup(child);
      child = SwcTreeNode::nextSibling(child);
    }
  }
}

void ZStackDocCommand::SwcEdit::ChangeSwcCommand::backupChildren(
    Swc_Tree_Node *tn, EOperation op, ERole role)
{
  if (tn != NULL) {
    Swc_Tree_Node *child = SwcTreeNode::firstChild(tn);
    while (child != NULL) {
      backup(child, op, role);
      child = SwcTreeNode::nextSibling(child);
    }
  }
}

void ZStackDocCommand::SwcEdit::ChangeSwcCommand::backup(
    Swc_Tree_Node *tn, EOperation op, ERole role)
{
  if (tn == NULL) {
    return;
  }

  backup(tn);

  switch (op) {
  case OP_SET_FIRST_CHILD:
    switch (role) {
    case ROLE_CHILD:
      backup(SwcTreeNode::parent(tn));
      backup(SwcTreeNode::nextSibling(tn));
      backup(SwcTreeNode::prevSibling(tn));
      break;
    case ROLE_PARENT:
      backup(SwcTreeNode::firstChild(tn));
      break;
    default:
      break;
    }
    break;
  case OP_SET_PARENT:
    switch (role) {
    case ROLE_CHILD:
      backup(SwcTreeNode::parent(tn));
      backup(SwcTreeNode::nextSibling(tn));
      backup(SwcTreeNode::prevSibling(tn));
      break;
    case ROLE_PARENT:
      backup(SwcTreeNode::lastChild(tn));
      break;
    default:
      break;
    }
    break;
  case OP_DETACH_PARENT:
    backup(SwcTreeNode::parent(tn));
    backup(SwcTreeNode::nextSibling(tn));
    backup(SwcTreeNode::prevSibling(tn));
    break;
  }
}

void ZStackDocCommand::SwcEdit::ChangeSwcCommand::addNewNode(Swc_Tree_Node *tn)
{
  backup(tn);
  m_newNodeSet.insert(tn);
}

void ZStackDocCommand::SwcEdit::ChangeSwcCommand::recordRemovedNode(Swc_Tree_Node *tn)
{
  m_removedNodeSet.insert(tn);
}

///////////////////////////////////////////////
class ChangeSwcCommand : public ZUndoCommand
{
public:
  ChangeSwcCommand(ZStackDoc *doc);
  virtual ~ChangeSwcCommand();
  void undo();
  void redo();

private:
  ZStackDoc *m_doc;
  std::map<Swc_Tree_Node*, Swc_Tree_Node> m_backupSet;
  std::set<Swc_Tree_Node*> m_newNodeSet;
  std::set<Swc_Tree_Node*> m_garbageSet;
};

ZStackDocCommand::SwcEdit::TranslateRoot::TranslateRoot(
    ZStackDoc *doc, double x, double y, double z, QUndoCommand *parent)
  :ZUndoCommand(parent), m_doc(doc), m_x(x), m_y(y), m_z(z)
{
  setText(QObject::tr("translate swc tree root to (%1,%2,%3)").
          arg(m_x).arg(m_y).arg(m_z));
}

ZStackDocCommand::SwcEdit::TranslateRoot::~TranslateRoot()
{
  for (int i=0; i<m_swcList.size(); i++) {
    delete m_swcList[i];
  }
}

void ZStackDocCommand::SwcEdit::TranslateRoot::undo()
{
  startUndo();
//  m_doc->blockSignals(true);
  m_doc->beginObjectModifiedMode(ZStackDoc::OBJECT_MODIFIED_CACHE);
  for (int i = 0; i < m_doc->getSwcList().size(); i++) {
    m_doc->removeObject(m_doc->getSwcTree(i), true);
  }
  for (int i=0; i<m_swcList.size(); i++) {
    m_doc->addObject(m_swcList[i]);
  }
  m_swcList.clear();
  m_doc->updateVirtualStackSize();

  m_doc->endObjectModifiedMode();
  m_doc->notifyObjectModified();
//  m_doc->blockSignals(false);
//  m_doc->notifySwcModified();
}

void ZStackDocCommand::SwcEdit::TranslateRoot::redo()
{
//  m_doc->blockSignals(true);
  m_swcList.clear();
  for (int i=0; i<m_doc->getSwcList().size(); i++) {
    ZSwcTree *doctree = m_doc->getSwcList()[i]->clone();
    m_swcList.push_back(doctree);
  }
  m_doc->swcTreeTranslateRootTo(m_x, m_y, m_z);
  m_doc->updateVirtualStackSize();


//  m_doc->blockSignals(false);
//  m_doc->notifySwcModified();
}

ZStackDocCommand::SwcEdit::Rescale::Rescale(
    ZStackDoc *doc, double scaleX, double scaleY, double scaleZ, QUndoCommand *parent)
  :ZUndoCommand(parent), m_doc(doc), m_scaleX(scaleX), m_scaleY(scaleY), m_scaleZ(scaleZ)
{
  setText(QObject::tr("rescale swc tree (%1,%2,%3)").arg(scaleX).arg(scaleY).arg(scaleZ));
}

ZStackDocCommand::SwcEdit::Rescale::Rescale(
    ZStackDoc *doc, double srcPixelPerUmXY, double srcPixelPerUmZ,
    double dstPixelPerUmXY, double dstPixelPerUmZ, QUndoCommand *parent)
  :ZUndoCommand(parent), m_doc(doc)
{
  m_scaleX = dstPixelPerUmXY/srcPixelPerUmXY;
  m_scaleY = m_scaleX;
  m_scaleZ = dstPixelPerUmZ/srcPixelPerUmZ;
  setText(QObject::tr("rescale swc tree (%1,%2,%3)").
          arg(m_scaleX).arg(m_scaleY).arg(m_scaleZ));
}

ZStackDocCommand::SwcEdit::Rescale::~Rescale()
{
  for (int i=0; i<m_swcList.size(); i++) {
    delete m_swcList[i];
  }
}

void ZStackDocCommand::SwcEdit::Rescale::undo()
{
  startUndo();
//  m_doc->blockSignals(true);

  m_doc->beginObjectModifiedMode(ZStackDoc::OBJECT_MODIFIED_CACHE);
  m_doc->removeAllSwcTree(true);

//  for (QList<ZSwcTree*>::iterator iter = m_doc->getSwcIteratorBegin();
//       iter != m_doc->getSwcIteratorEnd(); ++iter) {
//    m_doc->removeObject(*iter, true);
//  }

  m_doc->addSwcTree(m_swcList);

  m_swcList.clear();
  m_doc->updateVirtualStackSize();

  m_doc->endObjectModifiedMode();
  m_doc->notifyObjectModified();

//  m_doc->blockSignals(false);
//  m_doc->notifySwcModified();
}

void ZStackDocCommand::SwcEdit::Rescale::redo()
{
//  m_doc->blockSignals(true);
  m_doc->beginObjectModifiedMode(ZStackDoc::OBJECT_MODIFIED_CACHE);

  m_swcList.clear();

  QList<ZSwcTree*> swcList = m_doc->getSwcList();
  for (QList<ZSwcTree*>::iterator iter = swcList.begin();
       iter != swcList.end(); ++iter) {
    ZSwcTree *doctree = (*iter)->clone();
    m_swcList.push_back(doctree);
  }

  m_doc->swcTreeRescale(m_scaleX, m_scaleY, m_scaleZ);
  m_doc->updateVirtualStackSize();

  m_doc->endObjectModifiedMode();
  m_doc->notifyObjectModified();
//  m_doc->blockSignals(false);
//  m_doc->notifySwcModified();
}

ZStackDocCommand::SwcEdit::RescaleRadius::RescaleRadius(
    ZStackDoc *doc, double scale, int startdepth, int enddepth, QUndoCommand *parent)
  :ZUndoCommand(parent), m_doc(doc), m_scale(scale), m_startdepth(startdepth),
    m_enddepth(enddepth)
{
  if (enddepth < 0) {
    setText(QObject::tr("rescale radius of swc nodes with depth in [%1,max) by %2").
            arg(startdepth).arg(scale));
  } else {
    setText(QObject::tr("rescale radius of swc nodes with depth in [%1,%2) by %3").
            arg(startdepth).arg(enddepth).arg(scale));
  }
}

ZStackDocCommand::SwcEdit::RescaleRadius::~RescaleRadius()
{
  for (int i=0; i<m_swcList.size(); i++) {
    delete m_swcList[i];
  }
}

void ZStackDocCommand::SwcEdit::RescaleRadius::undo()
{
  startUndo();
//  m_doc->blockSignals(true);
  m_doc->beginObjectModifiedMode(ZStackDoc::OBJECT_MODIFIED_CACHE);

  m_doc->removeAllSwcTree(true);
  m_doc->addSwcTree(m_swcList);
  m_swcList.clear();
  m_doc->updateVirtualStackSize();

  m_doc->endObjectModifiedMode();
  m_doc->notifyObjectModified();

//  m_doc->blockSignals(false);

//  m_doc->notifySwcModified();
}

void ZStackDocCommand::SwcEdit::RescaleRadius::redo()
{
//  m_doc->blockSignals(true);
  m_swcList.clear();
  QList<ZSwcTree*> swcList = m_doc->getSwcList();
  for (QList<ZSwcTree*>::const_iterator iter = swcList.begin();
       iter != swcList.end(); ++iter) {
    ZSwcTree *doctree = (*iter)->clone();
    m_swcList.push_back(doctree);
  }
  m_doc->swcTreeRescaleRadius(m_scale, m_startdepth, m_enddepth);
  m_doc->updateVirtualStackSize();

//  m_doc->blockSignals(false);

//  m_doc->notifySwcModified();
}

ZStackDocCommand::SwcEdit::ReduceNodeNumber::ReduceNodeNumber(
    ZStackDoc *doc, double lengthThre, QUndoCommand *parent)
  :ZUndoCommand(parent), m_doc(doc), m_lengthThre(lengthThre)
{
  setText(QObject::tr("reduce number of swc node use length thre %1").arg(lengthThre));
}

ZStackDocCommand::SwcEdit::ReduceNodeNumber::~ReduceNodeNumber()
{
  for (int i=0; i<m_swcList.size(); i++) {
    delete m_swcList[i];
  }
}

void ZStackDocCommand::SwcEdit::ReduceNodeNumber::undo()
{
  startUndo();
//  m_doc->blockSignals(true);
  m_doc->beginObjectModifiedMode(ZStackDoc::OBJECT_MODIFIED_CACHE);
  m_doc->removeAllSwcTree(true);
  m_doc->addSwcTree(m_swcList);
  m_swcList.clear();

  m_doc->endObjectModifiedMode();
  m_doc->notifyObjectModified();
//  m_doc->blockSignals(false);

//  m_doc->notifySwcModified();
}

void ZStackDocCommand::SwcEdit::ReduceNodeNumber::redo()
{
//  m_doc->blockSignals(true);
  m_swcList.clear();
  QList<ZSwcTree*> swcList = m_doc->getSwcList();
  for (QList<ZSwcTree*>::const_iterator iter = swcList.begin();
       iter != swcList.end(); ++iter) {
    ZSwcTree *doctree = (*iter)->clone();
    m_swcList.push_back(doctree);
  }
  m_doc->swcTreeReduceNodeNumber(m_lengthThre);
//  m_doc->blockSignals(false);

//  m_doc->notifySwcModified();
}

ZStackDocCommand::SwcEdit::AddSwc::AddSwc(
    ZStackDoc *doc, ZSwcTree *tree, QUndoCommand *parent) :
  ZUndoCommand(parent), m_doc(doc), m_tree(tree), m_isInDoc(false)
{
  setText(QObject::tr("Add swc"));
}

ZStackDocCommand::SwcEdit::AddSwc::~AddSwc()
{
#ifdef _DEBUG_
  std::cout << "ZStackDocCommand::SwcEdit::AddSwc destroyed" << std::endl;
#endif
  if (!m_isInDoc) {
    delete m_tree;
  }
}

void ZStackDocCommand::SwcEdit::AddSwc::redo()
{
  m_doc->addObject(m_tree, false);
  m_isInDoc = true;
//  m_doc->notifySwcModified();
}

void ZStackDocCommand::SwcEdit::AddSwc::undo()
{
  startUndo();
  m_doc->removeObject(m_tree, false);
  m_isInDoc = false;
//  m_doc->notifySwcModified();
}

int ZStackDocCommand::SwcEdit::AddSwcNode::m_index = 1;

ZStackDocCommand::SwcEdit::AddSwcNode::AddSwcNode(
    ZStackDoc *doc, Swc_Tree_Node *tn, ZStackObjectRole::TRole role,
    QUndoCommand *parent)
  : ZUndoCommand(parent), m_doc(doc), m_node(tn), m_treeInDoc(false)
{
  setText(QObject::tr("Add Neuron Node"));
  m_tree = new ZSwcTree();
  m_tree->setRole(role);
//  if (doc->getTag() == NeuTube::Document::FLYEM_ROI) {
  if (ZStackObjectRole(role).hasRole(ZStackObjectRole::ROLE_ROI)) {
    m_tree->useCosmeticPen(true);
    m_tree->setStructrualMode(ZSwcTree::STRUCT_CLOSED_CURVE);
    m_tree->removeVisualEffect(NeuTube::Display::SwcTree::VE_FULL_SKELETON);
//    m_tree->setRole(ZStackObjectRole::ROLE_ROI);
  }

  m_tree->setDataFromNode(m_node);
  m_tree->setSource(QString("#added by add neuron node command %1").
                    arg(m_index++).toStdString());
}

ZStackDocCommand::SwcEdit::AddSwcNode::~AddSwcNode()
{
  if (!m_treeInDoc) {
    delete m_tree;
  }
#ifdef _DEBUG_2
  std::cout << "Command AddSwcNode destroyed" << endl;
#endif
}

void ZStackDocCommand::SwcEdit::AddSwcNode::undo()
{
  startUndo();
//  m_doc->blockSignals(true);
  m_doc->beginObjectModifiedMode(ZStackDoc::OBJECT_MODIFIED_CACHE);
  m_doc->removeObject(m_tree);
  m_doc->setSwcSelected(m_tree, false);
  m_doc->deselectSwcTreeNode(m_node);
  //m_doc->setSwcTreeNodeSelected(m_node, false);
//  m_doc->blockSignals(false);
  m_treeInDoc = false;

  m_doc->endObjectModifiedMode();
  m_doc->notifyObjectModified();
//  m_doc->notifySwcModified();
}

void ZStackDocCommand::SwcEdit::AddSwcNode::redo()
{
  m_doc->addObject(m_tree);
  m_treeInDoc = true;
}

ZStackDocCommand::SwcEdit::RemoveSubtree::RemoveSubtree(
    ZStackDoc *doc, Swc_Tree_Node *node, QUndoCommand *parent) :
  CompositeCommand(doc, parent)
{
  new SetParent(doc, node, NULL, false, parent);
}

ZStackDocCommand::SwcEdit::RemoveSubtree::~RemoveSubtree()
{
#ifdef _DEBUG_
  std::cout << "RemoveSubtree destroyed." << std::endl;
#endif

  if (m_node != NULL) {
    SwcTreeNode::killSubtree(m_node);
  }
}
#if 0
ZStackDocCommand::SwcEdit::BreakParentLink::BreakParentLink(
    ZStackDoc *doc, QUndoCommand *parent) : ChangeSwcCommand(doc, parent)
{

}

ZStackDocCommand::SwcEdit::BreakParentLink::~BreakParentLink()
{
#ifdef _DEBUG_
    std::cout << "SwcEdit::BreakParentLink destroyed" << std::endl;
#endif
}

ZStackDocCommand::SwcEdit::BreakParentLink::redo()
{
  backup()
}
#endif

////////////////////////////////////////////////
ZStackDocCommand::SwcEdit::MergeSwcNode::MergeSwcNode(
    ZStackDoc *doc, QUndoCommand *parent) : ChangeSwcCommand(doc, parent)
{
  setText(QObject::tr("Merge swc nodes"));
  m_coreNode = NULL;
}

void ZStackDocCommand::SwcEdit::MergeSwcNode::undo()
{
  startUndo();
  recover();
  m_doc->deselectAllSwcTreeNodes();
  m_doc->setSwcTreeNodeSelected(
        m_selectedNodeSet.begin(), m_selectedNodeSet.end(), true);
//  m_selectedNodeSet.clear();
}

void ZStackDocCommand::SwcEdit::MergeSwcNode::redo()
{
  Swc_Tree_Node *coreNode = m_coreNode;
  m_garbageSet.clear();

  std::set<Swc_Tree_Node*> nodeSet = m_selectedNodeSet;
  if (nodeSet.empty()) {
    nodeSet = m_doc->getSelectedSwcNodeSet();
    m_selectedNodeSet = m_doc->getSelectedSwcNodeSet();
    m_doc->deselectAllSwcTreeNodes();
  }

  if (nodeSet.size() > 1) {
    ZPoint center = SwcTreeNode::centroid(nodeSet);
    double radius = SwcTreeNode::maxRadius(nodeSet);

    if (coreNode == NULL) {
      coreNode = SwcTreeNode::makePointer(center, radius);
      m_coreNode = coreNode;
    }

    addNewNode(coreNode);
#ifdef _DEBUG_
    std::cout << coreNode << " created." << std::endl;
#endif

    set<Swc_Tree_Node*> parentSet;
    //set<Swc_Tree_Node*> childSet;

    for (set<Swc_Tree_Node*>::iterator iter = nodeSet.begin();
         iter != nodeSet.end(); ++iter) {
      TZ_ASSERT(*iter != NULL, "Null swc node");

      if (SwcTreeNode::isRegular(SwcTreeNode::parent(*iter))) {
        if (nodeSet.count(SwcTreeNode::parent(*iter)) == 0) {
          parentSet.insert(parentSet.end(), SwcTreeNode::parent(*iter));
        }
      }

      Swc_Tree_Node *child = SwcTreeNode::firstChild(*iter);
      while (child != NULL) {
        Swc_Tree_Node *nextChild = SwcTreeNode::nextSibling(child);
        if (nodeSet.count(child) == 0) {

//          new SwcEdit::SetParent(m_doc, child, coreNode, false, this);
          backup(child, OP_SET_FIRST_CHILD, ROLE_CHILD);

          SwcTreeNode::setFirstChild(coreNode, child);
//          SwcTreeNode::setParent(child, coreNode);

          //childSet.insert(childSet.end(), child);
        }
        child = nextChild;
      }
    }

    if (parentSet.empty()) { //try to attach to a virtual root
      for (set<Swc_Tree_Node*>::iterator iter = nodeSet.begin();
           iter != nodeSet.end(); ++iter) {
        if (SwcTreeNode::isVirtual(SwcTreeNode::parent(*iter))) {
//          new SwcEdit::SetParent(
//                m_doc, coreNode, SwcTreeNode::parent(*iter), false, this);
//          SwcTreeNode::setParent(coreNode, parent(*iter));

          Swc_Tree_Node *parentNode = SwcTreeNode::parent(*iter);
          backup(parentNode, OP_SET_FIRST_CHILD, ROLE_PARENT);
//          backup(parentNode);
//          backup(SwcTreeNode::firstChild(parentNode));
          SwcTreeNode::setFirstChild(SwcTreeNode::parent(*iter), coreNode);
          break;
        }
      }
    } else if (parentSet.size() > 1) {
      for (set<Swc_Tree_Node*>::iterator iter = parentSet.begin();
           iter != parentSet.end(); ++iter) {
//        SwcTreeNode::setParent(*iter, coreNode);
//        backup(*iter);
//        backup(SwcTreeNode::parent(*iter));
//        backup(SwcTreeNode::nextSibling(*iter));
//        backup(SwcTreeNode::prevSibling(*iter));
        backup(*iter, OP_SET_FIRST_CHILD, ROLE_CHILD);
        SwcTreeNode::setFirstChild(coreNode, *iter);
//        new SwcEdit::SetParent(m_doc, *iter, coreNode, false, this);
      }
    } else {
      Swc_Tree_Node *parentNode = *parentSet.begin();
//      backup(parentNode);
//      backup(SwcTreeNode::firstChild(parentNode));
      backup(parentNode, OP_SET_FIRST_CHILD, ROLE_PARENT);
      SwcTreeNode::setFirstChild(parentNode, coreNode);
      //SwcTreeNode::setParent(coreNode, *parentSet.begin());
//      new SwcEdit::SetParent(m_doc, coreNode, *parentSet.begin(), false, this);
    }

    for (set<Swc_Tree_Node*>::iterator iter = nodeSet.begin();
         iter != nodeSet.end(); ++iter) {
      //new SwcEdit::SetParent(m_doc, *iter, NULL, this);
      if (SwcTreeNode::parent(*iter) != NULL) { //orphan node aready handled by SetParent
        Swc_Tree_Node *root = SwcTreeNode::root(*iter);

        backup(*iter, OP_DETACH_PARENT);
        backupChildren(*iter, OP_SET_FIRST_CHILD, ROLE_CHILD);
        backup(root, OP_SET_FIRST_CHILD, ROLE_PARENT);

        SwcTreeNode::detachParent(*iter);

//        new SwcEdit::DeleteSwcNode(m_doc, *iter, SwcTreeNode::root(*iter), this);
      }
    }

    if (!m_backupSet.empty()) {
      setSwcModified(true);
      //m_doc->selectedSwcTreeNodes()->clear();

      m_doc->processSwcModified();
      m_doc->notifyObjectModified();

      m_doc->deprecateTraceMask();
    }

    if (coreNode != NULL) {
      m_doc->selectSwcTreeNode(coreNode);
    }
  }
}

ZStackDocCommand::SwcEdit::MergeSwcNode::~MergeSwcNode()
{
#ifdef _DEBUG_
    std::cout << "MergeSwcNode destroyed" << std::endl;
#endif
}

/////////////////////////////////////////////
ZStackDocCommand::SwcEdit::ResolveCrossover::ResolveCrossover(
    ZStackDoc *doc, QUndoCommand *parent) : ChangeSwcCommand(doc, parent)
{
  setText(QObject::tr("Resolve crossover"));
}

void ZStackDocCommand::SwcEdit::ResolveCrossover::undo()
{
  startUndo();
  recover();
  m_doc->deselectAllSwcTreeNodes();
  m_doc->setSwcTreeNodeSelected(
        m_selectedNodeSet.begin(), m_selectedNodeSet.end(), true);
//  m_selectedNodeSet.clear();
}

void ZStackDocCommand::SwcEdit::ResolveCrossover::redo()
{
  std::set<Swc_Tree_Node*> nodeSet = m_selectedNodeSet;
  if (nodeSet.empty()) {
    nodeSet = m_doc->getSelectedSwcNodeSet();
    m_selectedNodeSet = m_doc->getSelectedSwcNodeSet();
    m_doc->deselectAllSwcTreeNodes();
  }

  if (nodeSet.size() == 1) {
    Swc_Tree_Node *center = *(nodeSet.begin());
    size_t centerNeighborCount = SwcTreeNode::neighborArray(center).size();
    std::map<Swc_Tree_Node*, Swc_Tree_Node*> matched =
        SwcTreeNode::crossoverMatch(center, TZ_PI_2);
    if (!matched.empty()) {
      Swc_Tree_Node *root = SwcTreeNode::root(center);
      for (std::map<Swc_Tree_Node*, Swc_Tree_Node*>::const_iterator
           iter = matched.begin(); iter != matched.end(); ++iter) {
        if (SwcTreeNode::parent(iter->first) == center &&
            SwcTreeNode::parent(iter->second) == center) {
          backup(iter->first, OP_SET_PARENT, ROLE_CHILD);
          backup(iter->second, OP_SET_PARENT, ROLE_PARENT);
          SwcTreeNode::setParent(iter->first, iter->second);

          backup(root, OP_SET_PARENT, ROLE_PARENT);
          SwcTreeNode::setParent(iter->second, root);
        } else {
          backup(center, OP_SET_PARENT, ROLE_CHILD);
          backup(root, OP_SET_PARENT, ROLE_PARENT);
          SwcTreeNode::setParent(center, root);
          if (SwcTreeNode::parent(iter->first) == center) {
            backup(iter->first, OP_SET_PARENT, ROLE_CHILD);
            backup(iter->second, OP_SET_PARENT, ROLE_PARENT);
            SwcTreeNode::setParent(iter->first, iter->second);
          } else {
            backup(iter->second, OP_SET_PARENT, ROLE_CHILD);
            backup(iter->first, OP_SET_PARENT, ROLE_PARENT);
            SwcTreeNode::setParent(iter->second, iter->first);
          }
        }

        if (matched.size() * 2 == centerNeighborCount) {
          backup(center, OP_DETACH_PARENT, ROLE_CHILD);
          SwcTreeNode::detachParent(center);
          m_removedNodeSet.insert(center);
          /*
          new ZStackDocCommand::SwcEdit::SetParent(
                this, center, NULL, true, command);
                */
          /*
          new ZStackDocCommand::SwcEdit::DeleteSwcNode(
                this, center, root, command);
                */
        }
      }

      if (!m_backupSet.empty()) {
        setSwcModified(true);
        m_doc->deprecateTraceMask();

        m_doc->processSwcModified();
        m_doc->notifyObjectModified();
      }
    }
  }
}

ZStackDocCommand::SwcEdit::ResolveCrossover::~ResolveCrossover()
{
#ifdef _DEBUG_
    std::cout << "SwcEdit::ResolveCrossover destroyed" << std::endl;
#endif
}


//////////////////////////////////////////////////
ZStackDocCommand::SwcEdit::ExtendSwcNode::ExtendSwcNode(
    ZStackDoc *doc, Swc_Tree_Node *node, Swc_Tree_Node *pnode,
    QUndoCommand *parent)
  : ZUndoCommand(parent), m_doc(doc), m_node(node), m_parentNode(pnode),
    m_nodeInDoc(false)
{
  setText(QObject::tr("Extend Selected Swc Node"));
}

ZStackDocCommand::SwcEdit::ExtendSwcNode::~ExtendSwcNode()
{
  if (!m_nodeInDoc) {
#ifdef _DEBUG_
    std::cout << "ExtendSwcNode destroyed" << std::endl;
#endif
    SwcTreeNode::kill(m_node);
  }
}

void ZStackDocCommand::SwcEdit::ExtendSwcNode::undo()
{
  startUndo();
  // after undo, m_parentNode should be the only selected node
  SwcTreeNode::detachParent(m_node);

  m_doc->blockSignals(true);
  m_doc->deselectAllSwcTreeNodes();
  m_doc->selectSwcTreeNode(m_parentNode);
  m_doc->blockSignals(false);
//  m_doc->notifySwcModified();

  m_doc->processSwcModified();
  m_doc->notifyObjectModified();

  m_doc->notifySwcTreeNodeSelectionChanged();
  m_nodeInDoc = false;
}

void ZStackDocCommand::SwcEdit::ExtendSwcNode::redo()
{
  // after redo, m_node will be the only selected node, this allows continuous extending

  SwcTreeNode::setParent(m_node, m_parentNode);

  m_doc->blockSignals(true);
  m_doc->deselectAllSwcTreeNodes();
  m_doc->selectSwcTreeNode(m_node);
  m_doc->blockSignals(false);
//  m_doc->notifySwcModified();

  m_doc->processSwcModified();
  m_doc->notifyObjectModified();
  m_doc->notifySwcTreeNodeSelectionChanged();

  m_nodeInDoc = true;
}

ZStackDocCommand::SwcEdit::ChangeSwcNodeGeometry::ChangeSwcNodeGeometry(
    ZStackDoc *doc, Swc_Tree_Node *node, double x, double y, double z, double r,
    QUndoCommand *parent) :
  ZUndoCommand(parent), m_doc(doc), m_node(node), m_x(x), m_y(y), m_z(z), m_r(r),
  m_backupX(0.0), m_backupY(0.0), m_backupZ(0.0), m_backupR(0.0)
{
  setText(QObject::tr("Change geometry of Selected Swc Node"));
}

ZStackDocCommand::SwcEdit::ChangeSwcNodeGeometry::~ChangeSwcNodeGeometry()
{

}

void ZStackDocCommand::SwcEdit::ChangeSwcNodeGeometry::redo()
{
  if (m_node != NULL) {
    m_backupX = SwcTreeNode::x(m_node);
    m_backupY = SwcTreeNode::y(m_node);
    m_backupZ = SwcTreeNode::z(m_node);
    m_backupR = SwcTreeNode::radius(m_node);
    SwcTreeNode::setPos(m_node, m_x, m_y, m_z);
    SwcTreeNode::setRadius(m_node, m_r);

    m_doc->processSwcModified();
//    m_doc->notifySwcModified();
  }
}

void ZStackDocCommand::SwcEdit::ChangeSwcNodeGeometry::undo()
{
  startUndo();
  if (m_node != NULL) {
    SwcTreeNode::setPos(m_node, m_backupX, m_backupY, m_backupZ);
    SwcTreeNode::setRadius(m_node, m_backupR);

    m_doc->processSwcModified();
//    m_doc->notifySwcModified();
  }
}

ZStackDocCommand::SwcEdit::ChangeSwcNodeZ::ChangeSwcNodeZ(
    ZStackDoc *doc, Swc_Tree_Node *node, double z, QUndoCommand *parent) :
  ZUndoCommand(parent), m_doc(doc), m_node(node), m_z(z), m_backup(0.0)
{
  setText(QObject::tr("Change Z of Selected Swc Node"));
}

ZStackDocCommand::SwcEdit::ChangeSwcNodeZ::~ChangeSwcNodeZ()
{

}

void ZStackDocCommand::SwcEdit::ChangeSwcNodeZ::redo()
{
  if (m_node != NULL) {
    m_backup = SwcTreeNode::z(m_node);
    SwcTreeNode::setZ(m_node, m_z);

    m_doc->processSwcModified();
//    m_doc->notifySwcModified();
  }
}

void ZStackDocCommand::SwcEdit::ChangeSwcNodeZ::undo()
{
  startUndo();
  if (m_node != NULL) {
    SwcTreeNode::setZ(m_node, m_backup);
//    m_doc->notifySwcModified();
    m_doc->processSwcModified();
  }
}

//////////////////
ZStackDocCommand::SwcEdit::ChangeSwcNodeRadius::ChangeSwcNodeRadius(
    ZStackDoc *doc, Swc_Tree_Node *node, double radius, QUndoCommand *parent) :
  ZUndoCommand(parent), m_doc(doc), m_node(node), m_radius(radius), m_backup(0.0)
{
  setText(QObject::tr("Change Radius of Selected Swc Node"));
}

ZStackDocCommand::SwcEdit::ChangeSwcNodeRadius::~ChangeSwcNodeRadius()
{

}

void ZStackDocCommand::SwcEdit::ChangeSwcNodeRadius::redo()
{
  if (m_node != NULL) {
    m_backup = SwcTreeNode::radius(m_node);
    SwcTreeNode::setRadius(m_node, m_radius);
//    m_doc->notifySwcModified();
    m_doc->processSwcModified();
  }
}

void ZStackDocCommand::SwcEdit::ChangeSwcNodeRadius::undo()
{
  startUndo();
  if (m_node != NULL) {
    SwcTreeNode::setRadius(m_node, m_backup);
//    m_doc->notifySwcModified();
    m_doc->processSwcModified();
  }
}


/////////////////////////

ZStackDocCommand::SwcEdit::ChangeSwcNode::ChangeSwcNode(
    ZStackDoc *doc, Swc_Tree_Node *node, const Swc_Tree_Node &newNode,
    QUndoCommand *parent) : ZUndoCommand(parent), m_doc(doc), m_node(node)
{
  m_newNode = newNode;
}

ZStackDocCommand::SwcEdit::ChangeSwcNode::~ChangeSwcNode()
{

}

void ZStackDocCommand::SwcEdit::ChangeSwcNode::redo()
{
  if (m_node != NULL) {
    m_backup = *m_node;
    *m_node = m_newNode;

    m_doc->processSwcModified();
  }
}

void ZStackDocCommand::SwcEdit::ChangeSwcNode::undo()
{
  startUndo();
  if (m_node != NULL) {
    *m_node = m_backup;
    m_doc->processSwcModified();
  }
}

ZStackDocCommand::SwcEdit::DeleteSwcNode::DeleteSwcNode(
    ZStackDoc *doc, Swc_Tree_Node *node, Swc_Tree_Node *root,
    QUndoCommand *parent) :
  CompositeCommand(doc, parent), m_doc(doc), m_node(node), m_root(root),
  m_prevSibling(NULL), m_lastChild(NULL), m_nodeInDoc(true)
{
  TZ_ASSERT(m_root != NULL, "Null root");
  setText(QObject::tr("Delete swc node"));

  new SetParent(doc, m_node, NULL, false, this);
  Swc_Tree_Node *child = SwcTreeNode::firstChild(m_node);
  while (child != NULL) {
    new SetParent(doc, child, m_root, false, this);
    child = SwcTreeNode::nextSibling(child);
  }

  //m_nodeInDoc(false);
  //SwcTreeNode::setDefault(&m_backup);
}

ZStackDocCommand::SwcEdit::DeleteSwcNode::~DeleteSwcNode()
{
  if (SwcTreeNode::parent(m_node) == NULL && m_isExecuted) {
    m_nodeInDoc = false;
  }

  if (!m_nodeInDoc) {
#ifdef _DEBUG_
    std::cout << "DeleteSwcNode destroyed" << std::endl;
#endif
    SwcTreeNode::kill(m_node);
  }
}


ZStackDocCommand::SwcEdit::DeleteSwcNodeSet::DeleteSwcNodeSet(
    ZStackDoc *doc, std::set<Swc_Tree_Node*> &nodeSet,
    QUndoCommand *parent) :
  CompositeCommand(doc, parent), m_doc(doc), m_nodeSet(nodeSet),
  m_nodeInDoc(true)
{
  //TZ_ASSERT(root != NULL, "Null root");
  setText(QObject::tr("Delete swc node"));

  for (std::set<Swc_Tree_Node*>::iterator iter = m_nodeSet.begin();
       iter != m_nodeSet.end(); ++iter) {
    Swc_Tree_Node *node = *iter;
    new SetParent(doc, node, NULL, false, this);
    Swc_Tree_Node *child = SwcTreeNode::firstChild(node);
    while (child != NULL) {
      if (m_nodeSet.count(child) == 0) { //Not in the deletion set
        new SetParent(doc, child, SwcTreeNode::root(child), false, this);
      }
      child = SwcTreeNode::nextSibling(child);
    }
  }

  m_nodeInDoc = false;

  //SwcTreeNode::setDefault(&m_backup);
}

ZStackDocCommand::SwcEdit::DeleteSwcNodeSet::~DeleteSwcNodeSet()
{
  if (!m_nodeInDoc && m_isExecuted) {
#ifdef _DEBUG_
    std::cout << "DeleteSwcNode destroyed" << std::endl;
#endif

    for (std::set<Swc_Tree_Node*>::iterator iter = m_nodeSet.begin();
         iter != m_nodeSet.end(); ++iter) {
      if (SwcTreeNode::parent(*iter) == NULL) {
        SwcTreeNode::kill(*iter);
      }
    }
      //SwcTreeNode::kill(m_node);
  }
}

ZStackDocCommand::SwcEdit::CompositeCommand::CompositeCommand(
    ZStackDoc *doc, QUndoCommand *parent) :
  ZUndoCommand(parent), m_doc(doc), m_isExecuted(false)
{
}

ZStackDocCommand::SwcEdit::CompositeCommand::~CompositeCommand()
{
  ZOUT(LTRACE(), 5) << "Composite command (" << this->text() << ") destroyed";
}

void ZStackDocCommand::SwcEdit::CompositeCommand::redo()
{
//  m_doc->blockSignals(true);
  m_doc->beginObjectModifiedMode(ZStackDoc::OBJECT_MODIFIED_CACHE);
  QUndoCommand::redo();
  m_doc->endObjectModifiedMode();
  m_doc->notifyObjectModified();

//  m_doc->blockSignals(false);
//  m_doc->notifySwcModified();
  m_doc->notifySwcTreeNodeSelectionChanged();
  m_isExecuted = true;
}


void ZStackDocCommand::SwcEdit::CompositeCommand::undo()
{
  startUndo();
//  m_doc->blockSignals(true);

  m_doc->beginObjectModifiedMode(ZStackDoc::OBJECT_MODIFIED_CACHE);
  QUndoCommand::undo();
  m_doc->endObjectModifiedMode();
  m_doc->notifyObjectModified();

//  m_doc->blockSignals(false);
//  m_doc->notifySwcModified();
  m_doc->notifySwcTreeNodeSelectionChanged();
  m_isExecuted = false;
}

ZStackDocCommand::SwcEdit::SetParent::SetParent(ZStackDoc *doc, Swc_Tree_Node *node, Swc_Tree_Node *parentNode,
    bool deletingOrphan, QUndoCommand *parent) :
  ZUndoCommand(parent), m_doc(doc), m_node(node), m_newParent(parentNode),
  m_oldParent(NULL), m_prevSibling(NULL), m_deletingOrphan(deletingOrphan),
  m_isExecuted(false)
{
  //TZ_ASSERT(parentNode != NULL, "Null pointer");
#ifdef _DEBUG_
  std::cout << m_newParent << " --> " << m_node << std::endl;
#endif

  setText(QObject::tr("Set swc node parent"));
}

ZStackDocCommand::SwcEdit::SetParent::~SetParent()
{
  if (m_node != NULL && m_isExecuted) {
    if (m_deletingOrphan && SwcTreeNode::parent(m_node) == NULL) { //orphan node
      SwcTreeNode::killSubtree(m_node);
    }
  }
}

void ZStackDocCommand::SwcEdit::SetParent::redo()
{
  m_oldParent = SwcTreeNode::parent(m_node);

#ifdef _DEBUG_
  if (m_newParent == NULL) {
    std::cout << "ZStackDocCommand::SwcEdit::SetParent : Null parent" << std::endl;
  }
#endif

  if (m_oldParent != m_newParent) {
    m_prevSibling = SwcTreeNode::prevSibling(m_node);
    SwcTreeNode::setParent(m_node, m_newParent);
    m_doc->processSwcModified();
//    m_doc->notifySwcModified();
  }
  m_isExecuted = true;
}

void ZStackDocCommand::SwcEdit::SetParent::undo()
{
  startUndo();
  if (m_oldParent != m_newParent) {
    //Recover child set of the new parent
    SwcTreeNode::detachParent(m_node);

    //Recover its supervisor
    Swc_Tree_Node *nextSibling = NULL;
    if (m_prevSibling == NULL) {
      nextSibling = SwcTreeNode::firstChild(m_oldParent);
      SwcTreeNode::setLink(m_oldParent, m_node,
                           SwcTreeNode::FIRST_CHILD);
    } else {
      nextSibling = SwcTreeNode::nextSibling(m_prevSibling);
      SwcTreeNode::setLink(m_prevSibling, m_node, SwcTreeNode::NEXT_SIBLING);
    }

    //Recover forward set
    SwcTreeNode::setLink(m_node, m_oldParent, SwcTreeNode::PARENT);
    SwcTreeNode::setLink(m_node, nextSibling, SwcTreeNode::NEXT_SIBLING);

    m_doc->processSwcModified();
//    m_doc->notifySwcModified();
  }
  m_isExecuted = false;
}

////////////////////////////////
#if 1
ZStackDocCommand::SwcEdit::SetSwcNodeSeletion::SetSwcNodeSeletion(
    ZStackDoc *doc, ZSwcTree *host, const std::set<Swc_Tree_Node *> nodeSet,
    bool appending, QUndoCommand *parent) :
  ZUndoCommand(parent), m_doc(doc), m_host(host), m_nodeSet(nodeSet),
  m_appending(appending)
{
}

ZStackDocCommand::SwcEdit::SetSwcNodeSeletion::~SetSwcNodeSeletion()
{
}

void ZStackDocCommand::SwcEdit::SetSwcNodeSeletion::redo()
{
  if (m_host != NULL) {
    m_oldNodeSet = m_host->getSelectedNode();
    //m_host->deselectAllNode();
    m_host->selectNode(m_nodeSet.begin(), m_nodeSet.end(), m_appending);
    m_doc->notifySelectionAdded(m_oldNodeSet, m_nodeSet);
    m_doc->notifySelectionRemoved(m_oldNodeSet, m_nodeSet);
  }
}

void ZStackDocCommand::SwcEdit::SetSwcNodeSeletion::undo()
{
  startUndo();
  if (m_host != NULL) {
    //m_nodeSet = m_host->getSelectedNode();
    m_host->deselectAllNode();
    m_host->selectNode(m_oldNodeSet.begin(), m_oldNodeSet.end(), true);
    m_doc->notifySelectionAdded(m_nodeSet, m_oldNodeSet);
    m_doc->notifySelectionRemoved(m_nodeSet, m_oldNodeSet);
  }
}
#endif

//////////////
ZStackDocCommand::SwcEdit::SwcTreeLabeTraceMask::SwcTreeLabeTraceMask(
    ZStackDoc *doc, Swc_Tree *tree, QUndoCommand *parent) :
  ZUndoCommand(parent), m_doc(doc), m_tree(tree)
{
}

ZStackDocCommand::SwcEdit::SwcTreeLabeTraceMask::~SwcTreeLabeTraceMask() {}

void ZStackDocCommand::SwcEdit::SwcTreeLabeTraceMask::undo()
{
  startUndo();
  if (!m_doc && m_tree != NULL) {
    Swc_Tree_Node_Label_Workspace workspace;
    Default_Swc_Tree_Node_Label_Workspace(&workspace);
    workspace.sdw.color.r = 0;
    workspace.sdw.color.g = 0;
    workspace.sdw.color.b = 0;
    Swc_Tree_Label_Stack(
          m_tree, m_doc->getTraceWorkspace()->trace_mask, &workspace);
  }
}

void ZStackDocCommand::SwcEdit::SwcTreeLabeTraceMask::redo()
{
  if (!m_doc && m_tree != NULL) {
    Swc_Tree_Node_Label_Workspace workspace;
    Default_Swc_Tree_Node_Label_Workspace(&workspace);
    Swc_Tree_Label_Stack(
          m_tree, m_doc->getTraceWorkspace()->trace_mask, &workspace);
  }
}
////////////

ZStackDocCommand::SwcEdit::SwcPathLabeTraceMask::SwcPathLabeTraceMask(
    ZStackDoc *doc, const ZSwcPath &branch, QUndoCommand *parent) :
  ZUndoCommand(parent), m_doc(doc)
{
  m_branch = branch;
}

ZStackDocCommand::SwcEdit::SwcPathLabeTraceMask::~SwcPathLabeTraceMask() {}

void ZStackDocCommand::SwcEdit::SwcPathLabeTraceMask::undo()
{
  startUndo();
//  m_branch.labelStack(m_doc->getTraceWorkspace()->trace_mask, 0);
  m_doc->deprecateTraceMask();
}

void ZStackDocCommand::SwcEdit::SwcPathLabeTraceMask::redo()
{
  m_branch.labelStack(m_doc->getTraceWorkspace()->trace_mask, 255);
}

//////////////////

ZStackDocCommand::SwcEdit::SetRoot::SetRoot(
    ZStackDoc *doc, Swc_Tree_Node *tn, QUndoCommand *parent) :
  ZUndoCommand(parent), m_doc(doc), m_node(tn)
  //CompositeCommand(doc, parent)
{
  setText(QObject::tr("Set root"));
#if 0
  if (tn != NULL) {
    Swc_Tree_Node *buffer1, *buffer2, *buffer3;
    buffer1 = tn;
    buffer2 = buffer1->parent;
    new SetParent(doc, buffer1, NULL, this);
    //Swc_Tree_Node_Detach_Parent(buffer1);

    //weight[0] = buffer1->weight;
    while (Swc_Tree_Node_Is_Regular(buffer1) == TRUE) {
      if (Swc_Tree_Node_Is_Regular(buffer2) == TRUE) {
        //weight[1] = buffer2->weight;
        buffer3 = buffer2->parent;
        //buffer2->weight = weight[0];
        //weight[0] = weight[1];
        //Swc_Tree_Node_Add_Child(buffer1, buffer2);
        new SetParent(doc, buffer2, buffer1, this);
      }
      buffer1 = buffer2;
      buffer2 = buffer3;
    }

    //Swc_Tree_Node_Set_Parent(tn, buffer1);
    new SetParent(doc, tn, buffer1, this);
  }
#endif
}

void ZStackDocCommand::SwcEdit::SetRoot::redo()
{
  m_originalParentArray.clear();
  if (m_node != NULL) {
    Swc_Tree_Node *parent = SwcTreeNode::parent(m_node);

    while (SwcTreeNode::isRegular(parent)) {
      m_originalParentArray.push_back(parent);
      parent = SwcTreeNode::parent(parent);
    }
    Swc_Tree_Node *virtualRoot = parent;

    SwcTreeNode::setParent(m_node, virtualRoot);
    Swc_Tree_Node *currentParent = m_node;
    for (std::vector<Swc_Tree_Node*>::iterator
         iter = m_originalParentArray.begin();
         iter != m_originalParentArray.end(); ++iter) {
      SwcTreeNode::setParent(*iter, currentParent);
      currentParent = *iter;
    }

    m_doc->notifySwcModified();
  }
}

void ZStackDocCommand::SwcEdit::SetRoot::undo()
{
  startUndo();
  if (!m_originalParentArray.empty()) {
    Swc_Tree_Node *virtualRoot = SwcTreeNode::parent(m_node);
    Swc_Tree_Node *currentParent = virtualRoot;
    for (std::vector<Swc_Tree_Node*>::reverse_iterator
         iter = m_originalParentArray.rbegin();
         iter != m_originalParentArray.rend(); ++iter) {
      SwcTreeNode::setParent(*iter, currentParent);
      currentParent = *iter;
    }
    SwcTreeNode::setParent(m_node, m_originalParentArray.front());

    m_doc->notifySwcModified();
  }
}


ZStackDocCommand::SwcEdit::ConnectSwcNode::ConnectSwcNode(
    ZStackDoc *doc, QUndoCommand *parent) :
  CompositeCommand(doc, parent)
{
  setText("Connect Swc Nodes");

  ZSwcConnector connector;
  connector.useSurfaceDist(true);
  std::set<Swc_Tree_Node*> nodeSet = doc->getSelectedSwcNodeSet();

  ZCuboid boundBox = SwcTreeNode::boundBox(nodeSet);
  const int nodeNumberThreshold = 500;
  double minDist =
      boundBox.getDiagonalLength() * nodeNumberThreshold / nodeSet.size();
  connector.setMinDist(minDist);
  connector.setResolution(doc->getResolution());

  ZGraph *graph = connector.buildConnection(nodeSet);

  QMap<const Swc_Tree_Node*, const ZSwcTree*> swcMap =
      doc->getSelectedSwcNodeMap();

  if (graph->size() > 0) {
    std::vector<Swc_Tree_Node*> nodeArray;
    for (set<Swc_Tree_Node*>::iterator iter = nodeSet.begin();
         iter != nodeSet.end(); ++iter) {
      nodeArray.push_back(*iter);
    }

    for (size_t i = 0; i < graph->size(); ++i) {
      if (graph->edgeWeight(i) > 0.0) {
        int e1 = graph->edgeStart(i);
        int e2 = graph->edgeEnd(i);
        TZ_ASSERT(e1 != e2, "Invalid edge");

        Swc_Tree_Node *upNode = nodeArray[e1];
        Swc_Tree_Node *downNode = nodeArray[e2];

        //Check source
        if (ZFileType::fileType(swcMap[downNode]->getSource()) ==
            ZFileType::SWC_FILE) {
          upNode = nodeArray[e2];
          downNode = nodeArray[e1];
        }

        new SetRoot(doc, downNode, this);
        new SetParent(doc, downNode, upNode, false, this);
      }
    }
    new RemoveEmptyTreePost(doc, this);
  }

  delete graph;
}

ZStackDocCommand::SwcEdit::RemoveSwc::RemoveSwc(
    ZStackDoc *doc, ZSwcTree *tree, QUndoCommand *parent) :
  ZUndoCommand(parent), m_doc(doc), m_tree(tree), m_isInDoc(true)
{
}

ZStackDocCommand::SwcEdit::RemoveSwc::~RemoveSwc()
{
#ifdef _DEBUG_
  std::cout << "ZStackDocCommand::SwcEdit::RemoveSwc destroyed" << std::endl;
#endif
  if (!m_isInDoc) {
    delete m_tree;
  }
}

void ZStackDocCommand::SwcEdit::RemoveSwc::redo()
{
  if (m_tree != NULL) {
    m_doc->removeObject(m_tree, false);
    m_isInDoc = false;
    m_doc->notifySwcModified();
  }
}

void ZStackDocCommand::SwcEdit::RemoveSwc::undo()
{
  startUndo();
  if (m_tree != NULL) {
    //m_doc->addSwcTree(m_tree);
    m_doc->addObject(m_tree);
    m_isInDoc = true;
  }
}

////////////////////////////
ZStackDocCommand::SwcEdit::RemoveSwcIfEmpty::RemoveSwcIfEmpty(
    ZStackDoc *doc, ZSwcTree *tree, QUndoCommand *parent) :
  ZUndoCommand(parent), m_doc(doc), m_tree(tree), m_isInDoc(true)
{
}

ZStackDocCommand::SwcEdit::RemoveSwcIfEmpty::~RemoveSwcIfEmpty()
{
#ifdef _DEBUG_
  std::cout << "ZStackDocCommand::SwcEdit::RemoveSwc destroyed" << std::endl;
#endif
  if (!m_isInDoc) {
    delete m_tree;
  }
}

void ZStackDocCommand::SwcEdit::RemoveSwcIfEmpty::redo()
{
  if (m_tree != NULL) {
    if (m_tree->isEmpty()) {
      m_doc->removeObject(m_tree, false);
      m_isInDoc = false;
      m_doc->notifySwcModified();
    }
  }
}

void ZStackDocCommand::SwcEdit::RemoveSwcIfEmpty::undo()
{
  startUndo();
  if (m_tree != NULL) {
    //m_doc->addSwcTree(m_tree);
    m_doc->addObject(m_tree);
    m_isInDoc = true;
  }
}

//////////////////////////////

ZStackDocCommand::SwcEdit::RemoveEmptyTree::RemoveEmptyTree(
    ZStackDoc *doc, QUndoCommand *parent) :
  CompositeCommand(doc, parent), m_doc(doc)
{
  if (doc != NULL) {
    QList<ZSwcTree*> treeList = doc->getSwcList();
    foreach (ZSwcTree *tree, treeList) {
      new ZStackDocCommand::SwcEdit::RemoveSwcIfEmpty(doc, tree, this);
    }

    /*
    std::set<ZSwcTree*> emptyTreeSet = doc->getEmptySwcTreeSet();
    for (std::set<ZSwcTree*>::iterator iter = emptyTreeSet.begin();
         iter != emptyTreeSet.end(); ++iter) {
      ZSwcTree *tree = *iter;
      new ZStackDocCommand::SwcEdit::RemoveSwc(doc, tree, this);
    }
    */
  }
}

ZStackDocCommand::SwcEdit::RemoveEmptyTree::~RemoveEmptyTree()
{
#ifdef _DEBUG_
  std::cout << "RemoveEmptyTree destroyed" << std::endl;
#endif


//  for (std::set<ZSwcTree*>::iterator iter = m_emptyTreeSet.begin();
//       iter != m_emptyTreeSet.end(); ++iter) {
//    delete *iter;
//  }
}

//////////////////////////////

ZStackDocCommand::SwcEdit::RemoveEmptyTreePost::RemoveEmptyTreePost(
    ZStackDoc *doc, QUndoCommand *parent) :
  CompositeCommand(doc, parent), m_doc(doc)
{
}

ZStackDocCommand::SwcEdit::RemoveEmptyTreePost::~RemoveEmptyTreePost()
{
#ifdef _DEBUG_
  std::cout << "RemoveEmptyTreePost destroyed" << std::endl;
#endif


  for (std::set<ZSwcTree*>::iterator iter = m_emptyTreeSet.begin();
       iter != m_emptyTreeSet.end(); ++iter) {
    delete *iter;
  }
}

void ZStackDocCommand::SwcEdit::RemoveEmptyTreePost::redo()
{
  if (m_doc != NULL) {
    m_emptyTreeSet = m_doc->removeEmptySwcTree(false);
  }
}

void ZStackDocCommand::SwcEdit::RemoveEmptyTreePost::undo()
{
  startUndo();
  if (m_doc != NULL) {
    m_doc->beginObjectModifiedMode(ZStackDoc::OBJECT_MODIFIED_CACHE);
    for (std::set<ZSwcTree*>::iterator iter = m_emptyTreeSet.begin();
         iter != m_emptyTreeSet.end(); ++iter) {
      m_doc->addObject(*iter, false);
    }
    m_emptyTreeSet.clear();
    m_doc->endObjectModifiedMode();
    m_doc->notifyObjectModified();
  }
}

////////////////////////////////

ZStackDocCommand::SwcEdit::BreakForest::BreakForest(
    ZStackDoc *doc, QUndoCommand *parent) :
  CompositeCommand(doc, parent)
{
  setText(QObject::tr("Break swc forest"));

  if (m_doc != NULL) {
    QList<ZSwcTree*> treeSet =
        m_doc->getSelectedObjectList<ZSwcTree>(ZStackObject::TYPE_SWC);
    //std::set<ZSwcTree*> *treeSet = m_doc->selectedSwcs();

    if (!treeSet.empty()) {
      for (QList<ZSwcTree*>::iterator iter = treeSet.begin();
           iter != treeSet.end(); ++iter) {
        Swc_Tree_Node *root = (*iter)->firstRegularRoot();
        if (root != NULL) {
          root = SwcTreeNode::nextSibling(root);
          while (root != NULL) {
            Swc_Tree_Node *sibling = SwcTreeNode::nextSibling(root);

            ZSwcTree *tree = new ZSwcTree;
            tree->forceVirtualRoot();
            new ZStackDocCommand::SwcEdit::SetParent(
                  doc, root, tree->root(), false, this);
            new ZStackDocCommand::SwcEdit::AddSwc(doc, tree, this);
            root = sibling;
          }
        }
      }
    }
  }
}

ZStackDocCommand::SwcEdit::GroupSwc::GroupSwc(
    ZStackDoc *doc, QUndoCommand *parent) : CompositeCommand(doc, parent)
{
  setText(QObject::tr("Group swc"));

  QList<ZSwcTree*> treeSet =
      m_doc->getSelectedObjectList<ZSwcTree>(ZStackObject::TYPE_SWC);
  //std::set<ZSwcTree*> *treeSet = m_doc->selectedSwcs();

  if (treeSet.size() > 1) {
    QList<ZSwcTree*>::iterator iter = treeSet.begin();
    Swc_Tree_Node *root = (*iter)->root();

    for (++iter; iter != treeSet.end(); ++iter) {
      Swc_Tree_Node *subroot = (*iter)->firstRegularRoot();
      new ZStackDocCommand::SwcEdit::SetParent(doc, subroot, root, false, this);
    }

    new ZStackDocCommand::SwcEdit::RemoveEmptyTree(doc, this);
  }
}

#if 0
ZStackDocCommand::SwcEdit::TraceSwcBranch::TraceSwcBranch(
    ZStackDoc *doc, double x, double y, double z, int c, QUndoCommand *parent) :
  QUndoCommand(parent), m_doc(doc), m_x(x), m_y(y), m_z(z), m_c(c)
{

}

ZStackDocCommand::SwcEdit::TraceSwcBranch::~TraceSwcBranch()
{

}

void ZStackDocCommand::SwcEdit::TraceSwcBranch::redo()
{
  ZNeuronTracer tracer;
  tracer.setIntensityField(m_doc->stack()->c_stack(m_c));
  tracer.setTraceWorkspace(m_doc->getTraceWorkspace());
  ZSwcPath branch = tracer.trace(m_x, m_y, m_z);
  tracer.updateMask(branch);
  ZSwcTree *tree = new ZSwcTree();
  tree->setDataFromNode(branch.front());
  m_doc->addSwcTree(tree, false);
  m_doc->notifySwcModified();
}

void ZStackDocCommand::SwcEdit::TraceSwcBranch::undo()
{

}
#endif

ZStackDocCommand::ObjectEdit::RemoveSelected::RemoveSelected(
    ZStackDoc *doc, QUndoCommand *parent) : ZUndoCommand(parent), doc(doc)
{
  setText(QObject::tr("remove selected objects"));
}

ZStackDocCommand::ObjectEdit::RemoveSelected::~RemoveSelected()
{
#ifdef _DEBUG_
  std::cout << "RemoveSelected destroyed" << std::endl;
#endif

  for (QList<ZStackObject*>::iterator iter = m_selectedObject.begin();
       iter != m_selectedObject.end(); ++iter) {
    delete *iter;
  }
}

void ZStackDocCommand::ObjectEdit::RemoveSelected::undo()
{
  startUndo();
  //Add the objects back
//  doc->blockSignals(true);
  //ZDocPlayer::TRole role = ZDocPlayer::ROLE_NONE;

  doc->beginObjectModifiedMode(ZStackDoc::OBJECT_MODIFIED_CACHE);

  for (int i = 0; i < m_selectedObject.size(); ++i) {
    doc->addObject(m_selectedObject[i], false);
    //role |= m_roleList[i];
  }

  doc->endObjectModifiedMode();
  doc->notifyObjectModified();
//  doc->blockSignals(false);

//  notifyObjectChanged(m_selectedObject);

  m_selectedObject.clear();
  //m_roleList.clear();
}

void ZStackDocCommand::ObjectEdit::RemoveSelected::notifyObjectChanged(
    const QList<ZStackObject *> &selectedObject) const
{
  doc->beginObjectModifiedMode(ZStackDoc::OBJECT_MODIFIED_CACHE);
  for (QList<ZStackObject*>::const_iterator iter = selectedObject.begin();
       iter != selectedObject.end(); ++iter) {
    doc->processObjectModified(*iter);
  }
  doc->endObjectModifiedMode();
  doc->notifyObjectModified();

  /*
  ZStackObjectRole role;

  //Send notifications
  std::set<ZStackObject::EType> typeSet;
  for (QList<ZStackObject*>::const_iterator iter = selectedObject.begin();
       iter != selectedObject.end(); ++iter) {
    const ZStackObject *obj = *iter;
    typeSet.insert(obj->getType());
    role.addRole(obj->getRole().getRole());
  }

  size_t s = typeSet.size();

  if (typeSet.count(ZStackObject::TYPE_PUNCTUM) > 0) {
    doc->notifyPunctumModified();
    --s;
  }

  if (typeSet.count(ZStackObject::TYPE_SWC) > 0) {
    doc->notifySwcModified();
    --s;
  }

  if (typeSet.count(ZStackObject::TYPE_STROKE) > 0) {
    doc->notifyStrokeModified();
    --s;
  }

  if (typeSet.count(ZStackObject::TYPE_SPARSE_OBJECT) > 0) {
    doc->notifySparseObjectModified();
    --s;
  }

  if (s > 0) {
    doc->notifyObjectModified();
  }

  doc->notifyPlayerChanged(role.getRole());
  */
}

void ZStackDocCommand::ObjectEdit::RemoveSelected::redo()
{
  //Remove and backup selected objects
  m_selectedObject = doc->getObjectGroup().takeSelected();
  //ZStackObjectRole allRole;
  foreach (ZStackObject *obj, m_selectedObject) {
    doc->getPlayerList().removePlayer(obj);
    /*
    allRole.addRole(doc->getPlayerList().removePlayer(obj));
    m_roleList.append(role);
    allRole |= role;
    */
  }
  notifyObjectChanged(m_selectedObject);
}

ZStackDocCommand::ObjectEdit::RemoveObject::RemoveObject(
    ZStackDoc *doc, ZStackObject *obj, QUndoCommand *parent) :
  ZUndoCommand(parent), m_doc(doc), m_obj(obj), m_isInDoc(true)
{
  setText(QObject::tr("Remove object"));
}

ZStackDocCommand::ObjectEdit::RemoveObject::~RemoveObject()
{
  if (!m_isInDoc) {
    delete m_obj;
  }
}

void ZStackDocCommand::ObjectEdit::RemoveObject::redo()
{
  m_obj->setSelected(false);
  m_doc->removeObject(m_obj, false);
  m_isInDoc = false;
}

void ZStackDocCommand::ObjectEdit::RemoveObject::undo()
{
  startUndo();
  m_doc->addObject(m_obj, false);
  m_isInDoc = true;
}

ZStackDocCommand::TubeEdit::Trace::Trace(
    ZStackDoc *doc, int x, int y, int z, QUndoCommand *parent)
  : ZUndoCommand(parent), m_doc(doc), m_x(x), m_y(y), m_z(z), m_c(0)
{
  setText(QObject::tr("trace tube from (%1,%2,%3)").arg(x).arg(y).arg(z));
}

ZStackDocCommand::TubeEdit::Trace::Trace(
    ZStackDoc *doc, int x, int y, int z, int c, QUndoCommand *parent)
  : ZUndoCommand(parent), m_doc(doc), m_x(x), m_y(y), m_z(z), m_c(c)
{
  setText(QObject::tr("trace tube from (%1,%2,%3) in channel %4").
          arg(x).arg(y).arg(z).arg(c));
}

void ZStackDocCommand::TubeEdit::Trace::redo()
{
  if (m_doc->getStack()->depth() == 1) {
    m_chain = m_doc->traceRect(m_x, m_y, m_z, 3.0, m_c);
  } else {
    m_chain = m_doc->traceTube(m_x, m_y, m_z, 3.0, m_c);
  }
}

void ZStackDocCommand::TubeEdit::Trace::undo()
{
  startUndo();
  if (m_chain != NULL) {
    m_doc->removeObject(m_chain, true);
    m_chain = NULL;
//    m_doc->notifyChainModified();
  }
}

ZStackDocCommand::TubeEdit::AutoTraceAxon::AutoTraceAxon(
    ZStackDoc *doc, QUndoCommand *parent)
  :ZUndoCommand(parent), m_doc(doc)
{
  setText(QObject::tr("auto trace axon"));
}

ZStackDocCommand::TubeEdit::AutoTraceAxon::~AutoTraceAxon()
{
  for (int i=0; i<m_swcList.size(); i++) {
    delete m_swcList[i];
  }
  for (int i=0; i<m_obj3dList.size(); i++) {
    delete m_obj3dList[i];
  }
  for (int i=0; i<m_connList.size(); i++) {
    delete m_connList[i];
  }
  for (int i=0; i<m_punctaList.size(); i++) {
    delete m_punctaList[i];
  }
  for (int i=0; i<m_chainList.size(); i++) {
    delete m_chainList[i];
  }
  m_swcList.clear();
  m_obj3dList.clear();
  m_connList.clear();
  m_punctaList.clear();
  m_chainList.clear();
}

void ZStackDocCommand::TubeEdit::AutoTraceAxon::undo()
{
  startUndo();
#if 0
  QList<ZLocsegChain*> chainList = m_doc->getChainList();
  QList<ZSwcTree*> swcList = m_doc->getSwcList();
  QList<ZLocsegChainConn*> connList = m_doc->getConnList();
  QList<ZObject3d*> obj3dList = m_doc->getObj3dList();
  QList<ZPunctum*> punctaList = m_doc->getPunctaList();
  m_doc->removeAllObject(false);
  // copy stuff back
  QMutableListIterator<ZPunctum*> iter1(m_punctaList);
  while (iter1.hasNext()) {
    ZPunctum *obj = iter1.next();
    m_doc->addPunctum(obj);
  }
  QMutableListIterator<ZSwcTree*> iter2(m_swcList);
  while (iter2.hasNext()) {
    ZSwcTree *obj = iter2.next();
    m_doc->addSwcTree(obj);
  }
  QMutableListIterator<ZObject3d*> iter3(m_obj3dList);
  while (iter3.hasNext()) {
    ZObject3d *obj = iter3.next();
    m_doc->addObj3d(obj);
  }
  QMutableListIterator<ZLocsegChain*> iter4(m_chainList);
  while (iter4.hasNext()) {
    ZLocsegChain *obj = iter4.next();
    m_doc->addLocsegChain(obj);
  }
  QMutableListIterator<ZLocsegChainConn*> iter(m_connList);
  while (iter.hasNext()) {
    ZLocsegChainConn *obj = iter.next();
    m_doc->addLocsegChainConn(obj);
  }
  if (!m_punctaList.empty() || !punctaList.empty()) {
    m_doc->notifyPunctumModified();
  }
  if (!m_swcList.empty() || !swcList.empty()) {
    m_doc->notifySwcModified();
  }
  if (!m_obj3dList.empty() || !obj3dList.empty()) {
    m_doc->notifyObj3dModified();
  }
  m_doc->notifyChainModified();
  m_chainList = chainList;
  m_swcList = swcList;
  m_connList = connList;
  m_obj3dList = obj3dList;
  m_punctaList = punctaList;
#endif
}

void ZStackDocCommand::TubeEdit::AutoTraceAxon::redo()
{
#if 0
  QList<ZLocsegChain*> chainList = m_doc->getChainList();
  QList<ZSwcTree*> swcList = m_doc->getSwcList();
  QList<ZLocsegChainConn*> connList = m_doc->getConnList();
  QList<ZObject3d*> obj3dList = m_doc->getObj3dList();
  QList<ZPunctum*> punctaList = m_doc->getPunctaList();
  m_doc->removeAllObject(false);
  if (m_punctaList.isEmpty() && m_swcList.isEmpty() && m_obj3dList.isEmpty()
      && m_chainList.isEmpty() && m_connList.isEmpty()) {  // first time execute, trace
    m_doc->autoTraceAxon();
  } else { // copy stuff back
    QMutableListIterator<ZPunctum*> iter1(m_punctaList);
    while (iter1.hasNext()) {
      ZPunctum *obj = iter1.next();
      m_doc->addPunctum(obj);
    }
    QMutableListIterator<ZSwcTree*> iter2(m_swcList);
    while (iter2.hasNext()) {
      ZSwcTree *obj = iter2.next();
      m_doc->addSwcTree(obj);
    }
    QMutableListIterator<ZObject3d*> iter3(m_obj3dList);
    while (iter3.hasNext()) {
      ZObject3d *obj = iter3.next();
      m_doc->addObj3d(obj);
    }
    QMutableListIterator<ZLocsegChain*> iter4(m_chainList);
    while (iter4.hasNext()) {
      ZLocsegChain *obj = iter4.next();
      m_doc->addLocsegChain(obj);
    }
    QMutableListIterator<ZLocsegChainConn*> iter(m_connList);
    while (iter.hasNext()) {
      ZLocsegChainConn *obj = iter.next();
      m_doc->addLocsegChainConn(obj);
    }
  }
  if (!m_punctaList.empty() || !punctaList.empty()) {
    m_doc->notifyPunctumModified();
  }
  if (!m_swcList.empty() || !swcList.empty()) {
    m_doc->notifySwcModified();
  }
  if (!m_obj3dList.empty() || !obj3dList.empty()) {
    m_doc->notifyObj3dModified();
  }
  m_doc->notifyChainModified();
  m_chainList = chainList;
  m_swcList = swcList;
  m_connList = connList;
  m_obj3dList = obj3dList;
  m_punctaList = punctaList;
#endif
}

// class ZStackDocCutSelectedLocsegChainCommand
ZStackDocCommand::TubeEdit::CutSegment::CutSegment(ZStackDoc *doc, QUndoCommand *parent)
  : ZUndoCommand(parent), m_doc(doc)
{
  setText(QObject::tr("cut selected tubes"));
}

ZStackDocCommand::TubeEdit::CutSegment::~CutSegment()
{
  for (int i = 0; i < m_oldChainList.size(); i++) {
    delete m_oldChainList.at(i);
  }
}

void ZStackDocCommand::TubeEdit::CutSegment::redo()
{
  QList<ZLocsegChain*> chainList = m_doc->getLocsegChainList();
  for (int i = 0; i < chainList.size(); i++) {
    if (chainList.at(i)->isSelected() == true) {
      m_oldChainList.append(chainList.at(i));
      m_doc->cutLocsegChain(chainList.at(i), &m_newChainList);
    }
  }
  m_doc->getSelected(ZStackObject::TYPE_LOCSEG_CHAIN).clear();
}

void ZStackDocCommand::TubeEdit::CutSegment::undo()
{
  startUndo();
  // restore previous selection state
  m_doc->beginObjectModifiedMode(ZStackDoc::OBJECT_MODIFIED_CACHE);

  m_doc->deselectAllObject();

  for (int i = 0; i < m_newChainList.size(); i++) {
    m_doc->removeObject(m_newChainList.at(i), true);
  }
  m_newChainList.clear();
  for (int i = 0; i < m_oldChainList.size(); i++) {
    m_doc->addObject(m_oldChainList.at(i));
  }
  m_oldChainList.clear();
//  m_doc->notifyChainModified();

  m_doc->endObjectModifiedMode();
  m_doc->notifyObjectModified();
}


// class ZStackDocBreakSelectedLocsegChainCommand
ZStackDocCommand::TubeEdit::BreakChain::BreakChain(ZStackDoc *doc, QUndoCommand *parent)
  : ZUndoCommand(parent), m_doc(doc)
{
  setText(QObject::tr("break selected tubes"));
}

ZStackDocCommand::TubeEdit::BreakChain::~BreakChain()
{
  for (int i = 0; i < m_oldChainList.size(); i++) {
    delete m_oldChainList.at(i);
  }
}

void ZStackDocCommand::TubeEdit::BreakChain::redo()
{

  QList<ZLocsegChain*> chainList = m_doc->getLocsegChainList();
  for (int i = 0; i < chainList.size(); i++) {
    if (chainList.at(i)->isSelected() == true) {
      m_oldChainList.append(chainList.at(i));
      m_doc->breakLocsegChain(chainList.at(i), &m_newChainList);
    }
  }
  m_doc->getSelected(ZStackObject::TYPE_LOCSEG_CHAIN).clear();
  m_doc->endObjectModifiedMode();
  m_doc->notifyObjectModified();
  //m_doc->selectedChains()->clear();
}

void ZStackDocCommand::TubeEdit::BreakChain::undo()
{
  startUndo();
  m_doc->beginObjectModifiedMode(ZStackDoc::OBJECT_MODIFIED_CACHE);

  // restore previous selection state
  m_doc->deselectAllObject();

  for (int i = 0; i < m_newChainList.size(); i++) {
    m_doc->removeObject(m_newChainList.at(i), true);
  }
  m_newChainList.clear();
  for (int i = 0; i < m_oldChainList.size(); i++) {
    m_doc->addObject(m_oldChainList.at(i));
  }
  m_oldChainList.clear();
//  m_doc->notifyChainModified();

  m_doc->endObjectModifiedMode();
  m_doc->notifyObjectModified();
}

ZStackDocCommand::TubeEdit::RemoveSmall::RemoveSmall(
    ZStackDoc *doc, double thre, QUndoCommand *parent)
  :ZUndoCommand(parent), m_doc(doc), m_thre(thre)
{
  setText(QObject::tr("remove small tube use thre %1").arg(thre));
}

ZStackDocCommand::TubeEdit::RemoveSmall::~RemoveSmall()
{
  for (int i=0; i<m_chainList.size(); i++) {
    delete m_chainList[i];
  }
}

void ZStackDocCommand::TubeEdit::RemoveSmall::undo()
{
  startUndo();
  m_doc->beginObjectModifiedMode(ZStackDoc::OBJECT_MODIFIED_CACHE);

  for (int i=0; i<m_chainList.size(); i++) {
    m_doc->addObject(m_chainList[i]);
  }
//  if (!m_chainList.empty())
//    m_doc->notifyChainModified();
  m_chainList.clear();

  m_doc->endObjectModifiedMode();
  m_doc->notifyObjectModified();
}

void ZStackDocCommand::TubeEdit::RemoveSmall::redo()
{
  m_doc->beginObjectModifiedMode(ZStackDoc::OBJECT_MODIFIED_CACHE);

  QList<ZLocsegChain*> chainList = m_doc->getLocsegChainList();
  QMutableListIterator<ZLocsegChain*> chainIter(chainList);
  while (chainIter.hasNext()) {
    ZLocsegChain *chain = chainIter.next();
    if (chain->geoLength() < m_thre) {
      m_doc->removeObject(chain, false);
      m_chainList.push_back(chain);
    }
  }

  m_doc->endObjectModifiedMode();
  m_doc->notifyObjectModified();
//  if (!m_chainList.empty())
//    m_doc->notifyChainModified();
}

ZStackDocCommand::TubeEdit::RemoveSelected::RemoveSelected(
    ZStackDoc *doc, QUndoCommand *parent) : ZUndoCommand(parent), m_doc(doc)
{
}

ZStackDocCommand::TubeEdit::RemoveSelected::~RemoveSelected()
{
  for (int i=0; i<m_chainList.size(); i++) {
    delete m_chainList[i];
  }
}

void ZStackDocCommand::TubeEdit::RemoveSelected::redo()
{
  m_doc->beginObjectModifiedMode(ZStackDoc::OBJECT_MODIFIED_CACHE);

  QList<ZLocsegChain*> chainList = m_doc->getSelectedObjectList<ZLocsegChain>(
        ZStackObject::TYPE_LOCSEG_CHAIN);
  for (QList<ZLocsegChain*>::iterator iter = chainList.begin();
       iter != chainList.end(); ++iter) {
    m_doc->removeObject(*iter, false);
    m_chainList.push_back(*iter);
  }

  m_doc->endObjectModifiedMode();
  m_doc->notifyObjectModified();

//  if (!m_chainList.empty()) {
//    m_doc->notifyChainModified();
//  }
}

void ZStackDocCommand::TubeEdit::RemoveSelected::undo()
{
  startUndo();
  m_doc->beginObjectModifiedMode(ZStackDoc::OBJECT_MODIFIED_CACHE);

  for (int i=0; i<m_chainList.size(); i++) {
    m_doc->addObject(m_chainList[i]);
  }
//  if (!m_chainList.empty()) {
//    m_doc->notifyChainModified();
//  }
  m_chainList.clear();

  m_doc->endObjectModifiedMode();
  m_doc->notifyObjectModified();
}

ZStackDocCommand::ObjectEdit::MoveSelected::MoveSelected(
    ZStackDoc *doc, double x, double y, double z, QUndoCommand *parent)
  : ZUndoCommand(parent), m_doc(doc), m_x(x), m_y(y), m_z(z), m_swcMoved(false),
    m_punctaMoved(false), m_swcScaleX(1.), m_swcScaleY(1.), m_swcScaleZ(1.),
    m_punctaScaleX(1.), m_punctaScaleY(1.), m_punctaScaleZ(1.)
{
  setText(QObject::tr("Move Selected"));
}

ZStackDocCommand::ObjectEdit::MoveSelected::~MoveSelected()
{
}

ZStackDocCommand::ObjectEdit::AddObject::AddObject(
    ZStackDoc *doc, ZStackObject *obj, bool uniqueSource, QUndoCommand *parent)
  : ZUndoCommand(parent), m_doc(doc), m_obj(obj),
    m_uniqueSource(uniqueSource), m_isInDoc(false)
{
  setText(QObject::tr("Add Object"));
}

ZStackDocCommand::ObjectEdit::AddObject::~AddObject()
{
  if (!m_isInDoc) {
    delete m_obj;
  } else {
    for (TStackObjectList::iterator iter = m_uniqueObjectList.begin();
         iter != m_uniqueObjectList.end(); ++iter) {
      delete *iter;
    }
  }
}

void ZStackDocCommand::ObjectEdit::AddObject::redo()
{
  //backup old objects
  m_doc->beginObjectModifiedMode(ZStackDoc::OBJECT_MODIFIED_CACHE);

  if (m_uniqueSource) {
    m_uniqueObjectList = m_doc->getObjectGroup().takeSameSource(
          m_obj->getType(), m_obj->getSource());
    for (TStackObjectList::iterator iter = m_uniqueObjectList.begin();
         iter != m_uniqueObjectList.end(); ++iter) {
      m_doc->getPlayerList().removePlayer(*iter);
      m_doc->processObjectModified(*iter);
    }
  }

  m_doc->addObject(m_obj, false);

  m_doc->endObjectModifiedMode();
  m_doc->notifyObjectModified();

  m_isInDoc = true;
}

void ZStackDocCommand::ObjectEdit::AddObject::undo()
{
  startUndo();
  m_doc->beginObjectModifiedMode(ZStackDoc::OBJECT_MODIFIED_CACHE);

  m_doc->removeObject(m_obj, false);

  //Add unique source back
  for (int i = 0; i < m_uniqueObjectList.size(); ++i) {
    m_doc->addObject(m_uniqueObjectList[i], false);
  }
  m_uniqueObjectList.clear();

  m_doc->endObjectModifiedMode();
  m_doc->notifyObjectModified();

  m_isInDoc = false;
}


void ZStackDocCommand::ObjectEdit::MoveSelected::setSwcCoordScale(double x, double y, double z)
{
  m_swcScaleX = x;
  m_swcScaleY = y;
  m_swcScaleZ = z;
}

void ZStackDocCommand::ObjectEdit::MoveSelected::setPunctaCoordScale(double x, double y, double z)
{
  m_punctaScaleX = x;
  m_punctaScaleY = y;
  m_punctaScaleZ = z;
}

bool ZStackDocCommand::ObjectEdit::MoveSelected::mergeWith(const QUndoCommand *other)
{
//  return true;

  if (other->id() != id())
    return false;

  const MoveSelected *oth = dynamic_cast<const MoveSelected *>(other);
  if (oth != NULL) {
    if (m_punctaList != oth->m_punctaList ||
        m_swcNodeList != oth->m_swcNodeList ||
        m_swcList != oth->m_swcList ||
        m_swcScaleX != oth->m_swcScaleX ||
        m_swcScaleY != oth->m_swcScaleY ||
        m_swcScaleZ != oth->m_swcScaleZ ||
        m_punctaScaleX != oth->m_punctaScaleX ||
        m_punctaScaleY != oth->m_punctaScaleY ||
        m_punctaScaleZ != oth->m_punctaScaleZ) {
      return false;
    }

    m_x += oth->m_x;
    m_y += oth->m_y;
    m_z += oth->m_z;

    return true;
  }

  return false;
}

void ZStackDocCommand::ObjectEdit::MoveSelected::undo()
{
  startUndo();
  m_doc->beginObjectModifiedMode(ZStackDoc::OBJECT_MODIFIED_CACHE);

  for (QList<ZPunctum*>::iterator it = m_punctaList.begin();
       it != m_punctaList.end(); ++it) {
    (*it)->translate(-m_x/m_punctaScaleX, -m_y/m_punctaScaleY, -m_z/m_punctaScaleZ);
    m_doc->processObjectModified(*it);
  }
  for (QList<ZSwcTree*>::iterator it = m_swcList.begin();
       it != m_swcList.end(); ++it) {
    (*it)->translate(-m_x/m_swcScaleX, -m_y/m_swcScaleY, -m_z/m_swcScaleZ);
    m_doc->processObjectModified(*it);
  }
  for (std::set<Swc_Tree_Node*>::iterator it = m_swcNodeList.begin();
       it != m_swcNodeList.end(); ++it) {
    Swc_Tree_Node* tn = *it;
    tn->node.x -= m_x/m_swcScaleX;
    tn->node.y -= m_y/m_swcScaleY;
    tn->node.z -= m_z/m_swcScaleZ;
  }

  if (!m_swcNodeList.empty()) {
    m_doc->processSwcModified();
  }

  // restore selection state
  m_doc->blockSignals(true);
  m_doc->deselectAllObject();
  m_doc->setPunctumSelected(m_punctaList.begin(), m_punctaList.end(), true);
  m_doc->setSwcSelected(m_swcList.begin(), m_swcList.end(), true);
  m_doc->setSwcTreeNodeSelected(m_swcNodeList.begin(), m_swcNodeList.end(), true);
  m_doc->blockSignals(false);

  m_doc->endObjectModifiedMode();
  m_doc->notifyObjectModified();

//  if (m_swcMoved)
//    m_doc->notifySwcModified();
//  if (m_punctaMoved)
//    m_doc->notifyPunctumModified();

  m_swcMoved = false;
  m_punctaMoved = false;
}

void ZStackDocCommand::ObjectEdit::MoveSelected::redo()
{
  m_swcMoved = false;
  m_punctaMoved = false;

  m_doc->beginObjectModifiedMode(ZStackDoc::OBJECT_MODIFIED_CACHE);

  m_punctaList =
      m_doc->getSelectedObjectList<ZPunctum>(ZStackObject::TYPE_PUNCTUM);
  if (!m_punctaList.empty())
    m_punctaMoved = true;


  m_swcNodeList = m_doc->getSelectedSwcNodeSet();
  if (!m_swcNodeList.empty())
    m_swcMoved = true;

  m_swcList = m_doc->getSelectedObjectList<ZSwcTree>(ZStackObject::TYPE_SWC);
  //m_swcList = *(m_doc->selectedSwcs());
  if (!m_swcList.empty())
    m_swcMoved = true;

  for (QList<ZPunctum*>::iterator it = m_punctaList.begin();
       it != m_punctaList.end(); ++it) {
    (*it)->translate(m_x/m_punctaScaleX, m_y/m_punctaScaleY, m_z/m_punctaScaleZ);
    m_doc->processObjectModified(*it);
  }
  for (QList<ZSwcTree*>::iterator it = m_swcList.begin();
       it != m_swcList.end(); ++it) {
    (*it)->translate(m_x/m_swcScaleX, m_y/m_swcScaleY, m_z/m_swcScaleZ);
    m_doc->processObjectModified(*it);
  }
  for (std::set<Swc_Tree_Node*>::iterator it = m_swcNodeList.begin();
       it != m_swcNodeList.end(); ++it) {
    Swc_Tree_Node* tn = *it;
    tn->node.x += m_x/m_swcScaleX;
    tn->node.y += m_y/m_swcScaleY;
    tn->node.z += m_z/m_swcScaleZ;
  }

  if (!m_swcNodeList.empty()) {
    m_doc->processSwcModified();
  }

  m_doc->endObjectModifiedMode();
  m_doc->notifyObjectModified();

//  if (m_swcMoved)
//    m_doc->notifySwcModified();
//  if (m_punctaMoved)
//    m_doc->notifyPunctumModified();
}

ZStackDocCommand::StrokeEdit::AddStroke::AddStroke(
    ZStackDoc *doc, ZStroke2d *stroke, QUndoCommand *parent) :
  ZUndoCommand(parent), m_doc(doc), m_stroke(stroke), m_isInDoc(false)
{
  setText(QObject::tr("Add stroke"));
}

ZStackDocCommand::StrokeEdit::AddStroke::~AddStroke()
{
  if (!m_isInDoc) {
    delete m_stroke;
  }
}

void ZStackDocCommand::StrokeEdit::AddStroke::redo()
{
  m_doc->addObject(m_stroke);
  m_isInDoc = true;
//  m_doc->notifyStrokeModified();
}

void ZStackDocCommand::StrokeEdit::AddStroke::undo()
{
  startUndo();
  m_doc->removeObject(m_stroke, false);
  m_isInDoc = false;
//  m_doc->notifyStrokeModified();
}


ZStackDocCommand::StrokeEdit::RemoveTopStroke::RemoveTopStroke(
    ZStackDoc *doc, QUndoCommand *parent) :
  ZUndoCommand(parent), m_doc(doc), m_stroke(NULL), m_isInDoc(true)
{
}

ZStackDocCommand::StrokeEdit::RemoveTopStroke::~RemoveTopStroke()
{
  if (!m_isInDoc) {
    delete m_stroke;
  }
}

void ZStackDocCommand::StrokeEdit::RemoveTopStroke::redo()
{
  m_stroke = m_doc->getStrokeList().front();
  if (m_stroke != NULL) {
    m_doc->removeObject(m_doc->getStrokeList().front(), false);
    m_isInDoc = false;
//    m_doc->notifyStrokeModified();
  }
}

void ZStackDocCommand::StrokeEdit::RemoveTopStroke::undo()
{
  startUndo();
  if (m_stroke != NULL) {
    m_doc->addObject(m_stroke);
//    m_doc->notifyStrokeModified();
    m_isInDoc = true;
  }
}

ZStackDocCommand::StrokeEdit::CompositeCommand::CompositeCommand(
    ZStackDoc *doc, QUndoCommand *parent) : ZUndoCommand(parent), m_doc(doc)
{
}

ZStackDocCommand::StrokeEdit::CompositeCommand::~CompositeCommand()
{
  ZOUT(LTRACE(), 5) << "Stroke composite command (" << this->text() << ") destroyed";
}

void ZStackDocCommand::StrokeEdit::CompositeCommand::redo()
{
//  m_doc->blockSignals(true);

  m_doc->beginObjectModifiedMode(ZStackDoc::OBJECT_MODIFIED_CACHE);
  QUndoCommand::redo();
  m_doc->endObjectModifiedMode();
//  m_doc->blockSignals(false);

  m_doc->notifyObjectModified();

//  m_doc->notifyStrokeModified();
}


void ZStackDocCommand::StrokeEdit::CompositeCommand::undo()
{
  startUndo();
//  m_doc->blockSignals(true);

  m_doc->beginObjectModifiedMode(ZStackDoc::OBJECT_MODIFIED_CACHE);
  QUndoCommand::undo();
  m_doc->endObjectModifiedMode();
  m_doc->notifyObjectModified();
//  m_doc->blockSignals(false);
//  m_doc->notifyStrokeModified();
}

/////////////////////////////////////////////////////

ZStackDocCommand::StackProcess::Binarize::Binarize(
    ZStackDoc *doc, int thre, QUndoCommand *parent)
  :ZUndoCommand(parent), doc(doc), zstack(NULL), thre(thre), success(false)
{
  setText(QObject::tr("Binarize Image with threshold %1").arg(thre));
}

ZStackDocCommand::StackProcess::Binarize::~Binarize()
{
  delete zstack;
}

void ZStackDocCommand::StackProcess::Binarize::undo()
{
  startUndo();
  if (success) {
    doc->loadStack(zstack);
    //doc->requestRedrawStack();
    zstack = NULL;
    success = false;
  } else {
    delete zstack;
    zstack = NULL;
  }
}

void ZStackDocCommand::StackProcess::Binarize::redo()
{
  //zstack = new ZStack(*(doc->m_stack));
  zstack = doc->getStack()->clone();
  success = doc->binarize(thre);
}

ZStackDocCommand::StackProcess::BwSolid::BwSolid(
    ZStackDoc *doc, QUndoCommand *parent)
  :ZUndoCommand(parent), doc(doc), zstack(NULL), success(false)
{
  setText(QObject::tr("Binary Image Solidify"));
}

ZStackDocCommand::StackProcess::BwSolid::~BwSolid()
{
  delete zstack;
}

void ZStackDocCommand::StackProcess::BwSolid::undo()
{
  startUndo();
  if (success) {
    doc->loadStack(zstack);
    //doc->requestRedrawStack();
    zstack = NULL;
    success = false;
  } else {
    delete zstack;
    zstack = NULL;
  }
}

void ZStackDocCommand::StackProcess::BwSolid::redo()
{
  //zstack = new ZStack(*(doc->m_stack));
  zstack = doc->getStack()->clone();
  success = doc->bwsolid();
}

ZStackDocCommand::StackProcess::Watershed::Watershed(
    ZStackDoc *doc, QUndoCommand *parent)
  :ZUndoCommand(parent), doc(doc), zstack(NULL), success(false)
{
  setText(QObject::tr("watershed"));
}

ZStackDocCommand::StackProcess::Watershed::~Watershed()
{
  delete zstack;
}

void ZStackDocCommand::StackProcess::Watershed::undo()
{
  startUndo();
  if (success) {
    doc->loadStack(zstack);
    //doc->requestRedrawStack();
    zstack = NULL;
    success = false;
  } else {
    delete zstack;
    zstack = NULL;
  }
}

void ZStackDocCommand::StackProcess::Watershed::redo()
{
  //zstack = new ZStack(*(doc->m_stack));
  zstack = doc->getStack()->clone();
  success = doc->watershed();
}

ZStackDocCommand::StackProcess::EnhanceLine::EnhanceLine(
    ZStackDoc *doc, QUndoCommand *parent)
  :ZUndoCommand(parent), doc(doc), zstack(NULL), success(false)
{
  setText(QObject::tr("Enhance Line"));
}

ZStackDocCommand::StackProcess::EnhanceLine::~EnhanceLine()
{
  delete zstack;
}

void ZStackDocCommand::StackProcess::EnhanceLine::undo()
{
  startUndo();
  if (success) {
    doc->loadStack(zstack);
    //doc->requestRedrawStack();
    zstack = NULL;
    success = false;
  } else {
    delete zstack;
    zstack = NULL;
  }
}

void ZStackDocCommand::StackProcess::EnhanceLine::redo()
{
  //zstack = new ZStack(*(doc->m_stack));
  zstack = doc->getStack()->clone();
  success = doc->enhanceLine();
}

