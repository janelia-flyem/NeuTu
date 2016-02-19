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

  /*!
   * \brief Get the dense stack representation of the sparse stack.
   *
   * \return Stack data pointer, which is owned by the sparse stack object.
   */
  ZStack* getStack();
  const ZStack* getStack() const;

  /*!
   * \brief Get the downsample interval for producing the dense stack.
   */
  inline const ZIntPoint& getDownsampleInterval() const {
    return m_dsIntv;
  }

  /*!
   * \brief Get a slice of the sparse stack
   *
   * The caller is responsible for deleting the returned pointer.
   */
  ZStack* getSlice(int z) const;

  /*!
   * \brief Get the maximum intensity projection of the sparse stack
   *
   * The caller is responsible for deleting the returned pointer.
   */
  ZStack* getMip() const;

  /*!
   * \brief Set the mask of the sparse stack
   */
  void setObjectMask(ZObject3dScan *obj);

  /*!
   * \brief Set greyscale data of the sparse stack
   */
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

  /*!
   * \brief Count foreground voxels.
   */
  size_t getObjectVolume() const;

  /*!
   * \brief Get the bound bound of the sparse stack.
   */
  ZIntCuboid getBoundBox() const;

  /*!
   * \brief Set the base value of the sparse stack
   */
  void setBaseValue(int baseValue);

  inline int getBaseValue() const { return m_baseValue; }

  bool isEmpty() const;

private:
  static void assignStackValue(ZStack *stack, const ZObject3dScan &obj,
                               const ZStackBlockGrid &stackGrid,
                               const int baseValue);

private:
  ZObject3dScan *m_objectMask;
  ZStackBlockGrid *m_stackGrid;

  mutable ZStack *m_stack;
  mutable ZIntPoint m_dsIntv;

  int m_baseValue;
};

#endif // ZSPARSESTACK_H
