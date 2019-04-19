#include "zstackwatershedcontainer.h"

#include "imgproc/zstackwatershed.h"
#include "zstack.hxx"
#include "zobject3d.h"
#include "zstroke2d.h"
#include "zsparsestack.h"
#include "zobject3dscanarray.h"
#include "zobject3dfactory.h"
#include "neutubeconfig.h"
#include "zswctree.h"
#include "tz_math.h"
#include "flyem/zflyemmisc.h"
#include "zstackobjectsourcefactory.h"
#include "imgproc/zstackmultiscalewatershed.h"
#include "zstackfactory.h"
#include "misc/miscutility.h"
#include "imgproc/zdownsamplefilter.h"

ZStackWatershedContainer::ZStackWatershedContainer(ZStack *stack)
{
  init();
  m_stack = stack;
  if (m_stack != NULL) {
    m_range = m_stack->getBoundBox();
    m_source = stack;
  }
}

ZStackWatershedContainer::ZStackWatershedContainer(ZSparseStack *stack)
{
  init();
  m_spStack = stack;
  if (m_spStack != NULL) {
    m_range = m_spStack->getBoundBox();
  }
}

ZStackWatershedContainer::ZStackWatershedContainer(
    ZStack *stack, ZSparseStack *spStack)
{
  init(stack, spStack);
}

ZStackWatershedContainer::ZStackWatershedContainer(
    const std::pair<ZStack *, ZSparseStack *> &data)
{
  init(data.first, data.second);
}

void ZStackWatershedContainer::init()
{
//  m_result = NULL;
  m_channel = 0;
  m_floodingZero = false;
//  m_usingSeedRange = false;
  m_scale=1;
}

void ZStackWatershedContainer::init(ZStack *stack, ZSparseStack *spStack)
{
  init();

  if (stack != NULL) {
    if (stack->hasData()) {
      m_stack = stack;
      m_range = m_stack->getBoundBox();
      m_source = stack;
    }
  }

  m_spStack = spStack;
  if (m_spStack != NULL) {
    m_range = m_spStack->getBoundBox();
  }
}

ZStackWatershedContainer::~ZStackWatershedContainer()
{
  clearWorkspace();
  clearSource();
  clearResult();
  clearSeed();
}

ZIntCuboid &ZStackWatershedContainer::getRange()
{
  if (m_range.isEmpty()) {
    updateRange();
  }

  return m_range;
}


void ZStackWatershedContainer::clearSeed()
{
  for (ZObject3d *seed : m_seedArray) {
    delete seed;
  }
  m_seedArray.clear();
}

void ZStackWatershedContainer::clearWorkspace()
{
  if (m_workspace != NULL) {
    Kill_Stack_Watershed_Workspace(m_workspace);
    m_workspace = NULL;
  }
}

void ZStackWatershedContainer::clearSource()
{
  if (m_stack != NULL) {
    if (m_source != m_stack) {
      delete m_source;
    }
    m_source = NULL;
  }
}

void ZStackWatershedContainer::clearResult()
{
  m_result.clear();
}

ZIntPoint ZStackWatershedContainer::getSourceDsIntv()
{
  ZIntPoint dsIntv(0, 0, 0);
  ZStack *source = getSourceStack();
  if (source != NULL) {
    dsIntv = source->getDsIntv();
  }

  return dsIntv;
}

void ZStackWatershedContainer::setRangeOption(ERangeOption option)
{
  if (m_rangeOption != option) {
    deprecate(COMP_RANGE);
    m_rangeOption = option;
  }
}

ZStackWatershedContainer::ERangeOption ZStackWatershedContainer::getRangeOption() const
{
  return m_rangeOption;
}

bool ZStackWatershedContainer::usingSeedRange() const
{
  return m_rangeOption == RANGE_SEED_BOUND || m_rangeOption == RANGE_SEED_ROI;
}

/*
void ZStackWatershedContainer::useSeedRange(bool on)
{
  if (m_usingSeedRange != on) {
    deprecate(COMP_RANGE);
  }
  m_usingSeedRange = on;
}
*/
/*
void ZStackWatershedContainer::expandRange(const ZIntCuboid &box)
{
  getRange().join(box);
  deprecateDependent(COMP_RANGE);
}
*/

void ZStackWatershedContainer::updateSeedMask()
{
  ZStackWatershed::AddSeed(
        getWorkspace(), getSourceOffset(), getSourceDsIntv(), m_seedArray);
}

bool ZStackWatershedContainer::isDeprecated(EComponent component) const
{
  switch (component) {
  case COMP_SEED_ARRAY:
    return m_seedArray.empty();
  case COMP_RANGE:
    return m_range.isEmpty();
  case COMP_SOURCE:
    return m_source == NULL;
  case COMP_WORKSPACE:
    return m_workspace == NULL;
  case COMP_RESULT:
    return m_result.empty();
  }

  return false;
}

void ZStackWatershedContainer::deprecate(EComponent component)
{
  switch (component) {
  case COMP_SEED_ARRAY:
    clearSeed();
    break;
  case COMP_RANGE:
    m_range.reset();
    break;
  case COMP_SOURCE:
    clearSource();
    break;
  case COMP_WORKSPACE:
    clearWorkspace();
    break;
  case COMP_RESULT:
    clearResult();
    break;
  }

  deprecateDependent(component);
}

void ZStackWatershedContainer::deprecateDependent(EComponent component)
{
  switch (component) {
  case COMP_SEED_ARRAY:
    if (usingSeedRange()) {
      deprecate(COMP_RANGE);
    } else {
      deprecate(COMP_WORKSPACE);
    }
    break;
  case COMP_RANGE:
    deprecate(COMP_WORKSPACE);
    deprecate(COMP_SOURCE);
    break;
  case COMP_SOURCE:
    deprecate(COMP_RESULT);
    break;
  case COMP_WORKSPACE:
    deprecate(COMP_RESULT);
    break;
  case COMP_RESULT:
    break;
  }
}

void ZStackWatershedContainer::expandSeedArray(ZObject3d *obj)
{
  if(obj != NULL) {
    m_seedArray.push_back(obj);
    deprecateDependent(COMP_SEED_ARRAY);
  }
}

void ZStackWatershedContainer::expandSeedArray(
    const std::vector<ZObject3d *> &objArray)
{
  for (ZObject3d *obj : objArray) {
    expandSeedArray(obj);
  }
//  m_seedArray.insert(m_seedArray.end(), objArray.begin(), objArray.end());
//  deprecateDependent(COMP_SEED_ARRAY);
}

template <typename T>
bool ZStackWatershedContainer::addSeed(const ZStackObject *seed)
{
  bool added = false;
  if (added == false) {
    const T *obj = dynamic_cast<const T*>(seed);
    if (obj != NULL) {
      addSeed(*obj);
      added = true;
    }
  }

  return added;
}

void ZStackWatershedContainer::addSeed(const ZStackObject *seed)
{
  if (seed != NULL) {
    bool added = false;
    if (added == false) {
      added = addSeed<ZStroke2d>(seed);
    }
    if (added == false) {
      added = addSeed<ZObject3dScan>(seed);
    }
    if (added == false) {
      added = addSeed<ZObject3d>(seed);
    }
    if (added == false) {
      added = addSeed<ZSwcTree>(seed);
    }
  }
}

void ZStackWatershedContainer::addSeed(const ZObject3dScan &seed)
{
  expandSeedArray(seed.toObject3d());

#if 0
  ZIntPoint dsIntv = getSourceDsIntv();

  if (dsIntv.isZero()) {
    ZStackWatershed::AddSeed(getWorkspace(), getSourceOffset(), seed);
  } else {
    ZObject3dScan newSeed = seed;
    newSeed.downsampleMax(dsIntv);
    ZStackWatershed::AddSeed(getWorkspace(), getSourceOffset(), newSeed);
  }

  if (usingSeedRange()) {
    expandRange(seed.getBoundBox());
  }
#endif
}

void ZStackWatershedContainer::addSeed(const ZStack &seed)
{
  std::vector<ZObject3d*> objArray = ZObject3dFactory::MakeObject3dArray(seed);
  expandSeedArray(objArray);
//  m_seedArray.insert(m_seedArray.end(), objArray.begin(), objArray.end());

#if 0
  ZIntPoint dsIntv = getSourceDsIntv();

  if (dsIntv.isZero()) {
    ZStackWatershed::AddSeed(getWorkspace(), getSourceOffset(), &seed);
  } else {
    Stack *block = C_Stack::downsampleMax(
          seed.c_stack(), dsIntv.getX(), dsIntv.getY(), dsIntv.getZ(), NULL);
    ZIntPoint pt = seed.getOffset() - getSourceOffset();
    int x0 = pt.getX() / (dsIntv.getX() + 1);
    int y0 = pt.getY() / (dsIntv.getY() + 1);
    int z0 = pt.getZ() / (dsIntv.getZ() + 1);

    C_Stack::setBlockValue(
          getWorkspace()->mask, block, x0, y0, z0, 0, STACK_WATERSHED_BARRIER);
    C_Stack::kill(block);
  }

  if (usingSeedRange()) {
    expandRange(seed.getBoundBox());
  }
#endif
}

void ZStackWatershedContainer::addSeed(const ZStroke2d &seed)
{
//  m_seedArray.push_back(seed.toObject3d());
  expandSeedArray(seed.toObject3d());
#if 0
  ZStack stack;
  makeMaskStack(stack);
  seed.labelStack(&stack, STACK_WATERSHED_BARRIER);
  if (usingSeedRange()) {
    expandRange(seed.getBoundBox().toIntCuboid());
  }
#endif
}

void ZStackWatershedContainer::addSeed(const ZObject3d &seed)
{
  expandSeedArray(seed.clone());
//  m_seedArray.push_back(seed.clone());
#if 0
  ZStack stack;
  makeMaskStack(stack);
  seed.labelStack(&stack);
  if (usingSeedRange()) {
    expandRange(seed.getBoundBox());
  }
#endif
}

void ZStackWatershedContainer::consumeSeed(const ZObject3d *seed)
{
  if (seed != NULL) {
    expandSeedArray(const_cast<ZObject3d*>(seed));
  }
}

void ZStackWatershedContainer::addSeed(const ZSwcTree &seed)
{
  ZStack *stack = seed.toTypeStack();
  addSeed(*stack);
  delete stack;
#if 0
  ZStack stack;
  makeMaskStack(stack);
  seed.labelStackByType(&stack);
  if (usingSeedRange()) {
    expandRange(seed.getBoundBox().toIntCuboid());
  }
#endif
 //seed.labelStack(&stack);
}

void ZStackWatershedContainer::prepareSeedMask(Stack *stack, Stack *mask)
{
  if (!m_floodingZero) {
    size_t voxelNumber = C_Stack::voxelNumber(stack);

    if (C_Stack::kind(stack) == GREY) {
      uint8_t *array = C_Stack::array8(stack);
      for (size_t i = 0; i < voxelNumber; ++i) {
        if (array[i] == 0) {
          mask->array[i] = STACK_WATERSHED_BARRIER;
        }
      }
    } else {
      uint16_t *array = C_Stack::guardedArray16(stack);
      for (size_t i = 0; i < voxelNumber; ++i) {
        if (array[i] == 0) {
          mask->array[i] = STACK_WATERSHED_BARRIER;
        }
      }
    }
  }
}

Stack* ZStackWatershedContainer::getSeedMask()
{
  Stack *mask = NULL;

  Stack_Watershed_Workspace *ws = getWorkspace();
  if (ws != NULL) {
    if (ws->mask == NULL) {
      ws->mask = C_Stack::make(
            GREY, m_range.getWidth(), m_range.getHeight(), m_range.getDepth());
      C_Stack::setZero(ws->mask);
      if (getSource() != NULL) {
        prepareSeedMask(getSource(), mask);
      }
    }
    mask = ws->mask;
  }

  return mask;
}

void ZStackWatershedContainer::makeMaskStack(ZStack &stack)
{
  stack.load(getSeedMask(), false);
  stack.setOffset(getSourceOffset());
  stack.setDsIntv(getSourceDsIntv());
}

void ZStackWatershedContainer::exportMask(const std::string &filePath)
{
  ZStack stack;
  makeMaskStack(stack);
  if (stack.hasData()) {
    stack.save(filePath);
  }
}

void ZStackWatershedContainer::exportSource(const std::string &filePath)
{
  getSourceStack()->save(filePath);
}

Stack_Watershed_Workspace* ZStackWatershedContainer::getWorkspace()
{
  if (m_workspace == NULL) {
    if (getSource() != NULL) {
      if (!getRange().isEmpty()) {
        m_workspace =
            ZStackWatershed::CreateWorkspace(getSource(), m_floodingZero);
      }
    }
  }

  return m_workspace;
}

ZIntPoint ZStackWatershedContainer::getSourceOffset() const
{
  ZIntPoint offset;
  if (m_source != NULL) {
    offset = m_source->getOffset();
  }

  return offset;
}

void ZStackWatershedContainer::setRange(
    const ZIntPoint &firstCorner, const ZIntPoint &lastCorner)
{
  setRange(firstCorner.getX(), firstCorner.getY(), firstCorner.getZ(),
           lastCorner.getX(), lastCorner.getY(), lastCorner.getZ());
}

void ZStackWatershedContainer::setRange(const ZIntCuboid &range)
{
  setRange(range.getFirstCorner(), range.getLastCorner());
}

void ZStackWatershedContainer::setRange(
    int x0, int y0, int z0, int x1, int y1, int z1)
{
  ZIntCuboid newRange(x0, y0, z0, x1, y1, z1);
  if (m_stack != NULL) {
    newRange.intersect(m_stack->getBoundBox());
  }

  if (!newRange.equals(getRange())) {
    deprecateDependent(COMP_RANGE);
  }
  m_range = newRange;
#if 0
    clearWorkspace();
    m_range = newRange;
    if (m_stack != NULL) {
      ZIntCuboid fullRange = m_stack->getBoundBox();
      if (m_source != m_stack) {
        delete m_source;
        m_source = NULL;
      }
      if (m_range.equals(fullRange)) {
        m_source = m_stack;
      } else {
        ZIntPoint corner = m_range.getFirstCorner() - fullRange.getFirstCorner();

        Stack *rawSource = C_Stack::crop(
              m_stack->c_stack(m_channel),
              corner.getX(), corner.getY(), corner.getZ(),
              m_range.getWidth(), m_range.getHeight(), m_range.getDepth(), NULL);
        m_source->consume(rawSource);
        m_source->setOffset(m_range.getFirstCorner());
      }
    }
  }
#endif
}

ZIntCuboid ZStackWatershedContainer::getDataRange() const
{
  if (m_stack != NULL) {
    return m_stack->getBoundBox();
  } else if (m_spStack != NULL) {
    return m_spStack->getBoundBox();
  }

  return ZIntCuboid();
}

ZIntPoint ZStackWatershedContainer::estimateDsIntv(const ZIntCuboid &box) const
{
  ZIntPoint dsIntv;

  ZIntCuboid range = box;
  range.intersect(getRangeUpdate(box));

  size_t volume = range.getVolume();
  double dsRatio = (double) volume / m_maxStackVolume;

  if (dsRatio > 1.0) {
    dsIntv = misc::getDsIntvFor3DVolume(dsRatio);
  }

  return dsIntv;
}

ZStack* ZStackWatershedContainer::getSourceStack()
{
  ZIntCuboid range = getRange();
  if (m_source == NULL) {
    if (m_spStack != NULL) {
      m_source = m_spStack->makeStack(range, m_maxStackVolume, true, NULL);
    } else {
      if (range.equals(m_stack->getBoundBox())) {
        m_source = m_stack;
      } else {
        ZIntCuboid fullRange = m_stack->getBoundBox();
        ZIntPoint corner = m_range.getFirstCorner() - fullRange.getFirstCorner();

        Stack *rawSource = C_Stack::crop(
              m_stack->c_stack(m_channel),
              corner.getX(), corner.getY(), corner.getZ(),
              m_range.getWidth(), m_range.getHeight(), m_range.getDepth(), NULL);
        m_source = new ZStack();
        m_source->consume(rawSource);
        m_source->setOffset(m_range.getFirstCorner());
      }
    }
  }

  return m_source;
}

Stack* ZStackWatershedContainer::getSource()
{
  return getSourceStack()->c_stack(m_channel);
}

bool ZStackWatershedContainer::hasResult() const
{
  for (const auto &result : m_result) {
    if (result) {
      return true;
    }
  }

  return false;
}

bool ZStackWatershedContainer::isEmpty() const
{
  return (m_stack == NULL) && (m_spStack ==NULL);
}

void ZStackWatershedContainer::run()
{
  std::cout << "Running watershed ..." << std::endl;
  deprecate(COMP_RESULT);

  if(m_stack && m_stack->hasData()){//for normal stack
    ZStackMultiScaleWatershed watershed;
    ZStackPtr stack = ZStackPtr(watershed.run(getSourceStack(),m_seedArray,m_scale,m_algorithm,m_dsMethod));
    stack->setOffset(getSourceOffset());
    m_result.push_back(stack);
  }
  else if (m_spStack){//for sparse stack
    if(m_scale!=1){
      ZDownsampleFilter* filter=ZDownsampleFilter::create(m_dsMethod);
      filter->setDsFactor(m_scale,m_scale,m_scale);
      m_source=filter->filterStack(*m_spStack);
      delete filter;
    }
    Stack *source = getSource();
    if (source != NULL){
      updateSeedMask();
      getWorkspace()->conn=6;
      Stack *out = C_Stack::watershed(source, getWorkspace());
      ZStackPtr stack = ZStackPtr::Make();
      stack->consume(out);
      stack->setOffset(getSourceOffset());
      stack->setDsIntv(getSourceStack()->getDsIntv());
      m_result.push_back(stack);

      std::cout << "Downsampling interval: "
                    << getSourceStack()->getDsIntv().toString() << std::endl;

      if (m_refiningBorder && !getSourceStack()->getDsIntv().isZero()) {
          ZIntCuboid dataRange = getRange();

          ZIntCuboid boundaryBox;
          ZStackPtr boundaryStack = MakeBoundaryStack(*stack, 26, boundaryBox);

          //Extract components from the boundary stack
          ZObject3dScan boundaryObject;
          boundaryObject.loadStack(boundaryStack->c_stack());
  //            ZObject3dFactory::MakeObject3dScan(*boundaryStack);
          std::vector<ZObject3dScan> boundaryArray =
              boundaryObject.getConnectedComponent(ZObject3dScan::ACTION_NONE);

  #if 0
          boundaryStack->save(GET_TEST_DATA_DIR + "/test.tif");
  #endif
          //For each component
          for (const ZObject3dScan &subbound : boundaryArray) {
            //  Compute split
            ZStackWatershedContainer container(m_stack, m_spStack);
  //          container.useSeedRange(true);
            container.setRangeOption(RANGE_SEED_BOUND);
            container.setRefiningBorder(false);

            std::vector<ZObject3d*> newSeeds = MakeBorderSeed(
                  *stack, *boundaryStack, subbound.getBoundBox());
  //          std::vector<ZObject3d*> newSeeds = MakeBorderSeed(*stack);
            for (ZObject3d *seed : newSeeds) {
              container.consumeSeed(seed);
            }
            ZIntPoint dsIntv = container.estimateDsIntv(dataRange);
            if (dsIntv.definiteLessThan(getSourceStack()->getDsIntv())) {
              container.run();
              ZStackArray newResult = container.getResult();
              m_result.append(newResult);
          }
        }
      }
    }
  }
}

bool ZStackWatershedContainer::computationDowsampled()
{
  return !getSourceDsIntv().isZero();
}

/*!
 * Return: 0: failed; 1: no border found; 2: border found
 */
inline static int BorderSeedTest(
    int nx, int ny, int nz, int width, int height, int depth, uint8_t label,
    const uint8_t *labelArray, const uint8_t *boundaryArray, int currentState)
{
  int state = currentState;

  if (state > 0) {
    ssize_t nbrIndex = C_Stack::indexFromCoord(nx, ny, nz, width, height, depth);
    if (nbrIndex >= 0) {
      uint8_t nbrLabel = labelArray[nbrIndex];
      if (nbrLabel > 0) {
        if (boundaryArray[nbrIndex] == label) {
          state = 2;
        } else if (nbrLabel != label) {
          state = 0;
        }
      }
    }
  }

  return state;
}

ZIntCuboid ZStackWatershedContainer::GetSeedRange(
    const std::vector<ZObject3d *> &seedArray)
{
  ZIntCuboid box;
  for (ZObject3d *obj : seedArray) {
    box.join(obj->getBoundBox());
  }

  return box;
}

ZStackPtr ZStackWatershedContainer::MakeBoundaryStack(
    const ZStack &stack, int conn, ZIntCuboid &boundaryBox)
{
  int width = stack.width();
  int height = stack.height();
  int depth = stack.depth();

  //Label boundary
  ZStackPtr boundaryStack(
        ZStackFactory::MakeZeroStack(GREY, stack.getBoundBox()));
  int neighbor[26];
  int isInBound[26];
  C_Stack::neighborOffset(conn, width, height, neighbor);

  const uint8_t *array = stack.array8();
  size_t volume = stack.getVoxelNumber();
  uint8_t *boundaryArray = boundaryStack->array8();

  for (size_t currentIndex = 0; currentIndex < volume; ++currentIndex) {
    uint8_t currentLabel = array[currentIndex];
    if (currentLabel > 0) {
      int nInBound = C_Stack::neighborTest(
            conn, width, height, depth, currentIndex, isInBound);
      for (int n = 0; n < conn; ++n) {
        if (isInBound[n] || nInBound == conn) { //The nth neighbor is in bound
          size_t nbrIndex = currentIndex + neighbor[n];
          size_t nbrLabel = array[nbrIndex];
          if (nbrLabel > 0 && nbrLabel != currentLabel) { //boundary voxel
            boundaryArray[currentIndex] = currentLabel;
            int x = 0;
            int y = 0;
            int z = 0;
            C_Stack::indexToCoord(currentIndex, width, height, &x, &y, &z);
            boundaryBox.join(x, y, z);
            break;
          }
        }
      }
    }
  }

  return boundaryStack;
}

std::vector<ZObject3d*> ZStackWatershedContainer::MakeBorderSeed(
      const ZStack &stack, const ZStack &boundaryStack, const ZIntCuboid &range)
{
  std::vector<ZObject3d*> result;

  if (!stack.getDsIntv().isZero()) {
    int sx = stack.getDsIntv().getX() + 1;
    int sy = stack.getDsIntv().getY() + 1;
    int sz = stack.getDsIntv().getZ() + 1;
    int width = stack.width();
    int height = stack.height();
    int depth = stack.depth();

    ZIntCuboid boundaryBox = range;
    const uint8_t *boundaryArray = boundaryStack.array8();
    const uint8_t *array = stack.array8();

    boundaryBox.scale(ZIntPoint(sx, sy, sz));
    boundaryBox.expand(1, 1, 1);
    boundaryBox.intersect(
          ZIntCuboid(0, 0, 0, width * sx - 1, height * sy - 1, depth * sz - 1));

    int bx0 = boundaryBox.getFirstCorner().getX();
    int by0 = boundaryBox.getFirstCorner().getY();
    int bz0 = boundaryBox.getFirstCorner().getZ();

    int bx1 = boundaryBox.getLastCorner().getX();
    int by1 = boundaryBox.getLastCorner().getY();
    int bz1 = boundaryBox.getLastCorner().getZ();

    int x0 = stack.getOffset().getX() * sx;
    int y0 = stack.getOffset().getY() * sy;
    int z0 = stack.getOffset().getZ() * sz;


    //For each voxel in the boundary box
    for (int z = bz0; z <= bz1; ++z) {
      for (int y = by0; y <= by1; ++y) {
        for (int x = bx0; x <= bx1; ++x) {
#ifdef _DEBUG_2
          std::cout << "Checking " << x << " " << y << " " << z << std::endl;
#endif
          ssize_t index = C_Stack::indexFromCoord(
                x / sx, y / sy, z / sz, width, height, depth);
          uint8_t label = array[index];
          if (label > 0 && boundaryArray[index] == 0) {
            int state = 1;
            state = BorderSeedTest((x - 1) / sx, y / sy, z / sz,
                                   width, height, depth, label,
                                   array, boundaryArray, state);
            state = BorderSeedTest((x + 1) / sx, y / sy, z / sz,
                                   width, height, depth, label,
                                   array, boundaryArray, state);
            state = BorderSeedTest(x / sx, (y - 1) / sy, z / sz,
                                   width, height, depth, label,
                                   array, boundaryArray, state);
            state = BorderSeedTest(x / sx, (y + 1) / sy, z / sz,
                                   width, height, depth, label,
                                   array, boundaryArray, state);
            state = BorderSeedTest(x / sx, y / sy, (z - 1) / sz,
                                   width, height, depth, label,
                                   array, boundaryArray, state);
            state = BorderSeedTest(x / sx, y / sy, (z + 1) / sz,
                                   width, height, depth, label,
                                   array, boundaryArray, state);
            if (state == 2) {
              if (result.size() <= label) {
                result.resize(label + 1, NULL);
              }
              if (result[label] == NULL) {
                result[label] = new ZObject3d;
                result[label]->setLabel(label);
              }
              ZObject3d *obj = result[label];
              obj->append(x + x0, y + y0, z + z0);
            }
          }
        } //x loop
      }
    } //z loop
  }

  return result;
}

std::vector<ZObject3d*> ZStackWatershedContainer::MakeBorderSeed(
    const ZStack &stack)
{
  std::vector<ZObject3d*> result;

  if (!stack.getDsIntv().isZero()) {
    ZIntCuboid boundaryBox;
    ZStackPtr boundaryStack = MakeBoundaryStack(stack, 26, boundaryBox);
    result = MakeBorderSeed(stack, *boundaryStack, boundaryBox);
#if 0
    int sx = stack.getDsIntv().getX() + 1;
    int sy = stack.getDsIntv().getY() + 1;
    int sz = stack.getDsIntv().getZ() + 1;
    int width = stack.width();
    int height = stack.height();
    int depth = stack.depth();

    //Label boundary
    ZIntCuboid boundaryBox;
    ZStackPtr boundaryStack = MakeBoundaryStack(stack, 26, boundaryBox);
    uint8_t *boundaryArray = boundaryStack->array8();
    const uint8_t *array = stack.array8();

    boundaryBox.scale(ZIntPoint(sx, sy, sz));
    boundaryBox.expand(1, 1, 1);
    boundaryBox.intersect(
          ZIntCuboid(0, 0, 0, width * sx - 1, height * sy - 1, depth * sz - 1));

    int bx0 = boundaryBox.getFirstCorner().getX();
    int by0 = boundaryBox.getFirstCorner().getY();
    int bz0 = boundaryBox.getFirstCorner().getZ();

    int bx1 = boundaryBox.getLastCorner().getX();
    int by1 = boundaryBox.getLastCorner().getY();
    int bz1 = boundaryBox.getLastCorner().getZ();

    int x0 = stack.getOffset().getX() * sx;
    int y0 = stack.getOffset().getY() * sy;
    int z0 = stack.getOffset().getZ() * sz;


    //For each voxel in the boundary box
    for (int z = bz0; z <= bz1; ++z) {
      for (int y = by0; y <= by1; ++y) {
        for (int x = bx0; x <= bx1; ++x) {
#ifdef _DEBUG_2
          std::cout << "Checking " << x << " " << y << " " << z << std::endl;
#endif
          ssize_t index = C_Stack::indexFromCoord(
                x / sx, y / sy, z / sz, width, height, depth);
          uint8_t label = array[index];
          if (label > 0 && boundaryArray[index] == 0) {
            int state = 1;
            state = BorderSeedTest((x - 1) / sx, y / sy, z / sz,
                                   width, height, depth, label,
                                   array, boundaryArray, state);
            state = BorderSeedTest((x + 1) / sx, y / sy, z / sz,
                                   width, height, depth, label,
                                   array, boundaryArray, state);
            state = BorderSeedTest(x / sx, (y - 1) / sy, z / sz,
                                   width, height, depth, label,
                                   array, boundaryArray, state);
            state = BorderSeedTest(x / sx, (y + 1) / sy, z / sz,
                                   width, height, depth, label,
                                   array, boundaryArray, state);
            state = BorderSeedTest(x / sx, y / sy, (z - 1) / sz,
                                   width, height, depth, label,
                                   array, boundaryArray, state);
            state = BorderSeedTest(x / sx, y / sy, (z + 1) / sz,
                                   width, height, depth, label,
                                   array, boundaryArray, state);
            if (state == 2) {
              if (result.size() <= label) {
                result.resize(label + 1, NULL);
              }
              if (result[label] == NULL) {
                result[label] = new ZObject3d;
                result[label]->setLabel(label);
              }
              ZObject3d *obj = result[label];
              obj->append(x + x0, y + y0, z + z0);
            }
          }
        } //x loop
      }
    }
#endif
  }

  return result;
}

bool ZStackWatershedContainer::ccaPost() const
{
  if (m_range.contains(getDataRange())) {
    return false;
  }

  return m_ccaPost;
}

void ZStackWatershedContainer::test()
{
#if 1
  m_source = m_spStack->makeIsoDsStack(100 * 100 * 100, true);

  std::cout << "Ds intv: " << m_source->getDsIntv().toString() << std::endl;

  run();
#endif

#if 0
  ZStack *stack = ZStackFactory::MakeZeroStack(4, 4, 3);
  stack->setIntValue(0, 0, 0, 0, 3);
  stack->setIntValue(1, 1, 0, 0, 1);
  stack->setIntValue(1, 2, 0, 0, 2);
  stack->setIntValue(0, 2, 0, 0, 2);
  stack->setIntValue(2, 2, 0, 0, 2);
  stack->setIntValue(1, 3, 0, 0, 2);
  stack->setIntValue(0, 3, 0, 0, 2);
  stack->setIntValue(2, 3, 0, 0, 2);
  stack->setIntValue(2, 3, 1, 0, 2);
  stack->setIntValue(2, 2, 1, 0, 2);
  C_Stack::printValue(stack->c_stack());

  ZIntCuboid box;
  ZStackPtr boundaryStack = MakeBoundaryStack(*stack, 26, box);
  C_Stack::printValue(boundaryStack->c_stack());
  std::cout << box.toString() << std::endl;
#endif

#if 0
  ZStack *stack = ZStackFactory::MakeZeroStack(4, 4, 1);
  stack->setIntValue(0, 0, 0, 0, 1);
  stack->setIntValue(1, 0, 0, 0, 1);
  stack->setIntValue(0, 1, 0, 0, 1);
  stack->setIntValue(1, 1, 0, 0, 1);

  stack->setIntValue(2, 0, 0, 0, 2);
  stack->setIntValue(3, 0, 0, 0, 2);
  stack->setIntValue(2, 1, 0, 0, 2);
  stack->setIntValue(3, 1, 0, 0, 2);

  stack->setIntValue(0, 2, 0, 0, 3);
  stack->setIntValue(1, 2, 0, 0, 3);
  stack->setIntValue(0, 3, 0, 0, 3);
  stack->setIntValue(1, 3, 0, 0, 3);

  stack->setIntValue(2, 2, 0, 0, 4);
  stack->setIntValue(3, 2, 0, 0, 4);
  stack->setIntValue(2, 3, 0, 0, 4);
  stack->setIntValue(3, 3, 0, 0, 4);

  stack->setDsIntv(1, 1, 1);

  C_Stack::printValue(stack->c_stack());


  std::vector<ZObject3d*> objArray = MakeBorderSeed(*stack);
  for (ZObject3d *obj : objArray) {
    if (obj != NULL) {
      obj->print();
    }
  }
#endif
}

ZObject3dScan* ZStackWatershedContainer::processSplitResult(
    const ZObject3dScan &obj, ZObject3dScan *remainBody, bool adpoting)
{
  ZObject3dScan *currentBody = new ZObject3dScan;
  *currentBody = remainBody->subtract(obj);
  uint64_t splitLabel = obj.getLabel();
  currentBody->setLabel(splitLabel);

  if (adpoting) {
    std::vector<ZObject3dScan> ccArray =
        currentBody->getConnectedComponent(ZObject3dScan::ACTION_NONE);
    for (std::vector<ZObject3dScan>::iterator iter = ccArray.begin();
         iter != ccArray.end(); ++iter) {
      ZObject3dScan &subobj = *iter;
      bool isAdopted = false;
      if (subobj.getVoxelNumber() < m_minIsolationSize &&
          currentBody->getVoxelNumber() / subobj.getVoxelNumber() > 10) {
        if (remainBody->isAdjacentTo(subobj)) {
          remainBody->concat(subobj);
          isAdopted = true;
        }
      }

      if (!isAdopted) {//Treated as a split region
        currentBody->concat(subobj);
      }
    }
  }

  return currentBody;
}

void ZStackWatershedContainer::assignComponent(
    ZObject3dScan &remainBody, ZObject3dScan &mainBody,
    ZObject3dScanArray *result)
{
  std::vector<ZObject3dScan> partArray =
      remainBody.getConnectedComponent(ZObject3dScan::ACTION_NONE);

  for (std::vector<ZObject3dScan>::iterator iter = partArray.begin();
       iter != partArray.end(); ++iter) {
    ZObject3dScan &obj = *iter;

    //Check single connect for bound box split
    //Any component only connected to one split part?
    int count = 0;
    int splitIndex = 0;

    if (!obj.isAdjacentTo(mainBody)) {
      for (size_t index = 0; index < result->size(); ++index) {
        ZObject3dScan *resultObj = (*result)[index];
        if (obj.isAdjacentTo(*resultObj)) {
          ++count;
          if (count > 1) {
            break;
          }
          splitIndex = index;
        }
      }
    }

    if (count > 0) {
      ZObject3dScan *split = (*result)[splitIndex];
      split->concat(obj);
    }
  }
}

void ZStackWatershedContainer::configResult(ZObject3dScanArray *result)
{
  if (result != NULL) {
    for (ZObject3dScanArray::iterator iter = result->begin();
         iter != result->end(); ++iter) {
      ZObject3dScan *obj = *iter;
      obj->setColor(ZStroke2d::GetLabelColor(obj->getLabel()));
      obj->setObjectClass(ZStackObjectSourceFactory::MakeSplitResultSource());
      obj->setSource(
            ZStackObjectSourceFactory::MakeSplitResultSource(obj->getLabel()));
      obj->setHitProtocal(ZStackObject::HIT_NONE);
      obj->setVisualEffect(neutube::display::SparseObject::VE_PLANE_BOUNDARY);
      obj->setProjectionVisible(false);
      obj->setRole(ZStackObjectRole::ROLE_TMP_RESULT);
      obj->addRole(ZStackObjectRole::ROLE_SEGMENTATION);
    }
  }
}

ZObject3dScanArray* ZStackWatershedContainer::makeSplitResult(uint64_t minLabel,
    ZObject3dScanArray *result)
{
  if (m_result.empty()) {
    return result;
  }

  ZObject3dScanArray *objArray =
      ZObject3dFactory::MakeObject3dScanArray(m_result);

  //For sparse stack only for the current version
  if (m_spStack != NULL) {
    //Extract labeled regions
//    ZObject3dScanArray *objArray = ZObject3dFactory::MakeObject3dScanArray(
//          *(resultStack), neutube::Z_AXIS, true, NULL);

    if (result == NULL) {
      result = new ZObject3dScanArray;
    }

//    ZStackPtr firstStack = m_result.front();

//    ZIntPoint dsIntv = getSourceDsIntv();
    ZIntPoint dsIntv = m_result.front()->getDsIntv();

    ZIntPoint lastDsIntv = m_result.back()->getDsIntv();
    bool adopting = true;
    if (lastDsIntv.isZero()) {
      adopting = false;
    }

    ZObject3dScan mainBody;

    //The whole object
    ZObject3dScan *wholeBody = m_spStack->getObjectMask();

    ZObject3dScan remainBody;
    if (ccaPost()) {
      remainBody = *wholeBody;
    } else {
      if (getRange().contains(getDataRange())) {
        remainBody = *wholeBody;
      } else {
        wholeBody->subobject(getRange(), NULL, &remainBody);
      }
    }

    for (ZObject3dScanArray::iterator iter = objArray->begin();
         iter != objArray->end(); ++iter) {
      ZObject3dScan &obj = **iter;
      if (obj.getLabel() >= minLabel) {
        std::cout << "Processing label " << obj.getLabel() << std::endl;

        if (!dsIntv.isZero()) { //Process downsampled regions
          ZObject3dScan *currentBody =
              processSplitResult(obj, &remainBody, adopting);
          result->append(currentBody);
        } else {
          if (ccaPost()) {
            remainBody.subtractSliently(obj);
          }
          result->append(obj);
        }
      } else { //Treat labels below the threshold as the main body
        mainBody.concat(obj);
      }
    }

    if (ccaPost()) {
//      mainBody.upSample(dsIntv);
      assignComponent(remainBody, mainBody, result);
    }
    delete objArray;
  } else {
    if (result == NULL) {
      result = objArray;
    } else {
      result->swap(*objArray);
      delete objArray;
    }
  }

  configResult(result);

  return result;
}

void ZStackWatershedContainer::printState() const
{
  if (m_stack != NULL) {
    m_stack->printInfo();
  }
  if (m_spStack != NULL) {
    m_spStack->printInfo();
  }
  std::cout << "Range: " << m_range.toJsonArray().dumpString(0) << std::endl;
  std::cout << "Flooding zero: " << m_floodingZero << std::endl;
  if (m_source != NULL) {
    std::cout << "Downsampling: " << m_source->getDsIntv().toString() << std::endl;
  }
}


ZSegmentationScanArray* ZStackWatershedContainer::makeSplitResult(uint64_t minLabel)
{
  //pass
  return NULL;
}


ZIntCuboid ZStackWatershedContainer::getRangeUpdate(
    const ZIntCuboid &dataRange) const
{
  ZIntCuboid range = dataRange;

  if (usingSeedRange()) {
//  if (m_rangeOption == RANGE_SEED_BOUND || m_rangeOption == RANGE_SEED_ROI) {
    ZIntCuboid seedBox = GetSeedRange(m_seedArray);
    if (m_rangeOption == RANGE_SEED_ROI) {
      if (!seedBox.isEmpty()) {
        seedBox = ZFlyEmMisc::EstimateSplitRoi(seedBox);
      }
    } else {
      seedBox.expand(5, 5, 5);
    }

    if (range.isEmpty()) {
      range = seedBox;
    } else {
      range.intersect(seedBox);
    }
  }

  return range;
}

ZIntCuboid ZStackWatershedContainer::getRangeUpdate() const
{
  ZIntCuboid range;
  if (m_spStack != NULL) {
    range = m_spStack->getBoundBox();
  } else if (m_stack != NULL) {
    range = m_stack->getBoundBox();
  }

  return getRangeUpdate(range);
}

void ZStackWatershedContainer::updateRange()
{
  m_range = getRangeUpdate();
}
