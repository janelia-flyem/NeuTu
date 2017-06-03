#include "zstackwatershedcontainer.h"

#include "imgproc/zstackwatershed.h"
#include "zstack.hxx"
#include "zobject3d.h"
#include "zstroke2d.h"
#include "zsparsestack.h"
#include "zobject3dscanarray.h"
#include "zobject3dfactory.h"
#include "neutubeconfig.h"

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

ZStackWatershedContainer::~ZStackWatershedContainer()
{
  clearWorkspace();
  clearSource();
  clearResult();
}

void ZStackWatershedContainer::init()
{
  m_stack = NULL;
  m_spStack = NULL;
  m_source = NULL;
  m_workspace = NULL;
  m_result = NULL;
  m_channel = 0;
  m_floodingZero = false;
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
  delete m_result;
  m_result = NULL;
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

void ZStackWatershedContainer::addSeed(const ZObject3dScan &seed)
{
  ZIntPoint dsIntv = getSourceDsIntv();

  if (dsIntv.isZero()) {
    ZStackWatershed::AddSeed(getWorkspace(), getSourceOffset(), seed);
  } else {
    ZObject3dScan newSeed = seed;
    newSeed.downsampleMax(dsIntv);
    ZStackWatershed::AddSeed(getWorkspace(), getSourceOffset(), newSeed);
  }
}

void ZStackWatershedContainer::addSeed(const ZStack &seed)
{
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
}

void ZStackWatershedContainer::addSeed(const ZStroke2d &seed)
{
  ZStack stack;
  makeMaskStack(stack);
  seed.labelStack(&stack);
}

void ZStackWatershedContainer::addSeed(const ZObject3d &seed)
{
  ZStack stack;
  makeMaskStack(stack);
  seed.labelStack(&stack);
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
      if (!m_range.isEmpty()) {
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
    int x0, int y0, int z0, int x1, int y1, int z1)
{
  ZIntCuboid newRange(x0, y0, z0, x1, y1, z1);
  if (m_stack != NULL) {
    newRange.intersect(m_stack->getBoundBox());
  }

  if (!newRange.equals(m_range)) {
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
}

ZStack* ZStackWatershedContainer::getSourceStack()
{
  if (m_source == NULL && m_spStack != NULL) {
    m_source = m_spStack->makeStack(m_range);
  }

  return m_source;
}

Stack* ZStackWatershedContainer::getSource()
{
  return getSourceStack()->c_stack(m_channel);
}

void ZStackWatershedContainer::run()
{
  m_result = NULL;
  Stack *source = getSource();
  if (source != NULL) {
    Stack *out = C_Stack::watershed(source, getWorkspace());
    m_result = new ZStack;
    m_result->consume(out);
    m_result->setOffset(getSourceOffset());
  }
}

ZObject3dScanArray* ZStackWatershedContainer::makeSplitResult(
    ZObject3dScanArray *result)
{
  if (result == NULL) {
    result = new ZObject3dScanArray;
  }
  //For sparse stack only for the current version
  if (getResultStack() != NULL) {
    if (m_spStack != NULL) {
      ZObject3dScanArray *objArray = ZObject3dFactory::MakeObject3dScanArray(
            *(getResultStack()), NeuTube::Z_AXIS, true, NULL);
      ZIntPoint dsIntv = getSourceDsIntv();
      const size_t minIsolationSize = 50;
      ZObject3dScan mainBody;
      ZObject3dScan *wholeBody = m_spStack->getObjectMask();
      ZObject3dScan body = *wholeBody;
      for (ZObject3dScanArray::iterator iter = objArray->begin();
           iter != objArray->end(); ++iter) {
        ZObject3dScan &obj = *iter;
        if (obj.getLabel() > 1) {
          obj.upSample(dsIntv);
          std::cout << "Processing label " << obj.getLabel() << std::endl;
#ifdef _DEBUG_2
          if (obj.getLabel() == 128) {
            obj.save(GET_TEST_DATA_DIR + "/test3.sobj");
          }
#endif
          ZObject3dScan currentBody = body.subtract(obj);
#ifdef _DEBUG_2
          if (obj.getLabel() == 128) {
            currentBody.save(GET_TEST_DATA_DIR + "/test4.sobj");
          }
#endif

          if (!dsIntv.isZero()) {
            std::vector<ZObject3dScan> ccArray =
                currentBody.getConnectedComponent(ZObject3dScan::ACTION_NONE);
            for (std::vector<ZObject3dScan>::iterator iter = ccArray.begin();
                 iter != ccArray.end(); ++iter) {
              ZObject3dScan &subobj = *iter;
              bool isAdopted = false;
              if (subobj.getVoxelNumber() < minIsolationSize &&
                  currentBody.getVoxelNumber() / subobj.getVoxelNumber() > 10) {
                if (body.isAdjacentTo(subobj)) {
                  body.concat(subobj);
                  isAdopted = true;
                }
              }
#ifdef _DEBUG_2
              std::cout << "Split size: " << subobj.getVoxelNumber() << std::endl;
              std::cout << "Remain size: " << body.getVoxelNumber() << std::endl;
              if (body.getVoxelNumber() == 1) {
                body.print();
              }
#endif
              if (!isAdopted) {//Treated as a split region
                result->push_back(subobj);
              }
            }
          } else {
            result->push_back(currentBody);
          }
        } else {
          mainBody.concat(obj);
        }
      }

      mainBody.upSample(dsIntv);
      std::vector<ZObject3dScan> partArray =
          body.getConnectedComponent(ZObject3dScan::ACTION_NONE);

      for (std::vector<ZObject3dScan>::iterator iter = partArray.begin();
           iter != partArray.end(); ++iter) {
        ZObject3dScan &obj = *iter;

        //Check single connect for bound box split
        //Any component only connected to one split part?
        int count = 0;
        int splitIndex = 0;

        if (!obj.isAdjacentTo(mainBody)) {
          for (size_t index = 0; index < result->size(); ++index) {
            if (obj.isAdjacentTo((*result)[index])) {
              ++count;
              if (count > 1) {
                break;
              }
              splitIndex = index;
            }
          }
        }

        if (count > 0) {
          ZObject3dScan &split = (*result)[splitIndex];
          split.concat(obj);
        }
      }

      delete objArray;
    } else if (m_stack != NULL) {
      result = ZObject3dFactory::MakeObject3dScanArray(
            *(getResultStack()), NeuTube::Z_AXIS, true, NULL);
    }
  }

  return result;
}

void ZStackWatershedContainer::printInfo() const
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
