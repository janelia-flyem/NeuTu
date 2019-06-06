#ifndef ZDVIDSPARSESTACK_H
#define ZDVIDSPARSESTACK_H

#include <QMutex>
#include <QMap>

#include "zstackobject.h"
#include "zsparsestack.h"
#include "zdvidtarget.h"
#include "zdvidreader.h"
#include "zthreadfuturemap.h"

class ZIntCuboid;

class ZDvidSparseStack : public ZStackObject
{
public:
  ZDvidSparseStack();
  ~ZDvidSparseStack();

  static ZStackObject::EType GetType() {
    return ZStackObject::EType::DVID_SPARSE_STACK;
  }

  void display(ZPainter &painter, int slice, EDisplayStyle option,
               neutu::EAxis sliceAxis) const;

//  const std::string& className() const;

  ZStack *getSlice(int z) const;

  ZStack* getSlice(int z, int x0, int y0, int width, int height) const;

  void setLabelType(neutu::EBodyLabelType type);

  /*!
   * \brief Set the mask of the sparse stack
   */
  void setObjectMask(ZObject3dScan *obj);

  /*!
   * \brief Get the dense representation of the sparse stack
   *
   * \return The returned stack is owned by the object.
   */
  ZStack* getStack();

  /*!
   * \brief Get the dense representation of the sparse stack after
   *        updating a certain region.
   *
   * \param updateBox The box region to update
   * \return The returned stack is owned by the object.
   */
  ZStack* getStack(const ZIntCuboid &updateBox);

  /*!
   * \brief Make a stack from the sparse representation.
   *
   * Other than the behavior of \a getStack, this function makes a new stack and
   * returns it. So the caller is responsible for deleting the returned object.
   * It returns NULL if the range is invalid.
   *
   * \param range Range of the produced stack.
   * \return A new stack in the range.
   */
  ZStack* makeStack(const ZIntCuboid &range, bool preservingBorder);

  ZStack* makeIsoDsStack(size_t maxVolume, bool preservingGap);

  ZStack* makeDsStack(int xintv, int yintv, int zintv);

  bool stackDownsampleRequired();

  const ZDvidTarget& getDvidTarget() const {
    return m_dvidReader.getDvidTarget();
  }

  bool prefetching() const { return m_prefectching; }

  int getValue(int x, int y, int z) const;

  ZIntPoint getDenseDsIntv() const;

  void setDvidTarget(const ZDvidTarget &target);

  ZIntCuboid getBoundBox() const;
//  using ZStackObject::getBoundBox; // fix warning -Woverloaded-virtual

  void loadBody(uint64_t bodyId, bool canonizing = false);
  void loadBodyAsync(uint64_t bodyId);
  void setMaskColor(const QColor &color);
  void setLabel(uint64_t bodyId);

  void loadBody(
      uint64_t bodyId, const ZIntCuboid &range,
      bool canonizing = false);

  uint64_t getLabel() const;
  neutu::EBodyLabelType getLabelType() const;

//  const ZObject3dScan *getObjectMask() const;
  ZObject3dScan *getObjectMask();
  ZStackBlockGrid* getStackGrid();

  const ZSparseStack* getSparseStack() const;
  ZSparseStack *getSparseStack();

  ZSparseStack *getSparseStack(const ZIntCuboid &box);
  const ZSparseStack *getSparseStack(const ZIntCuboid &box) const;

//  void downloadBodyMask(ZDvidReader &reader);

  bool hit(double x, double y, double z);
  bool hit(double x, double y, neutu::EAxis axis);

  bool isEmpty() const;

  ZDvidSparseStack* getCrop(const ZIntCuboid &box) const;

  void deprecateStackBuffer();

  int getReadStatusCode() const;

  void runFillValueFunc();
  void runFillValueFunc(const ZIntCuboid &box, bool syncing, bool cont = true);

  void setCancelFillValue(bool flag);
  void cancelFillValueSync();
//  void cancelFillValueFunc();

  /*!
   * \brief Only keep the largest component.
   */
  void shakeOff();

  bool fillValue(bool cancelable = false);
  bool fillValue(const ZIntCuboid &box, bool cancelable = false);
  bool fillValue(const ZIntCuboid &box, bool cancelable, bool fillingAll);

  bool fillValue(const ZIntCuboid &box, int zoom,
                 bool cancelable, bool fillingAll);

private:
  void init();
  void initBlockGrid();
  QString getLoadBodyThreadId() const;
  QString getFillValueThreadId() const;
  void pushMaskColor();
  void pushLabel();
  bool loadingObjectMask() const;
  void finishObjectMaskLoading();
  void syncObjectMask();
  void pushAttribute();

  bool isValueFilled(int zoom) const;
  void setValueFilled(int zoom, bool state);

  ZDvidReader& getMaskReader() const;
  ZDvidReader& getGrayscaleReader() const;

  /*
  void assignStackValue(ZStack *stack, const ZObject3dScan &obj,
                               const ZStackBlockGrid &stackGrid);
                               */

private:
  ZSparseStack m_sparseStack;
  ZDvidTarget m_dvidTarget;
  std::vector<bool> m_isValueFilled;
//  bool m_isValueFilled;
  bool m_prefectching = false;
  uint64_t m_label;
  neutu::EBodyLabelType m_labelType = neutu::EBodyLabelType::BODY;
  mutable ZDvidReader m_dvidReader;
  mutable ZDvidReader m_grayScaleReader;
  mutable ZDvidReader m_maskReader;
  mutable ZDvidInfo m_grayscaleInfo;

  ZThreadFutureMap m_futureMap;
  bool m_cancelingValueFill;

  mutable QMutex m_fillValueMutex;
};

#endif // ZDVIDSPARSESTACK_H
