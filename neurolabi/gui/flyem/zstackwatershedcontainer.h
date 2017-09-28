#ifndef ZSTACKWATERSHEDCONTAINER_H
#define ZSTACKWATERSHEDCONTAINER_H

#include <utility>
#include <vector>

#include "tz_stack_watershed.h"
#include "zintcuboid.h"

class ZStack;
class ZObject3dScan;
class ZStroke2d;
class ZObject3d;
class ZSparseStack;
class ZObject3dScanArray;
class ZSwcTree;

/*!
 * \brief The wrapper class for running watershed split
 *
 * The class can be used to split a normal or sparse stack. Its usage is simple:
 *
 * ZStackWatershedContainer container(stack);
 * container.addSeed(seed1);
 * container.addSeed(seed2);
 * container.run();
 * ZObject3dScanArray *result = container.makeSplitResult();
 */
class ZStackWatershedContainer
{
public:
  ZStackWatershedContainer(ZStack *stack);
  ZStackWatershedContainer(ZSparseStack *stack);
  /*!
   * \brief A convenient constructor for setting stacks
   *
   * \a spStack will suppress \a stack if \a spStack is not NULL.
   */
  ZStackWatershedContainer(ZStack *stack, ZSparseStack *spStack);
  ZStackWatershedContainer(const std::pair<ZStack*, ZSparseStack*> &data);

  ~ZStackWatershedContainer();

  bool isEmpty() const;

  enum EComponent {
    COMP_SEED_ARRAY, COMP_RANGE, COMP_SOURCE, COMP_WORKSPACE, COMP_RESULT
  };

  bool isDeprecated(EComponent component) const;
  void deprecateDependent(EComponent component);
  void deprecate(EComponent component);

  void run();

  void addSeed(const ZStack &seed);
  void addSeed(const ZObject3dScan &seed);
  void addSeed(const ZStroke2d &seed);
  void addSeed(const ZObject3d &seed);
  void addSeed(const ZSwcTree &seed);

  void setRange(const ZIntCuboid &range);
  void setRange(const ZIntPoint &firstCorner, const ZIntPoint &lastCorner);
  void setRange(int x0, int y0, int z0, int x1, int y1, int z1);

  void exportMask(const std::string &filePath);
  void exportSource(const std::string &filePath);

  void setFloodFillingZero(bool on) {
    m_floodingZero = on;
  }

  ZStack* getResultStack() const {
    return m_result;
  }

  void useSeedRange(bool on);
  bool usingSeedRange() const;
//  void expandRange(const ZIntCuboid &box);

  ZObject3dScanArray* makeSplitResult(ZObject3dScanArray *result = NULL);

  void printState() const;

  ZIntCuboid& getRange();

private:
  void init();
  void init(ZStack *stack, ZSparseStack *spStack);

  Stack_Watershed_Workspace* getWorkspace();
  void clearWorkspace();
  void clearSource();
  void clearResult();
  void clearSeed();
  Stack* getSource();
  ZStack* getSourceStack();
  void expandSeedArray(ZObject3d *obj);
  void expandSeedArray(const std::vector<ZObject3d*> &objArray);

  ZIntPoint getSourceOffset() const;

  Stack* getSeedMask();

  void prepareSeedMask(Stack *stack, Stack *mask);

  void makeMaskStack(ZStack &stack);

  ZIntPoint getSourceDsIntv();

  void updateRange();
  void updateSeedMask();

private:
  ZStack *m_stack;
  ZSparseStack *m_spStack;
  ZStack *m_result;
  Stack_Watershed_Workspace *m_workspace;
//  ZIntPoint m_sourceOffset;
  ZIntCuboid m_range;
  ZStack *m_source; //Source stack to refer to data in m_stack or m_spStack
  std::vector<ZObject3d*> m_seedArray;

  bool m_floodingZero;
  int m_channel;
  bool m_usingSeedRange = false;
};

#endif // ZSTACKWATERSHEDCONTAINER_H
