#ifndef ZSTACKDOCCOMMAND_H
#define ZSTACKDOCCOMMAND_H

#include <QUndoCommand>
#include <QList>
#include <QMap>

#include "swctreenode.h"
#include "neutube.h"
#include "zswcpath.h"
#include "zdocplayer.h"
#include "zstackobjectrole.h"

class ZSwcTree;
class ZLocsegChain;
class ZObject3d;
class ZLocsegChainConn;
class ZPunctum;
class ZProgrogressReporter;
class ZStack;
class ZStroke2d;
class ZStackDoc;
class ZDocumentable;

class ZUndoCommand : public QUndoCommand
{
public:
  explicit ZUndoCommand(QUndoCommand *parent = 0);
  explicit ZUndoCommand(const QString &text, QUndoCommand *parent = 0);

  bool isSaved(NeuTube::EDocumentableType type) const;
  void setSaved(NeuTube::EDocumentableType type, bool state);

  void enableLog(bool on);
  bool loggingCommand() const;
  void logCommand(const QString &msg) const;
  void logCommand() const;
  void logUndoCommand() const;
  void setLogMessage(const QString &msg);
  void setLogMessage(const std::string &msg);
  void setLogMessage(const char *msg);

  void startUndo();

private:
  bool m_isSwcSaved;
  bool m_loggingCommand;
  QString m_logMessage;
};

namespace ZStackDocCommand {
namespace SwcEdit {
/*!
 * \brief The basic command of modifying swc.
 */
class ChangeSwcCommand : public ZUndoCommand
{
public:
  ChangeSwcCommand(ZStackDoc *doc, QUndoCommand *parent = NULL);
  virtual ~ChangeSwcCommand();
  void undo();
  void redo();

  enum EOperation {
    OP_SET_PARENT, OP_SET_FIRST_CHILD, OP_DETACH_PARENT
  };

  enum ERole {
    ROLE_NONE, ROLE_CHILD, ROLE_PARENT
  };

  bool isSwcModified() const {
    return m_isSwcModified;
  }

  void setSwcModified(bool state) {
    m_isSwcModified = state;
  }


protected:
  /*!
   * \brief Backup a node.
   *
   * The properties and links of \a tn will be backed up after the function call.
   * The function has no effect on \a tn if \a tn has already been backed up in
   * the command.
   */
  void backup(Swc_Tree_Node *tn);

  /*!
   * \brief Backup a node and its neighbors.
   *
   * Bacup all nodes affected by the operation \a op, which can be setting parent,
   * setting first child and detaching from parent. \a role specifies the role
   * of \a tn for the operation.
   *
   * \param tn The node to backup.
   * \param op Operation supposed to be performed after the backup.
   * \param role Role of \a tn.
   */
  void backup(Swc_Tree_Node *tn, EOperation op, ERole role = ROLE_NONE);

  /*!
   * \brief Backup the children of a node.
   */
  void backupChildren(Swc_Tree_Node *tn);


  /*!
   * \brief Backup childrend of a node according to an operation.
   */
  void backupChildren(Swc_Tree_Node *tn, EOperation op, ERole role = ROLE_NONE);

  /*!
   * \brief Record newly created node by the command
   *
   * It tracks newly created nodes so that the nodes can be freed in an undone
   * command when the command is destroyed.
   */
  void addNewNode(Swc_Tree_Node *tn);

  /*!
   * \brief Record removed node.
   */
  void recordRemovedNode(Swc_Tree_Node *tn);

  void recover();

protected:
  ZStackDoc *m_doc;
  std::map<Swc_Tree_Node*, Swc_Tree_Node> m_backupSet;
  std::set<Swc_Tree_Node*> m_newNodeSet;
  std::set<Swc_Tree_Node*> m_removedNodeSet;
  std::set<Swc_Tree_Node*> m_garbageSet;
  bool m_isSwcModified;
};

class TranslateRoot : public ZUndoCommand
{
public:
  TranslateRoot(ZStackDoc *doc, double x, double y, double z,
                QUndoCommand *parent = NULL);
  virtual ~TranslateRoot();
  void undo();
  void redo();

private:
  ZStackDoc *m_doc;
  QList<ZSwcTree*> m_swcList;
  double m_x;
  double m_y;
  double m_z;
};

class Rescale : public ZUndoCommand
{
public:
  Rescale(ZStackDoc *doc, double scaleX, double scaleY, double scaleZ,
            QUndoCommand *parent = NULL);
  Rescale(ZStackDoc *doc, double srcPixelPerUmXY, double srcPixelPerUmZ,
            double dstPixelPerUmXY, double dstPixelPerUmZ,
            QUndoCommand *parent = 0);
  virtual ~Rescale();
  void undo();
  void redo();

private:
  ZStackDoc *m_doc;
  QList<ZSwcTree*> m_swcList;
  double m_scaleX;
  double m_scaleY;
  double m_scaleZ;
};

class RescaleRadius : public ZUndoCommand
{
public:
  RescaleRadius(ZStackDoc *doc, double scale, int startdepth,
                  int enddepth, QUndoCommand *parent = NULL);
  virtual ~RescaleRadius();
  void undo();
  void redo();

private:
  ZStackDoc *m_doc;
  QList<ZSwcTree*> m_swcList;
  double m_scale;
  int m_startdepth;
  int m_enddepth;
};

class ReduceNodeNumber : public ZUndoCommand
{
public:
  ReduceNodeNumber(ZStackDoc *doc, double lengthThre, QUndoCommand *parent = NULL);
  virtual ~ReduceNodeNumber();
  void undo();
  void redo();

private:
  ZStackDoc *m_doc;
  QList<ZSwcTree*> m_swcList;
  double m_lengthThre;
};

class CompositeCommand : public ZUndoCommand
{
public:
  CompositeCommand(ZStackDoc *doc, QUndoCommand *parent = NULL);
  virtual ~CompositeCommand();

  void redo();
  void undo();
protected:
  ZStackDoc *m_doc;
  bool m_isExecuted;
};

class AddSwc : public ZUndoCommand
{
public:
  AddSwc(ZStackDoc *doc, ZSwcTree *tree, QUndoCommand *parent = NULL);
  virtual ~AddSwc();
  void undo();
  void redo();

private:
  ZStackDoc *m_doc;
  ZSwcTree *m_tree;
  bool m_isInDoc;
};

class AddSwcNode : public ZUndoCommand
{
public:
  AddSwcNode(ZStackDoc *doc, Swc_Tree_Node* tn, ZStackObjectRole::TRole role,
             QUndoCommand *parent = NULL);
  virtual ~AddSwcNode();
  void undo();
  void redo();

private:
  ZStackDoc *m_doc;
  Swc_Tree_Node *m_node;
  ZSwcTree* m_tree;
  bool m_treeInDoc;
  static int m_index;  // used to generate unique source for each new swc tree
};

class ExtendSwcNode : public ZUndoCommand
{
public:
  ExtendSwcNode(ZStackDoc *doc, Swc_Tree_Node* node, Swc_Tree_Node* pnode,
                QUndoCommand *parent = NULL);
  virtual ~ExtendSwcNode();
  void undo();
  void redo();

private:
  ZStackDoc *m_doc;
  Swc_Tree_Node *m_node;
  Swc_Tree_Node *m_parentNode;
  bool m_nodeInDoc;
};
/*
class BreakParentLink : public ChangeSwcCommand
{
public:
  BreakParentLink(ZStackDoc *doc, QUndoCommand *parent = NULL);
  virtual ~BreakParentLink();

  void redo();
  void undo();
};
*/

class MergeSwcNode : public ChangeSwcCommand
{
public:
  MergeSwcNode(ZStackDoc *doc, QUndoCommand *parent = NULL);
  virtual ~MergeSwcNode();

  void redo();
  void undo();

private:
  std::set<Swc_Tree_Node*> m_selectedNodeSet;
  Swc_Tree_Node *m_coreNode;
};

class ResolveCrossover : public ChangeSwcCommand
{
public:
  ResolveCrossover(ZStackDoc *doc, QUndoCommand *parent = NULL);
  virtual ~ResolveCrossover();

  void redo();
  void undo();

private:
  std::set<Swc_Tree_Node*> m_selectedNodeSet;
};

class ChangeSwcNodeGeometry : public ZUndoCommand
{
public:
  ChangeSwcNodeGeometry(ZStackDoc *doc, Swc_Tree_Node* node, double x, double y,
                        double z, double r, QUndoCommand *parent = NULL);
  virtual ~ChangeSwcNodeGeometry();
  void undo();
  void redo();

private:
  ZStackDoc *m_doc;
  Swc_Tree_Node *m_node;
  double m_x;
  double m_y;
  double m_z;
  double m_r;
  double m_backupX;
  double m_backupY;
  double m_backupZ;
  double m_backupR;
};

class ChangeSwcNodeZ : public ZUndoCommand
{
public:
  ChangeSwcNodeZ(ZStackDoc *doc, Swc_Tree_Node* node, double z,
                QUndoCommand *parent = NULL);
  virtual ~ChangeSwcNodeZ();
  void undo();
  void redo();

private:
  ZStackDoc *m_doc;
  Swc_Tree_Node *m_node;
  double m_z;
  double m_backup;
};

class ChangeSwcNodeRadius : public ZUndoCommand
{
public:
  ChangeSwcNodeRadius(ZStackDoc *doc, Swc_Tree_Node* node, double radius,
                QUndoCommand *parent = NULL);
  virtual ~ChangeSwcNodeRadius();
  void undo();
  void redo();

private:
  ZStackDoc *m_doc;
  Swc_Tree_Node *m_node;
  double m_radius;
  double m_backup;
};

class ChangeSwcNode : public ZUndoCommand
{
public:
  ChangeSwcNode(ZStackDoc *doc, Swc_Tree_Node* node,
                const Swc_Tree_Node &newNode, QUndoCommand *parent = NULL);
  virtual ~ChangeSwcNode();
  void undo();
  void redo();

private:
  ZStackDoc *m_doc;
  Swc_Tree_Node *m_node;
  Swc_Tree_Node m_backup;
  Swc_Tree_Node m_newNode;
};

class DeleteSwcNode : public CompositeCommand
{
public:
  DeleteSwcNode(ZStackDoc *doc, Swc_Tree_Node* node, Swc_Tree_Node *root,
                QUndoCommand *parent = NULL);
  virtual ~DeleteSwcNode();

#if 0
  void undo();
  void redo();
#endif

private:
  ZStackDoc *m_doc;
  Swc_Tree_Node *m_node;
  Swc_Tree_Node *m_root;
  Swc_Tree_Node m_backup;
  Swc_Tree_Node *m_prevSibling;
  Swc_Tree_Node *m_lastChild;
  bool m_nodeInDoc;
};

class DeleteSwcNodeSet : public CompositeCommand
{
public:
  DeleteSwcNodeSet(ZStackDoc *doc, std::set<Swc_Tree_Node*> &nodeSet,
                   QUndoCommand *parent = NULL);
  virtual ~DeleteSwcNodeSet();
private:
  ZStackDoc *m_doc;
  std::set<Swc_Tree_Node*> m_nodeSet;
  bool m_nodeInDoc;
};

class SetParent : public ZUndoCommand
{
public:
  SetParent(ZStackDoc *doc, Swc_Tree_Node *node, Swc_Tree_Node *parentNode,
            bool deletingOrphan, QUndoCommand *parent);
  virtual ~SetParent();

  void undo();
  void redo();

private:
  ZStackDoc *m_doc;
  Swc_Tree_Node *m_node;
  Swc_Tree_Node *m_newParent;
  Swc_Tree_Node *m_oldParent;
  Swc_Tree_Node *m_prevSibling;
  bool m_deletingOrphan;
  bool m_isExecuted;
};


class SetSwcNodeSeletion : public ZUndoCommand
{
public:
  SetSwcNodeSeletion(ZStackDoc *doc, ZSwcTree *host,
                     const std::set<Swc_Tree_Node*> nodeSet,
                     bool appending,
                     QUndoCommand *parent = NULL);
  virtual ~SetSwcNodeSeletion();

  void undo();
  void redo();

private:
  ZStackDoc *m_doc;
  ZSwcTree *m_host;
  std::set<Swc_Tree_Node*> m_nodeSet;
  bool m_appending;
  std::set<Swc_Tree_Node*> m_oldNodeSet;
};


class RemoveSubtree : public CompositeCommand
{
public:
  RemoveSubtree(ZStackDoc *doc, Swc_Tree_Node *node, QUndoCommand *parent = NULL);
  virtual ~RemoveSubtree();

private:
  ZStackDoc *m_doc;
  Swc_Tree_Node *m_node;
};

class SwcTreeLabeTraceMask : public ZUndoCommand
{
public:
  SwcTreeLabeTraceMask(ZStackDoc *doc, Swc_Tree *tree, QUndoCommand *parent = NULL);
  virtual ~SwcTreeLabeTraceMask();

  void undo();
  void redo();

private:
  ZStackDoc *m_doc;
  Swc_Tree *m_tree;
};

class SwcPathLabeTraceMask : public ZUndoCommand
{
public:
  SwcPathLabeTraceMask(ZStackDoc *doc, const ZSwcPath& branch,
                       QUndoCommand *parent = NULL);
  virtual ~SwcPathLabeTraceMask();

  void undo();
  void redo();

private:
  ZStackDoc *m_doc;
  ZSwcPath m_branch;
};

class SetRoot : public ZUndoCommand//: public CompositeCommand
{
public:
  SetRoot(ZStackDoc *doc, Swc_Tree_Node *tn, QUndoCommand *parent = NULL);
  void undo();
  void redo();

private:
  ZStackDoc *m_doc;
  Swc_Tree_Node *m_node;
  std::vector<Swc_Tree_Node*> m_originalParentArray;
};

class ConnectSwcNode : public CompositeCommand
{
public:
  ConnectSwcNode(ZStackDoc *doc, QUndoCommand *parent = NULL);
};

class RemoveSwc : public ZUndoCommand
{
public:
  RemoveSwc(ZStackDoc *doc, ZSwcTree *tree, QUndoCommand *parent = NULL);
  ~RemoveSwc();
  void redo();
  void undo();

private:
  ZStackDoc *m_doc;
  ZSwcTree *m_tree;
  bool m_isInDoc;
};

class RemoveSwcIfEmpty : public ZUndoCommand
{
public:
  RemoveSwcIfEmpty(ZStackDoc *doc, ZSwcTree *tree, QUndoCommand *parent = NULL);
  ~RemoveSwcIfEmpty();
  void redo();
  void undo();

private:
  ZStackDoc *m_doc;
  ZSwcTree *m_tree;
  bool m_isInDoc;
};

class RemoveEmptyTree : public CompositeCommand
{
public:
  RemoveEmptyTree(ZStackDoc *doc, QUndoCommand *parent = NULL);
  virtual ~RemoveEmptyTree();

private:
  ZStackDoc *m_doc;
//  std::set<ZSwcTree*> m_emptyTreeSet;
};

/*!
 * \brief Remove empty trees at the action point
 */
class RemoveEmptyTreePost : public CompositeCommand
{
public:
  RemoveEmptyTreePost(ZStackDoc *doc, QUndoCommand *parent = NULL);
  virtual ~RemoveEmptyTreePost();

  void undo();
  void redo();

private:
  ZStackDoc *m_doc;
  std::set<ZSwcTree*> m_emptyTreeSet;
};

//Not operation invariant
class BreakForest : public CompositeCommand
{
public:
  BreakForest(ZStackDoc *doc, QUndoCommand *parent = NULL);
};

//Not operation invariant
class GroupSwc : public CompositeCommand
{
public:
  GroupSwc(ZStackDoc *doc, QUndoCommand *parent = NULL);
};


#if 0
class TraceSwcBranch : public QUndoCommand
{
public:
  TraceSwcBranch(ZStackDoc *doc, double x, double y, double z, int c,
                 QUndoCommand *parent = NULL);
  virtual ~TraceSwcBranch();

  void redo();
  void undo();

private:
  ZStackDoc *m_doc;
  double m_x;
  double m_y;
  double m_z;
  int m_c;
  ZSwcTree *m_tree;
  bool m_isTreeInDoc;
};
#endif

}

namespace ObjectEdit {
class AddObject : public ZUndoCommand
{
public:
  AddObject(ZStackDoc *doc, ZStackObject *obj,
            bool uniqueSource, QUndoCommand *parent = NULL);
  ~AddObject();
  void redo();
  void undo();

private:
  ZStackDoc *m_doc;
  ZStackObject *m_obj;
  bool m_uniqueSource;
  QList<ZStackObject*> m_uniqueObjectList;
  bool m_isInDoc;
};

class RemoveObject : public ZUndoCommand
{
public:
  RemoveObject(ZStackDoc *doc, ZStackObject *obj,
               QUndoCommand *parent = NULL);
  virtual ~RemoveObject();

  void undo();
  void redo();

private:
  ZStackDoc *m_doc;
  ZStackObject *m_obj;
  bool m_isInDoc;
};

class RemoveSelected : public ZUndoCommand
{
public:
  RemoveSelected(ZStackDoc *doc, QUndoCommand *parent = NULL);
  virtual ~RemoveSelected();

  void undo();
  void redo();

private:
  void notifyObjectChanged(const QList<ZStackObject*> &selectedObject) const;

private:
  ZStackDoc *doc;
  QList<ZStackObject*> m_selectedObject;
};

class MoveSelected : public ZUndoCommand
{
  ZStackDoc *m_doc;
  QList<ZSwcTree*> m_swcList;
  QList<ZPunctum*> m_punctaList;
  std::set<Swc_Tree_Node*> m_swcNodeList;
  double m_x;
  double m_y;
  double m_z;
  bool m_swcMoved;
  bool m_punctaMoved;
  double m_swcScaleX;
  double m_swcScaleY;
  double m_swcScaleZ;
  double m_punctaScaleX;
  double m_punctaScaleY;
  double m_punctaScaleZ;
public:
  MoveSelected(ZStackDoc *doc, double x, double y,
               double z, QUndoCommand *parent = NULL);
  virtual ~MoveSelected();
  void setSwcCoordScale(double x, double y, double z);
  void setPunctaCoordScale(double x, double y, double z);
  virtual int id() const { return 1; }
  virtual bool mergeWith(const QUndoCommand *other);
  void undo();
  void redo();
};
}

namespace TubeEdit {
class RemoveSmall : public ZUndoCommand
{
public:
  RemoveSmall(ZStackDoc *doc, double thre, QUndoCommand *parent = NULL);
  virtual ~RemoveSmall();

  void undo();    // conn information may be lost after undo
  void redo();

private:
  ZStackDoc *m_doc;
  double m_thre;
  QList<ZLocsegChain*> m_chainList;
};

class RemoveSelected : public ZUndoCommand
{
public:
  RemoveSelected(ZStackDoc *doc, QUndoCommand *parent = NULL);
  virtual ~RemoveSelected();

  void undo();    // conn information may be lost after undo
  void redo();

private:
  ZStackDoc *m_doc;
  QList<ZLocsegChain*> m_chainList;
};

class Trace : public ZUndoCommand
{
public:
  Trace(ZStackDoc *doc, int x, int y, int z, QUndoCommand *parent = NULL);
  Trace(ZStackDoc *doc, int x, int y, int z, int c, QUndoCommand *parent = NULL);
  void undo();
  void redo();
private:
  ZStackDoc *m_doc;
  int m_x;
  int m_y;
  int m_z;
  int m_c;
  ZLocsegChain* m_chain;
};

class CutSegment : public ZUndoCommand
{
public:
  CutSegment(ZStackDoc *doc, QUndoCommand *parent = NULL);
  virtual ~CutSegment();

  void undo();
  void redo();
private:
  ZStackDoc *m_doc;
  QList<ZLocsegChain*> m_oldChainList;
  QList<ZLocsegChain*> m_newChainList;
};

class BreakChain : public ZUndoCommand
{
public:
  BreakChain(ZStackDoc *doc, QUndoCommand *parent = NULL);
  virtual ~BreakChain();

  void undo();
  void redo();
private:
  ZStackDoc *m_doc;
  QList<ZLocsegChain*> m_oldChainList;
  QList<ZLocsegChain*> m_newChainList;
};
#if 0
class AutoTrace : public QUndoCommand
{
public:
  AutoTrace(ZStackDoc *doc, QUndoCommand *parent = NULL);
  virtual ~AutoTrace();

  void undo();
  void redo();

private:
  ZStackDoc *m_doc;
  QList<ZLocsegChain*> m_chainList;
  QList<ZSwcTree*> m_swcList;
  QList<ZLocsegChainConn*> m_connList;
  QList<ZObject3d*> m_obj3dList;
  QList<ZPunctum*> m_punctaList;
};
#endif
class AutoTraceAxon : public ZUndoCommand
{
public:
  AutoTraceAxon(ZStackDoc *m_doc, QUndoCommand *parent = NULL);
  virtual ~AutoTraceAxon();

  void undo();
  void redo();

private:
  ZStackDoc *m_doc;
  QList<ZLocsegChain*> m_chainList;
  QList<ZSwcTree*> m_swcList;
  QList<ZLocsegChainConn*> m_connList;
  QList<ZObject3d*> m_obj3dList;
  QList<ZPunctum*> m_punctaList;
};
}

namespace StrokeEdit {
class AddStroke : public ZUndoCommand
{
public:
  AddStroke(ZStackDoc *doc, ZStroke2d *stroke, QUndoCommand *parent = NULL);
  virtual ~AddStroke();
  void undo();
  void redo();

private:
  ZStackDoc *m_doc;
  ZStroke2d *m_stroke;
  bool m_isInDoc;
};

class RemoveTopStroke : public ZUndoCommand
{
public:
  RemoveTopStroke(ZStackDoc *doc, QUndoCommand *parent = NULL);
  virtual ~RemoveTopStroke();
  void undo();
  void redo();

private:
  ZStackDoc *m_doc;
  ZStroke2d *m_stroke;
  bool m_isInDoc;
};

class CompositeCommand : public ZUndoCommand
{
public:
  CompositeCommand(ZStackDoc *doc, QUndoCommand *parent = NULL);
  virtual ~CompositeCommand();

  void redo();
  void undo();
protected:
  ZStackDoc *m_doc;
};
}

namespace StackProcess {
class Binarize : public ZUndoCommand
{
  ZStackDoc *doc;
  ZStack *zstack;
  int thre;
  bool success;
public:
  Binarize(ZStackDoc *doc, int thre, QUndoCommand *parent = NULL);
  virtual ~Binarize();
  void undo();
  void redo();
};

class BwSolid : public ZUndoCommand
{
  ZStackDoc *doc;
  ZStack *zstack;
  bool success;
public:
  BwSolid(ZStackDoc *doc, QUndoCommand *parent = NULL);
  virtual ~BwSolid();
  void undo();
  void redo();
};

class EnhanceLine : public ZUndoCommand
{
  ZStackDoc *doc;
  ZStack *zstack;
  bool success;
public:
  EnhanceLine(ZStackDoc *doc, QUndoCommand *parent = NULL);
  virtual ~EnhanceLine();
  void undo();
  void redo();
};

class Watershed : public ZUndoCommand
{
  ZStackDoc *doc;
  ZStack *zstack;
  bool success;
public:
  Watershed(ZStackDoc *doc, QUndoCommand *parent = NULL);
  virtual ~Watershed();
  void undo();
  void redo();
};
}

}

#endif // ZSTACKDOCCOMMAND_H
