#ifndef ZDVIDSPARSESTACK_H
#define ZDVIDSPARSESTACK_H

#include "zstackobject.h"
#include "zsparsestack.h"
#include "zdvidtarget.h"
#include "dvid/zdvidreader.h"

class ZIntCuboid;

class ZDvidSparseStack : public ZStackObject
{
public:
  ZDvidSparseStack();
  ~ZDvidSparseStack();

  void display(ZPainter &painter, int slice, EDisplayStyle option) const;

  const std::string& className() const;

  ZStack *getSlice(int z) const;
  ZStack* getStack();
  ZStack* getStack(const ZIntCuboid &updateBox);

  const ZDvidTarget& getDvidTarget() const {
    return m_dvidTarget;
  }


  int getValue(int x, int y, int z) const;

  const ZIntPoint& getDownsampleInterval() const;

  void setDvidTarget(const ZDvidTarget &target);

  ZIntCuboid getBoundBox() const;
  using ZStackObject::getBoundBox; // fix warning -Woverloaded-virtual

  void loadBody(int bodyId);
  void setMaskColor(const QColor &color);

  uint64_t getLabel() const;

  const ZObject3dScan *getObjectMask() const;
  ZObject3dScan *getObjectMask();

  const ZSparseStack* getSparseStack() const;
  ZSparseStack *getSparseStack();

  void downloadBodyMask();

  bool hit(double x, double y, double z);
  bool hit(double x, double y);

  bool isEmpty() const;


private:
  void initBlockGrid();
  bool fillValue();
  bool fillValue(const ZIntCuboid &box);
  /*
  void assignStackValue(ZStack *stack, const ZObject3dScan &obj,
                               const ZStackBlockGrid &stackGrid);
                               */

private:
  ZSparseStack m_sparseStack;
  ZDvidTarget m_dvidTarget;
  mutable ZDvidReader m_dvidReader;
};

#endif // ZDVIDSPARSESTACK_H
