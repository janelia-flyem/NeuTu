#ifndef ZSTACKWATERSHEDCONTAINER_H
#define ZSTACKWATERSHEDCONTAINER_H

#include <utility>
#include <vector>

#include "tz_stack_watershed.h"
#include "zintcuboid.h"
#include "zstackptr.h"
#include "zstackarray.h"
#include "zsegmentationscan.h"
#include <QString>

class ZStack;
class ZObject3dScan;
class ZStroke2d;
class ZObject3d;
class ZSparseStack;
class ZObject3dScanArray;
class ZSwcTree;
class ZStackPtr;
class ZStackObject;

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
 *
 * Note that the class is not designed for reuse, i.e. each container object
 * can only be used for one stack data.
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

  enum ERangeOption {
    RANGE_FULL, //Full range
    RANGE_SEED_ROI, //ROI expanded from seeds
    RANGE_SEED_BOUND //Exact seed bound
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
  void consumeSeed(const ZObject3d *seed);

  void addSeed(const ZStackObject *seed);

  void setRange(const ZIntCuboid &range);
  void setRange(const ZIntPoint &firstCorner, const ZIntPoint &lastCorner);
  void setRange(int x0, int y0, int z0, int x1, int y1, int z1);

  void exportMask(const std::string &filePath);
  void exportSource(const std::string &filePath);

  ZIntCuboid getDataRange() const;

  void setFloodFillingZero(bool on) {
    m_floodingZero = on;
  }

  void setScale(int scale){
      m_scale=scale;
  }

  void setAlgorithm(const QString &algorithm){
    m_algorithm=algorithm;
  }

  void setDsMethod(const QString & method){
    m_dsMethod=method;
  }

  ZStackPtr getResultStack() const {
    return m_result.front();
  }

  ZStackArray getResult() const {
    return m_result;
  }


  bool hasResult() const;

  void setRangeOption(ERangeOption option);
  ERangeOption getRangeOption() const;
//  void useSeedRange(bool on);
  bool usingSeedRange() const;
//  void expandRange(const ZIntCuboid &box);


  ZObject3dScanArray* makeSplitResult(
      uint64_t minLabel, ZObject3dScanArray *result);

  ZSegmentationScanArray* makeSplitResult(uint64_t minLabel);
  /*!
   * \brief Check if the actual computation is done in the downsampled space.
   */
  bool computationDowsampled();

  void printState() const;

  ZIntCuboid& getRange();

  void setCcaPost(bool on) {
    m_ccaPost = on;
  }
  bool ccaPost() const;

  const std::vector<ZObject3d*>& getSeedArray() const {
    return m_seedArray;
  }

  void setRefiningBorder(bool on) {
    m_refiningBorder = on;
  }

  void setMaxVolume(size_t v) {
    m_maxStackVolume = v;
  }

  static std::vector<ZObject3d*> MakeBorderSeed(const ZStack &stack);

  void test();

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
  ZIntPoint estimateDsIntv(const ZIntCuboid &box) const;
  void expandSeedArray(ZObject3d *obj);
  void expandSeedArray(const std::vector<ZObject3d*> &objArray);

  ZIntPoint getSourceOffset() const;

  Stack* getSeedMask();

  void prepareSeedMask(Stack *stack, Stack *mask);

  void makeMaskStack(ZStack &stack);

  ZIntPoint getSourceDsIntv();

  void updateRange();
  ZIntCuboid getRangeUpdate() const;
  ZIntCuboid getRangeUpdate(const ZIntCuboid &dataRange) const;
  void updateSeedMask();

  ZObject3dScan* processSplitResult(
      const ZObject3dScan &obj, ZObject3dScan *remainBody, bool adpoting);
  void assignComponent(
      ZObject3dScan &remainBody, ZObject3dScan &mainBody,
      ZObject3dScanArray *result);
  void configResult(ZObject3dScanArray *result);

  static ZStackPtr MakeBoundaryStack(
      const ZStack &stack, int conn, ZIntCuboid &boundaryBox);
  static ZIntCuboid GetSeedRange(const std::vector<ZObject3d*> &seedArray);
  static std::vector<ZObject3d*> MakeBorderSeed(
      const ZStack &stack, const ZStack &boundaryStack, const ZIntCuboid &range);

  template<class T>
  bool addSeed(const ZStackObject *obj);

private:
  ZStack *m_stack = NULL;
  ZSparseStack *m_spStack = NULL;
  ZStackArray m_result;
  Stack_Watershed_Workspace *m_workspace = NULL;
//  ZIntPoint m_sourceOffset;
  ZIntCuboid m_range;
  ZStack *m_source = NULL; //Source stack to refer to data in m_stack or m_spStack
  std::vector<ZObject3d*> m_seedArray;

  bool m_floodingZero;
  int m_channel;
//  bool m_usingSeedRange = false;
  ERangeOption m_rangeOption = RANGE_FULL;
  bool m_refiningBorder = true;
  bool m_ccaPost = true; //connected component analysis as post-processing
  int m_scale;
  size_t m_minIsolationSize = 50;
  size_t m_maxStackVolume = neutube::HALFGIGA;
  QString m_algorithm;
  QString m_dsMethod;
};

#endif // ZSTACKWATERSHEDCONTAINER_H
