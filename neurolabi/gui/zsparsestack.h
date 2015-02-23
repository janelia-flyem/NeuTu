#ifndef ZSPARSESTACK_H
#define ZSPARSESTACK_H

#include "zobject3dscan.h"
#include "bigdata/zstackblockgrid.h"
#include "zuncopyable.h"

/*!
 * \brief Sparse stack representation
 */
class ZSparseStack : public ZUncopyable
{
public:
  ZSparseStack();
  ~ZSparseStack();

  enum EComponent {
    STACK, GREY_SCALE, OBJECT_MASK, ALL_COMPONET
  };

  void deprecateDependent(EComponent component);
  void deprecate(EComponent component);
  bool isDeprecated(EComponent component) const;

  ZStack* getStack();
  const ZStack* getStack() const;

  inline const ZIntPoint& getDownsampleInterval() const {
    return m_dsIntv;
  }


  //ZStack* toDownsampledStack(int xIntv, int yIntv, int zIntv);

  /*!
   * \brief Get a slice of the sparse stack
   *
   * The caller is responsible for deleting the returned pointer.
   */
  ZStack* getSlice(int z) const;

  ZStack* getMip() const;

  void setObjectMask(ZObject3dScan *obj);
  void setGreyScale(ZStackBlockGrid *stackGrid);

  inline const ZObject3dScan* getObjectMask() const {
    return m_objectMask;
  }

  inline ZObject3dScan* getObjectMask() {
    return m_objectMask;
  }

  inline const ZStackBlockGrid* getStackGrid() const {
    return m_stackGrid;
  }

  inline ZStackBlockGrid* getStackGrid() {
    return m_stackGrid;
  }

  size_t getObjectVolume() const;

  /*!
   * \brief Get the bound bound of the sparse stack.
   */
  ZIntCuboid getBoundBox() const;

private:
  static void assignStackValue(ZStack *stack, const ZObject3dScan &obj,
                               const ZStackBlockGrid &stackGrid);

private:
  ZObject3dScan *m_objectMask;
  ZStackBlockGrid *m_stackGrid;

  mutable ZStack *m_stack;
  mutable ZIntPoint m_dsIntv;
};

#endif // ZSPARSESTACK_H
