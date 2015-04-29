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

  void display(ZPainter &painter, int slice, EDisplayStyle option) const;

  const std::string& className() const;

  ZStack *getSlice(int z) const;

  const ZDvidTarget& getDvidTarget() const {
    return m_dvidTarget;
  }

  int getValue(int x, int y, int z) const;

  void setDvidTarget(const ZDvidTarget &target);

  ZIntCuboid getBoundBox() const;

  void loadBody(int bodyId);

private:
  void initBlockGrid();
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
