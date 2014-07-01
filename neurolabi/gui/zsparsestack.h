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
    STACK, ALL_COMPONET
  };

  void deprecateDependent(EComponent component);
  void deprecate(EComponent component);
  bool isDeprecated(EComponent component) const;

  ZStack* getStack();

  /*!
   * \brief Get a slice of the sparse stack
   *
   * The caller is responsible for deleting the returned pointer.
   */
  ZStack* getSlice(int z) const;

  inline void setObjectMask(ZObject3dScan *obj) {
    m_objectMask = obj;
  }

  inline void setGreyScale(ZStackBlockGrid *stackGrid) {
    m_stackGrid = stackGrid;
  }

  size_t getObjectVolume() const;

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
