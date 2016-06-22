#include "zobject3dscan.h"

#include <iostream>
#include <fstream>
#include <stdlib.h>
#include <algorithm>
#include <sstream>
#include <cmath>
#include <cstring>
#if _QT_GUI_USED_
#include <QtGui>
#endif

#include "zobject3d.h"
#include "tz_error.h"
#include "zgraph.h"
#include "tz_stack_objlabel.h"
#include "zerror.h"
#include "tz_constant.h"
#include "neutubeconfig.h"
#include "tz_stack_bwmorph.h"
#include "c_stack.h"
#include "tz_stack_stat.h"
#include "tz_stack_math.h"
#include "include/tz_stdint.h"
#include "zfiletype.h"
#include "zeigensolver.h"
#include "zdoublevector.h"
#include "zstack.hxx"
#include "zstring.h"
#include "zhdf5reader.h"
#include "zstringarray.h"
#include "tz_math.h"
#include "tz_stack_bwmorph.h"
#include "zstackfactory.h"
#include "tz_stack_bwmorph.h"
#include "zpainter.h"
#include "tz_stack_bwmorph.h"
#include "geometry/zgeometry.h"

using namespace std;


///////////////////////////////////////////////////

const ZObject3dScan::TEvent ZObject3dScan::EVENT_NULL = 0;
const ZObject3dScan::TEvent ZObject3dScan::EVENT_OBJECT_VIEW_CHANGED = 0x1;
const ZObject3dScan::TEvent ZObject3dScan::EVENT_OBJECT_MODEL_CHANGED =
    0x2 | ZObject3dScan::EVENT_OBJECT_VIEW_CHANGED;
const ZObject3dScan::TEvent ZObject3dScan::EVENT_OBJECT_UNCANONIZED =
    0x4 | ZObject3dScan::EVENT_OBJECT_VIEW_CHANGED;
const ZObject3dScan::TEvent ZObject3dScan::EVENT_OBJECT_CANONIZED =
    0x8 | ZObject3dScan::EVENT_OBJECT_VIEW_CHANGED;


ZObject3dScan::ZObject3dScan()
{
  init();
}

ZObject3dScan::ZObject3dScan(const ZObject3dScan &obj) : ZStackObject(obj),
  m_zProjection(NULL)
{
  *this = obj;
}

ZObject3dScan::~ZObject3dScan()
{
  deprecate(COMPONENT_ALL);
}

void ZObject3dScan::init()
{
  setTarget(TARGET_OBJECT_CANVAS);
  m_type = ZStackObject::TYPE_OBJECT3D_SCAN;

  m_isCanonized = false;
  m_label = 0;
  m_blockingEvent = false;
  m_zProjection = NULL;
  m_sliceAxis = NeuTube::Z_AXIS;
}


ZObject3dScan& ZObject3dScan::operator=(const ZObject3dScan& obj)
{
  deprecate(COMPONENT_ALL);

  dynamic_cast<ZStackObject&>(*this) = dynamic_cast<const ZStackObject&>(obj);

  m_stripeArray = obj.m_stripeArray;
  m_isCanonized = obj.m_isCanonized;
  m_label = obj.m_label;
  m_blockingEvent = false;
  m_sliceAxis = obj.m_sliceAxis;
//  uint64_t m_label;

//  this->m_zProjection = NULL;

  return *this;
}

void ZObject3dScan::labelStack(Stack *stack, int startLabel, const int *offset)
{
  std::vector<ZObject3dScan> objArray = getConnectedComponent(ACTION_NONE);

#ifdef _DEBUG_
  std::cout << "Number of components: " << objArray.size() << std::endl;
#endif

  if (objArray.size() + startLabel > 256) {
    Translate_Stack(stack, GREY16, 1);
  }
  for (size_t i = 0; i < objArray.size(); ++i) {
    objArray[i].drawStack(stack, startLabel  + i, offset);
  }
}

void ZObject3dScan::maskStack(ZStack *stack)
{
  ZStack *mask = ZStackFactory::makeZeroStack(
        stack->kind(), stack->getBoundBox());
  drawStack(mask->c_stack(), 1);
  for (int c = 0; c < stack->channelNumber(); ++c) {
    Stack_Mul(stack->c_stack(c), mask->c_stack(), stack->c_stack(c));
  }
  delete mask;
}


void ZObject3dScan::copyDataFrom(const ZObject3dScan &obj)
{
  deprecate(COMPONENT_ALL);

  m_stripeArray = obj.m_stripeArray;
  m_isCanonized = obj.m_isCanonized;
  m_sliceAxis = obj.m_sliceAxis;
}


bool ZObject3dScan::isDeprecated(EComponent comp) const
{
  switch (comp) {
  case COMPONENT_STRIPE_INDEX_MAP:
    return m_stripeMap.empty();
  case COMPONENT_INDEX_SEGMENT_MAP:
    return m_indexSegmentMap.empty();
  case COMPONENT_ACCUMULATED_STRIPE_NUMBER:
    return m_accNumberArray.empty();
  case COMPONENT_SLICEWISE_VOXEL_NUMBER:
    return m_slicewiseVoxelNumber.empty();
  case COMPONENT_Z_PROJECTION:
    return m_zProjection == NULL;
  default:
    break;
  }

  return false;
}

void ZObject3dScan::deprecate(EComponent comp)
{
  deprecateDependent(comp);

  switch (comp) {
  case COMPONENT_STRIPE_INDEX_MAP:
    m_stripeMap.clear();
    break;
  case COMPONENT_INDEX_SEGMENT_MAP:
    m_indexSegmentMap.clear();
    break;
  case COMPONENT_ACCUMULATED_STRIPE_NUMBER:
    m_accNumberArray.clear();
    break;
  case COMPONENT_SLICEWISE_VOXEL_NUMBER:
    m_slicewiseVoxelNumber.clear();
    break;
  case COMPONENT_Z_PROJECTION:
    delete m_zProjection;
    m_zProjection = NULL;
    break;
  case COMPONENT_ALL:
    deprecate(COMPONENT_STRIPE_INDEX_MAP);
    deprecate(COMPONENT_INDEX_SEGMENT_MAP);
    deprecate(COMPONENT_ACCUMULATED_STRIPE_NUMBER);
    deprecate(COMPONENT_SLICEWISE_VOXEL_NUMBER);
    deprecate(COMPONENT_Z_PROJECTION);
    break;
  }
}

void ZObject3dScan::deprecateDependent(EComponent comp)
{
  switch (comp) {
  default:
    break;
  }
}

bool ZObject3dScan::isEmpty() const
{
  return m_stripeArray.empty();
}

size_t ZObject3dScan::getStripeNumber() const
{
  return m_stripeArray.size();
}

size_t ZObject3dScan::getVoxelNumber() const
{
  size_t voxelNumber = 0;
  for (size_t i = 0; i < getStripeNumber(); ++i) {
    voxelNumber += m_stripeArray[i].getVoxelNumber();
  }

  return voxelNumber;
}

size_t ZObject3dScan::getVoxelNumber(int z) const
{
  size_t voxelNumber = 0;
  size_t stripeNumber = getStripeNumber();
  for (size_t i = 0; i < stripeNumber; ++i) {
    if (m_stripeArray[i].getZ() == z) {
      voxelNumber += m_stripeArray[i].getVoxelNumber();
    }
  }

  return voxelNumber;
}

const std::map<int, size_t> &ZObject3dScan::getSlicewiseVoxelNumber() const
{
  if (isDeprecated(COMPONENT_SLICEWISE_VOXEL_NUMBER)) {
    //std::vector<size_t> voxelNumber;
    size_t stripeNumber = getStripeNumber();
    for (size_t i = 0; i < stripeNumber; ++i) {
      int z = m_stripeArray[i].getZ();
      if (m_slicewiseVoxelNumber.count(z) == 0) {
        m_slicewiseVoxelNumber[z] = m_stripeArray[i].getVoxelNumber();
      } else {
        m_slicewiseVoxelNumber[z] += m_stripeArray[i].getVoxelNumber();
      }
    }
  }

  return m_slicewiseVoxelNumber;
}

std::map<int, size_t> &ZObject3dScan::getSlicewiseVoxelNumber()
{
  return const_cast<std::map<int, size_t>&>(
        static_cast<const ZObject3dScan&>(*this).getSlicewiseVoxelNumber());
}

const ZObject3dStripe &ZObject3dScan::getStripe(size_t index) const
{
  return m_stripeArray[index];
}

ZObject3dStripe &ZObject3dScan::getStripe(size_t index)
{
  return m_stripeArray[index];
}

void ZObject3dScan::addStripe(int z, int y, bool canonizing)
{
  bool adding = true;
  if (!m_stripeArray.empty()) {
    if (y == m_stripeArray.back().getY() &&
        z == m_stripeArray.back().getZ()) {
      adding = false;
    }
  }

  if (adding) {
    ZObject3dStripe stripe;
    stripe.setY(y);
    stripe.setZ(z);

    addStripe(stripe, canonizing);
  }
}

void ZObject3dScan::addStripeFast(int z, int y)
{
//  m_stripeArray.reserve(m_stripeArray.size() + 1);
  ZObject3dStripe stripe;
  stripe.setY(y);
  stripe.setZ(z);
  m_stripeArray.push_back(stripe);
}

void ZObject3dScan::addStripeFast(const ZObject3dStripe &stripe)
{
  m_stripeArray.push_back(stripe);
}

void ZObject3dScan::addStripe(const ZObject3dStripe &stripe, bool canonizing)
{
  bool lastStripeMergable = false;

  TEvent event = EVENT_NULL;

  if (!m_stripeArray.empty()) {
    const ZObject3dStripe &lastStripe = m_stripeArray.back();
    if (stripe.getY() != lastStripe.getY() ||
        stripe.getZ() != lastStripe.getZ()) {
      if (m_isCanonized) {
        if (stripe.getZ() < lastStripe.getZ()) {
          //m_isCanonized = false;
          event = EVENT_OBJECT_UNCANONIZED;
        } else if (stripe.getZ() == lastStripe.getZ()) {
          if (stripe.getY() < lastStripe.getY()) {
            //m_isCanonized = false;
            event |= EVENT_OBJECT_UNCANONIZED;
          }
        }
      }
    } else {
      lastStripeMergable = true;
    }
  }

  if (lastStripeMergable) {
    ZObject3dStripe &lastStripe = m_stripeArray.back();
    for (int i = 0; i < stripe.getSegmentNumber(); ++i) {
      lastStripe.addSegment(
            stripe.getSegmentStart(i), stripe.getSegmentEnd(i), canonizing);
    }
  } else {
    m_stripeArray.push_back(stripe);
  }

  if (!m_blockingEvent) {
    event |= EVENT_OBJECT_MODEL_CHANGED;
    processEvent(event);
  }

  if (canonizing) {
    canonize();
  }

  //deprecate(ALL_COMPONENT);
}

void ZObject3dScan::addSegment(int x1, int x2, bool canonizing)
{
  if (!isEmpty()) {
    TEvent event = EVENT_NULL;

    m_stripeArray.back().addSegment(x1, x2, canonizing);

    if (!m_blockingEvent) {
      event |= EVENT_OBJECT_MODEL_CHANGED;

      if (!m_stripeArray.back().isCanonized()) {
        //m_isCanonized = false;
        if (isCanonized()) {
          event |= EVENT_OBJECT_UNCANONIZED;
        }
      }

      /*
    deprecate(INDEX_SEGMENT_MAP);
    deprecate(ACCUMULATED_STRIPE_NUMBER);
    */

      processEvent(event);
    }

    if (canonizing) {
      canonize();
    }
  }
}

void ZObject3dScan::addSegmentFast(int x1, int x2)
{
  if (!isEmpty()) {
    ZObject3dStripe &stripe = m_stripeArray.back();
    stripe.getSegmentArray().push_back(x1);
    stripe.getSegmentArray().push_back(x2);
//    m_stripeArray.back().addSegment(x1, x2, false);
  }
}

void ZObject3dScan::addSegment(int z, int y, int x1, int x2, bool canonizing)
{
  addStripe(z, y, false);
  addSegment(x1, x2, canonizing);
}

/*
const int* ZObject3dScan::getLastStripe() const
{
  if (isDeprecated(LAST_STRIPE)) {
    size_t numStripe = getStripeNumber();
    const int *stripe = getFirstStripe();

    for (size_t i = 0; i < numStripe - 1; i++) {
      int numScanLine = getStripeSize(stripe);
      const int *scanLine = getSegment(stripe);
      for (int j = 0; j < numScanLine; j++) {
        if (j < numScanLine - 1) {
          scanLine += 2;
        }
      }
      stripe = scanLine + 1;
    }

    m_lastStripe = const_cast<int*>(stripe);
  }

  return m_lastStripe;
}

int* ZObject3dScan::getLastStripe()
{
  return const_cast<int*>(static_cast<const ZObject3dScan&>(*this).getLastStripe());
}
*/
ZObject3d* ZObject3dScan::toObject3d() const
{
  if (isEmpty()) {
    return NULL;
  }

  ZObject3d *obj = new ZObject3d();

  for (vector<ZObject3dStripe>::const_iterator iter = m_stripeArray.begin();
       iter != m_stripeArray.end(); ++iter) {
    int y = iter->getY();
    int z = iter->getZ();
    int nseg = iter->getSegmentNumber();
    for (int i = 0; i < nseg; ++i) {
      const int *seg = iter->getSegment(i);
      for (int x = seg[0]; x <= seg[1]; ++x) {
        obj->append(x, y, z);
      }
    }
  }

  obj->setColor(this->getColor());
  obj->setLabel(this->getLabel());

  return obj;
}

const std::map<size_t, std::pair<size_t, size_t> >&
ZObject3dScan::getIndexSegmentMap() const
{
  if (isDeprecated(COMPONENT_INDEX_SEGMENT_MAP)) {
    m_indexSegmentMap.clear();
    size_t currentIndex = 0;
    for (size_t stripeIndex = 0; stripeIndex < getStripeNumber(); ++stripeIndex) {
      size_t segmentNumber = m_stripeArray[stripeIndex].getSegmentNumber();
      for (size_t segIndex = 0; segIndex < segmentNumber; ++segIndex) {
        m_indexSegmentMap[currentIndex++] =
            std::pair<size_t, size_t>(stripeIndex, segIndex);
      }
    }
  }

  return m_indexSegmentMap;
}

bool ZObject3dScan::getSegment(size_t index, int *z, int *y, int *x1, int *x2)
{
  const std::map<size_t, std::pair<size_t, size_t> >&segMap =
      getIndexSegmentMap();
  if (segMap.count(index) > 0) {
    std::pair<size_t, size_t> location = segMap.at(index);
    *z = m_stripeArray[location.first].getZ();
    *y = m_stripeArray[location.first].getY();
    *x1 = m_stripeArray[location.first].getSegmentStart(location.second);
    *x2 = m_stripeArray[location.first].getSegmentEnd(location.second);
    return true;
  }

  return false;
}

void ZObject3dScan::loadStack(const Stack *stack)
{
  clear();

  if (stack == NULL) {
    RECORD_ERROR(true, "null stack");
    return;
  }

  if (C_Stack::kind(stack) != GREY) {
    RECORD_ERROR(true, "unsupported kind");
    return;
  }

  int width = C_Stack::width(stack);
  int height = C_Stack::height(stack);
  int depth = C_Stack::depth(stack);

  int sw = width;
  int sh = height;
  int sd = depth;
  ZGeometry::shiftSliceAxis(sw, sh, sd, m_sliceAxis);

  uint8_t *array = stack->array;
  uint8_t *arrayOrigin = array;
  size_t area = width * height;

  size_t xStride = 1;
  size_t yStride = width;
  size_t zStride = area;

  ZGeometry::shiftSliceAxis(xStride, yStride, zStride, m_sliceAxis);

  switch (m_sliceAxis) {
  case NeuTube::Z_AXIS: //XY plane
  for (int z = 0; z < depth; ++z) {
    for (int y = 0; y < height; ++y) {
      int x1 = -1;
      int x2 = -1;
      int scanState = 0; //0: no ON pixel found;
                         //1: in ON region; 2: in OFF region
      for (int x = 0; x < width; ++x) {
        if (*array > 0) {
          switch (scanState) {
          case 0:
            addStripe(z, y);
            x1 = x;
            break;
          case 1:
            break;
          case 2:
            x1 = x;
            break;
          }
          scanState = 1;
        } else {
          if (scanState == 1) {
            x2 = x - 1;
            scanState = 2;
          }
        }
        if (x1 >= 0 && x2 >= 0) {
          addSegment(x1, x2, false);
          x1 = -1;
          x2 = -1;
        }
        ++array;
      }
      if (x1 >= 0) {
        addSegment(x1, width - 1, false);
      }
    }
  }
  break;
  case NeuTube::Y_AXIS: //XZ plane
  case NeuTube::X_AXIS: //ZY plane
    for (int z = 0; z < sd; ++z) {
      for (int y = 0; y < sh; ++y) {
        int x1 = -1;
        int x2 = -1;
        int scanState = 0; //0: no ON pixel found;
                           //1: in ON region; 2: in OFF region
        array = arrayOrigin + z * zStride + y * yStride;
        for (int x = 0; x < sw; ++x) {
          if (*array > 0) {
            switch (scanState) {
            case 0:
              addStripe(z, y);
              x1 = x;
              break;
            case 1:
              break;
            case 2:
              x1 = x;
              break;
            }
            scanState = 1;
          } else {
            if (scanState == 1) {
              x2 = x - 1;
              scanState = 2;
            }
          }
          if (x1 >= 0 && x2 >= 0) {
            addSegment(x1, x2, false);
            x1 = -1;
            x2 = -1;
          }
          array += xStride;
        }
        if (x1 >= 0) {
          addSegment(x1, sw - 1, false);
        }
      }
    }
    break;
  }

  setCanonized(true);
}

void ZObject3dScan::loadStack(const ZStack &stack)
{
  loadStack(stack.c_stack());
  translate(stack.getOffset().getX(), stack.getOffset().getY(),
            stack.getOffset().getZ());
}

void ZObject3dScan::print() const
{
  cout << getStripeNumber() << " stripes" << endl;
  for (vector<ZObject3dStripe>::const_iterator iter = m_stripeArray.begin();
       iter != m_stripeArray.end(); ++iter) {
    //cout << "  Z:" << iter->getZ() << " Y:" << iter->getY() << " #S:"
    //     << iter->getSegmentNumber() << " #V:" << iter->getVoxelNumber() << endl;
    iter->print(2);
  }
}

bool ZObject3dScan::load(const string &filePath)
{
  clear();

  bool succ = false;
  ZString filePath2(filePath);
#ifdef _DEBUG_2
    std::cout << filePath << std::endl;
#endif

  if (filePath2.contains(":")) {
    std::vector<std::string> strArray = filePath2.tokenize(':');
    if (strArray.size() >= 2) {
      if (ZFileType::fileType(strArray[0]) == ZFileType::HDF5_FILE) {
        succ = importHdf5(strArray[0], strArray[1]);
      }
    }
  } else if (ZFileType::fileType(filePath) == ZFileType::DVID_OBJECT_FILE) {
    succ = importDvidObject(filePath);
  } else if (ZFileType::fileType(filePath) == ZFileType::OBJECT_SCAN_FILE) {
    FILE *fp = fopen(filePath.c_str(), "rb");
    if (fp != NULL) {
      int stripeNumber = 0;
      fread(&stripeNumber, sizeof(int), 1, fp);

      PROCESS_ERROR(stripeNumber < 0, "Invalid stripe number", return false);
      /*
    if (stripeNumber < 0) {
      RECORD_ERROR(true, "Invalid stripe number");
      return false;
    }
    */

      m_stripeArray.resize(stripeNumber);
      for (vector<ZObject3dStripe>::iterator iter = m_stripeArray.begin();
           iter != m_stripeArray.end(); ++iter) {
        iter->read(fp);
      }
      fclose(fp);

      if (isCanonizedActually()) {
        m_isCanonized = true;
      } else {
        m_isCanonized = false;
      }

      deprecate(COMPONENT_ALL);

      succ = true;
    }
  }

  if (succ) {
    setSource(filePath);
  }

  RECORD_WARNING(!succ, "Cannont open file " + filePath);

  return succ;
}

bool ZObject3dScan::load(const char *filePath)
{
  return load(std::string(filePath));
}

bool ZObject3dScan::save(const char *filePath)
{
  return save(string(filePath));
}

bool ZObject3dScan::save(const char *filePath) const
{
  return save(string(filePath));
}

bool ZObject3dScan::save(const string &filePath) const
{
#ifdef _DEBUG_
  std::cout << "Saving " << filePath << std::endl;
#endif
  bool succ = false;
  FILE *fp = fopen(filePath.c_str(), "wb");
  if (fp != NULL) {
    int stripeNumber = (int) getStripeNumber();
    fwrite(&stripeNumber, sizeof(int), 1, fp);
    for (vector<ZObject3dStripe>::const_iterator iter = m_stripeArray.begin();
         iter != m_stripeArray.end(); ++iter) {
      iter->write(fp);
    }
    fclose(fp);
    succ = true;
  } else {
    RECORD_WARNING(true, "Cannont open file " + filePath);
  }

  return succ;
}

size_t ZObject3dScan::countForegroundOverlap(Stack *stack, const int *offset)
{
  size_t count = 0;
  for (vector<ZObject3dStripe>::const_iterator iter = m_stripeArray.begin();
       iter != m_stripeArray.end(); ++iter) {
    count += iter->countForegroundOverlap(stack, offset);
  }

  return count;
}

void ZObject3dScan::drawStack(Stack *stack, int v, const int *offset) const
{
  for (vector<ZObject3dStripe>::const_iterator iter = m_stripeArray.begin();
       iter != m_stripeArray.end(); ++iter) {
    iter->drawStack(stack, v, offset);
  }
}

void ZObject3dScan::drawStack(ZStack *stack, int v) const
{
  int offset[3];
  offset[0] = -stack->getOffset().getX();
  offset[1] = -stack->getOffset().getY();
  offset[2] = -stack->getOffset().getZ();

  drawStack(stack->c_stack(0), v, offset);
}

void ZObject3dScan::drawStack(
    Stack *stack, uint8_t red, uint8_t green, uint8_t blue, const int *offset) const
{
  for (vector<ZObject3dStripe>::const_iterator iter = m_stripeArray.begin();
       iter != m_stripeArray.end(); ++iter) {
    const ZObject3dStripe &stripe = *iter;
    stripe.drawStack(stack, red, green, blue, m_sliceAxis, offset);
  }
}

void ZObject3dScan::drawStack(
    Stack *stack, uint8_t red, uint8_t green, uint8_t blue, double alpha,
    const int *offset) const
{
  for (vector<ZObject3dStripe>::const_iterator iter = m_stripeArray.begin();
       iter != m_stripeArray.end(); ++iter) {
    const ZObject3dStripe &stripe = *iter;
    stripe.drawStack(stack, red, green, blue, alpha, offset);
  }
}

void ZObject3dScan::drawStack(
    ZStack *stack, uint8_t red, uint8_t green, uint8_t blue) const
{
  int offset[3];
  offset[0] = -stack->getOffset().getX();
  offset[1] = -stack->getOffset().getY();
  offset[2] = -stack->getOffset().getZ();

  drawStack(stack->c_stack(0), red, green, blue, offset);
}


static int ZObject3dStripeCompare(const void *e1, const void *e2)
{
  ZObject3dStripe *v1 = (ZObject3dStripe*) e1;
  ZObject3dStripe *v2 = (ZObject3dStripe*) e2;

  if (v1->getZ() < v2->getZ()) {
    return -1;
  } else if (v1->getZ() > v2->getZ()) {
    return 1;
  } else {
    if (v1->getY() < v2->getY()) {
      return -1;
    } else if (v1->getY() > v2->getY()) {
      return 1;
    }
  }

  return 0;
}

void ZObject3dScan::sort()
{
  if (!isEmpty()) {
    qsort(&m_stripeArray[0], m_stripeArray.size(), sizeof(ZObject3dStripe),
        ZObject3dStripeCompare);
    //deprecate(ALL_COMPONENT);
    processEvent(EVENT_OBJECT_VIEW_CHANGED);
  }
}

void ZObject3dScan::canonize()
{
  if (!isEmpty() && !isCanonized()) {
    sort();

    std::cout << "Sorting done in canozing" << std::endl;

#ifdef _DEBUG_2
  int count = 0;
  int ncount = 0;
  for (size_t i = 0; i < getStripeNumber(); ++i) {
    const ZObject3dStripe &stripe = getStripe(i);
    if (stripe.isCanonized()) {
      ++count;
    } else {
      ++ncount;
    }
  }
  std::cout << "Canonized: " << count << std::endl;
  std::cout << "Uncanonized: " << ncount << std::endl;
#endif

    vector<ZObject3dStripe> newStripeArray(m_stripeArray.size());
    size_t length = 0;
    //newStripeArray.reserve(m_stripeArray.size());
    m_stripeArray[0].canonize();
    //newStripeArray.push_back(m_stripeArray[0]);
    newStripeArray[length++] = m_stripeArray[0];
    for (size_t i = 1; i < m_stripeArray.size(); ++i) {
      ZObject3dStripe &newStripe = newStripeArray[length - 1];
      ZObject3dStripe &stripe = m_stripeArray[i];
      if (!newStripe.unify(stripe)) {
        stripe.canonize();
        //newStripeArray.push_back(m_stripeArray[i]);
        newStripeArray[length++] = stripe;
      }
    }

    newStripeArray.resize(length);
    std::cout << length << " stripes finalized." << std::endl;

    //m_stripeArray = newStripeArray;
    m_stripeArray.swap(newStripeArray);

    //m_isCanonized = true;
    processEvent(EVENT_OBJECT_CANONIZED);
  }
}

void ZObject3dScan::unify(const ZObject3dScan &obj)
{
  concat(obj);
  canonize();
}

void ZObject3dScan::concat(const ZObject3dScan &obj)
{
  m_stripeArray.insert(m_stripeArray.end(), obj.m_stripeArray.begin(),
                       obj.m_stripeArray.end());

  processEvent(EVENT_OBJECT_MODEL_CHANGED | EVENT_OBJECT_UNCANONIZED);
  //deprecate(ALL_COMPONENT);
}

void ZObject3dScan::downsample(int xintv, int yintv, int zintv)
{
  TEvent event = EVENT_NULL;

  if (yintv > 0 || zintv > 0) {
    vector<ZObject3dStripe> newStripeArray;
    for (vector<ZObject3dStripe>::const_iterator iter = m_stripeArray.begin();
         iter != m_stripeArray.end(); ++iter) {
      if ((iter->getY() % (yintv + 1) == 0) &&
          (iter->getZ() % (zintv + 1) == 0)) {
        newStripeArray.push_back(*iter);
      }
    }
    m_stripeArray = newStripeArray;
    //m_isCanonized = false;
    event |= EVENT_OBJECT_UNCANONIZED;
  }

  if (xintv > 0) {
    for (vector<ZObject3dStripe>::iterator iter = m_stripeArray.begin();
         iter != m_stripeArray.end(); ++iter) {
      iter->downsample(xintv);
      iter->setY(iter->getY() / (yintv + 1));
      iter->setZ(iter->getZ() / (zintv + 1));
    }
    //m_isCanonized = false;
    event |= EVENT_OBJECT_UNCANONIZED;
  }

  if (xintv > 0 || yintv > 0 || zintv > 0) {
    //deprecate(ALL_COMPONENT);
    event |= EVENT_OBJECT_MODEL_CHANGED;
  }

  processEvent(event);
}

void ZObject3dScan::downsampleMax(int xintv, int yintv, int zintv)
{
  if (xintv == 0 && yintv == 0 && zintv == 0) {
    return;
  }

  TEvent event = EVENT_NULL;

  if (yintv > 0 || zintv > 0) {
    for (vector<ZObject3dStripe>::iterator iter = m_stripeArray.begin();
         iter != m_stripeArray.end(); ++iter) {
      iter->setY(iter->getY() / (yintv + 1));
      iter->setZ(iter->getZ() / (zintv + 1));
    }
    //m_isCanonized = false;
    event |= EVENT_OBJECT_UNCANONIZED;
  }

  if (xintv > 0) {
    for (vector<ZObject3dStripe>::iterator iter = m_stripeArray.begin();
         iter != m_stripeArray.end(); ++iter) {
      iter->downsampleMax(xintv);
    }
    //m_isCanonized = false;
    event |= EVENT_OBJECT_UNCANONIZED;
  }

  processEvent(event);

  canonize();

  //deprecate(ALL_COMPONENT);
}

void ZObject3dScan::upSample(int xIntv, int yIntv, int zIntv)
{
  if (xIntv == 0 && yIntv == 0 && zIntv == 0) {
    return;
  }

  int stripeNumber = getStripeNumber();
  for (int i = 0; i < stripeNumber; ++i) {
    ZObject3dStripe &stripe = getStripe(i);
    stripe.upSample(xIntv);
  }

  ZObject3dScan grownPart;

  for (int i = 0; i < stripeNumber; ++i) {
    ZObject3dStripe &stripe = getStripe(i);

    int y = stripe.getY();
    int z = stripe.getZ();

    int z0 = z * (zIntv + 1);
    int z1 = z0 + zIntv;
    int y0 = y * (yIntv + 1);
    int y1 = y0 + yIntv;

#ifdef _DEBUG_2
    std::cout << y0 << ", " << y1 << std::endl;
#endif

    for (z = z0; z <= z1; ++z) {
      for (y = y0; y <= y1; ++y) {
        ZObject3dStripe newStripe = stripe;
        newStripe.setY(y);
        newStripe.setZ(z);
        grownPart.addStripe(newStripe, false);
      }
    }

#ifdef _DEBUG_2
        std::cout << "New Stripe: ";
        stripe.print();
        std::cout << "---" << std::endl;
#endif
  }

  grownPart.setLabel(this->getLabel());
  *this = grownPart;

  canonize();
}

bool ZObject3dScan::isAdjacentTo(ZObject3dScan &obj)
{
  canonize();
  obj.canonize();

  if (getVoxelNumber() < obj.getVoxelNumber()) {
    ZObject3dScan tmpObj = *this;
    tmpObj.dilate();
    return obj.hasOverlap(tmpObj);
  } else {
    ZObject3dScan tmpObj = obj;
    tmpObj.dilate();
    return hasOverlap(tmpObj);
  }

  return false;
}

bool ZObject3dScan::hasOverlap(ZObject3dScan &obj)
{
  if (isEmpty() || obj.isEmpty()) {
    return false;
  }

  int minZ = imax2(getMinZ(), obj.getMinZ());
  int maxZ = imin2(getMaxZ(), obj.getMaxZ());

  for (int z = minZ; z <= maxZ; ++z) {
    ZObject3dScan slice1 = getSlice(z);
    ZObject3dScan slice2 = obj.getSlice(z);
    ZStack *stack1 = slice1.toStackObject();
    int stripeNumber = slice2.getStripeNumber();
    for (int i = 0; i < stripeNumber; ++i) {
      const ZObject3dStripe &stripe = slice2.getStripe(i);
      int segmentNumber = stripe.getSegmentNumber();
      int y = stripe.getY();
      for (int j = 0; j < segmentNumber; ++j) {
        int minX = stripe.getSegmentStart(j);
        int maxX = stripe.getSegmentEnd(j);
        for (int x = minX; x <= maxX; ++x) {
          if (stack1->getIntValue(x, y, z) > 0) {
            delete stack1;
            return true;
          }
        }
      }
    }
    delete stack1;
  }

  return false;
}

Stack* ZObject3dScan::toStack(int *offset, int v) const
{
  if (isEmpty()) {
    return NULL;
  }

  ZIntCuboid boundBox = getBoundBox();
  if (offset != NULL) {
    offset[0] = boundBox.getFirstCorner().getX();
    offset[1] = boundBox.getFirstCorner().getY();
    offset[2] = boundBox.getFirstCorner().getZ();
  }

#if 0
  std::cout << "Stack size: " << boundBox.getWidth() << "x"
            << boundBox.getHeight() << "x" << boundBox.getDepth() << std::endl;
#endif

  Stack *stack = C_Stack::make(GREY, boundBox.getWidth(),
                               boundBox.getHeight(),
                               boundBox.getDepth());
  C_Stack::setZero(stack);


  int drawingOffet[3];
  drawingOffet[0] = -boundBox.getFirstCorner().getX();
  drawingOffet[1] = -boundBox.getFirstCorner().getY();
  drawingOffet[2] = -boundBox.getFirstCorner().getZ();

  drawStack(stack, v, drawingOffet);

  return stack;
}

Stack* ZObject3dScan::toStackWithMargin(int *offset, int v, int margin) const
{
  if (isEmpty()) {
    return NULL;
  }

  ZIntCuboid boundBox = getBoundBox();
  if (offset != NULL) {
    offset[0] = boundBox.getFirstCorner().getX();
    offset[1] = boundBox.getFirstCorner().getY();
    offset[2] = boundBox.getFirstCorner().getZ();
  }

#if 0
  std::cout << "Stack size: " << boundBox.getWidth() << "x"
            << boundBox.getHeight() << "x" << boundBox.getDepth() << std::endl;
#endif

  Stack *stack = C_Stack::make(GREY, boundBox.getWidth() + margin * 2,
                               boundBox.getHeight() + margin * 2,
                               boundBox.getDepth() + margin * 2);
  C_Stack::setZero(stack);


  int drawingOffet[3];
  drawingOffet[0] = -boundBox.getFirstCorner().getX() + margin;
  drawingOffet[1] = -boundBox.getFirstCorner().getY() + margin;
  drawingOffet[2] = -boundBox.getFirstCorner().getZ() + margin;

  drawStack(stack, v, drawingOffet);

  return stack;
}

ZStack* ZObject3dScan::toStackObject(int v) const
{
  int offset[3] = {0, 0, 0};
  Stack *stack = toStack(offset, v);

  ZStack *stackObject = new ZStack;

  if (stack != NULL) {
    stackObject->load(stack);
    stackObject->setOffset(offset[0], offset[1], offset[2]);
  }

  return stackObject;
}

ZStack* ZObject3dScan::toStackObjectWithMargin(int v, int margin) const
{
  int offset[3] = {0, 0, 0};
  Stack *stack = toStackWithMargin(offset, v, margin);

  ZStack *stackObject = new ZStack;

  if (stack != NULL) {
    stackObject->load(stack);
    stackObject->setOffset(offset[0], offset[1], offset[2]);
  }

  return stackObject;
}

ZStack* ZObject3dScan::toVirtualStack() const
{
  ZIntCuboid box = getBoundBox();

  ZStack *stack = new ZStack(GREY, box.getWidth(), box.getHeight(),
                             box.getDepth(), 1, true);
  stack->setOffset(box.getFirstCorner());

  return stack;
}

ZIntCuboid ZObject3dScan::getBoundBox() const
{
  ZIntCuboid boundBox;

  bool isFirst = true;
  for (vector<ZObject3dStripe>::const_iterator iter = m_stripeArray.begin();
       iter != m_stripeArray.end(); ++iter) {
    if (!iter->isEmpty()) {
      if (isFirst) {
        boundBox.set(iter->getMinX(), iter->getY(), iter->getZ(),
                     iter->getMaxX(), iter->getY(), iter->getZ());
        isFirst = false;
      } else {
        boundBox.joinY(iter->getY());
        boundBox.joinZ(iter->getZ());
        boundBox.joinX(iter->getMinX());
        boundBox.joinX(iter->getMaxX());
      }
#ifdef _DEBUG_2
      std::cout << iter->getMinX() << " " << iter->getMaxX() << " "
                << iter->getY() << " " << iter->getZ() << std::endl;
      std::cout << boundBox.getFirstCorner().toString() << " "
                << boundBox.getLastCorner().toString() << std::endl;
#endif
    }
  }

  boundBox.shiftSliceAxis(m_sliceAxis);

  return boundBox;
}

void ZObject3dScan::getBoundBox(Cuboid_I *box) const
{
  ZIntCuboid boundBox = getBoundBox();

  Cuboid_I_Set_S(box, boundBox.getFirstCorner().getX(),
                 boundBox.getFirstCorner().getY(),
                 boundBox.getFirstCorner().getZ(),
                 boundBox.getWidth(),
                 boundBox.getHeight(), boundBox.getDepth());
}

void ZObject3dScan::getBoundBox(ZIntCuboid *box) const
{
  if (box != NULL) {
    *box = getBoundBox();
  }
}

const std::vector<size_t>& ZObject3dScan::getStripeNumberAccumulation() const
{
  if (isDeprecated(COMPONENT_ACCUMULATED_STRIPE_NUMBER)) {
    m_accNumberArray.resize(getStripeNumber() + 1);
    m_accNumberArray[0] = 0;
    for (size_t i = 0; i < getStripeNumber(); ++i) {
      m_accNumberArray[i + 1] =
          m_accNumberArray[i] + m_stripeArray[i].getSegmentNumber();
    }
  }

  return m_accNumberArray;
}

const std::map<std::pair<int, int>, size_t> &ZObject3dScan::getStripeMap() const
{
  if (isDeprecated(COMPONENT_STRIPE_INDEX_MAP)) {
    m_stripeMap.clear();
    for (size_t i = 0; i < getStripeNumber(); ++i) {
      m_stripeMap[std::pair<int, int>(
            m_stripeArray[i].getZ(), m_stripeArray[i].getY())] = i;
    }
  }

  return m_stripeMap;
}

ZGraph* ZObject3dScan::buildConnectionGraph()
{
  if (isEmpty()) {
    return NULL;
  }

  ZGraph *graph = new ZGraph(ZGraph::UNDIRECTED_WITHOUT_WEIGHT);

#ifdef _DEBUG_
  std::cout << "Canonizing ..." << std::endl;
#endif

  canonize();

  const std::vector<size_t>& stripeNumberAccumulation =
      getStripeNumberAccumulation();

  const std::map<std::pair<int, int>, size_t> &stripeMap = getStripeMap();

  size_t stripeNumber = getStripeNumber();
  size_t round = stripeNumber / 10;
  for (size_t i = 0; i < stripeNumber - 1; ++i) {
#ifdef _DEBUG_
    if (round > 0) {
      if (i % round == 0) {
        std::cout << "  " << i + 1 << "/" << stripeNumber << std::endl;
      }
    }
#endif
    //Check along Y
    for (int j = 0; j < m_stripeArray[i].getSegmentNumber(); ++j) {
      const int *stripe = m_stripeArray[i].getSegment(j);
      int v1 = stripeNumberAccumulation[i] + j;

      //Check along Y
      if ((m_stripeArray[i].getZ() == m_stripeArray[i + 1].getZ()) &&
          (m_stripeArray[i].getY() + 1 == m_stripeArray[i + 1].getY())) {
        for (int k = 0; k < m_stripeArray[i + 1].getSegmentNumber(); ++k) {
          const int *downStairStripe = m_stripeArray[i + 1].getSegment(k);

          if (IS_IN_CLOSE_RANGE(stripe[0], downStairStripe[0] - 1, downStairStripe[1] + 1) ||
              IS_IN_CLOSE_RANGE(downStairStripe[0], stripe[0] - 1, stripe[1] + 1)) {
            int v2 = stripeNumberAccumulation[i + 1] + k;
            graph->addEdgeFast(v1, v2);
            //break;
          }
        }
      }

      //Check along Z

      for (int dy = -1; dy <= 1; ++dy) {
        std::pair<int, int> neighborStripeZY(
              m_stripeArray[i].getZ() + 1, m_stripeArray[i].getY() + dy);
#ifdef _DEBUG_2
        std::cout << "Neighbor stripe position: Y=" << neighborStripeZY.second
                  << ", Z=" << neighborStripeZY.first << endl;
#endif
        if (stripeMap.count(neighborStripeZY) > 0) {
          size_t neighborStripeIndex = stripeMap.at(neighborStripeZY);
          for (int k = 0;
               k < m_stripeArray[neighborStripeIndex].getSegmentNumber(); ++k) {
            const int *downStairStripe =
                m_stripeArray[neighborStripeIndex].getSegment(k);

            if (IS_IN_CLOSE_RANGE(stripe[0], downStairStripe[0] - 1, downStairStripe[1] + 1) ||
                IS_IN_CLOSE_RANGE(downStairStripe[0], stripe[0] - 1, stripe[1] + 1)) {
              int v2 = stripeNumberAccumulation[neighborStripeIndex] + k;
              graph->addEdgeFast(v1, v2);
              //break;
            }
          }
        }
      }
    }
  }

  return graph;
}

void ZObject3dScan::clear()
{
  m_stripeArray.clear();
  m_isCanonized = true;
  deprecate(COMPONENT_ALL);
}

std::vector<size_t> ZObject3dScan::getConnectedObjectSize()
{
  std::vector<size_t> sizeArray;
#if 0
  ZObject3d *obj3d = toObject3d();
  if (obj3d != NULL) {
    Stack *stack = obj3d->toStack();
    C_Stack::translate(stack, GREY16, 1);

    Stack_Label_Objects_N(stack, NULL, 1, 2, 26);

    std::vector<size_t> hist(65535, 0);
    uint16_t *array16 = (uint16_t*) stack->array;
    size_t voxelNumber = C_Stack::voxelNumber(stack);
    for (size_t i = 0; i < voxelNumber; ++i) {
      hist[array16[i]]++;
    }

    for (size_t i = 2; i < 65535; i++) {
      if (hist[i] > 0) {
        sizeArray.push_back(hist[i]);
      }
    }

    delete obj3d;
    C_Stack::kill(stack);
  }
#else
  if (!isEmpty()) {
    std::vector<ZObject3dScan> objArray = getConnectedComponent(ACTION_NONE);

    sizeArray.resize(objArray.size());
    for (size_t i = 0; i < objArray.size(); ++i) {
      sizeArray[i] = objArray[i].getVoxelNumber();
#if _DEBUG_2
      std::cout << "Sub object:" << std::endl;
      objArray[i].print();
#endif
    }
  }
#endif

  if (!sizeArray.empty()) {
    std::sort(sizeArray.begin(), sizeArray.end());
    std::reverse(sizeArray.begin(), sizeArray.end());
  }

  return sizeArray;
}

std::vector<ZObject3dScan> ZObject3dScan::getConnectedComponent(
    EAction ppAction)
{
  std::vector<ZObject3dScan> objArray;

#if 1
  std::cout << "Building connection graph ..." << std::endl;
#endif

  ZGraph *graph = buildConnectionGraph();

#if 1
  std::cout << "Connection graph ready." << std::endl;
#endif

  if (graph != NULL) {
    const std::vector<ZGraph*> &subGraph = graph->getConnectedSubgraph();

    const std::map<size_t, std::pair<size_t, size_t> >&segMap =
        getIndexSegmentMap();

    std::vector<bool> isAdded(segMap.size(), false);

#if 1
    std::cout << "Extracting components ..." << std::endl;
#endif

    size_t index = 0;
    for (std::vector<ZGraph*>::const_iterator iter = subGraph.begin();
         iter != subGraph.end(); ++iter, ++index) {
#if 1
      std::cout << "  " << index + 1 << "/" << subGraph.size() << std::endl;
      std::cout << "  Processing " << (*iter)->getEdgeNumber() << " edges"
                << std::endl;
#endif
      ZObject3dScan subobj;
      for (int edgeIndex = 0; edgeIndex < (*iter)->getEdgeNumber(); ++edgeIndex) {
        int v1 = (*iter)->edgeStart(edgeIndex);
        int v2 = (*iter)->edgeEnd(edgeIndex);
        int z, y, x1, x2;
        getSegment(v1, &z, &y, &x1, &x2);
        subobj.addSegment(z, y, x1, x2, false);
        getSegment(v2, &z, &y, &x1, &x2);
        subobj.addSegment(z, y, x1, x2, false);
        isAdded[v1] = true;
        isAdded[v2] = true;
      }
#if 1
      std::cout << "  Edge processing done." << std::endl;
#endif
      switch (ppAction) {
      case ACTION_CANONIZE:
        std::cout << "  Canonizing ..." << std::endl;
        subobj.canonize();
        break;
      case ACTION_SORT_YZ:
        std::cout << "  Sorting ..." << std::endl;
        subobj.sort();
        break;
      default:
        break;
      }

//      subobj.sort();
//      subobj.setCanonized(true);
//      TZ_ASSERT(subobj.isCanonizedActually(), "Inconsisten data assumption");
//      subobj.canonize();

      objArray.push_back(subobj);
    }

#if 1
    std::cout << objArray.size() << " components extracted." << std::endl;
#endif

    delete graph;

#if 1
    std::cout << "Checking remains ..." << std::endl;
#endif
    for (size_t i = 0; i < isAdded.size(); ++i) {
      if (!isAdded[i]) {
        ZObject3dScan subobj;
        int z, y, x1, x2;
        getSegment(i, &z, &y, &x1, &x2);
        subobj.addSegment(z, y, x1, x2);
        objArray.push_back(subobj);
      }
    }

#if 1
    std::cout << objArray.size() << " components extracted." << std::endl;
#endif
  }

  for (std::vector<ZObject3dScan>::iterator iter = objArray.begin();
       iter != objArray.end(); ++iter) {
    ZObject3dScan &obj = *iter;
    obj.setSliceAxis(m_sliceAxis);
  }

  return objArray;
}

size_t ZObject3dScan::getSegmentNumber() const
{
  const std::vector<size_t>& accArray = getStripeNumberAccumulation();

  return accArray.back();
}

void ZObject3dScan::translate(int dx, int dy, int dz)
{
  ZGeometry::shiftSliceAxis(dx, dy, dz, m_sliceAxis);

  for (size_t i = 0; i < getStripeNumber(); ++i) {
    m_stripeArray[i].translate(dx, dy, dz);
  }

  processEvent(EVENT_OBJECT_MODEL_CHANGED);
}

void ZObject3dScan::translate(const ZIntPoint &dp)
{
  translate(dp.getX(), dp.getY(), dp.getZ());
}

/*
void ZObject3dScan::addZ(int dz)
{
  for (size_t i = 0; i < getStripeNumber(); ++i) {
    m_stripeArray[i].addZ(dz);
  }

  processEvent(EVENT_OBJECT_MODEL_CHANGED);
}
*/

bool ZObject3dScan::isCanonizedActually()
{
  for (size_t i = 0; i < getStripeNumber(); ++i) {
    if (i > 0) {
      if (m_stripeArray[i - 1].getZ() > m_stripeArray[i].getZ()) {
#ifdef _DEBUG_
        std::cout << "Previous Z is bigger: " << m_stripeArray[i - 1].getZ()
                  << " " << m_stripeArray[i].getZ() << std::endl;
#endif
        return false;
      } else if (m_stripeArray[i - 1].getZ() == m_stripeArray[i].getZ()) {
        if (m_stripeArray[i - 1].getY() >= m_stripeArray[i].getY()) {
#ifdef _DEBUG_
          std::cout << "Previous Z/Y is bigger: " << m_stripeArray[i - 1].getY()
                    << " " << m_stripeArray[i].getY() << std::endl;
#endif
          return false;
        }
      }
    }
    if (!m_stripeArray[i].isCanonizedActually()) {
      return false;
    }
  }

  return true;
}

void ZObject3dScan::duplicateSlice(int depth)
{
  for (size_t i = 0; i < getStripeNumber(); ++i) {
    m_stripeArray[i].setZ(0);
  }
  setCanonized(false);
  canonize();

  size_t orignalStripeNumber = m_stripeArray.size();

  for (int z = 1; z < depth; ++z) {
    for (size_t i = 0; i < orignalStripeNumber; ++i) {
      ZObject3dStripe stripe = m_stripeArray[i];
      stripe.setZ(z);
      addStripe(stripe, false);
    }
  }
  //deprecate(ALL_COMPONENT);
  processEvent(EVENT_OBJECT_MODEL_CHANGED);
}

void ZObject3dScan::displaySolid(
    ZPainter &painter, int z, bool isProj, int stride) const
{
#ifdef _QT_GUI_USED_
  if (stride < 1) {
    stride = 1;
  }
  std::vector<QLine> lineArray;

  ZObject3dScan slice;
//  bool painted = false;
  if (isProj) {
    slice = *getZProjection();
  } else {
    slice = getSlice(z);
  }

  size_t stripeNumber = slice.getStripeNumber();
  //  std::vector<QPoint> pointArray(slice.getVoxelNumber());
  //  size_t pointIndex = 0;

  //  std::vector<QPoint> pointArray;

  //  size_t lineIndex = 0;
  //int offsetX = iround(painter.getOffset().x());
  //int offsetY = iround(painter.getOffset().y());
  for (size_t i = 0; i < stripeNumber; i += stride) {
    const ZObject3dStripe &stripe = slice.getStripe(i);
    if (stripe.getZ() == z || isProj) {
      int nseg = stripe.getSegmentNumber();
      for (int j = 0; j < nseg; ++j) {
        int x0 = stripe.getSegmentStart(j);// - offsetX;
        int x1 = stripe.getSegmentEnd(j);// - offsetX;
        int y = stripe.getY();// - offsetY;

        //        lineArray[lineIndex++] = QLine(x0, y, x1, y);
        //        for (int x = x0; x <= x1; ++x) {
        //          pointArray.push_back(QPoint(x, y));
        //          pointArray[pointIndex++] = QPoint(x, y);
        //        }
        lineArray.push_back(QLine(x0, y, x1, y));
      }
    }
  }


  painter.drawLines(lineArray);
#endif

//  if (!lineArray.empty()) {
//  painter.drawPoints(pointArray);
//    painter.drawLines(&(lineArray[0]), lineArray.size());
//    painted = true;
//  }

//  return painted;
}

void ZObject3dScan::display(ZPainter &painter, int slice, EDisplayStyle style,
                            NeuTube::EAxis sliceAxis) const
{
#if _QT_GUI_USED_
//  if (isSelected() && style == ZStackObject::SOLID) {
//    return;
//  }

//  bool painted = false;

  if (sliceAxis != m_sliceAxis || getColor().alpha() == 0) {
    return;
  }

  bool isProj = (slice < 0);

  if (isProj) {
    if (!isProjectionVisible()) {
      return;
    }
  }

  int z = slice + iround(painter.getZOffset());

  QPen pen(m_color);

  if (hasVisualEffect(NeuTube::Display::SparseObject::VE_FORCE_SOLID)) {
    style = ZStackObject::SOLID;
  }
  //QImage *targetImage = dynamic_cast<QImage*>(painter.device());

  switch (style) {
  case ZStackObject::SOLID:
  {
    if (isSelected()) {
//      QColor color = pen.color();
      QColor color(Qt::white);
      color.setAlpha(164);
      pen.setColor(color);
//      pen.setCosmetic(true);
//      pen.setStyle(Qt::DotLine);
    }

    if (pen.color().alpha() > 0) {
      painter.setPen(pen);
      if (isSelected()) {
        displaySolid(painter, z, isProj, 1);
      } else {
        displaySolid(painter, z, isProj, 1);
      }
    }
  }
    break;
  case ZStackObject::BOUNDARY:
  {
    QColor color = pen.color();
//    color.setAlpha(255);
    pen.setColor(color);
    painter.setPen(pen);

    if (isSelected()) {
      displaySolid(painter, z, isProj, 5);
    } else {
      std::vector<QPoint> ptArray;
      ZObject3dScan slice = getSlice(z);

      if (!slice.isEmpty()) {
        ZStack *stack = slice.toStackObject();
        int width = stack->width();
        int height = stack->height();
        int conn = 4;
        Stack *pre = Stack_Perimeter(stack->c_stack(), NULL, conn);
        size_t offset = 0;
        for (int y = 0; y < height; ++y) {
          for (int x = 0; x < width; ++x) {
            if (pre->array[offset++] > 0) {
              ptArray.push_back(QPoint(x + stack->getOffset().getX(),
                                       y + stack->getOffset().getY()));
            }
          }
        }
      }
      if (!ptArray.empty()) {
        painter.drawPoints(&(ptArray[0]), ptArray.size());
//        painted = true;
      }
    }
  }
    break;
  default:
    break;
  }


//  return painted;

#else
  UNUSED_PARAMETER(&painter);
  UNUSED_PARAMETER(slice);
  UNUSED_PARAMETER(style);
#endif
}

void ZObject3dScan::dilatePlane()
{
  size_t oldStripeNumber = getStripeNumber();
  for (size_t i = 0; i < oldStripeNumber; ++i) {
    ZObject3dStripe baseStripe = m_stripeArray[i];
    ZObject3dStripe stripe = baseStripe;
    stripe.setY(stripe.getY() - 1);
    m_stripeArray.push_back(stripe);
    //addStripe(stripe, false);
    stripe = baseStripe;
    stripe.setY(stripe.getY() + 1);
    m_stripeArray.push_back(stripe);
    //addStripe(stripe, false);
//    stripe = baseStripe;
//    stripe.setZ(stripe.getZ() - 1);
//    m_stripeArray.push_back(stripe);
//    //addStripe(stripe, false);
//    stripe = baseStripe;
//    stripe.setZ(stripe.getZ() + 1);
//    m_stripeArray.push_back(stripe);
    //addStripe(stripe, false);
    m_stripeArray[i].dilate();
  }

  setCanonized(false);
  canonize();

  processEvent(EVENT_OBJECT_MODEL_CHANGED);
}

void ZObject3dScan::dilate()
{
  size_t oldStripeNumber = getStripeNumber();
  for (size_t i = 0; i < oldStripeNumber; ++i) {
    ZObject3dStripe baseStripe = m_stripeArray[i];
    ZObject3dStripe stripe = baseStripe;
    stripe.setY(stripe.getY() - 1);
    m_stripeArray.push_back(stripe);
    //addStripe(stripe, false);
    stripe = baseStripe;
    stripe.setY(stripe.getY() + 1);
    m_stripeArray.push_back(stripe);
    //addStripe(stripe, false);
    stripe = baseStripe;
    stripe.setZ(stripe.getZ() - 1);
    m_stripeArray.push_back(stripe);
    //addStripe(stripe, false);
    stripe = baseStripe;
    stripe.setZ(stripe.getZ() + 1);
    m_stripeArray.push_back(stripe);
    //addStripe(stripe, false);
    m_stripeArray[i].dilate();
  }

  setCanonized(false);
  canonize();

  processEvent(EVENT_OBJECT_MODEL_CHANGED);
}

ZObject3dScan ZObject3dScan::interpolateSlice(int z) const
{
  ZObject3dScan slice;
  if (!isEmpty()) {
    const_cast<ZObject3dScan&>(*this).canonize();
    int minZ = getMinZ();
    int maxZ = getMaxZ();
    if (z <= minZ) {
      slice = getSlice(minZ);
    } else if (z >= maxZ) {
      slice = getSlice(maxZ);
    } else {
      int z0 = minZ;
      int z1 = minZ;
      int stripeNumber = getStripeNumber();
      for (int i = 0; i < stripeNumber; ++i) {
        const ZObject3dStripe &stripe = getStripe(i);
        if (z < stripe.getZ()) {
          z1 = stripe.getZ();
          break;
        } else {
          z0 = stripe.getZ();
        }
      }

      if (z0 == z) {
        slice = getSlice(z);
      } else {
        ZObject3dScan slice1 = getSlice(z0);
        ZObject3dScan slice2 = getSlice(z1);

        ZIntPoint c1 = slice1.getCentroid().toIntPoint();
        ZIntPoint c2 = slice2.getCentroid().toIntPoint();
        slice2.translate(c1.getX() - c2.getX(), c1.getY() - c2.getY(), 0);

        {
          int stripeNumber = slice1.getStripeNumber();
          for (int i = 0; i < stripeNumber; ++i) {
            ZObject3dStripe &stripe = slice1.getStripe(i);
            stripe.setZ(0);
          }
        }

        {
          int stripeNumber = slice2.getStripeNumber();
          for (int i = 0; i < stripeNumber; ++i) {
            ZObject3dStripe &stripe = slice2.getStripe(i);
            stripe.setZ(0);
          }
        }


        ZIntCuboid box = slice1.getBoundBox().join(slice2.getBoundBox());
        box.expandX(1);
        box.expandY(1);
//        box.setFirstX(box.getFirstCorner().getX() - 1);
//        box.setLastX(box.getLastCorner().getX() + 1);

        ZStack *stack1 = ZStackFactory::makeZeroStack(GREY, box);
        ZStack *stack2 = ZStackFactory::makeZeroStack(GREY, box);

        ZStack *negStack1 = ZStackFactory::makeZeroStack(GREY, box);
        negStack1->setOne();
        ZStack *negStack2 = ZStackFactory::makeZeroStack(GREY, box);
        negStack2->setOne();


        slice1.drawStack(stack1, 1);
        slice2.drawStack(stack2, 1);

        slice1.drawStack(negStack1, 0);
        slice2.drawStack(negStack2, 0);

        Stack *dist1 = Stack_Bwdist_L_P(stack1->c_stack(0), NULL, NULL);
        Stack *dist2 = Stack_Bwdist_L_P(stack2->c_stack(0), NULL, NULL);

        Stack *negDist1 = Stack_Bwdist_L_P(negStack1->c_stack(0), NULL, NULL);
        Stack *negDist2 = Stack_Bwdist_L_P(negStack2->c_stack(0), NULL, NULL);


        size_t voxelNumber = stack1->getVoxelNumber();
        float *array1 = (float*) C_Stack::array8(dist1);
        float *array2 = (float*) C_Stack::array8(dist2);

        float *negArray1 = (float*) C_Stack::array8(negDist1);
        float *negArray2 = (float*) C_Stack::array8(negDist2);

        float beta = (float) (z - z0) / (z1 - z0);
        float alpha = 1.0 - beta;


        ZStack *newStack = ZStackFactory::makeZeroStack(GREY, box);
        uint8_t *outArray = newStack->array8(0);

        for (size_t i = 0; i < voxelNumber; ++i) {
          if (array1[i] > 0.0 && array2[i] > 0.0) {
            outArray[i] = 1;
          } else if (array1[i] <= 0.0 && array2[i] <= 0.0) {
            outArray[i] = 0;
          } else if (array1[i] > 0.0) {
            outArray[i] = ((array1[i] * alpha - negArray2[i] * beta) > 0.0);
          } else {
            outArray[i] = ((array2[i] * beta - negArray1[i] * alpha) > 0.0);
          }
        }

        slice.loadStack(*newStack);

        slice.translate(iround((c2.getX() - c1.getX()) * beta),
                        iround((c2.getY() - c1.getY()) * beta), 0 );

        delete stack1;
        delete stack2;
        delete negStack1;
        delete negStack2;
        delete newStack;

        C_Stack::kill(negDist1);
        C_Stack::kill(negDist2);
        C_Stack::kill(dist1);
        C_Stack::kill(dist2);
      }
    }

    int stripeNumber = slice.getStripeNumber();
    for (int i = 0; i < stripeNumber; ++i) {
      ZObject3dStripe &stripe = slice.getStripe(i);
      stripe.setZ(z);
    }
  }

  return slice;
}

ZObject3dScan ZObject3dScan::getFirstSlice() const
{
  return getSlice(getMinZ());
}

ZObject3dScan ZObject3dScan::getMedianSlice() const
{
  std::set<int> zSet;
  for (size_t i = 0; i < getStripeNumber(); ++i) {
    const ZObject3dStripe &stripe = m_stripeArray[i];
    zSet.insert(stripe.getZ());
  }

  size_t count = zSet.size();
  size_t index = 0;
  int z = getMinZ();
  for (std::set<int>::const_iterator iter = zSet.begin(); iter != zSet.end();
       ++iter, ++index) {
    if (index >= count / 2) {
      z = *iter;
      break;
    }
  }

  return getSlice(z);
}

ZObject3dScan ZObject3dScan::getSlice(int z) const
{
  ZObject3dScan slice;

  if (!isEmpty()) {
    const_cast<ZObject3dScan&>(*this).canonize();

    int stripeNumber = getStripeNumber();

    int pi = stripeNumber / 2;
    int pz = m_stripeArray[pi].getZ();

    int minIndex = 0;
    int maxIndex = stripeNumber - 1;

    if (z == m_stripeArray[minIndex].getZ() &&
        z == m_stripeArray[maxIndex].getZ()) {
      return *this;
    }

    while (z != pz) {
      if (z < pz) {
        maxIndex = pi;
      } else {
        minIndex = pi;
      }

      pi = (minIndex + maxIndex) / 2;
      pz = m_stripeArray[pi].getZ();

      if (minIndex == pi) {
        if (minIndex == maxIndex) {
          break;
        } else {
          if (z != pz) {
            pz = m_stripeArray[maxIndex].getZ();
            pi = maxIndex;
            break;
          }
        }
      }
    }

    if (z == pz) {
      //slice.addStripe(m_stripeArray[pi]);
      int index = pi;
      int startIndex = pi;
      while (--index >= 0) {
        pz = m_stripeArray[index].getZ();
        if (z == pz) {
          startIndex = index;
        } else {
          break;
        }
      }

      for (index = startIndex; index <= pi; ++index) {
        slice.addStripeFast(m_stripeArray[index]);
      }

      index = pi;
      while (++index < stripeNumber) {
        pz = m_stripeArray[index].getZ();
        if (z == pz) {
          slice.addStripe(m_stripeArray[index]);
        } else {
          break;
        }
      }
    }

#ifdef _DEBUG_2
  ZObject3dScan testSlice;
  for (int i = 0; i < stripeNumber; ++i) {
    const ZObject3dStripe &stripe = m_stripeArray[i];
    if (stripe.getZ() == z) {
      testSlice.addStripe(stripe, false);
    }
  }

  TZ_ASSERT(slice.getVoxelNumber() == testSlice.getVoxelNumber(), "Bug?");
  TZ_ASSERT(slice.getStripeNumber() == testSlice.getStripeNumber(), "Bug?");
  TZ_ASSERT(slice.getSegmentNumber() == testSlice.getSegmentNumber(), "Bug?");
#endif
  }

  slice.setCanonized(true);

  return slice;
}

ZObject3dScan ZObject3dScan::getSlice(int minZ, int maxZ) const
{
  ZObject3dScan slice;
  for (size_t i = 0; i < getStripeNumber(); ++i) {
    const ZObject3dStripe &stripe = m_stripeArray[i];
    if (stripe.getZ() >= minZ) {
      if (stripe.getZ() <= maxZ) {
        slice.addStripe(stripe, false);
      }
    }
  }

  if (isCanonized()) {
    slice.setCanonized(true);
  }

  return slice;
}

bool ZObject3dScan::hit(double x, double y, double z)
{
  if (!isHittable()) {
    return false;
  }

  m_hitPoint.set(0, 0, 0);
  int tx = iround(x);
  int ty = iround(y);
  int tz = iround(z);
  ZGeometry::shiftSliceAxis(tx, ty, tz, m_sliceAxis);
  for (size_t i = 0; i < getStripeNumber(); ++i) {
    const ZObject3dStripe &stripe = m_stripeArray[i];
    if (stripe.contains(tx, ty, tz)) {
      m_hitPoint.set(iround(x), iround(y), iround(z));
      return true;
    }
  }

  return false;
}

bool ZObject3dScan::hit(double x, double y, NeuTube::EAxis axis)
{
  if (!isHittable() || m_sliceAxis != axis) {
    return false;
  }

  m_hitPoint.set(0, 0, 0);
  for (size_t i = 0; i < getStripeNumber(); ++i) {
    const ZObject3dStripe &stripe = m_stripeArray[i];
    if (stripe.contains(iround(x), iround(y), stripe.getZ())) {
      m_hitPoint.set(iround(x), iround(y), stripe.getZ());
      return true;
    }
  }

  return false;
}

/*
ZIntPoint ZObject3dScan::getHitPoint() const
{
  return m_hitPoint;
}
*/

ZPoint ZObject3dScan::getCentroid() const
{
  ZPoint center(0, 0, 0);
  int count = 0;
  for (size_t i = 0; i < getStripeNumber(); ++i) {
    int nseg = m_stripeArray[i].getSegmentNumber();
    for (int j = 0; j < nseg; ++j) {
      int x1 = m_stripeArray[i].getSegmentStart(j);
      int x2 = m_stripeArray[i].getSegmentEnd(j);
      int len = x2 - x1 + 1;
      count += len;
      center.translate((x1 + x2) * len /2, m_stripeArray[i].getY() * len,
                       m_stripeArray[i].getZ() * len);
    }
  }

  if (count > 0) {
    center /= count;
  }

  center.shiftSliceAxis(m_sliceAxis);

  return center;
}

ZObject3dScan ZObject3dScan::makeZProjection(int minZ, int maxZ)
{
  return getSlice(minZ, maxZ).makeZProjection();
}

ZHistogram ZObject3dScan::getRadialHistogram(int z) const
{
  ZHistogram hist;
  hist.setInterval(1.0);
  hist.setStart(0.0);

  ZObject3dScan slice = getSlice(z);

  ZPoint center = slice.getCentroid();
#ifdef _DEBUG_2
  std::cout << center.toString() << std::endl;
#endif
  for (size_t i = 0; i < slice.getStripeNumber(); ++i) {
    int nseg = slice.m_stripeArray[i].getSegmentNumber();
    int y = slice.m_stripeArray[i].getY();
    for (int j = 0; j < nseg; ++j) {
      int x1 = slice.m_stripeArray[i].getSegmentStart(j);
      int x2 = slice.m_stripeArray[i].getSegmentEnd(j);
      for (int x = x1; x <= x2; ++x) {
        double dist = center.distanceTo(x, y, z);
        hist.addCount(dist, 1.0 / (dist + 0.5));
      }
    }
  }

  return hist;
}

const ZObject3dScan *ZObject3dScan::getZProjection() const
{
  if (isDeprecated(COMPONENT_Z_PROJECTION)) {
    m_zProjection = new ZObject3dScan;
    makeZProjection(m_zProjection);
  }

  return m_zProjection;
}

void ZObject3dScan::makeZProjection(ZObject3dScan *obj) const
{
  if (obj != NULL) {
    obj->clear();
    for (size_t i = 0; i < getStripeNumber(); ++i) {
      obj->addStripe(0, m_stripeArray[i].getY(), false);
      int nseg = m_stripeArray[i].getSegmentNumber();
      for (int j = 0; j < nseg; ++j) {
        int x1 = m_stripeArray[i].getSegmentStart(j);
        int x2 = m_stripeArray[i].getSegmentEnd(j);
        obj->addSegment(x1, x2, false);
      }
    }

    obj->canonize();
  }
}

ZObject3dScan ZObject3dScan::makeZProjection() const
{
  ZObject3dScan proj;

  makeZProjection(&proj);

  return proj;
}

ZObject3dScan ZObject3dScan::makeYProjection() const
{
  ZObject3dScan proj;
  for (size_t i = 0; i < getStripeNumber(); ++i) {
    proj.addStripe(0, m_stripeArray[i].getZ(), false);
    int nseg = m_stripeArray[i].getSegmentNumber();
    for (int j = 0; j < nseg; ++j) {
      int x1 = m_stripeArray[i].getSegmentStart(j);
      int x2 = m_stripeArray[i].getSegmentEnd(j);
      proj.addSegment(x1, x2, false);
    }
  }

  proj.canonize();

  return proj;
}


int ZObject3dScan::getMinZ() const
{
  int minZ = 0;
  if (isCanonized()) {
    if (!isEmpty()) {
      minZ = m_stripeArray[0].getZ();
    }
  } else {
    for (size_t i = 0; i < getStripeNumber(); ++i) {
      int z = m_stripeArray[i].getZ();
      if (i == 0) {
        minZ = z;
      } else {
        if (z < minZ) {
          minZ = z;
        }
      }
    }
  }

  return minZ;
}

int ZObject3dScan::getMaxZ() const
{
  int maxZ = 0;
  if (isCanonized()) {
    if (!isEmpty()) {
      maxZ = m_stripeArray.back().getZ();
    }
  } else {
    for (size_t i = 0; i < getStripeNumber(); ++i) {
      int z = m_stripeArray[i].getZ();
      if (i == 0) {
        maxZ = z;
      } else {
        if (z > maxZ) {
          maxZ = z;
        }
      }
    }
  }

  return maxZ;
}

int ZObject3dScan::getMinY() const
{
  int minY = 0;
  for (size_t i = 0; i < getStripeNumber(); ++i) {
    int y = m_stripeArray[i].getY();
    if (i == 0) {
      minY = y;
    } else {
      if (y < minY) {
        minY = y;
      }
    }
  }

  return minY;
}

int ZObject3dScan::getMaxY() const
{
  int maxY = 0;
  for (size_t i = 0; i < getStripeNumber(); ++i) {
    int y = m_stripeArray[i].getY();
    if (i == 0) {
      maxY = y;
    } else {
      if (y > maxY) {
        maxY = y;
      }
    }
  }

  return maxY;
}

ZVoxel ZObject3dScan::getMarker() const
{
  if (isEmpty()) {
    return ZVoxel(-1, -1, -1);
  }

#if 1
  int offset[3] = {0, 0, 0};
  ZVoxel voxel;
  size_t stripeNumber = getStripeNumber();
  std::vector<ZObject3dScan> sliceArray(getMaxZ() + 1);
  std::set<size_t> sliceIndexSet;
  for (size_t i = 0; i < stripeNumber; ++i) {
    const ZObject3dStripe &stripe = getStripe(i);
    int currentZ = stripe.getZ();
    sliceArray[currentZ].addStripe(stripe, false);
    sliceIndexSet.insert(currentZ);
  }

  std::vector<size_t> sliceIndexArray;
  sliceIndexArray.insert(sliceIndexArray.end(), sliceIndexSet.begin(),
                         sliceIndexSet.end());

  size_t step = sliceIndexArray.size() / 100 + 1;

  for (size_t i = 0; i < sliceIndexArray.size(); i += step) {
    ZObject3dScan &slice = sliceArray[sliceIndexArray[i]];

    if (!slice.isEmpty()) {
      int currentZ = slice.getStripe(0).getZ();
      Stack *stack = slice.toStack(offset);
      Stack *dist = Stack_Bwdist_L_U16P(stack, NULL, 0);
      size_t index;
      double v = sqrt(Stack_Max(dist, &index));
      if (v > voxel.value()) {
        int x, y, tmpz;
        C_Stack::indexToCoord(index, C_Stack::width(stack), C_Stack::height(stack),
                              &x, &y, &tmpz);

        voxel.set(x + offset[0], y + offset[1], currentZ, v);
      }
      Kill_Stack(dist);
      Kill_Stack(stack);
      slice.clear();
    }
  }
#else
  int minZ = getMinZ();
  int maxZ = getMaxZ();
  int offset[3];
  ZVoxel voxel;
  int zStep = 2;
  for (int z = minZ; z < maxZ; z += zStep) {
    ZObject3dScan slice = getSlice(z);
    if (!slice.isEmpty()) {
      Stack *stack = slice.toStack(offset);
      Stack *dist = Stack_Bwdist_L_U16P(stack, NULL, 0);
      size_t index;
      double v = sqrt(Stack_Max(dist, &index));
      if (v > voxel.value()) {
        int x, y, tmpz;
        C_Stack::indexToCoord(index, C_Stack::width(stack), C_Stack::height(stack),
                              &x, &y, &tmpz);

        voxel.set(x + offset[0], y + offset[1], z, v);
      }
      Kill_Stack(dist);
      Kill_Stack(stack);
    }
  }
#endif

  voxel.shiftSliceAxis(m_sliceAxis);

  return voxel;
}

bool ZObject3dScan::equalsLiterally(const ZObject3dScan &obj) const
{
  if (m_sliceAxis != obj.m_sliceAxis) {
    return false;
  }

  if (m_stripeArray.size() != obj.m_stripeArray.size()) {
    return false;
  }

  for (size_t i = 0; i < m_stripeArray.size(); ++i) {
    if (!m_stripeArray[i].equalsLiterally(obj.m_stripeArray[i])) {
      return false;
    }
  }

  return true;
}

ZObject3dScan ZObject3dScan::getComplementObject()
{
  ZObject3dScan compObj;

  int offset[3];
  Stack *stack = toStack(offset);
  Stack_Not(stack, stack);
  compObj.loadStack(stack);
  compObj.translate(offset[0], offset[1], offset[2]);

  C_Stack::kill(stack);

  return compObj;
}

ZObject3dScan ZObject3dScan::getSurfaceObject() const
{
  int offset[3];
  Stack *stack = toStack(offset);

  Stack *surface = Stack_Perimeter(stack, NULL, 6);
  ZObject3dScan surfaceObj;
  surfaceObj.loadStack(surface);
  surfaceObj.translate(offset[0], offset[1], offset[2]);
  C_Stack::kill(stack);
  C_Stack::kill(surface);

  surfaceObj.setSliceAxis(m_sliceAxis);

  return surfaceObj;
}

ZObject3dScan ZObject3dScan::findHoleObject()
{
  ZObject3dScan obj;

  ZObject3dScan compObj = getComplementObject();
  std::vector<ZObject3dScan> objList =
      compObj.getConnectedComponent(ACTION_CANONIZE);

  Cuboid_I boundBox;
  getBoundBox(&boundBox);
  for (std::vector<ZObject3dScan>::iterator iter = objList.begin();
       iter != objList.end(); ++iter) {
    ZObject3dScan &subobj = *iter;
#ifdef _DEBUG_2
    subobj.print();
#endif
    Cuboid_I subbox;
    subobj.getBoundBox(&subbox);
    if (Cuboid_I_Hit_Internal(&boundBox, subbox.cb[0], subbox.cb[1], subbox.cb[2]) &&
        Cuboid_I_Hit_Internal(&boundBox, subbox.ce[0], subbox.ce[1], subbox.ce[2])) {
      obj.unify(subobj);
    }
  }

  obj.setSliceAxis(m_sliceAxis);

  return obj;
}

std::vector<ZObject3dScan> ZObject3dScan::findHoleObjectArray()
{
  std::vector<ZObject3dScan> objArray;

  ZObject3dScan compObj = getComplementObject();
  std::vector<ZObject3dScan> objList =
      compObj.getConnectedComponent(ACTION_NONE);

  Cuboid_I boundBox;
  getBoundBox(&boundBox);
  for (std::vector<ZObject3dScan>::iterator iter = objList.begin();
       iter != objList.end(); ++iter) {
    ZObject3dScan &subobj = *iter;
    Cuboid_I subbox;
    subobj.getBoundBox(&subbox);
    if (Cuboid_I_Hit_Internal(&boundBox, subbox.cb[0], subbox.cb[1], subbox.cb[2]) &&
        Cuboid_I_Hit_Internal(&boundBox, subbox.ce[0], subbox.ce[1], subbox.ce[2])) {
      subobj.canonize();
      objArray.push_back(subobj);
    }
  }

  return objArray;
}

void ZObject3dScan::fillHole()
{
  ZObject3dScan holeObj = findHoleObject();
  unify(holeObj);
}

#define READ_TEST(real, expected, action) \
  if (real != expected) { \
    action; \
    fclose(fp); \
    return false; \
  }

void ZObject3dScan::exportDvidObject(const string &filePath) const
{
  FILE *fp = fopen(filePath.c_str(), "w");

  tz_uint8 flag = 0;
  fwrite(&flag, 1, 1, fp);

  tz_uint8 numberOfDimensions = 3;
  fwrite(&numberOfDimensions, 1, 1, fp);

  tz_uint8 dimOfRun = 0;
  fwrite(&dimOfRun, 1, 1, fp);

  tz_uint8 reserved = 0;
  fwrite(&reserved, 1, 1, fp);

  tz_uint32 numberOfVoxels = getVoxelNumber();
  fwrite(&numberOfVoxels, 4, 1, fp);

  tz_uint32 numberOfSpans = getSegmentNumber();
  fwrite(&numberOfSpans, 4, 1, fp);

  //For each segment
  ConstSegmentIterator iter(this);
  while (iter.hasNext()) {
    const ZObject3dScan::Segment &seg = iter.next();
    tz_int32 coord[3];
    coord[0] = seg.getStart();
    coord[1] = seg.getY();
    coord[2] = seg.getZ();
    fwrite(coord, 4, 3, fp);

    tz_int32 runLength = seg.getEnd() - seg.getStart() + 1;
    fwrite(&runLength, 4, 1, fp);
  }

  fclose(fp);
}

#if _QT_GUI_USED_
QByteArray ZObject3dScan::toDvidPayload() const
{
  QByteArray buffer;

  tz_uint8 flag = 0;
  buffer.append((char*) (&flag), 1);

  tz_uint8 numberOfDimensions = 3;
  buffer.append((char*) (&numberOfDimensions), 1);

  tz_uint8 dimOfRun = 0;
  buffer.append((char*) (&dimOfRun), 1);

  tz_uint8 reserved = 0;
  buffer.append((char*) (&reserved), 1);

  tz_uint32 numberOfVoxels = getVoxelNumber();
  buffer.append((char*) (&numberOfVoxels), 4);

  tz_uint32 numberOfSpans = getSegmentNumber();
  buffer.append((char*) (&numberOfSpans), 4);

  //For each segment
  ConstSegmentIterator iter(this);
  while (iter.hasNext()) {
    const ZObject3dScan::Segment &seg = iter.next();
    tz_int32 coord[3];
    coord[0] = seg.getStart();
    coord[1] = seg.getY();
    coord[2] = seg.getZ();
    buffer.append((char*) (coord), 12);
//    fwrite(coord, 4, 3, fp);

    tz_int32 runLength = seg.getEnd() - seg.getStart() + 1;
//    fwrite(&runLength, 4, 1, fp);
    buffer.append((char*) (&runLength), 4);
  }

  return buffer;
//  fclose(fp);
}
#endif

bool ZObject3dScan::importDvidObject(const std::string &filePath)
{
  clear();

  FILE *fp = fopen(filePath.c_str(), "r");
  if (fp != NULL) {
    tz_uint8 flag = 0;
    size_t n = fread(&flag, 1, 1, fp);
    READ_TEST(n, 1, RECORD_ERROR_UNCOND("Reading error"));

    tz_uint8 numberOfDimensions = 0;
    n = fread(&numberOfDimensions, 1, 1, fp);
    if (n != 1) {
      fclose(fp);
      return false;
    }

    if (numberOfDimensions != 3) {
      RECORD_ERROR_UNCOND("Current version only supports 3D");
      fclose(fp);
      return false;
    }

    tz_uint8 dimOfRun = 0;
    n = fread(&dimOfRun, 1, 1, fp);
    if (n != 1) {
      fclose(fp);
      return false;
    }

    if (dimOfRun != 0) {
      RECORD_ERROR_UNCOND("Unspported run dimension");
      fclose(fp);
      return false;
    }

    tz_uint8 reserved = 0;
    n = fread(&reserved, 1, 1, fp);
    if (n != 1) {
      fclose(fp);
      return false;
    }

    tz_uint32 numberOfVoxels = 0;
    n = fread(&numberOfVoxels, 4, 1, fp);

#ifdef _DEBUG_
    std::cout << "#dvid voxels: " << numberOfVoxels << std::endl;
#endif

    if (n != 1) {
      fclose(fp);
      return false;
    }

    tz_uint32 numberOfSpans = 0;
    n = fread(&numberOfSpans, 4, 1, fp);
    READ_TEST(n, 1, RECORD_ERROR_UNCOND("Failed to read number of spans"));

    tz_uint32 readSegmentNumber = 0;
    while (!feof(fp)) {
      tz_int32 coord[3];
      n = fread(coord, 4, 3, fp);
      READ_TEST(n, 3, RECORD_ERROR_UNCOND("Failed to read span"));

      if (feof(fp)) {
        RECORD_ERROR_UNCOND("Reading error. File ends prematurely")
      }

#ifdef _DEBUG_
      if (coord[1] == 6525 && coord[2] ==1437) {
        std::cout << "debug here" << std::endl;
      }
#endif

      tz_int32 runLength;
      n = fread(&runLength, 4, 1, fp);
      READ_TEST(n, 1, RECORD_ERROR_UNCOND("Failed to read run length"));
      if (runLength <= 0) {
        fclose(fp);
        RECORD_ERROR_UNCOND("Invalid run length");
        return false;
      }

      addSegment(coord[2], coord[1], coord[0], coord[0] + runLength - 1, false);
      ++readSegmentNumber;
      if (readSegmentNumber == numberOfSpans) {
        break;
      }
    }

    if (readSegmentNumber != numberOfSpans) {
      RECORD_ERROR_UNCOND("Unmatched numbers of spans");
      fclose(fp);
      return false;
    }
  } else {
    RECORD_ERROR_UNCOND("Unable to open file");
    return false;
  }

  fclose(fp);
  return true;
}

#define READ_BYTE_BUFFER(target, type) \
  if (byteNumber < sizeof(type)) { \
    RECORD_ERROR_UNCOND("Buffer ended prematurely."); \
    return false; \
  } \
  target = *(const type*)(byteArray + currentIndex); \
  currentIndex += sizeof(type); \
  byteNumber -= sizeof(type);

bool ZObject3dScan::importDvidObjectBuffer(
    const char *byteArray, size_t byteNumber)
{
  clear();

  if (byteArray == NULL || byteNumber <= 12) {
    RECORD_ERROR_UNCOND("Invalid byte buffer");
    return false;
  }

  size_t currentIndex = 0;
  tz_uint8 flag = 0;
  READ_BYTE_BUFFER(flag, tz_uint8);


  tz_uint8 numberOfDimensions = 0;
  READ_BYTE_BUFFER(numberOfDimensions, tz_uint8);

  if (numberOfDimensions != 3) {
    RECORD_ERROR_UNCOND("Current version only supports 3D");
    return false;
  }

  tz_uint8 dimOfRun = 0;
  READ_BYTE_BUFFER(dimOfRun, tz_uint8);
  if (dimOfRun != 0) {
    RECORD_ERROR_UNCOND("Unspported run dimension");
    return false;
  }

  //tz_uint8 reserved = *static_cast<tz_uint8*>(byteArray + currentIndex);
  ++currentIndex;

  //tz_uint32 numberOfVoxels = *static_cast<tz_uint32*>(byteArray + currentIndex);
  currentIndex += 4;

  tz_uint32 numberOfSpans = 0;
  READ_BYTE_BUFFER(numberOfSpans, tz_uint32);

  for (tz_uint32 span = 0; span < numberOfSpans; ++span) {
    tz_int32 coord[3];
    for (int i = 0; i < 3; ++i) {
      READ_BYTE_BUFFER(coord[i], tz_int32);
    }

    tz_int32 runLength = 0;
    READ_BYTE_BUFFER(runLength, tz_int32);

    if (runLength <= 0) {
      RECORD_ERROR_UNCOND("Invalid run length");
      return false;
    }

//    addStripeFast(coord[2], coord[1]);
//    addSegmentFast(coord[0], coord[0] + runLength - 1);
    addSegment(coord[2], coord[1], coord[0], coord[0] + runLength - 1, false);
  }

  return true;
}

bool ZObject3dScan::importDvidObjectBuffer(
    const std::vector<char> &byteArray)
{
  return importDvidObjectBuffer(&(byteArray[0]), byteArray.size());
}

void ZObject3dScan::addForeground(ZStack *stack)
{
  ConstSegmentIterator iterator(this);
  while (iterator.hasNext()) {
    const ZObject3dScan::Segment &seg = iterator.next();
    int z = seg.getZ();
    int y = seg.getY();
    for (int x = seg.getStart(); x <= seg.getEnd(); ++x) {
      ZGeometry::shiftSliceAxis(x, y, z, m_sliceAxis);
      if (stack->getIntValue(x, y, z) > 0) {
        stack->addIntValue(x, y, z, 0, 1);
      }
    }
  }
}

void ZObject3dScan::addForegroundSlice8(ZStack *stack)
{
  ConstSegmentIterator iterator(this);
  int z0 = stack->getOffset().getZ();
  int y0 = stack->getOffset().getY();
  int x0 = stack->getOffset().getX();
  size_t stride_y = stack->width();
  uint8_t *stackArray = stack->array8();

  while (iterator.hasNext()) {
    const ZObject3dScan::Segment &seg = iterator.next();
    int z = seg.getZ() - z0;
    if (z == 0) {
      int y = seg.getY() - y0;
      if (y >= 0 && y < stack->height()) {
        int startX = imax2(0, seg.getStart() - x0);
        int endX = imin2(seg.getEnd() - x0, stack->width() - 1);
        for (int x = startX; x <= endX; ++x) {
          size_t offset = stride_y * y +  x;
          if (stackArray[offset] > 0) {
            stackArray[offset] += 1;
          }
        }
      }
    }
  }
}

int ZObject3dScan::subtractForegroundSlice8(ZStack *stack)
{
  ConstSegmentIterator iterator(this);
  int z0 = stack->getOffset().getZ();
  int y0 = stack->getOffset().getY();
  int x0 = stack->getOffset().getX();
  size_t stride_y = stack->width();
  uint8_t *stackArray = stack->array8();

  int count = 0;
  while (iterator.hasNext()) {
    const ZObject3dScan::Segment &seg = iterator.next();
    int z = seg.getZ() - z0;
    if (z == 0) {
      int y = seg.getY() - y0;
      if (y >= 0 && y < stack->height()) {
        int startX = imax2(0, seg.getStart() - x0);
        int endX = imin2(seg.getEnd() - x0, stack->width() - 1);
        for (int x = startX; x <= endX; ++x) {
          size_t offset = stride_y * y +  x;
          if (stackArray[offset] > 0) {
            stackArray[offset] -= 1;
            ++count;
          }
        }
      }
    }
  }

  return count;
}

ZObject3dScan ZObject3dScan::subtract(const ZObject3dScan &obj)
{
  int minZ = getMinZ();
  int maxZ = getMaxZ();

  ZObject3dScan subtracted;
  ZObject3dScan remained;

  for (int z = minZ; z <= maxZ; ++z) {
    ZObject3dScan slice = getSlice(z);
    ZObject3dScan slice2 = obj.getSlice(z);
    if (slice2.isEmpty()) {
      remained.concat(slice);
    } else {
      ZStack *plane = slice.toStackObject();
      slice2.addForegroundSlice8(plane); //1: remained; 2: subtracted

      std::vector<ZObject3dScan*> objArray = extractAllObject(*plane);
      for (std::vector<ZObject3dScan*>::const_iterator iter = objArray.begin();
           iter != objArray.end(); ++iter) {
        ZObject3dScan *obj = *iter;
        if (obj->getLabel() == 1) {
          remained.concat(*obj);
        } else if (obj->getLabel() == 2) {
          subtracted.concat(*obj);
        }
        delete obj;
      }
      delete plane;
    }
  }

  remained.canonize();
  subtracted.canonize();

  remained.setSliceAxis(m_sliceAxis);
  subtracted.setSliceAxis(m_sliceAxis);

  this->copyDataFrom(remained);

  return subtracted;
}

ZObject3dScan operator - (
    const ZObject3dScan &obj1, const ZObject3dScan &obj2)
{
  const_cast<ZObject3dScan&>(obj1).canonize();
  const_cast<ZObject3dScan&>(obj2).canonize();

  size_t index1 = 0;
  size_t index2 = 0;


  ZObject3dScan remained;

  while (index1 < obj1.m_stripeArray.size() &&
         index2 < obj2.m_stripeArray.size()) {
    const ZObject3dStripe &s1 = obj1.m_stripeArray[index1];
    const ZObject3dStripe &s2 = obj2.m_stripeArray[index2];

    if (s1.getY() == s2.getY() && s1.getZ() == s2.getZ()) {
      ZObject3dStripe diff = s1 - s2;
      if (!diff.isEmpty()) {
        remained.m_stripeArray.push_back(diff);
      }
      ++index1;
      ++index2;
    } else if (s1.getZ() < s2.getZ() ||
               (s1.getZ() == s2.getZ() && s1.getY() < s2.getY())) {
      remained.m_stripeArray.push_back(obj1.m_stripeArray[index1]);
      ++index1;
    } else {
      ++index2;
    }
  }

  for (; index1 < obj1.m_stripeArray.size(); ++index1) {
    remained.m_stripeArray.push_back(obj1.m_stripeArray[index1]);
  }

  remained.setCanonized(true);

  return remained;
}

void ZObject3dScan::subtractSliently(const ZObject3dScan &obj)
{
  int originalMinZ = getMinZ();
  int originalMaxZ = getMaxZ();

  int minZ = std::max(originalMinZ, obj.getMinZ());
  int maxZ = std::min(originalMaxZ, obj.getMaxZ());

//  ZObject3dScan subtracted;
  ZObject3dScan remained;

  if (originalMinZ < minZ) {
    remained = getSlice(originalMinZ, minZ - 1);
  }

  for (int z = minZ; z <= maxZ; ++z) {
    ZObject3dScan slice = getSlice(z);
    ZObject3dScan slice2 = obj.getSlice(z);

    if (slice2.isEmpty()) {
      remained.concat(slice);
    } else if (!slice.isEmpty()) {
      ZIntCuboid box1 = slice.getBoundBox();
      ZIntCuboid box2 = slice2.getBoundBox();
      bool processed = false;
      if (box1.hasOverlap(box2)) {
        ZStack *plane = slice.toStackObject();
        if (slice2.subtractForegroundSlice8(plane) > 0) {
          std::vector<ZObject3dScan*> objArray = extractAllObject(*plane);
          for (std::vector<ZObject3dScan*>::const_iterator iter = objArray.begin();
               iter != objArray.end(); ++iter) {
            ZObject3dScan *obj = *iter;
            if (obj->getLabel() == 1) {
              remained.concat(*obj);
            }
            delete obj;
          }
          processed = true;
        }
        delete plane;
      }

      if (!processed) {
        remained.concat(slice);
      }
    }
  }

  if (originalMaxZ > maxZ) {
    remained.concat(getSlice(maxZ + 1, originalMaxZ));
  }

//  remained.canonize();

  this->copyDataFrom(remained);
}

ZObject3dScan ZObject3dScan::intersect(const ZObject3dScan &obj) const
{
  int minZ = std::max(getMinZ(), obj.getMinZ());
  int maxZ = std::min(getMaxZ(), obj.getMaxZ());

  ZObject3dScan result;

  for (int z = minZ; z <= maxZ; ++z) {
    ZObject3dScan slice = getSlice(z);
    ZObject3dScan slice2 = obj.getSlice(z);
    if (!slice.isEmpty() && !slice2.isEmpty()) {
      ZStack *plane = slice.toStackObject();
      slice2.addForeground(plane); //1: remained; 2: subtracted

      std::vector<ZObject3dScan*> objArray = extractAllObject(*plane);
      for (std::vector<ZObject3dScan*>::const_iterator iter = objArray.begin();
           iter != objArray.end(); ++iter) {
        ZObject3dScan *obj = *iter;
        if (obj->getLabel() == 2) {
          result.concat(*obj);
        }
        delete obj;
      }
      delete plane;
    }
  }

  result.canonize();

  return result;
}

ZObject3dScan* ZObject3dScan::subobject(const ZIntCuboid &box,
                                        ZObject3dScan *result) const
{
  if (result == NULL) {
    result = new ZObject3dScan;
  }
  ConstSegmentIterator iter(this);
  while (iter.hasNext()) {
    const ZObject3dScan::Segment &seg = iter.next();
    if (box.containYZ(seg.getY(), seg.getZ())) {
      int x0 = imax2(seg.getStart(), box.getFirstCorner().getX());
      int x1 = imin2(seg.getEnd(), box.getLastCorner().getX());
      if (x0 <= x1) {
        result->addSegment(seg.getZ(), seg.getY(), x0, x1, false);
      }
    }
  }
  result->canonize();

  result->setSliceAxis(m_sliceAxis);

  return result;
}

void ZObject3dScan::switchYZ()
{
  for (size_t i = 0; i < m_stripeArray.size(); ++i) {
    m_stripeArray[i].switchYZ();
  }

  processEvent(EVENT_OBJECT_MODEL_CHANGED | EVENT_OBJECT_UNCANONIZED);
}

std::vector<double> ZObject3dScan::getPlaneCov() const
{
  std::vector<double> cov(3, 0.0);

  if (!isEmpty()) {
    Cuboid_I boundBox;
    getBoundBox(&boundBox);

    double xMean = 0.0;
    double yMean = 0.0;
    double xSquareMean = 0.0;
    double ySquareMean = 0.0;
    double xyCorr = 0.0;

    size_t count = 0;
    for (vector<ZObject3dStripe>::const_iterator iter = m_stripeArray.begin();
         iter != m_stripeArray.end(); ++iter) {
      int y = iter->getY();
      int nseg = iter->getSegmentNumber();
      for (int i = 0; i < nseg; ++i) {
        const int *seg = iter->getSegment(i);
        for (int x = seg[0]; x <= seg[1]; ++x) {
          xMean += x;
          xSquareMean += x * x;
          yMean += y;
          ySquareMean += y * y;
          xyCorr += x * y;
          ++count;
        }
      }
    }

    xMean /= count;
    yMean /= count;
    xSquareMean /= count;
    ySquareMean /= count;
    xyCorr /= count;

#ifdef _DEBUG_2
    std::cout << count << std::endl;
#endif

    double factor = 1.0;
    if (count > 1) {
      factor = static_cast<double>(count) / (count - 1);
    }

    cov[0] = (xSquareMean - xMean * xMean) * factor;
    cov[1] = (ySquareMean - yMean * yMean) * factor;
    cov[2] = (xyCorr - xMean * yMean) * factor;
  }

  ZGeometry::shiftSliceAxis(cov[0], cov[1], cov[2], m_sliceAxis);

  return cov;
}

double ZObject3dScan::getSpread(int z) const
{
  ZObject3dScan objSlice = getSlice(z, z);
  const std::vector<double> cov = objSlice.getPlaneCov();
  ZEigenSolver solver;
  if (solver.solveCovEigen(cov)) {
    return sqrt(solver.getEigenValue(0) * solver.getEigenValue(1)) * 2.0;
  }

  return 0.0;
}


bool ZObject3dScan::contains(const ZIntPoint &pt)
{
  return contains(pt.getX(), pt.getY(), pt.getZ());
}

bool ZObject3dScan::contains(int x, int y, int z)
{
  ZGeometry::shiftSliceAxis(x, y, z, m_sliceAxis);

  canonize();

  //Binary search
  size_t lowIndex = 0;
  size_t highIndex = getStripeNumber();

  while (highIndex >= lowIndex) {
    size_t medIndex = (lowIndex + highIndex) / 2;
    ZObject3dStripe &stripe = m_stripeArray[medIndex];
    if (stripe.getZ() == z && stripe.getY() == y) {
      return stripe.containsX(x);
    } else {
      //On the left side
      if (stripe.getZ() > z || (stripe.getZ() == z && stripe.getY() > y)) {
        if (medIndex == 0) {
          if (highIndex == 1) {
            return m_stripeArray[highIndex].contains(x, y, z);
          }
          return false;
        } else {
          highIndex = medIndex - 1;
        }
      } else { //On the right side
        lowIndex = medIndex + 1;
      }
    }
  }

  return false;
}

void ZObject3dScan::blockEvent(bool blocking)
{
  m_blockingEvent = blocking;
}

void ZObject3dScan::processEvent(TEvent event)
{
  if (!m_blockingEvent) {
    if (event & EVENT_OBJECT_MODEL_CHANGED & ~EVENT_OBJECT_VIEW_CHANGED) {
      deprecate(COMPONENT_ACCUMULATED_STRIPE_NUMBER);
      deprecate(COMPONENT_SLICEWISE_VOXEL_NUMBER);
    }

    if (event & EVENT_OBJECT_UNCANONIZED & ~EVENT_OBJECT_VIEW_CHANGED) {
      setCanonized(false);
    }

    if (event & EVENT_OBJECT_CANONIZED & ~EVENT_OBJECT_VIEW_CHANGED) {
      setCanonized(true);
    }

    if (event & EVENT_OBJECT_VIEW_CHANGED) {
      deprecate(COMPONENT_STRIPE_INDEX_MAP);
      deprecate(COMPONENT_INDEX_SEGMENT_MAP);
    }
  }
}

#define VERIFY_DATA(condition) \
  if (!(condition)) { \
    clear(); \
    return false; \
  }

#define CHECK_DATA_LENGTH(s)  VERIFY_DATA(length >= (size_t) (s))

bool ZObject3dScan::load(const int *data, size_t length)
{
  clear();

  if (data != NULL) {
    m_stripeArray.resize(*(data++));
    --length;
    for (std::vector<ZObject3dStripe>::iterator iter = m_stripeArray.begin();
         iter != m_stripeArray.end(); ++iter) {
      CHECK_DATA_LENGTH(3)

      ZObject3dStripe &stripe = *iter;
      stripe.setZ(*(data++));
      stripe.setY(*(data++));
      int nseg = *(data++);

      VERIFY_DATA(nseg > 0)

      length -= 3;

      CHECK_DATA_LENGTH(nseg * 2)

      for (int i = 0; i < nseg; ++i) {
        stripe.addSegment(data[0], data[1], false);
        data += 2;
      }

      length -= nseg * 2;
    }
  }

  return true;
}

std::vector<int> ZObject3dScan::toIntArray() const
{ 
  std::vector<int> array;
  if (!isEmpty()) {
    //Count total size
    size_t arraySize = 1;
    for (std::vector<ZObject3dStripe>::const_iterator iter = m_stripeArray.begin();
         iter != m_stripeArray.end(); ++iter) {
      const ZObject3dStripe &stripe = *iter;
      arraySize += 3 + stripe.getSegmentNumber() * 2;
    }

    array.resize(arraySize);
    array[0] = getStripeNumber();
//    array.push_back(getStripeNumber());
    size_t index = 1;
    for (std::vector<ZObject3dStripe>::const_iterator iter = m_stripeArray.begin();
         iter != m_stripeArray.end(); ++iter) {
      const ZObject3dStripe &stripe = *iter;
      size_t stripeSize = 3 + stripe.getSegmentNumber() * 2;
//      array.resize(array.size() + stripeSize);
      stripe.fillIntArray(&(array[index]));
      index += stripeSize;
      /*
      array.push_back(stripe.getZ());
      array.push_back(stripe.getY());
      array.push_back(stripe.getSegmentNumber());
      for (int i = 0; i < stripe.getSegmentNumber(); ++i) {
        array.push_back(stripe.getSegmentStart(i));
        array.push_back(stripe.getSegmentEnd(i));
      }
      */
    }
  }

  return array;
}

bool ZObject3dScan::importHdf5(const string &filePath, const string &key)
{
  clear();

  ZHdf5Reader reader;
  if (reader.open(filePath)) {
    std::vector<int> array = reader.readIntArray(key);
    if (!array.empty()) {
      return load(&(array[0]), array.size());
    }
  }

  return false;
}


size_t ZObject3dScan::getSurfaceArea() const
{
  size_t area = 0;

  int minZ = getMinZ();
  int maxZ = getMaxZ();

  area += getSlice(minZ).getVoxelNumber();
  if (maxZ > minZ) {
    area += getSlice(maxZ).getVoxelNumber();
  }

  for (int z = minZ + 1; z < maxZ - 1; ++z) {
    ZObject3dScan subobj = getSlice(z - 1, z + 1);

    Stack *stack = subobj.toStack();
    Stack *edge = Stack_Perimeter(stack, NULL, 26);
    C_Stack::kill(stack);

    Stack slice = C_Stack::sliceView(edge, 1);
    area += Stack_Sum(&slice);

    C_Stack::kill(edge);
  }

  return area;
}

/////////////////////////Iterators/////////////////////////
ZObject3dScan::ConstSegmentIterator::ConstSegmentIterator(
    const ZObject3dScan *obj) : m_obj(obj), m_stripeIndex(0), m_segmentIndex(0)
{
}

const ZObject3dScan::Segment& ZObject3dScan::ConstSegmentIterator::next()
{
  if (hasNext()) {
    const ZObject3dStripe &stripe = m_obj->getStripe(m_stripeIndex);
    m_seg.set(stripe.getZ(), stripe.getY(),
              stripe.getSegmentStart(m_segmentIndex),
              stripe.getSegmentEnd(m_segmentIndex));
    advance();
  }
  return m_seg;
}

bool ZObject3dScan::ConstSegmentIterator::hasNext() const
{
  if (m_obj == NULL) {
    return false;
  }
  if (m_stripeIndex >= m_obj->getStripeNumber()) {
    return false;
  }

  const ZObject3dStripe &stripe = m_obj->getStripe(m_stripeIndex);
  if (m_segmentIndex >= stripe.getSegmentNumber()) {
    return false;
  }

  return true;
}

void ZObject3dScan::ConstSegmentIterator::advance()
{
  if (m_stripeIndex < m_obj->getStripeNumber()) {
    const ZObject3dStripe &stripe = m_obj->getStripe(m_stripeIndex);
    if (m_segmentIndex < stripe.getSegmentNumber() - 1) {
      ++m_segmentIndex;
    } else {
      m_segmentIndex = 0;
      ++m_stripeIndex;
    }
  }
}

std::vector<ZObject3dScan*> ZObject3dScan::extractAllObject(
    const ZStack &stack, int yStep)
{
  std::vector<ZObject3dScan*> result;
  std::map<uint64_t, ZObject3dScan*> *objMap = NULL;

  switch (stack.kind()) {
  case GREY:
    objMap = extractAllForegroundObject(
          stack.array8(), stack.width(), stack.height(), stack.depth(),
          stack.getOffset().getX(), stack.getOffset().getY(),
          stack.getOffset().getZ(), yStep, NULL);
    break;
  case GREY16:
    objMap = extractAllForegroundObject(
          stack.array16(), stack.width(), stack.height(), stack.depth(),
          stack.getOffset().getX(), stack.getOffset().getY(),
          stack.getOffset().getZ(), yStep, NULL);
    break;
  default:
    break;
  }

  if (objMap != NULL) {
    for (std::map<uint64_t, ZObject3dScan*>::iterator iter = objMap->begin();
         iter != objMap->end(); ++iter) {
      result.push_back(iter->second);
    }

    delete objMap;
  }

  return result;
}

bool ZObject3dScan::importDvidRoi(const string &filePath)
{
  ZJsonArray objJson;
  objJson.load(filePath);

  return importDvidRoi(objJson);
}

bool ZObject3dScan::importDvidRoi(const ZJsonArray &obj)
{
  clear();

  if (obj.isEmpty()) {
    return false;
  }

  bool succ = true;
  for (size_t i = 0; i < obj.size(); ++i) {
    if (ZJsonParser::isArray(obj.at(i))) {
      ZJsonArray subarray(obj.at(i), ZJsonValue::SET_INCREASE_REF_COUNT);
      if (subarray.size() != 4) {
        succ = false;
        break;
      }
    } else {
      succ = false;
      break;
    }
  }

  if (succ) {
    for (size_t i = 0; i < obj.size(); ++i) {
      ZJsonArray subarray(obj.at(i), ZJsonValue::SET_INCREASE_REF_COUNT);
      int z = ZJsonParser::integerValue(subarray.at(0));
      int y = ZJsonParser::integerValue(subarray.at(1));
      int x0 = ZJsonParser::integerValue(subarray.at(2));
      int x1 = ZJsonParser::integerValue(subarray.at(3));
      addSegment(z, y, x0, x1);
    }
  }

  return succ;
}

ZSTACKOBJECT_DEFINE_CLASS_NAME(ZObject3dScan)
