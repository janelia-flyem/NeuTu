#include "zbiocytinprojectiondoc.h"
#include "zstackframe.h"

ZBiocytinProjectionDoc::ZBiocytinProjectionDoc(QObject *parent) :
  ZStackDoc(parent)
{
  setTag(NeuTube::Document::BIOCYTIN_PROJECTION);
}

ZBiocytinProjectionDoc::~ZBiocytinProjectionDoc()
{
  disconnect(this, SIGNAL(swcModified()),
          m_parentDoc.get(), SIGNAL(swcModified()));
  removeObject(ZStackObject::TYPE_SWC, false);
}

void ZBiocytinProjectionDoc::setParentDoc(ZSharedPointer<ZStackDoc> parentDoc)
{
  ZStackDoc::setParentDoc(parentDoc);

  connect(this, SIGNAL(zoomingToSelectedSwcNode()),
          m_parentDoc.get(), SIGNAL(zoomingToSelectedSwcNode()));

  connect(m_parentDoc.get(), SIGNAL(swcModified()),
          this, SLOT(updateSwc()));
  connect(this, SIGNAL(swcModified()),
          m_parentDoc.get(), SIGNAL(swcModified()));
  connect(this, SIGNAL(swcTreeNodeSelectionChanged(
                         QList<Swc_Tree_Node*>,QList<Swc_Tree_Node*>)),
          m_parentDoc.get(), SIGNAL(swcTreeNodeSelectionChanged(
                                      QList<Swc_Tree_Node*>,QList<Swc_Tree_Node*>)));
  updateSwc();
}

void ZBiocytinProjectionDoc::updateSwc()
{
  disconnect(this, SIGNAL(swcModified()),
             m_parentDoc.get(), SIGNAL(swcModified()));
  removeObject(ZStackObject::TYPE_SWC, false);
  QList<ZSwcTree*> treeList = m_parentDoc->getSwcList();
  for (QList<ZSwcTree*>::iterator iter = treeList.begin();
       iter != treeList.end(); ++iter) {
    addObject(*iter);
  }
  connect(this, SIGNAL(swcModified()),
               m_parentDoc.get(), SIGNAL(swcModified()));
}

void ZBiocytinProjectionDoc::selectSwcNode(const ZRect2d &roi)
{
  ZStackDoc::selectSwcNode(roi);
  if (m_parentDoc.get() != NULL) {
    m_parentDoc->selectSwcNode(roi);
  }
}

bool ZBiocytinProjectionDoc::executeDeleteSwcNodeCommand()
{
  if (m_parentDoc.get() != NULL) {
    return m_parentDoc->executeDeleteSwcNodeCommand();
  }

  return ZStackDoc::executeDeleteSwcNodeCommand();
}

bool ZBiocytinProjectionDoc::executeConnectSwcNodeCommand()
{
  if (m_parentDoc.get() != NULL) {
    return m_parentDoc->executeConnectSwcNodeCommand();
  }

  return ZStackDoc::executeConnectSwcNodeCommand();
}


void ZBiocytinProjectionDoc::processRectRoiUpdate()
{
  ZRect2d roi = getRect2dRoi();
  if (roi.isValid()) {
    selectSwcNode(roi);
    removeRect2dRoi();
  }
}

void ZBiocytinProjectionDoc::processRectRoiUpdate(ZRect2d *rect)
{
  if (rect != NULL) {
    if (rect->isValid()) {
      selectSwcNode(*rect);
      removeObject(rect);
    }
  }
}

bool ZBiocytinProjectionDoc::executeAddSwcBranchCommand(
    ZSwcTree *tree, double minConnDist)
{
  if (m_parentDoc.get() != NULL) {
    return m_parentDoc->executeAddSwcBranchCommand(tree, minConnDist);
  }

  return ZStackDoc::executeAddSwcBranchCommand(tree, minConnDist);
}

bool ZBiocytinProjectionDoc::executeAddSwcCommand(ZSwcTree *tree)
{
  if (m_parentDoc.get() != NULL) {
    return m_parentDoc->executeAddSwcCommand(tree);
  }

  return ZStackDoc::executeAddSwcCommand(tree);
}

bool ZBiocytinProjectionDoc::executeReplaceSwcCommand(ZSwcTree *tree)
{
  if (m_parentDoc.get() != NULL) {
    return m_parentDoc->executeReplaceSwcCommand(tree);
  }

  return ZStackDoc::executeReplaceSwcCommand(tree);
}

void ZBiocytinProjectionDoc::executeSwcRescaleCommand(
    const ZRescaleSwcSetting &setting)
{
  if (m_parentDoc.get() != NULL) {
    m_parentDoc->executeSwcRescaleCommand(setting);
  } else {
    ZStackDoc::executeSwcRescaleCommand(setting);
  }
}

bool ZBiocytinProjectionDoc::executeSwcNodeExtendCommand(const ZPoint &/*center*/)
{
  return false;
}

bool ZBiocytinProjectionDoc::executeSwcNodeExtendCommand(
    const ZPoint &/*center*/, double /*radius*/)
{
  return false;
}

bool ZBiocytinProjectionDoc::executeSwcNodeSmartExtendCommand(
    const ZPoint &/*center*/)
{
  return false;
}

bool ZBiocytinProjectionDoc::executeSwcNodeSmartExtendCommand(
    const ZPoint &/*center*/, double /*radius*/)
{
  return false;
}

bool ZBiocytinProjectionDoc::executeSwcNodeChangeZCommand(double /*z*/)
{
  return false;
}

bool ZBiocytinProjectionDoc::executeSwcNodeEstimateRadiusCommand()
{
  if (m_parentDoc.get() != NULL) {
    return m_parentDoc->executeSwcNodeEstimateRadiusCommand();
  }

  return ZStackDoc::executeSwcNodeEstimateRadiusCommand();
}

bool ZBiocytinProjectionDoc::executeMoveSwcNodeCommand(
    double dx, double dy, double dz)
{
  if (m_parentDoc.get() != NULL) {
    return m_parentDoc->executeMoveSwcNodeCommand(dx, dy, dz);
  }

  return ZStackDoc::executeMoveSwcNodeCommand(dx, dy, dz);
}

bool ZBiocytinProjectionDoc::executeScaleSwcNodeCommand(
    double sx, double sy, double sz, const ZPoint &center)
{
  if (m_parentDoc.get() != NULL) {
    return m_parentDoc->executeScaleSwcNodeCommand(sx, sy, sz, center);
  }

  return ZStackDoc::executeScaleSwcNodeCommand(sx, sy, sz, center);
}

bool ZBiocytinProjectionDoc::executeRotateSwcNodeCommand(
    double theta, double psi, bool aroundCenter)
{
  if (m_parentDoc.get() != NULL) {
    return m_parentDoc->executeRotateSwcNodeCommand(theta, psi, aroundCenter);
  }

  return ZStackDoc::executeRotateSwcNodeCommand(theta, psi, aroundCenter);
}

bool ZBiocytinProjectionDoc::executeTranslateSelectedSwcNode()
{
  if (m_parentDoc.get() != NULL) {
    return m_parentDoc->executeTranslateSelectedSwcNode();
  }

  return ZStackDoc::executeTranslateSelectedSwcNode();
}

bool ZBiocytinProjectionDoc::executeChangeSelectedSwcNodeSize()
{
  if (m_parentDoc.get() != NULL) {
    return m_parentDoc->executeChangeSelectedSwcNodeSize();
  }

  return ZStackDoc::executeChangeSelectedSwcNodeSize();
}

bool ZBiocytinProjectionDoc::executeConnectSwcNodeCommand(Swc_Tree_Node *tn)
{
  if (m_parentDoc.get() != NULL) {
    return m_parentDoc->executeConnectSwcNodeCommand(tn);
  }

  return ZStackDoc::executeConnectSwcNodeCommand(tn);
}

bool ZBiocytinProjectionDoc::executeConnectSwcNodeCommand(
    Swc_Tree_Node *tn1, Swc_Tree_Node *tn2)
{
  if (m_parentDoc.get() != NULL) {
    return m_parentDoc->executeConnectSwcNodeCommand(tn1, tn2);
  }

  return ZStackDoc::executeConnectSwcNodeCommand(tn1, tn2);
}

bool ZBiocytinProjectionDoc::executeSmartConnectSwcNodeCommand(
    Swc_Tree_Node *tn1, Swc_Tree_Node *tn2)
{
  if (m_parentDoc.get() != NULL) {
    return m_parentDoc->executeSmartConnectSwcNodeCommand(tn1, tn2);
  }

  return ZStackDoc::executeSmartConnectSwcNodeCommand(tn1, tn2);
}

bool ZBiocytinProjectionDoc::executeSmartConnectSwcNodeCommand()
{
  if (m_parentDoc.get() != NULL) {
    return m_parentDoc->executeSmartConnectSwcNodeCommand();
  }

  return ZStackDoc::executeSmartConnectSwcNodeCommand();
}

bool ZBiocytinProjectionDoc::executeBreakSwcConnectionCommand()
{
  if (m_parentDoc.get() != NULL) {
    return m_parentDoc->executeBreakSwcConnectionCommand();
  }

  return ZStackDoc::executeBreakSwcConnectionCommand();
}

bool ZBiocytinProjectionDoc::executeAddSwcNodeCommand(
    const ZPoint &center, double radius)
{
  if (m_parentDoc.get() != NULL) {
    return m_parentDoc->executeAddSwcNodeCommand(center, radius);
  }

  return ZStackDoc::executeAddSwcNodeCommand(center, radius);
}

bool ZBiocytinProjectionDoc::executeSwcNodeChangeSizeCommand(double dr)
{
  if (m_parentDoc.get() != NULL) {
    return m_parentDoc->executeSwcNodeChangeSizeCommand(dr);
  }

  return ZStackDoc::executeSwcNodeChangeSizeCommand(dr);
}

bool ZBiocytinProjectionDoc::executeMergeSwcNodeCommand()
{
  if (m_parentDoc.get() != NULL) {
    return m_parentDoc->executeMergeSwcNodeCommand();
  }

  return ZStackDoc::executeMergeSwcNodeCommand();
}

bool ZBiocytinProjectionDoc::executeTraceSwcBranchCommand(
    double x, double y, double z)
{
  if (m_parentDoc.get() != NULL) {
    return m_parentDoc->executeTraceSwcBranchCommand(x, y, z);
  }

  return ZStackDoc::executeTraceSwcBranchCommand(x, y, z);
}

bool ZBiocytinProjectionDoc::executeTraceSwcBranchCommand(double x, double y)
{
  if (m_parentDoc.get() != NULL) {
    return m_parentDoc->executeTraceSwcBranchCommand(x, y);
  }

  return ZStackDoc::executeTraceSwcBranchCommand(x, y);
}

bool ZBiocytinProjectionDoc::executeInterpolateSwcZCommand()
{
  if (m_parentDoc.get() != NULL) {
    return m_parentDoc->executeInterpolateSwcZCommand();
  }

  return ZStackDoc::executeInterpolateSwcZCommand();
}

bool ZBiocytinProjectionDoc::executeInterpolateSwcRadiusCommand()
{
  if (m_parentDoc.get() != NULL) {
    return m_parentDoc->executeInterpolateSwcRadiusCommand();
  }

  return ZStackDoc::executeInterpolateSwcRadiusCommand();
}

bool ZBiocytinProjectionDoc::executeInterpolateSwcPositionCommand()
{
  if (m_parentDoc.get() != NULL) {
    return m_parentDoc->executeInterpolateSwcPositionCommand();
  }

  return ZStackDoc::executeInterpolateSwcPositionCommand();
}

bool ZBiocytinProjectionDoc::executeInterpolateSwcCommand()
{
  if (m_parentDoc.get() != NULL) {
    return m_parentDoc->executeInterpolateSwcCommand();
  }

  return ZStackDoc::executeInterpolateSwcCommand();
}

bool ZBiocytinProjectionDoc::executeBreakForestCommand()
{
  if (m_parentDoc.get() != NULL) {
    return m_parentDoc->executeBreakForestCommand();
  }

  return ZStackDoc::executeBreakForestCommand();
}

bool ZBiocytinProjectionDoc::executeGroupSwcCommand()
{
  if (m_parentDoc.get() != NULL) {
    return m_parentDoc->executeGroupSwcCommand();
  }

  return ZStackDoc::executeGroupSwcCommand();
}

bool ZBiocytinProjectionDoc::executeSetRootCommand()
{
  if (m_parentDoc.get() != NULL) {
    return m_parentDoc->executeSetRootCommand();
  }

  return ZStackDoc::executeSetRootCommand();
}

bool ZBiocytinProjectionDoc::executeRemoveTurnCommand()
{
  if (m_parentDoc.get() != NULL) {
    return m_parentDoc->executeRemoveTurnCommand();
  }

  return ZStackDoc::executeRemoveTurnCommand();
}

bool ZBiocytinProjectionDoc::executeResolveCrossoverCommand()
{
  if (m_parentDoc.get() != NULL) {
    return m_parentDoc->executeResolveCrossoverCommand();
  }

  return ZStackDoc::executeResolveCrossoverCommand();
}

bool ZBiocytinProjectionDoc::executeInsertSwcNode()
{
  if (m_parentDoc.get() != NULL) {
    return m_parentDoc->executeInsertSwcNode();
  }

  return ZStackDoc::executeInsertSwcNode();
}

bool ZBiocytinProjectionDoc::executeSetBranchPoint()
{
  if (m_parentDoc.get() != NULL) {
    return m_parentDoc->executeSetBranchPoint();
  }

  return ZStackDoc::executeSetBranchPoint();
}

bool ZBiocytinProjectionDoc::executeConnectIsolatedSwc()
{
  if (m_parentDoc.get() != NULL) {
    return m_parentDoc->executeConnectIsolatedSwc();
  }

  return ZStackDoc::executeConnectIsolatedSwc();
}

bool ZBiocytinProjectionDoc::executeResetBranchPoint()
{
  if (m_parentDoc.get() != NULL) {
    return m_parentDoc->executeResetBranchPoint();
  }

  return ZStackDoc::executeResetBranchPoint();
}

bool ZBiocytinProjectionDoc::executeMoveAllSwcCommand(
    double dx, double dy, double dz)
{
  if (m_parentDoc.get() != NULL) {
    return m_parentDoc->executeMoveAllSwcCommand(dx, dy, dz);
  }

  return ZStackDoc::executeMoveAllSwcCommand(dx, dy, dz);
}

bool ZBiocytinProjectionDoc::executeScaleAllSwcCommand(
    double sx, double sy, double sz, bool aroundCenter)
{
  if (m_parentDoc.get() != NULL) {
    return m_parentDoc->executeScaleAllSwcCommand(sx, sy, sz, aroundCenter);
  }

  return ZStackDoc::executeScaleAllSwcCommand(sx, sy, sz, aroundCenter);
}

bool ZBiocytinProjectionDoc::executeRotateAllSwcCommand(
    double theta, double psi, bool aroundCenter)
{
  if (m_parentDoc.get() != NULL) {
    return m_parentDoc->executeRotateAllSwcCommand(theta, psi, aroundCenter);
  }

  return ZStackDoc::executeRotateAllSwcCommand(theta, psi, aroundCenter);
}
