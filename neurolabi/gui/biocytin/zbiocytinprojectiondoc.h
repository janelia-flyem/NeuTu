#ifndef ZBIOCYTINPROJECTIONDOC_H
#define ZBIOCYTINPROJECTIONDOC_H

#include "zstackdoc.h"

class ZBiocytinProjectionDoc : public ZStackDoc
{
  Q_OBJECT
public:
  explicit ZBiocytinProjectionDoc(QObject *parent = 0);
  ~ZBiocytinProjectionDoc();

  void setParentDoc(ZSharedPointer<ZStackDoc> doc);

  using ZStackDoc::processRectRoiUpdate; // suppress warning: hides overloaded virtual function [-Woverloaded-virtual]
  void processRectRoiUpdate(ZRect2d *rect);

signals:

public slots:
  virtual bool executeAddSwcBranchCommand(ZSwcTree *tree, double minConnDist);
  virtual bool executeAddSwcCommand(ZSwcTree *tree);
  virtual bool executeReplaceSwcCommand(ZSwcTree *tree);
  virtual void executeSwcRescaleCommand(const ZRescaleSwcSetting &setting);

  virtual bool executeSwcNodeExtendCommand(const ZPoint &center);
  virtual bool executeSwcNodeExtendCommand(const ZPoint &center, double radius);
  virtual bool executeSwcNodeSmartExtendCommand(const ZPoint &center);
  virtual bool executeSwcNodeSmartExtendCommand(const ZPoint &center, double radius);
  virtual bool executeSwcNodeChangeZCommand(double z);
  virtual bool executeSwcNodeEstimateRadiusCommand();
  virtual bool executeMoveSwcNodeCommand(double dx, double dy, double dz);
  virtual bool executeScaleSwcNodeCommand(
      double sx, double sy, double sz, const ZPoint &center);
  virtual bool executeRotateSwcNodeCommand(
      double theta, double psi, bool aroundCenter);
  virtual bool executeTranslateSelectedSwcNode();
  virtual bool executeDeleteSwcNodeCommand();
  virtual bool executeConnectSwcNodeCommand();
  virtual bool executeChangeSelectedSwcNodeSize();
  virtual bool executeConnectSwcNodeCommand(Swc_Tree_Node *tn);
  virtual bool executeConnectSwcNodeCommand(
      Swc_Tree_Node *tn1, Swc_Tree_Node *tn2);
  virtual bool executeSmartConnectSwcNodeCommand(
      Swc_Tree_Node *tn1, Swc_Tree_Node *tn2);
  virtual bool executeSmartConnectSwcNodeCommand();
  virtual bool executeBreakSwcConnectionCommand();
  virtual bool executeAddSwcNodeCommand(const ZPoint &center, double radius);
  virtual bool executeSwcNodeChangeSizeCommand(double dr);
  virtual bool executeMergeSwcNodeCommand();
  virtual bool executeTraceSwcBranchCommand(double x, double y, double z);
  virtual bool executeTraceSwcBranchCommand(double x, double y);
  virtual bool executeInterpolateSwcZCommand();
  virtual bool executeInterpolateSwcRadiusCommand();
  virtual bool executeInterpolateSwcPositionCommand();
  virtual bool executeInterpolateSwcCommand();
  virtual bool executeBreakForestCommand();
  virtual bool executeGroupSwcCommand();
  virtual bool executeSetRootCommand();
  virtual bool executeRemoveTurnCommand();
  virtual bool executeResolveCrossoverCommand();
  virtual bool executeInsertSwcNode();
  virtual bool executeSetBranchPoint();
  virtual bool executeConnectIsolatedSwc();
  virtual bool executeResetBranchPoint();

  virtual bool executeMoveAllSwcCommand(double dx, double dy, double dz);
  virtual bool executeScaleAllSwcCommand(double sx, double sy, double sz,
                                 bool aroundCenter = false);
  virtual bool executeRotateAllSwcCommand(
      double theta, double psi, bool aroundCenter = false);

  void updateSwc();

protected:
  void selectSwcNode(const ZRect2d &roi);
  void processRectRoiUpdate();
};

#endif // ZBIOCYTINPROJECTIONDOC_H
