#ifndef ZDVIDSPARSESTACK_H
#define ZDVIDSPARSESTACK_H

#include <QMutex>
#include <QMap>

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

  bool stackDownsampleRequired();

  const ZDvidTarget& getDvidTarget() const {
    return m_dvidTarget;
  }


  int getValue(int x, int y, int z) const;

  const ZIntPoint& getDownsampleInterval() const;

  void setDvidTarget(const ZDvidTarget &target);

  ZIntCuboid getBoundBox() const;
//  using ZStackObject::getBoundBox; // fix warning -Woverloaded-virtual

  void loadBody(uint64_t bodyId, bool canonizing = false);
  void loadBodyAsync(uint64_t bodyId);
  void setMaskColor(const QColor &color);
  void setLabel(uint64_t bodyId);

  uint64_t getLabel() const;

//  const ZObject3dScan *getObjectMask() const;
  ZObject3dScan *getObjectMask();
  ZStackBlockGrid* getStackGrid();

  const ZSparseStack* getSparseStack() const;
  ZSparseStack *getSparseStack();

  ZSparseStack *getSparseStack(const ZIntCuboid &box);

//  void downloadBodyMask(ZDvidReader &reader);

  bool hit(double x, double y, double z);
  bool hit(double x, double y, NeuTube::EAxis axis);

  bool isEmpty() const;

  ZDvidSparseStack* getCrop(const ZIntCuboid &box) const;

  void deprecateStackBuffer();

  int getReadStatusCode() const;

  void runFillValueFunc();
  void runFillValueFunc(const ZIntCuboid &box, bool syncing);

  void setCancelFillValue(bool flag);
  void cancelFillValueSync();
//  void cancelFillValueFunc();

private:
  void init();
  void initBlockGrid();
  bool fillValue(bool cancelable = false);
  bool fillValue(const ZIntCuboid &box, bool cancelable = false);
  bool fillValue(const ZIntCuboid &box, bool cancelable, bool fillingAll);
  QString getLoadBodyThreadId() const;
  QString getFillValueThreadId() const;
  void pushMaskColor();
  void pushLabel();
  bool loadingObjectMask() const;
  void finishObjectMaskLoading();
  void syncObjectMask();
  void pushAttribute();

  ZDvidReader& getMaskReader() const;
  ZDvidReader& getGrayscaleReader() const;

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
  mutable ZDvidReader m_grayScaleReader;
  mutable ZDvidReader m_maskReader;
  ZThreadFutureMap m_futureMap;
  bool m_cancelingValueFill;

  mutable QMutex m_fillValueMutex;
};

#endif // ZDVIDSPARSESTACK_H
