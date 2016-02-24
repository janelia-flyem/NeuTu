#ifndef ZDVIDSPARSESTACK_H
#define ZDVIDSPARSESTACK_H

#include "zstackobject.h"
#include "zsparsestack.h"
#include "zdvidtarget.h"
#include "dvid/zdvidreader.h"
#include "zthreadfuturemap.h"

class ZIntCuboid;

class ZDvidSparseStack : public ZStackObject
{
public:
  ZDvidSparseStack();
  ~ZDvidSparseStack();

  static ZStackObject::EType GetType() {
    return ZStackObject::TYPE_DVID_SPARSE_STACK;
  }

  void display(ZPainter &painter, int slice, EDisplayStyle option,
               NeuTube::EAxis sliceAxis) const;

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

  void loadBody(uint64_t bodyId, bool canonizing = false);
  void loadBodyAsync(uint64_t bodyId);
  void setMaskColor(const QColor &color);
  void setLabel(uint64_t bodyId);

  uint64_t getLabel() const;

//  const ZObject3dScan *getObjectMask() const;
  ZObject3dScan *getObjectMask();

  const ZSparseStack* getSparseStack() const;
  ZSparseStack *getSparseStack();

  void downloadBodyMask();

  bool hit(double x, double y, double z);
  bool hit(double x, double y, NeuTube::EAxis axis);

  bool isEmpty() const;

  ZDvidSparseStack* getCrop(const ZIntCuboid &box) const;

  void deprecateStackBuffer();


private:
  void init();
  void initBlockGrid();
  bool fillValue();
  bool fillValue(const ZIntCuboid &box);
  QString getLoadBodyThreadId() const;
  void pushMaskColor();
  void pushLabel();
  bool loadingObjectMask() const;
  void finishObjectMaskLoading();
  void syncObjectMask();
  void pushAttribute();
  /*
  void assignStackValue(ZStack *stack, const ZObject3dScan &obj,
                               const ZStackBlockGrid &stackGrid);
                               */

private:
  ZSparseStack m_sparseStack;
  ZDvidTarget m_dvidTarget;
  bool m_isValueFilled;
  uint64_t m_label;
  mutable ZDvidReader m_dvidReader;
  ZThreadFutureMap m_futureMap;
};

#endif // ZDVIDSPARSESTACK_H
