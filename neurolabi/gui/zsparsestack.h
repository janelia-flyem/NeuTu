#ifndef ZSPARSESTACK_H
#define ZSPARSESTACK_H

#include <iostream>

#include "bigdata/zstackblockgrid.h"
#include "zuncopyable.h"
#include "zstackblocksource.h"

class ZIntCuboid;
class ZObject3dScan;

/*!
 * \brief Sparse stack representation
 */
class ZSparseStack : public ZUncopyable
{
public:
  ZSparseStack();
  ~ZSparseStack();

  enum EComponent {
    STACK, GREY_SCALE, OBJECT_MASK, BLOCK_MASK, ALL_COMPONET
  };

  void deprecateDependent(EComponent component);
  void deprecate(EComponent component);
  bool isDeprecated(EComponent component) const;

  ZIntPoint getDsIntv() const {
    return m_dsIntv;
  }

  void setDsIntv(int x, int y, int z) {
    m_dsIntv.set(x, y, z);
  }

  void setDsIntv(const ZIntPoint &dsIntv) {
    m_dsIntv = dsIntv;
  }

  void pushDsIntv(int x, int y, int z);
  void pushDsIntv(const ZIntPoint &dsIntv);

  /*!
   * \brief Get the dense stack representation of the sparse stack.
   *
   * \return Stack data pointer, which is owned by the sparse stack object.
   */
  ZStack* getStack();
  const ZStack* getStack() const;

  ZStack *makeStack(
      const ZIntCuboid &box, bool preservingGap);
  ZStack *makeStack(const ZIntCuboid &box, size_t maxVolume, bool preservingGap);

//  Stack* makeRawStack(const ZIntCuboid &box);

  ZStack *makeIsoDsStack(size_t maxVolume, bool preservingGap);
  ZStack* makeDsStack(int xintv, int yintv, int zintv, bool preservingGap);

  static bool DownsampleRequired(const ZIntCuboid &box);
  static size_t GetMaxStackVolume();
  bool downsampleRequired() const;

  /*!
   * \brief Get the downsample interval for producing the dense stack.
   */
  ZIntPoint getDenseDsIntv() const;

  /*!
   * \brief Get a slice of the sparse stack
   *
   * The caller is responsible for deleting the returned pointer.
   */
  ZStack* getSlice(int z) const;

  double getValue(int x,int y,int z) const ;
  void getLineValue(int x,int y,int z,int cnt,double* buffer) const ;

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

  void setBlockMask(ZObject3dScan *obj);

  /*!
   * \brief Set greyscale data of the sparse stack
   */
  void setGreyScale(ZStackBlockGrid *stackGrid);

  void setGreyScale(int zoom, ZStackBlockGrid *stackGrid);

  inline const ZObject3dScan* getObjectMask() const {
    return m_objectMask;
  }

  inline ZObject3dScan* getObjectMask() {
    return m_objectMask;
  }

  ZObject3dScan* getBlockMask();

  const ZStackBlockGrid* getStackGrid() const;
  ZStackBlockGrid* getStackGrid();

  const ZStackBlockGrid* getStackGrid(int zoom) const;
  ZStackBlockGrid* getStackGrid(int zoom);

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

  void clearGrayscale();

  void merge(ZSparseStack &sparseStack);

  void shakeOff();

  bool save(const std::string &filePath) const;
  bool load(const std::string &filePath);

  void read(std::istream &stream);
  void write(std::ostream &stream) const;

  void printInfo() const;

  ZSparseStack* downsample(int xintv, int yintv, int zintv);

  void setBlockFactory(ZStackBlockFactory *factory);
  void setBlockSize(const ZIntPoint size);

  void cacheStackSource(int zoom);
  void cacheStackSource(const ZIntCuboid blockRange, int zoom);

private:
  static void assignStackValue(ZStack *stack, const ZObject3dScan &obj,
                               const ZStackBlockGrid &stackGrid,
                               const int baseValue);

  static void assignStackValue(ZStack *stack, const ZObject3dScan &obj,
                               const ZObject3dScan &border,
                               const ZStackBlockGrid &stackGrid,
                               const int baseValue);
  ZStackBlockGrid* makeDownsampleGrid(int xintv, int yintv, int zintv);
  ZStackBlockGrid* makeDownsampleGrid(
      int xintv, int yintv, int zintv, const ZIntCuboid &range);
  int getOptimalZoom(int xintv, int yintv, int zintv) const;

private:
  ZObject3dScan *m_objectMask = nullptr;
  ZObject3dScan *m_blockMask = nullptr;
  ZStackBlockSource m_stackSource;
//  std::vector<ZStackBlockGrid*> m_stackGrid;
//  ZStackBlockGrid *m_stackGrid;
  ZIntPoint m_dsIntv;

  mutable ZStack *m_stack = nullptr;
//  mutable ZIntPoint m_dsIntv;

  int m_baseValue = 1;
};

#endif // ZSPARSESTACK_H
