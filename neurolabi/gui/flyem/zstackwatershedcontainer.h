#ifndef ZSTACKWATERSHEDCONTAINER_H
#define ZSTACKWATERSHEDCONTAINER_H

#include <utility>
#include <vector>
#include <functional>
#include <iostream>

#include "tz_stack_watershed.h"
#include "geometry/zintcuboid.h"
#include "zstackptr.h"
#include "zstackarray.h"
#include "zsegmentationscan.h"

class ZStack;
class ZObject3dScan;
class ZStroke2d;
class ZObject3d;
class ZSparseStack;
class ZObject3dScanArray;
class ZSwcTree;
class ZStackPtr;
class ZStackObject;
class ZGraphPtr;

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
 * can only be associated with a signal stack once.
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

  enum EComponent {
    COMP_SEED_ARRAY, COMP_RANGE, COMP_SOURCE, COMP_WORKSPACE, COMP_RESULT
  };

  enum ERangeOption {
    RANGE_FULL, //Full range
    RANGE_SEED_ROI, //ROI expanded from seeds
    RANGE_SEED_BOUND //Exact seed bound
  };


  //For late binding only
  void setData(ZStack *stack, ZSparseStack *spStack);

  /*!
   * \brief Returns true iff there is no associated stack.
   */
  bool isEmpty() const;

  bool isDeprecated(EComponent component) const;
  void deprecateDependent(EComponent component);
  void deprecate(EComponent component);

  void run();

  /*!
   * \brief Add a seed.
   *
   * It does not take the ownership of \a seed.
   */
  void addSeed(const ZStack &seed);
  void addSeed(const ZObject3dScan &seed);
  void addSeed(const ZStroke2d &seed);
  void addSeed(const ZObject3d &seed);
  void addSeed(const ZSwcTree &seed);

  void addSeed(const ZStackObject *seed);
  void addSeed(const std::vector<ZObject3d*> &seed);


  /*!
   * \brief Consume a seed.
   * It takes the ownership of \a seed.
   */
  void consumeSeed(const ZObject3d *seed);

  /*!
   * \brief Set the explicit range.
   *
   * The watershed computation will be constrained in \a range when it is not
   * empty.
   *
   * \param range Range of running watershed.
   */
  void setRange(const ZIntCuboid &range);
  void setRange(const ZIntPoint &firstCorner, const ZIntPoint &lastCorner);
  void setRange(int x0, int y0, int z0, int x1, int y1, int z1);

  /*!
   * \brief Get the range of watershed computation.
   */
  ZIntCuboid& getRange();

  /*!
   * \brief Set the hint of watershed range.
   *
   * The option takes effect only when the explicit range is empty.
   */
  void setRangeHint(ERangeOption option);
  ERangeOption getRangeHint() const;
  bool usingSeedRange() const;

  /*!
   * \brief Get the bound box of the associated stack.
   */
  ZIntCuboid getDataRange() const;

  void exportMask(const std::string &filePath);
  void exportSource(const std::string &filePath);

  void setFloodFillingZero(bool on) {
    m_floodingZero = on;
  }

  void setScale(int scale){
      m_scale=scale;
  }

  void setAlgorithm(const std::string &algorithm) {
    m_algorithm=algorithm;
  }

  void setDsMethod(const std::string & method) {
    m_dsMethod=method;
  }

  void printState() const;

  ZStackArray getResult() const {
    return m_result;
  }

  bool hasResult() const;

  void addResult(const ZStackArray &result);

//  ZSegmentationScanArray* makeSplitResult(uint64_t minLabel);
  /*!
   * \brief Get the first result stack.
   *
   * Obsolete function. To be removed.
   */
  ZStackPtr getResultStack() const {
    if (m_result.empty()) {
      return ZStackPtr();
    }

    return m_result.front();
  }

  ZObject3dScanArray* makeSplitResult(
      uint64_t minLabel, ZObject3dScanArray *result,
      ZObject3dScan *remainBody = NULL);


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

  void setPreservingGap(bool on) {
    m_preservingGap = on;
  }

  void test();
  static bool Test();

  ZStackWatershedContainer* makeSubContainer(
      const std::vector<size_t> &seedIndices, ZStackWatershedContainer *out);
  std::vector<ZStackWatershedContainer*> makeLocalSeedContainer(double maxDist);

  void refineBorder();

  void downsampleSeed(int intvx, int intvy, int intvz);

  /*!
   * \brief Check if the actual computation is done in the downsampled space.
   */
  bool computationDowsampled();

public:
  void setProfileLogger(std::function<void(int64_t, const std::string&)> logger);

private:
  void init();
  void init(ZStack *stack, ZSparseStack *spStack);

  Stack_Watershed_Workspace* getWorkspace();
  void clearWorkspace();
  void clearSource();
  void clearResult();
  void clearSeed();
  Stack* getSource();
  Stack* getRawSourceStack(ZStack* stack);
  ZStack* getSourceStack();
  ZIntPoint estimateDsIntv(const ZIntCuboid &box) const;
  void expandSeedArray(ZObject3d *obj);
  void expandSeedArray(const std::vector<ZObject3d*> &objArray);

  ZIntPoint getSourceOffset() const;

  Stack* getSeedMask();

  void prepareSeedMask(Stack *stack, Stack *mask);

  void makeMaskStack(ZStack &stack);

  ZIntPoint getSourceDsIntv();
  ZIntPoint getOriginalDsIntv();

  void updateRange();
  ZIntCuboid getRangeUpdate() const;
  ZIntCuboid getRangeUpdate(const ZIntCuboid &dataRange) const;
  void updateSeedMask();

  ZObject3dScan* processSplitResult(
      const ZObject3dScan &obj, ZObject3dScan *remainBody, bool adopting);
  void assignComponent(
      ZObject3dScan &remainBody, ZObject3dScan &mainBody,
      ZObject3dScanArray *result);
  void configureResult(ZObject3dScanArray *result);

  static ZStackPtr MakeBoundaryStack(
      const ZStack &stack, int conn, ZIntCuboid &boundaryBox);
  static ZIntCuboid GetSeedRange(const std::vector<ZObject3d*> &seedArray);
  static std::vector<ZObject3d*> MakeBorderSeed(
      const ZStack &stack, const ZStack &boundaryStack, const ZIntCuboid &range);

  static std::vector<ZObject3d*> MakeBorderSeed(const ZStack &stack);

  template<class T>
  bool addSeed(const ZStackObject *obj);

  static size_t ComputeSeedVolume(
      const ZStackObject &obj1, const ZStackObject &obj2);
  static double ComputeSeedDist(
      const ZStackObject &obj1, const ZStackObject &obj2);
  ZGraphPtr buildSeedGraph(double maxDist) const;

  void refineBorder(const ZStackPtr &stack);

  std::string getName() const;
  void setName(const std::string &name);

  void logProfile(int64_t duration, const std::string &info);

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
  bool m_preservingGap = false; //Preserve gaps while downsampling
  bool m_ccaPost = true; //connected component analysis as post-processing
  int m_scale;
  size_t m_minIsolationSize = 50;
  size_t m_maxStackVolume = neutu::HALFGIGA;
  std::string m_algorithm;
  std::string m_dsMethod;

  std::string m_name; //Name for logging purpose

  std::function<void(int64_t, const std::string&)> m_profileLogger =
      [](int64_t duration, const std::string &info) {
    std::cout << info << ": " << duration << "ms" << std::endl; };
};

#endif // ZSTACKWATERSHEDCONTAINER_H
