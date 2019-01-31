#include "zqtheader.h"

#ifdef _QT_GUI_USED_
#include <QDebug>
#include <QFileInfo>
#include <QDir>
#include <QElapsedTimer>
#endif
#include <string.h>
#include <iostream>
#include <cmath>
#include "tz_utilities.h"
#include "neutubeconfig.h"
#include "tz_stack_lib.h"
#include "zstack.hxx"
#include "tz_fimage_lib.h"
//#include "tz_xml_utils.h"
#include "tz_stack_relation.h"
#include "zfilelist.h"
#include "c_stack.h"
#include "tz_stack.h"
#include "tz_stack_objlabel.h"
#include "tz_int_histogram.h"
#include "zjsonparser.h"
#include "zfiletype.h"
#include "tz_math.h"
#ifdef _QT_GUI_USED_
#include "QsLog.h"
#endif
#include "zstring.h"
#include "zstackfactory.h"
#include "geometry/zpoint.h"
#include "geometry/zintcuboid.h"
#include "misc/miscutility.h"
#include "common/utilities.h"

using namespace std;

ZStack::ZStack()
{
  init();
}

ZStack::ZStack(int kind, int width, int height, int depth,
               int nchannel, bool isVirtual)
{  
  init();

  Mc_Stack *stack = NULL;
  C_Stack::Mc_Stack_Deallocator *delloc = NULL;
  if (isVirtual) {
    stack = new Mc_Stack;
    stack->array = NULL;
    C_Stack::setAttribute(stack, kind, width, height, depth, nchannel);
    delloc = C_Stack::cppDelete;
  } else {
    stack = C_Stack::make(kind, width, height, depth, nchannel);
    delloc = C_Stack::kill;
  }
  setData(stack, delloc);
}

ZStack::ZStack(int kind, const ZIntCuboid &box, int nchannel, bool isVirtual)
{
  init();

  int width = box.getWidth();
  int height = box.getHeight();
  int depth = box.getDepth();

  Mc_Stack *stack = NULL;
  C_Stack::Mc_Stack_Deallocator *delloc = NULL;
  if (isVirtual) {
    stack = new Mc_Stack;
    stack->array = NULL;
    C_Stack::setAttribute(stack, kind, width, height, depth, nchannel);
    delloc = C_Stack::cppDelete;
  } else {
    stack = C_Stack::make(kind, width, height, depth, nchannel);
    delloc = C_Stack::kill;
  }

  m_dealloc = NULL;
  setData(stack, delloc);
  setOffset(box.getFirstCorner());
}

ZStack::ZStack(Mc_Stack *stack/*, C_Stack::Mc_Stack_Deallocator *dealloc*/)
{
  init();

  setData(stack, C_Stack::kill);
}

#if 0
ZStack::ZStack(Mc_Stack *stack, C_Stack::Mc_Stack_Deallocator *dealloc)
{
  init();

  setData(stack, dealloc);
}
#endif

ZStack::ZStack(const ZStack &/*src*/)
{
//  init();

//  m_stack = src.m_stack;
//  m_offset = src.m_offset;
}

ZStack::~ZStack()
{
//  ZOUT(LTRACE(), 5) << "Deleting stack: " << this;
  #ifdef _QT_GUI_USED_
  tic();
  #endif
  clear();
  #ifdef _QT_GUI_USED_
  ZOUT(LTRACE(), 5) << "Stack deleted" << this << toc() << "ms";
  #endif
}

size_t ZStack::getByteNumber(EStackUnit unit) const
{
  if (unit == WHOLE_DATA) {
    return getVoxelNumber() * channelNumber() * kind();
  } else {
    return getVoxelNumber(unit) * kind();
  }
}

void ZStack::setOffset(int dx, int dy)
{
  m_offset.setX(dx);
  m_offset.setY(dy);
}

void ZStack::setOffset(int dx, int dy, int dz)
{
  m_offset.set(dx, dy, dz);
}

void ZStack::setOffset(const ZIntPoint &pt)
{
  m_offset = pt;
}

void ZStack::setZOffset(int z)
{
  m_offset.setZ(z);
}

void ZStack::translate(int dx, int dy, int dz)
{
  m_offset += ZIntPoint(dx, dy, dz);
}

void ZStack::translate(const ZIntPoint &pt)
{
  m_offset += pt;
}

size_t ZStack::getVoxelNumber(EStackUnit unit) const
{
  switch (unit) {
  case SINGLE_VOXEL:
    return 1;
  case SINGLE_ROW:
    return width();
  case SINGLE_PLANE:
    return getVoxelNumber(SINGLE_ROW) * height();
  case SINGLE_CHANNEL:
  case WHOLE_DATA:
    return getVoxelNumber(SINGLE_PLANE) * depth();
  }

  return 0;
}

void ZStack::setData(Mc_Stack *stack, C_Stack::Mc_Stack_Deallocator *dealloc)
{
  deprecate(MC_STACK);
  m_stack = stack;
  m_dealloc = dealloc;
}

void ZStack::consume(Stack *stack)
{
  load(stack, true);
}

void ZStack::consume(ZStack *stack)
{
  this->setData(stack->m_stack, stack->m_dealloc);
  stack->m_dealloc = NULL;
  setSource(stack->source());
//  m_resolution = stack->getResolution();
  setOffset(stack->getOffset());
//  m_preferredZScale = stack->m_preferredZScale;

  delete stack;
}

const Stack* ZStack::c_stack(int c) const
{
  const void *dataArray = rawChannelData(c);

  if (dataArray != NULL) {
    m_stackView.resize(channelNumber());
    C_Stack::setAttribute(&(m_stackView[c]), kind(), width(), height(), depth());

    m_stackView[c].array = (uint8*) dataArray;
    m_stackView[c].text = m_buffer;

    return &(m_stackView[c]);
  }

  return NULL;
}

Stack* ZStack::c_stack(int c)
{
  return const_cast<Stack*>(
        static_cast<const ZStack&>(*this).c_stack(c));
}

const void* ZStack::rawChannelData(int c) const
{
  void *array = NULL;

  if (c >= 0 && c < channelNumber()) {
    array = m_stack->array + getByteNumber(SINGLE_CHANNEL) * c;
  }

  return array;
}

void* ZStack::rawChannelData(int c)
{
  return const_cast<void*>(static_cast<const ZStack&>(*this).rawChannelData(c));
}

ZSingleChannelStack* ZStack::singleChannelStack(int c)
{
  return const_cast<ZSingleChannelStack*>(
        static_cast<const ZStack&>(*this).singleChannelStack(c));
}

void ZStack::deprecateSingleChannelView(int c)
{
  if (c < (int) m_singleChannelStack.size()) {
    delete m_singleChannelStack[c];
    m_singleChannelStack[c] = NULL;
  }
}

ZStack* ZStack::getSingleChannel(int c) const
{
  Mc_Stack *data = new Mc_Stack;
  C_Stack::view(m_stack, data, c);
  ZStack *stack = new ZStack;
  stack->setData(data, C_Stack::cppDelete);
  stack->setOffset(getOffset());

  return stack;
}

bool ZStack::isSingleChannelViewDeprecated(int channel) const
{
  bool obselete = true;

  if (channel < (int) m_singleChannelStack.size()) {
    obselete = m_singleChannelStack[channel] == NULL;
  }

  return obselete;
}

const ZSingleChannelStack* ZStack::singleChannelStack(int c) const
{
  if (isSingleChannelViewDeprecated(c)) {
    if (c >= (int) m_singleChannelStack.size()) {
      m_singleChannelStack.resize(c + 1, NULL);
    }
    m_singleChannelStack[c] = new ZSingleChannelStack;
    m_singleChannelStack[c]->setData(const_cast<Stack*>(c_stack(c)), NULL);
  }

  return m_singleChannelStack[c];

  /*
  m_singleChannelStack.setData(const_cast<Stack*>(c_stack(c)), false);

  return &m_singleChannelStack;
  */
}

/*
bool ZStack::releaseOwnership(int c)
{
  return m_singleChannelStackVector[c]->releaseOwnership();
}
*/
void ZStack::shiftLocation(int *offset, int c, int width, int height, int depth)
{
  singleChannelStack(c)->shiftLocation(offset, width, height, depth);
}

Mc_Stack *ZStack::makeMcStack(const Stack *stack1, const Stack *stack2, const Stack *stack3)
{
  Mc_Stack *out = NULL;

  if (stack1 != NULL && stack1->kind == 1 && Stack_Same_Attribute(stack1, stack2)) {
    if (stack3 == NULL || Stack_Same_Attribute(stack2, stack3)) {
      out = C_Stack::make(1, stack1->width, stack1->height, stack1->depth, 3);
      size_t volume = Stack_Voxel_Number(stack1);
      memcpy(out->array, stack1->array, volume);
      memcpy(out->array+volume, stack2->array, volume);
      if (stack3 != NULL) {
        memcpy(out->array+volume*2, stack3->array, volume);
      } else {
        memset(out->array + volume*2, 0, volume);
      }
    }
  }

  return out;
}

void ZStack::subMostCommonValue(int c)
{
  if (c < channelNumber())
  {
    singleChannelStack(c)->subMostCommonValue();
  }
}

Stack *ZStack::averageOfAllChannels()
{
  Stack *stack = NULL;
  int nchannel = channelNumber();
  if (nchannel == 1) {
    stack = C_Stack::clone(c_stack());
  }
  if (nchannel > 1) {
    stack = C_Stack::make(data()->kind, data()->width, data()->height, data()->depth);
    size_t nvoxel = getVoxelNumber();
    Image_Array ima;
    ima.array = stack->array;
    int k = kind();
    switch (k) {
    case GREY:
      for (size_t i=0; i<nvoxel; i++) {
        double value = 0.0;
        for (int j=0; j<nchannel; j++) {
          value += c_stack(j)->array[i];
        }
        value /= nchannel;
        ima.array8[i] = value;
      }
      break;
    case GREY16:
      for (size_t i=0; i<nvoxel; i++) {
        double value = 0.0;
        for (int j=0; j<nchannel; j++) {
          value += ((uint16*)c_stack(j)->array)[i];
        }
        value /= nchannel;
        ima.array16[i] = value;
      }
      break;
    case FLOAT32:
      for (size_t i=0; i<nvoxel; i++) {
        double value = 0.0;
        for (int j=0; j<nchannel; j++) {
          value += ((float*)c_stack(j)->array)[i];
        }
        value /= nchannel;
        ima.array32[i] = value;
      }
      break;
    case FLOAT64:
      for (size_t i=0; i<nvoxel; i++) {
        double value = 0.0;
        for (int j=0; j<nchannel; j++) {
          value += ((double*)c_stack(j)->array)[i];
        }
        value /= nchannel;
        ima.array64[i] = value;
      }
      break;
    }
  }
  return stack;
}


void ZStack::init()
{
//  m_stack = NULL;
//  m_dealloc = NULL;
//  m_buffer[0] = '\0';
  //m_singleChannelStackVector.resize(nchannel);
//  m_preferredZScale = 1.0;
  //m_source = NULL;
}

bool ZStack::canMerge(const Stack *s1, const Stack *s2)
{
  if ((s1 && C_Stack::kind(s1) == 3) || (s2 && C_Stack::kind(s2) == 3))
    return false;
  if (s1 == NULL || s2 == NULL)
    return true;
  else
    return Stack_Same_Attribute(s1, s2);
}

void ZStack::deprecateDependent(EComponent component)
{
  switch (component) {
  case MC_STACK:
    //deprecate(STACK_PROJECTION);
    //deprecate(STACK_STAT);
    deprecate(SINGLE_CHANNEL_VIEW);
    break;
  case SINGLE_CHANNEL_VIEW:
    break;
    /*
  case STACK_PROJECTION:
    break;
  case STACK_STAT:
    break;
    */
  }
}

void ZStack::deprecate(EComponent component)
{
  deprecateDependent(component);

  switch (component) {
  case MC_STACK:  
    if (m_stack != NULL && m_dealloc != NULL) {
      m_dealloc(m_stack);
    }
    m_stack = NULL;
    m_dealloc = NULL;
    break;
    /*
  case STACK_PROJECTION:
    delete m_proj;
    m_proj = NULL;
    break;
  case STACK_STAT:
    delete m_stat;
    m_stat = NULL;
    break;
    */
  case SINGLE_CHANNEL_VIEW:
    for (size_t i = 0; i < m_singleChannelStack.size(); ++i) {
      delete m_singleChannelStack[i];
      m_singleChannelStack[i] = NULL;
    }
    break;
  }
}

bool ZStack::isDeprecated(EComponent component) const
{
  switch (component) {
  case MC_STACK:
    return m_stack == NULL;
    break;
    /*
  case STACK_PROJECTION:
    return m_proj == NULL;
    break;
  case STACK_STAT:
    return m_stat == NULL;
    break;
    */
  case SINGLE_CHANNEL_VIEW:
    for (int c = 0; c < channelNumber(); ++c) {
      if (isSingleChannelViewDeprecated(c)) {
        return true;
      }
    }
    break;
  }

  return false;
}

void ZStack::clear()
{
  deprecate(ZStack::MC_STACK);
  clearChannelColors();
}

void ZStack::clearChannelColors()
{
#ifdef _QT_GUI_USED_
  for (size_t i=0; i<m_channelColors.size(); ++i)
    delete m_channelColors[i];
  m_channelColors.clear();
#endif
}

void ZStack::setChannelNumber(int nchannel)
{
  C_Stack::setChannelNumber(m_stack, nchannel);
}

void ZStack::useChannelColors(bool on)
{
  m_usingChannelColors = on;
}

void ZStack::initChannelColors()
{
#ifdef _QT_GUI_USED_
  if (m_usingChannelColors) {
    if (m_channelColors.size() == (size_t)channelNumber()) {
      return;
    }
    for (int i=0; i<channelNumber(); ++i) {
      m_channelColors.push_back(new ZVec3Parameter(QString("Ch%1").arg(i+1),
                                                   glm::vec3(0.f)));
      m_channelColors[i]->setStyle("COLOR");
    }
    if (channelNumber() == 1)
      m_channelColors[0]->set(glm::vec3(1.f,1.f,1.f));
    else {
      m_channelColors[0]->set(glm::vec3(1.f,0.f,0.f));
      m_channelColors[1]->set(glm::vec3(0.f,1.f,0.f));
      if (channelNumber() > 2)
        m_channelColors[2]->set(glm::vec3(0.f,0.f,1.f));
    }
  }
#endif
}

std::string ZStack::getTransformMeta() const
{
  std::string meta;
  ZIntPoint offset = getOffset();
  ZIntPoint dsIntv = getDsIntv();
  if (!offset.isZero() || !dsIntv.isZero()) {
    meta = "@transform ";
    meta += offset.toString() + " " + dsIntv.toString();
  }

  return meta;
}

void ZStack::removeChannel(int c)
{
  if (c >= 0 && c < channelNumber()) {
    size_t byteNumber = getByteNumber(SINGLE_CHANNEL);
    for (int k = c + 1; k < channelNumber(); ++k) {
      void *dst = rawChannelData(c - 1);
      void *src = rawChannelData(c);
      memmove(dst, src, byteNumber);
    }
    deprecateDependent(MC_STACK);
  }
}

bool ZStack::load(Stack *stack, bool isOwner)
{
  deprecate(MC_STACK);

  if (C_Stack::kind(stack) == 3) {
    m_stack = C_Stack::make(C_Stack::kind(stack), C_Stack::width(stack), C_Stack::height(stack),
                            C_Stack::depth(stack), 3);
    m_dealloc = C_Stack::kill;
    Stack *stack0 = C_Stack::channelExtraction(stack, 0);
    C_Stack::copyChannelValue(m_stack, 0, stack0);
    C_Stack::kill(stack0);
    Stack *stack1 = C_Stack::channelExtraction(stack, 1);
    C_Stack::copyChannelValue(m_stack, 1, stack1);
    C_Stack::kill(stack1);
    Stack *stack2 = C_Stack::channelExtraction(stack, 2);
    C_Stack::copyChannelValue(m_stack, 2, stack2);
    C_Stack::kill(stack2);
    if (isOwner)
      C_Stack::kill(stack);
  } else {
    m_stack = (Mc_Stack*) malloc(sizeof(Mc_Stack));

    C_Stack::view(stack, m_stack);

    if (isOwner) {
      stack->array = NULL;
      C_Stack::kill(stack);
      m_dealloc = C_Stack::systemKill;
    } else {
      m_dealloc = NULL;
    }
  }

  initChannelColors();

  return true;
}

bool ZStack::load(const string &filepath, bool initColor)
{
  deprecate(MC_STACK);

  ZStackFile stackFile;
  stackFile.import(filepath);

  ZStack *res = stackFile.readStack(this, initColor);
  if (res) {
    res->setSource(filepath);
  }

  return res;
}

bool ZStack::load(const Stack *ch1, const Stack *ch2, const Stack *ch3)
{
  deprecate(MC_STACK);

  if (ch1 == NULL && ch2 == NULL && ch3 == NULL)
    return false;
  if (!canMerge(ch1, ch2) || !canMerge(ch2, ch3) || !canMerge(ch1, ch3))
    return false;

  if (ch3 != NULL) {
    m_stack = C_Stack::make(C_Stack::kind(ch3), C_Stack::width(ch3), C_Stack::height(ch3),
                            C_Stack::depth(ch3), 3);
    m_dealloc = C_Stack::kill;
    C_Stack::copyChannelValue(m_stack, 2, ch3);
    if (ch2 != NULL) {
      C_Stack::copyChannelValue(m_stack, 1, ch2);
    }
    if (ch1 != NULL) {
      C_Stack::copyChannelValue(m_stack, 0, ch1);
    }
  } else if (ch2 != NULL) {
    m_stack = C_Stack::make(C_Stack::kind(ch2), C_Stack::width(ch2), C_Stack::height(ch2),
                            C_Stack::depth(ch2), 2);
    m_dealloc = C_Stack::kill;
    C_Stack::copyChannelValue(m_stack, 1, ch2);
    if (ch1 != NULL) {
      C_Stack::copyChannelValue(m_stack, 0, ch1);
    }
  } else {
    m_stack = C_Stack::make(C_Stack::kind(ch1), C_Stack::width(ch1), C_Stack::height(ch1),
                            C_Stack::depth(ch1), 1);
    m_dealloc = C_Stack::kill;
    C_Stack::copyChannelValue(m_stack, 0, ch1);
  }
  return true;
}

void ZStack::read(std::istream &stream)
{
  clear();

  int kind = GREY;
  neutube::read(stream, kind);
  int channel = 0;
  neutube::read(stream, channel);

  m_offset.read(stream);
  ZIntPoint dim;
  dim.read(stream);

  m_stack = C_Stack::make(kind, dim.getX(), dim.getY(), dim.getZ(), channel);
  stream.read((char*)(m_stack->array), C_Stack::allByteNumber(m_stack));
}

void ZStack::write(std::ostream &stream) const
{
  neutube::write(stream, m_stack->kind);
  neutube::write(stream, m_stack->nchannel);
  m_offset.write(stream);
  ZIntPoint dim(width(), height(), depth());
  dim.write(stream);
  stream.write((const char*)(m_stack->array), C_Stack::allByteNumber(m_stack));
}

void ZStack::setSource(const string &filepath, int channel)
{
  m_source.import(filepath);
  m_source.setChannel(channel);
}

void ZStack::setSource(const ZStackFile &file)
{
  m_source = file;
}

void ZStack::setSource(Stack_Document *stackDoc)
{
  m_source.loadStackDocument(stackDoc);
}
/*
void ZStack::setResolution(double x, double y, double z, char unit)
{
  m_resolution.setVoxelSize(x, y, z);
  m_resolution.setUnit(unit);
  m_preferredZScale = z / (.5 * (x + y));
}
*/

int ZStack::getChannelNumber(const string &filepath)
{
  int nchannel = 0;
  ZFileType::EFileType type = ZFileType::FileType(filepath);

  if (type == ZFileType::FILE_TIFF ||
      type == ZFileType::FILE_LSM) {
    Tiff_Reader *reader;
    if (type == ZFileType::FILE_TIFF) {
      reader = Open_Tiff_Reader((char*) filepath.c_str(), NULL, 0);
    } else {
      reader = Open_Tiff_Reader((char*) filepath.c_str(), NULL, 1);
    }

    Tiff_Type type = TIFF_BYTE;
    int count = 0;
    Tiff_IFD *ifd = Read_Tiff_IFD(reader);
    uint32_t *val = (uint32_t*)Get_Tiff_Tag(ifd,TIFF_NEW_SUB_FILE_TYPE,&type,&count);
    if (val != NULL) {
      while (*val == 1) {
        Free_Tiff_IFD(ifd);
        Advance_Tiff_Reader(reader);
        if (End_Of_Tiff(reader)) {
          ifd = NULL;
          Free_Tiff_Reader(reader);
          return 0;
        }
        ifd = Read_Tiff_IFD(reader);
        val = (uint32_t*)Get_Tiff_Tag(ifd,TIFF_NEW_SUB_FILE_TYPE,&type,&count);
        if (val == NULL) {
          Free_Tiff_IFD(ifd);
          Free_Tiff_Reader(reader);
          return 0;
        }
      }
    } else {  // try TIFF_IMAGE_WIDTH
      val = (uint32_t*)Get_Tiff_Tag(ifd,TIFF_IMAGE_WIDTH,&type,&count);
      if (val == NULL) {
        Free_Tiff_IFD(ifd);
        Free_Tiff_Reader(reader);
        return 0;
      }
    }

    Tiff_Image *image = Extract_Image_From_IFD(ifd);

    if (image == NULL) {
      return 0;
    }

    nchannel = image->number_channels;
    Kill_Tiff_Image(image);
    Free_Tiff_Reader(reader);
  } else if (type == ZFileType::FILE_V3D_RAW) {
    FILE *fp = Guarded_Fopen(filepath.c_str(), "rb", "Read_Raw_Stack_C");

    char formatkey[] = "raw_image_stack_by_hpeng";
    int lenkey = strlen(formatkey);
    fread(formatkey, 1, lenkey, fp);

    if (strcmp(formatkey, "raw_image_stack_by_hpeng") != 0) {
      fclose(fp);
      return 0;
    }

    char endian;
    fread(&endian, 1, 1, fp);

    uint16_t dataType;
    char sz_buffer[16];
    uint32_t sz[4];

    fread(&dataType, 2, 1, fp);
    fread(sz_buffer, 1, 8, fp);

    int i;
    for (i = 0; i < 4; i++) {
      sz[i] = *((uint16_t*) (sz_buffer + i * 2));
    }

    if ((sz[0] == 0) || (sz[1] == 0) || (sz[2] == 0) || (sz[3] == 0)) {
      fread(sz_buffer + 8, 1, 8, fp);

      for (i = 0; i < 4; i++) {
        sz[i] = *((uint32_t*) (sz_buffer + i * 4));
      }
    }

    nchannel = sz[3];
    fclose(fp);
  } else if (type == ZFileType::FILE_PNG) {
    //No support for multi-channel png yet
    return 1;
  }

  return nchannel;
}

std::string ZStack::save(const string &filepath) const
{
  std::string resultFilePath;

  if (!isVirtual()) {
    resultFilePath = filepath;
    if ((channelNumber() > 1 && kind() != GREY && kind() != GREY16) ||
        (getVoxelNumber() > 2147483648)) { //save as raw
      if (ZFileType::FileType(filepath) != ZFileType::FILE_V3D_RAW ||
          ZFileType::FileType(filepath) != ZFileType::FILE_MC_STACK_RAW) {
        std::cout << "Unsupported data format for " << resultFilePath << endl;
        resultFilePath += ".raw";
        std::cout << resultFilePath << " saved instead." << endl;
      }
    }
  }

  if (!resultFilePath.empty()) {
    ZString meta = getTransformMeta();
    C_Stack::write(resultFilePath.c_str(), m_stack, meta.c_str());
  }

  return resultFilePath;
}

void* ZStack::projection(
    ZSingleChannelStack::EProjMode mode, ZSingleChannelStack::Stack_Axis axis,
    int c)
{
  return singleChannelStack(c)->projection(mode, axis);
}

void* ZStack::projection(
    neutube::EImageBackground bg, ZSingleChannelStack::Stack_Axis axis, int c)
{
  ZSingleChannelStack::EProjMode mode = ZSingleChannelStack::EProjMode::MAX_PROJ;
  if (bg == neutube::EImageBackground::BRIGHT) {
    mode = ZSingleChannelStack::EProjMode::MIN_PROJ;
  }

  return projection(mode, axis, c);
}

double ZStack::value(int x, int y, int z, int c) const
{
  /* Need better treatment for compatibility
  x -= iround(m_offset.x());
  y -= iround(m_offset.y());
  z -= iround(m_offset.z());
  */
  if (isVirtual()) {
    return 0.0;
  }

  if (!(IS_IN_CLOSE_RANGE(x, 0, width() - 1) &&
        IS_IN_CLOSE_RANGE(y, 0, height() - 1) &&
        (z < depth()) &&
        IS_IN_CLOSE_RANGE(c, 0, channelNumber() - 1))) {
    return 0.0;
  }

  if (z < 0) {
    z = maxIntensityDepth(x, y, c);
  }

  return singleChannelStack(c)->value(x, y, z);
}

void ZStack::setIntValue(int x, int y, int z, int c, int v)
{
  if (isVirtual()) {
    return;
  }

  x -= getOffset().getX();
  y -= getOffset().getY();
  z -= getOffset().getZ();

  if (x < 0 || x >= width() || y < 0 || y >= height() || z < 0 || z >= depth() ||
      c < 0 || c >= channelNumber()) {
    return;
  }

  size_t stride_y = C_Stack::width(m_stack);
  size_t stride_z = stride_y * C_Stack::height(m_stack);
  size_t stride_c = stride_z * C_Stack::depth(m_stack);

  Image_Array ima;
  ima.array = m_stack->array;
  size_t offset = stride_c * c + stride_z * z + stride_y * y +  x;

  switch(kind()) {
  case GREY:
    CLIP_VALUE(v, 0, 255);
    ima.array[offset] = v;
    break;
  case GREY16:
    CLIP_VALUE(v, 0, 65535);
    ima.array16[offset] = v;
    break;
  case FLOAT32:
    ima.array32[offset] = v;
    break;
  case FLOAT64:
    ima.array64[offset] = v;
    break;
  case COLOR:
    ima.arrayc[offset][0] = (uint8_t) (v & 0x000000FF);
    ima.arrayc[offset][1] = (uint8_t) ((v & 0x0000FF00) >> 8);
    ima.arrayc[offset][2] = (uint8_t) ((v & 0x00FF0000) >> 16);
    break;
  }
}

void ZStack::addIntValue(int x, int y, int z, int c, int v)
{
  if (isVirtual()) {
    return;
  }

  x -= getOffset().getX();
  y -= getOffset().getY();
  z -= getOffset().getZ();

  if (x < 0 || x >= width() || y < 0 || y >= height() || z < 0 || z >= depth() ||
      c < 0 || c >= channelNumber()) {
    return;
  }

  size_t stride_y = C_Stack::width(m_stack);
  size_t stride_z = stride_y * C_Stack::height(m_stack);
  size_t stride_c = stride_z * C_Stack::depth(m_stack);

  Image_Array ima;
  ima.array = m_stack->array;
  size_t offset = stride_c * c + stride_z * z + stride_y * y +  x;

  switch(kind()) {
  case GREY:
    v += ima.array[offset];
    CLIP_VALUE(v, 0, 255);
    ima.array[offset] = v;
    break;
  case GREY16:
    v += ima.array16[offset];
    CLIP_VALUE(v, 0, 65535);
    ima.array16[offset] = v;
    break;
  case FLOAT32:
    ima.array32[offset] += v;
    break;
  case FLOAT64:
    ima.array64[offset] += v;
    break;
  case COLOR:
  {
    int rv = (v & 0x000000FF) + ima.arrayc[offset][0];
    CLIP_VALUE(rv, 0, 255);
    ima.arrayc[offset][0] = (uint8_t) (rv);

    rv = ((v & 0x0000FF00) >> 8) + ima.arrayc[offset][1];
    CLIP_VALUE(rv, 0, 255);
    ima.arrayc[offset][1] = (uint8_t) (rv);

    rv = ((v & 0x00FF0000) >> 16) + ima.arrayc[offset][2];
    CLIP_VALUE(rv, 0, 255);
    ima.arrayc[offset][2] = (uint8_t) (rv);
  }
    break;
  }

  deprecateSingleChannelView(c);
}
int ZStack::getIntValue8WithXCheckOnly(int x, int y, int z, int c) const
{
  x -= getOffset().getX();
  y -= getOffset().getY();
  z -= getOffset().getZ();

  if (x < 0 || x >= width()) {
    return 0;
  }

  size_t stride_y = m_stack->width;
  size_t stride_z = stride_y * m_stack->height;
  size_t stride_c = stride_z * m_stack->depth;

  Image_Array ima;
  ima.array = m_stack->array;
  size_t offset = stride_c * c + stride_z * z + stride_y * y +  x;

  return ima.array8[offset];
}

int ZStack::getIntValue(int x, int y, int z, int c) const
{
  if (isVirtual()) {
    return 0;
  }

  x -= getOffset().getX();
  y -= getOffset().getY();
  z -= getOffset().getZ();

  if (x < 0 || x >= width() || y < 0 || y >= height() || z < 0 || z >= depth() ||
      c < 0 || c >= channelNumber()) {
    return 0;
  }

  size_t stride_y = m_stack->width;
  size_t stride_z = stride_y * m_stack->height;
  size_t stride_c = stride_z * m_stack->depth;

  Image_Array ima;
  ima.array = m_stack->array;
  size_t offset = stride_c * c + stride_z * z + stride_y * y +  x;

  switch (kind()) {
  case GREY:
    return ima.array8[offset];
  case COLOR:
  {
    int v = ima.arrayc[offset][2];
    v = (v << 8) + ima.arrayc[offset][1];
    v = (v << 8) + ima.arrayc[offset][0];
    return v;
  }
  case GREY16:
    return ima.array16[offset];
  case FLOAT32:
    return ima.array32[offset];
  case FLOAT64:
    return ima.array64[offset];
  }

  return 0;
}

int ZStack::getIntValueLocal(int x, int y, int z, int c) const
{
  if (isVirtual()) {
    return 0;
  }

  if (x < 0 || x >= width() || y < 0 || y >= height() || z < 0 || z >= depth() ||
      c < 0 || c >= channelNumber()) {
    return 0;
  }

  size_t stride_y = m_stack->width;
  size_t stride_z = stride_y * m_stack->height;
  size_t stride_c = stride_z * m_stack->depth;

  Image_Array ima;
  ima.array = m_stack->array;
  size_t offset = stride_c * c + stride_z * z + stride_y * y +  x;

  switch (kind()) {
  case GREY:
    return ima.array8[offset];
  case COLOR:
  {
    int v = ima.arrayc[offset][2];
    v = (v << 8) + ima.arrayc[offset][1];
    v = (v << 8) + ima.arrayc[offset][0];
    return v;
  }
  case GREY16:
    return ima.array16[offset];
  case FLOAT32:
    return ima.array32[offset];
  case FLOAT64:
    return ima.array64[offset];
  }

  return 0;
}

int ZStack::getIntValue(size_t index, int c) const
{
  if (isVirtual()) {
    return 0;
  }

  size_t voxelNumber = getVoxelNumber();

  if (index >= voxelNumber) {
    return 0;
  }


  Image_Array ima;
  ima.array = m_stack->array;
  size_t offset = voxelNumber * c + index;

  switch (kind()) {
  case GREY:
    return ima.array8[offset];
  case COLOR:
  {
    int v = ima.arrayc[offset][2];
    v = (v << 8) + ima.arrayc[offset][1];
    v = (v << 8) + ima.arrayc[offset][0];
    return v;
  }
  case GREY16:
    return ima.array16[offset];
  case FLOAT32:
    return ima.array32[offset];
  case FLOAT64:
    return ima.array64[offset];
  }

  return 0;
}

double ZStack::saturatedIntensity() const
{
  if (kind() == 1)
    return 255;
  if (kind() == 2) {
    for (int ch=0; ch<channelNumber(); ++ch) {
      if (const_cast<ZStack*>(this)->max(ch) > 4095)
        return 65535;
      else
        return 4095;
    }
  }
  return 1.0;
}

double ZStack::value(size_t index, int c) const
{
  return singleChannelStack(c)->value(index);
}

void ZStack::setValue(int x, int y, int z, int c, double v)
{
  if (!(IS_IN_CLOSE_RANGE(x, 0, width() - 1) &&
        IS_IN_CLOSE_RANGE(y, 0, height() - 1) &&
        IS_IN_CLOSE_RANGE(z, 0, depth() - 1) &&
        IS_IN_CLOSE_RANGE(c, 0, channelNumber() - 1))) {
    return;
  }

  singleChannelStack(c)->setValue(x, y, z, v);
}

int ZStack::autoThreshold(int ch) const
{
  const Stack *stack = c_stack(ch);

  int thre = 0;
  if (stack->array != NULL) {
    double scale = 0.5*stack->width * stack->height * stack->depth * stack->kind /
        (1024*1024*1024);
    if (scale >= 1.0) {
      scale = std::ceil(std::sqrt(scale + 0.1));
      stack = C_Stack::resize(stack, stack->width/scale, stack->height/scale, stack->depth);
    }

    int conn = 18;
    Stack *locmax = Stack_Locmax_Region(stack, conn);
    Stack_Label_Objects_Ns(locmax, NULL, 1, 2, 3, conn);
    int nvoxel = Stack_Voxel_Number(locmax);
    int i;

    for (i = 0; i < nvoxel; i++) {
      if (locmax->array[i] < 3) {
        locmax->array[i] = 0;
      } else {
        locmax->array[i] = 1;
      }
    }

    int *hist = Stack_Hist_M(stack, locmax);
    C_Stack::kill(locmax);

    if (hist != NULL) {
      int low, high;
      Int_Histogram_Range(hist, &low, &high);

      thre = Int_Histogram_Triangle_Threshold(hist, low, high - 1);

      if (stack != c_stack(ch))
        C_Stack::kill(const_cast<Stack*>(stack));
      free(hist);
    }
  }
  return thre;
}

vector<double> ZStack::color(size_t index) const
{
  vector<double> c(channelNumber());
  for (size_t i = 0; i < c.size(); ++i) {
    c[i] = value(index, i);
  }

  return c;
}

bool ZStack::equalColor(size_t index, const std::vector<double> &co) const
{
  int cn = channelNumber();
  for (int i = 0; i < cn; ++i) {
    if (value(index, i) != co[i]) {
      return false;
    }
  }

  return true;
}

//The object itself must be uint8_t based
bool ZStack::equalColor(size_t index, const std::vector<uint8_t> &co) const
{
  int cn = channelNumber();
  for (int i = 0; i < cn; ++i) {
    if (value8(index, i) != co[i]) {
      return false;
    }
  }

  return true;
}

bool ZStack::equalColor(size_t index, const uint8_t *co, size_t length) const
{
  for (size_t i = 0; i < length; ++i) {
    if (value8(index, i) != co[i]) {
      return false;
    }
  }

  return true;
}

bool ZStack::equalColor(size_t index, size_t channelOffset, const uint8_t *co, size_t length) const
{
  const uint8_t *array = array8() + index;
  for (size_t i = 0; i < length; ++i) {
    if (*array != co[i]) {
      return false;
    }
    array += channelOffset;
  }

  return true;
}

vector<double> ZStack::color(int x, int y, int z) const
{
  vector<double> c(channelNumber());
  for (size_t i = 0; i < c.size(); ++i) {
    c[i] = value(x, y, z, i);
  }

  return c;
}

void ZStack::setValue(size_t index, int c, double value)
{
  singleChannelStack(c)->setValue(index, value);
}

int ZStack::maxIntensityDepth(int x, int y, int c) const
{
  return singleChannelStack(c)->maxIntensityDepth(x, y);
}

bool ZStack::isThresholdable()
{
  if (!isVirtual()) {
    if (channelNumber() == 1) {
      return true;
    }
  }

  return false;
}

bool ZStack::isTracable()
{
  return isThresholdable();
}

bool ZStack::isSwc()
{
  if (isVirtual()) {
    return ZFileType::FileType(m_source.firstUrl()) == ZFileType::FILE_SWC;
    /*
    if (m_source != NULL) {
      if (m_source->type == STACK_DOC_SWC_FILE) {
        return true;
      }
    }
    */
  }

  return false;
}

void ZStack::bcAdjustHint(double *scale, double *offset, int c)
{
  //debug
#ifdef _DEBUG_
  std::cout << "Bc adjust hint" << std::endl;
#endif

  singleChannelStack(c)->bcAdjustHint(scale, offset);
  /*
  if (isDeprecated(STACK_STAT)) {
    m_stat = new ZStack_Stat();
    m_stat->update(c_stack(c));
  }

  *scale = m_stat->m_greyScale;
  *offset = m_stat->m_greyOffset;
  **/
}

bool ZStack::isBinary()
{
  if (isVirtual() || channelNumber() > 1) {
    return false;
  }
  return singleChannelStack(0)->isBinary();
}

bool ZStack::updateFromSource()
{
  /*
  if (m_source != NULL ) {   // use load to support 2-channel 16bit image       todo: add file bundle support as in Import_Stack_Document
    //    Stack *stack = Import_Stack_Document(m_source);
    //    if (stack != NULL) {
    //      clean();
    //      load(stack, true);
    //      return true;
    //    }
    return load(sourcePath());
  }

  return false;
  */

  if (m_source.readStack(this) == NULL) {
    return false;
  }



  return true;
}

bool ZStack::hasSameValue(size_t index1, size_t index2, size_t channelOffset)
{
  return C_Stack::hasSameValue(m_stack, index1, index2, channelOffset);
}

bool ZStack::hasSameValue(size_t index1, size_t index2)
{
  return C_Stack::hasSameValue(m_stack, index1, index2);
}

double ZStack::min()
{
  double minValue = 0.0;

  if (!isVirtual()) {
    minValue = min(0);
    for (int c = 1; c < channelNumber(); ++c) {
      double value = min(c);
      if (minValue > value) {
        minValue = value;
      }
    }
  }

  return minValue;
}

double ZStack::min(int c) const
{
  return singleChannelStack(c)->min();
}

double ZStack::max()
{
  double maxValue = 255;

  if (!isVirtual()) {
    maxValue = max(0);

    for (int c = 1; c < channelNumber(); ++c) {
      double value = max(c);
      if (maxValue < value) {
        maxValue = value;
      }
    }
  }

  return maxValue;
}

double ZStack::max(int c) const
{
  return singleChannelStack(c)->max();
}

bool ZStack::binarize(int threshold)
{
  bool isChanged = false;

  if (!isVirtual() && isThresholdable()) {
    isChanged = singleChannelStack(0)->binarize(threshold);
    if (kind() != GREY) {
      C_Stack::translate(singleChannelStack(0)->data(), GREY, 1);
      data()->kind = GREY;
    }
    if (isChanged) {
      deprecateDependent(MC_STACK);
    }
  }

  return isChanged;
}

bool ZStack::bwsolid()
{
  bool isChanged = false;
  if (isBinary()) {
    isChanged = singleChannelStack(0)->bwsolid();
    if (isChanged) {
      deprecateDependent(MC_STACK);
    }
  }

  return isChanged;
}

bool ZStack::bwperim()
{
  bool isChanged = false;
  if (isBinary()) {
    isChanged = singleChannelStack(0)->bwperim();
    if (isChanged) {
      deprecateDependent(MC_STACK);
    }
  }

  return isChanged;
}

bool ZStack::enhanceLine()
{
  bool isChanged = false;

  if (!isVirtual() && channelNumber() == 1) {
    isChanged = singleChannelStack(0)->enhanceLine();
    if (isChanged) {
      deprecateDependent(MC_STACK);
    }
  }

  return isChanged;
}

void ZStack::extractChannel(int c)
{
  if (!isVirtual()) {
    memmove(array8(c), array8(0), getByteNumber(SINGLE_CHANNEL));
    C_Stack::setChannelNumber(m_stack, 1);
  }
}

Stack *ZStack::copyChannel(int c)
{
  Stack *out = NULL;
  if (!isVirtual() && c < channelNumber()) {
    out = C_Stack::clone(c_stack(c));
  }
  return out;
}

std::string ZStack::sourcePath() const
{
  return m_source.firstUrl();
}

bool ZStack::isEmpty() const
{
  if (m_stack == NULL) {
    return true;
  }

  if ((m_stack->width  == 0) && (m_stack->height == 0) && (m_stack->depth == 0)) {
    return true;
  }

  return false;
}

bool ZStack::isVirtual() const
{
  if (isEmpty()) {
    return false;
  }

  return m_stack->array == NULL;
}

bool ZStack::hasData() const
{
  return !isEmpty() && !isVirtual();
}

void *ZStack::getDataPointer(int c, int slice) const
{
  const uint8_t *array = array8(c);
  array += getByteNumber(SINGLE_PLANE) * slice;

  return (void*) array;
}

const uint8_t* ZStack::getDataPointer(int x, int y, int z) const
{
  if (isVirtual()) {
    return NULL;
  }

  if (getBoundBox().contains(x, y, z)) {
    x -= getOffset().getX();
    y -= getOffset().getY();
    z -= getOffset().getZ();

    size_t area = width() * height();
    return array8() + area * z + y * width() + x;
  }

  return NULL;
}

bool ZStack::watershed(int c)
{
  if (!isVirtual() && c < channelNumber()) {
    return singleChannelStack(c)->watershed();
  }
  return false;
}

ZStack* ZStack::createSubstack(const std::vector<std::vector<double> > &selected)
{
  ZStack *substack =
      new ZStack(kind(), width(), height(), depth(), channelNumber());

  size_t volume = this->getVoxelNumber();

  for (size_t voxelIndex = 0; voxelIndex != volume; ++voxelIndex) {
    bool isSelected =  false;
    for (size_t selectIndex = 0; selectIndex < selected.size();
         ++selectIndex) {
      isSelected = equalColor(voxelIndex, selected[selectIndex]);
      if (isSelected) {
        break;
      }
    }

    for (int c = 0; c < channelNumber(); c++) {
      if (isSelected) {
        substack->setValue(voxelIndex, c, value(voxelIndex, c));
      } else {
        substack->setValue(voxelIndex, c, 0);
      }
    }
  }

  return substack;
}

ZStack* ZStack::clone() const
{
  ZStack *stack = NULL;

  if (hasData()) {
    stack = new ZStack(
          kind(), width(), height(), depth(), channelNumber());
    memcpy(stack->rawChannelData(), rawChannelData(), getByteNumber());

//    stack->m_resolution = m_resolution;
//    stack->m_preferredZScale = m_preferredZScale;
    stack->m_source = m_source;
    stack->m_offset = m_offset;
  }

  return stack;
}

double ZStack::averageIntensity(ZStack *mask)
{
  size_t volume = getVoxelNumber();
  double v = 0.0;
  int count = 0;
  for (size_t i = 0; i < volume; ++i) {
    if (mask->value8(i) > 0) {
      v += value(i);
      ++count;
    }
  }

  v /= count;

  return v;
}

void ZStack::copyValueFrom(const void *buffer, size_t length, int ch)
{
  memcpy(rawChannelData(ch), buffer, length);
  deprecateDependent(MC_STACK);
}

void ZStack::copyValueFrom(const void *buffer, size_t length, void *loc)
{
  memcpy(loc, buffer, length);
  deprecateDependent(MC_STACK);
}

#ifdef _QT_GUI_USED_
void ZStack::setChannelColor(int ch, double r, double g, double b)
{
  m_channelColors[ch]->set(glm::vec3(r, g, b));
}

bool ZStack::loadLSMInfo(const QString &filepath)
{
  if (!filepath.endsWith(".lsm", Qt::CaseInsensitive))
    return false;

  FILE *fp = fopen(filepath.toLocal8Bit().data(), "rb");

  uint16_t endian;
  fread(&endian, 2, 1, fp);
  if (endian != 0x4949) {
    fclose(fp);
    return false;
  }

  uint16_t magic;
  fread(&magic, 2, 1, fp);
  if (magic != 42) {
    fclose(fp);
    return false;
  }

  uint32_t ifd_offset;
  fread(&ifd_offset, 4, 1, fp);

  fseek(fp, ifd_offset, SEEK_SET);

  uint16_t nifd;
  fread(&nifd, 2, 1, fp);

  uint16_t ifd_label;
  fread(&ifd_label, 2, 1, fp);

  uint16_t i;
  for (i = 1; i < nifd; i++) {
    if (ifd_label == TIF_CZ_LSMINFO) {
      break;
    }
    fseek(fp, 10, SEEK_CUR);
    fread(&ifd_label, 2, 1, fp);
  }
  if (ifd_label != TIF_CZ_LSMINFO) {
    fclose(fp);
    return false;
  }

  uint16_t ifd_type;
  fread(&ifd_type, 2, 1, fp);

  uint32_t ifd_length;
  fread(&ifd_length, 4, 1, fp);

  fread(&ifd_offset, 4, 1, fp);

  fseek(fp, ifd_offset, SEEK_SET);
  fread(&(m_lsmInfo.m_basicInfo), sizeof(Cz_Lsminfo), 1, fp);

  //m_channelColors.clear();
  initChannelColors();

  m_lsmInfo.m_lsmChannelNames.clear();
  m_lsmInfo.m_lsmTimeStamps.clear();
  m_lsmInfo.m_lsmChannelDataTypes.clear();

  if (m_lsmInfo.m_basicInfo.u32OffsetChannelColors != 0) {
    fseek(fp, m_lsmInfo.m_basicInfo.u32OffsetChannelColors, SEEK_SET);
    fread(&(m_lsmInfo.m_lsmChannelInfo), sizeof(Lsm_Channel_Colors), 1, fp);

    char *chStruct = new char[m_lsmInfo.m_lsmChannelInfo.s32BlockSize];
    fseek(fp, m_lsmInfo.m_basicInfo.u32OffsetChannelColors, SEEK_SET);
    fread(chStruct, m_lsmInfo.m_lsmChannelInfo.s32BlockSize, 1, fp);
    std::vector<glm::col4> cls(m_lsmInfo.m_lsmChannelInfo.s32NumberColors);
    memcpy(&(cls[0]), chStruct+m_lsmInfo.m_lsmChannelInfo.s32ColorsOffset, sizeof(uint32_t)*cls.size());

    size_t offset = m_lsmInfo.m_lsmChannelInfo.s32NamesOffset;
    int nameIdx = 0;
    while (nameIdx < m_lsmInfo.m_lsmChannelInfo.s32NumberNames) {
      offset += 4;  // skip uint32_t name length
      std::string str(chStruct+offset);
      m_lsmInfo.m_lsmChannelNames.push_back(str);
      ++nameIdx;
      offset += str.size() + 1;
    }

    for (int ch=0; ch<m_lsmInfo.m_lsmChannelInfo.s32NumberColors; ++ch) {
      if (m_lsmInfo.m_lsmChannelNames.size() > (size_t)ch &&
          m_channelColors.size() > (size_t) ch) {
        std::string chName = m_lsmInfo.m_lsmChannelNames[ch];
        if (!chName.empty())
          m_channelColors[ch]->setName(chName.c_str());
        m_channelColors[ch]->set(glm::vec3(cls[ch])/255.f);
      } else {
        break;
      }
    }

    delete[] chStruct;
  }

  if (m_lsmInfo.m_basicInfo.u32OffsetTimeStamps != 0) {
    fseek(fp, m_lsmInfo.m_basicInfo.u32OffsetTimeStamps, SEEK_SET);
    fread(&m_lsmInfo.m_lsmTimeStampInfo, sizeof(Lsm_Time_Stamp_Info), 1, fp);
    double *stamps = new double[m_lsmInfo.m_lsmTimeStampInfo.s32NumberTimeStamps];
    fread(stamps, sizeof(double), m_lsmInfo.m_lsmTimeStampInfo.s32NumberTimeStamps, fp);
    for (int i=0; i<m_lsmInfo.m_lsmTimeStampInfo.s32NumberTimeStamps; ++i)
      m_lsmInfo.m_lsmTimeStamps.push_back(stamps[i]);
    delete[] stamps;
  }

  if (m_lsmInfo.m_basicInfo.u32OffsetChannelDataTypes != 0) {
    fseek(fp, m_lsmInfo.m_basicInfo.u32OffsetChannelDataTypes, SEEK_SET);
    uint32_t *dataTypes = new uint32_t[m_lsmInfo.m_basicInfo.s32DimensionChannels];
    fread(dataTypes, sizeof(uint32_t), m_lsmInfo.m_basicInfo.s32DimensionChannels, fp);
    for (int i=0; i<m_lsmInfo.m_basicInfo.s32DimensionChannels; ++i)
      m_lsmInfo.m_lsmChannelDataTypes.push_back(dataTypes[i]);
    delete[] dataTypes;
  }

  fclose(fp);

  // fill zresolution

//  m_isLSMFile = true;
  return true;
}

void ZStack::logLSMInfo()
{
//  if (!m_isLSMFile) {
//    LINFO() << sourcePath() << "is not a valid LSM file.";
//    return;
//  }

  LINFO() << "Start LSM Info for" << sourcePath();
  LINFO() << "MagicNumber:" << hex << m_lsmInfo.m_basicInfo.u32MagicNumber;
  LINFO() << "DimensionX:" << m_lsmInfo.m_basicInfo.s32DimensionX;
  LINFO() << "DimensionY:" << m_lsmInfo.m_basicInfo.s32DimensionY;
  LINFO() << "DimensionZ:" << m_lsmInfo.m_basicInfo.s32DimensionZ;
  LINFO() << "DimensionChannels:" << m_lsmInfo.m_basicInfo.s32DimensionChannels;
  LINFO() << "DimensionTime:" << m_lsmInfo.m_basicInfo.s32DimensionTime;
  switch (m_lsmInfo.m_basicInfo.s32DataType) {
  case 1: LINFO() << "DataType:" << "8-bit unsigned integer"; break;
  case 2: LINFO() << "DataType:" << "12-bit unsigned integer"; break;
  case 5: LINFO() << "DataType:" << "32-bit float(for \"Time Series Mean-of-ROIs\")"; break;
  //case 0: LINFO() << "DataType:" << "different data types for different channels, see 32OffsetChannelDataTypes"; break;
  }
  for (size_t i=0; i<m_lsmInfo.m_lsmChannelDataTypes.size(); ++i) {
    switch (m_lsmInfo.m_lsmChannelDataTypes[i]) {
    case 1: LINFO() << "Channel" << i+1 << "DataType:" << "8-bit unsigned integer"; break;
    case 2: LINFO() << "Channel" << i+1 << "DataType:" << "12-bit unsigned integer"; break;
    case 5: LINFO() << "Channel" << i+1 << "DataType:" << "32-bit float(for \"Time Series Mean-of-ROIs\")"; break;
    }
  }
  LINFO() << "ThumbnailX:" << m_lsmInfo.m_basicInfo.s32ThumbnailX;
  LINFO() << "ThumbnailY:" << m_lsmInfo.m_basicInfo.s32ThumbnailY;
  LINFO() << "VoxelSizeX in meter:" << m_lsmInfo.m_basicInfo.f64VoxelSizeX;
  LINFO() << "VoxelSizeY in meter:" << m_lsmInfo.m_basicInfo.f64VoxelSizeY;
  LINFO() << "VoxelSizeZ in meter:" << m_lsmInfo.m_basicInfo.f64VoxelSizeZ;
  switch (m_lsmInfo.m_basicInfo.u16ScanType) {
  case 0: LINFO() << "ScanType:" << "normal x-y-z-scan"; break;
  case 1: LINFO() << "ScanType:" << "z-Scan (x-z-plane)"; break;
  case 2: LINFO() << "ScanType:" << "line scan"; break;
  case 3: LINFO() << "ScanType:" << "time series x-y"; break;
  case 4: LINFO() << "ScanType:" << "time series x-z (release 2.0 or later)"; break;
  case 5: LINFO() << "ScanType:" << "time series \"Mean of ROIs\" (release 2.0 or later)"; break;
  case 6: LINFO() << "ScanType:" << "time series x-y-z (release 2.3 or later)"; break;
  case 7: LINFO() << "ScanType:" << "spline scan (release 2.5 or later)"; break;
  case 8: LINFO() << "ScanType:" << "spline plane x-z (release 2.5 or later)"; break;
  case 9: LINFO() << "ScanType:" << "time series spline plane x-z (release 2.5 or later)"; break;
  case 10: LINFO() << "ScanType:" << "point mode (release 3.0 or later)"; break;
  }
  switch (m_lsmInfo.m_basicInfo.u16SpectralScan) {
  case 0: LINFO() << "SpectralScan:" << "no spectral scan"; break;
  case 1: LINFO() << "SpectralScan:" << "image has been acquired in spectral scan mode with a META detector (release 3.0 or later)"; break;
  }
  switch (m_lsmInfo.m_basicInfo.u32DataType) {
  case 0: LINFO() << "DataType:" << "Original scan data"; break;
  case 1: LINFO() << "DataType:" << "Calculated data"; break;
  case 2: LINFO() << "DataType:" << "Animation"; break;
  }
  if (m_lsmInfo.m_basicInfo.f64TimeInterval != 0) {
    LINFO() << "TimeInterval in s:" << m_lsmInfo.m_basicInfo.f64TimeInterval;
  }
  for (size_t i=0; i<m_lsmInfo.m_lsmTimeStamps.size(); ++i) {
    LINFO() << "TimeStamp" << i+1 << "in s:" << m_lsmInfo.m_lsmTimeStamps[i];
  }
  LINFO() << "DisplayAspectX:" << m_lsmInfo.m_basicInfo.f64DisplayAspectX;
  LINFO() << "DisplayAspectY:" << m_lsmInfo.m_basicInfo.f64DisplayAspectY;
  LINFO() << "DisplayAspectZ:" << m_lsmInfo.m_basicInfo.f64DisplayAspectZ;
  LINFO() << "DisplayAspectTime:" << m_lsmInfo.m_basicInfo.f64DisplayAspectTime;
  LINFO() << "ObjectiveSphereCorrection:" << m_lsmInfo.m_basicInfo.f64objectiveSphereCorrection;
  for (size_t i=0; i<m_channelColors.size(); ++i) {
    LINFO() << "Channel" << i+1 << "Name:" << m_lsmInfo.m_lsmChannelNames[i] << "Color(RGB):" << m_channelColors[i]->get();
  }
  LINFO() << "End LSM Info for" << sourcePath();
}
#endif

bool ZStack::hasOffset() const
{
  return (m_offset.getX() != 0) || (m_offset.getY() != 0) ||
      (m_offset.getZ() != 0);
}

void ZStack::setZero()
{
  if (!isEmpty() && ! isVirtual()) {
    C_Stack::setZero(m_stack);
    deprecate(SINGLE_CHANNEL_VIEW);
  }
}

void ZStack::setOne()
{
  if (!isEmpty() && ! isVirtual()) {
    C_Stack::setOne(m_stack);
    deprecate(SINGLE_CHANNEL_VIEW);
  }
}

template <typename T>
void SwapValue(T *array, size_t length, int v1, int v2)
{
  for (size_t i = 0; i < length; ++i) {
    if (array[i] == v1) {
      array[i] = v2;
    } else if (array[i] == v2) {
      array[i] = v1;
    }
  }
}

void ZStack::swapValue(int v1, int v2)
{
  size_t voxelNumber = getVoxelNumber();
  for (int c = 0; c < channelNumber(); ++c) {
    switch (kind()) {
    case GREY:
      SwapValue(array8(c), voxelNumber, v1, v2);
      break;
    case GREY16:
      SwapValue(array16(c), voxelNumber, v1, v2);
      break;
    case FLOAT32:
      SwapValue(array32(c), voxelNumber, v1, v2);
      break;
    case FLOAT64:
      SwapValue(array64(c), voxelNumber, v1, v2);
      break;
    }
  }
}

void ZStack::printInfo() const
{
  std::cout << "Stack: " << std::endl;
  std::cout << "  Size: (" << width() << " x " << height() << " x " << depth()
            << ")" <<  std::endl;
  std::cout << "  Channel number: " << channelNumber() << std::endl;
  std::cout << "  Voxel type: " << kind() << std::endl;
  std::cout << "  Offset: " << getOffset().toString() << std::endl;
  std::cout << "  Ds Intv: " << getDsIntv().toString() << std::endl;

  if (isEmpty()) {
    std::cout << "  Empty stack." << std::endl;
  }
}

bool ZStack::reshape(int width, int height, int depth)
{
  size_t v = (size_t) width * height * depth;
  if (v == getVoxelNumber()) {
    m_stack->width = width;
    m_stack->height = height;
    m_stack->depth = depth;

    return true;
  }

  return false;
}

bool ZStack::paste(ZStack *dst, int valueIgnored, double alpha) const
{
  if (dst != NULL) {
    if (kind() != dst->kind()) {
      return false;
    }

    if (isVirtual() || dst->isVirtual()) {
      return false;
    }

    if (isEmpty() || dst->isEmpty()) {
      return false;
    }

    int ch = imin2(dst->channelNumber(), channelNumber());
    ZIntPoint offset = getOffset() - dst->getOffset();
    int x0 = offset.getX();
    int y0 = offset.getY();
    int z0 = offset.getZ();

    for (int i = 0; i < ch; ++i) {
      C_Stack::setBlockValue(dst->c_stack(i), c_stack(i), x0, y0, z0,
                             valueIgnored, -1, alpha);
    }

    return true;
  }

  return false;
}

void ZStack::getBoundBox(Cuboid_I *box) const
{
  if (box != NULL) {
    int x0 = getOffset().getX();
    int y0 = getOffset().getY();
    int z0 = getOffset().getZ();

    Cuboid_I_Set_S(box, x0, y0, z0, width(), height(), depth());
  }
}

ZIntCuboid ZStack::getBoundBox() const
{
  ZIntCuboid box;
  box.setFirstCorner(getOffset());
  box.setSize(width(), height(), depth());

  return box;
}

bool ZStack::contains(int x, int y, int z) const
{
  Cuboid_I box;
  getBoundBox(&box);

  return Cuboid_I_Hit(&box, x, y, z) > 0;
}

bool ZStack::contains(const ZIntPoint &pt) const
{
  return contains(pt.getX(), pt.getY(), pt.getZ());
}

bool ZStack::contains(double x, double y) const
{
  return IS_IN_CLOSE_RANGE(x, m_offset.getX(), m_offset.getX() + width() - 1) &&
      IS_IN_CLOSE_RANGE(y, m_offset.getY(), m_offset.getY() + height() - 1);
}

bool ZStack::contains(const ZPoint &pt) const
{
  return IS_IN_CLOSE_RANGE3(
        pt.x(), pt.y(), pt.z(),
        m_offset.getX(), m_offset.getX() + width() - 1,
        m_offset.getY(), m_offset.getY() + height() - 1,
        m_offset.getZ(), m_offset.getZ() + depth() - 1);
}

bool ZStack::containsRaw(double x, double y, double z) const
{
  if (z >= 0) {
    return IS_IN_CLOSE_RANGE3(x, y, z, 0, width() - 1, 0, height() - 1,
                              0, depth() - 1);
  } else {
    return IS_IN_CLOSE_RANGE(x, 0, width() - 1) &&
        IS_IN_CLOSE_RANGE(y, 0, height() - 1);
  }
}

bool ZStack::containsRaw(const ZPoint &pt) const
{
  return containsRaw(pt.x(), pt.y(), pt.z());
}

void ZStack::setBlockValue(int x0, int y0, int z0, const ZStack *stack)
{
  x0 -= m_offset.getX();
  y0 -= m_offset.getY();
  z0 -= m_offset.getZ();

  for (int c = 0; c < channelNumber(); ++c) {
    const Stack *src = stack->c_stack(c);
    Stack *dst = c_stack(c);
    C_Stack::setBlockValue(dst, src, x0, y0, z0);
  }
}

void ZStack::setBlockValue(const ZStack *stack)
{
  if (stack) {
    setBlockValue(
          stack->getOffset().getX(), stack->getOffset().getY(),
          stack->getOffset().getZ(), stack);
  }
}

ZStack* ZStack::makeCrop(const ZIntCuboid &cuboid) const
{
  if (isEmpty()) {
    return NULL;
  }
  ZStack *cropped = NULL;

  if (isVirtual()) {
    cropped = ZStackFactory::MakeVirtualStack(cuboid);
  } else {
    if (!cuboid.isEmpty()) {
      int nchannel = channelNumber();
      Mc_Stack *newStack = C_Stack::make(kind(), cuboid.getWidth(),
                                         cuboid.getHeight(), cuboid.getDepth(),
                                         nchannel);

      for (int channel = 0; channel < nchannel; ++channel) {
        Stack stackView;
        Stack newStackView;
        C_Stack::view(m_stack, &stackView, channel);
        C_Stack::view(newStack, &newStackView, channel);

        ZIntPoint startPoint = cuboid.getFirstCorner() - m_offset;
        C_Stack::crop(&stackView, startPoint.getX(), startPoint.getY(),
                      startPoint.getZ(),
                      cuboid.getWidth(), cuboid.getHeight(),
                      cuboid.getDepth(), &newStackView);
#ifdef _DEBUG_2
        newStackView.text = NULL;
        C_Stack::write(GET_DATA_DIR + "/test.tif", &newStackView);
#endif
      }
      cropped = new ZStack;
      cropped->setData(newStack);
      cropped->setOffset(cuboid.getFirstCorner());
    }
  }

  return cropped;
}

void ZStack::crop(const ZIntCuboid &cuboid)
{
  if (isEmpty()) {
    return;
  }

  if (isVirtual()) {
    m_stack->width = cuboid.getWidth();
    m_stack->height = cuboid.getHeight();
    m_stack->depth = cuboid.getDepth();
    m_offset = cuboid.getFirstCorner();
  } else {
    if (!cuboid.isEmpty()) {
      int nchannel = channelNumber();
      Mc_Stack *newStack = C_Stack::make(kind(), cuboid.getWidth(),
                                         cuboid.getHeight(), cuboid.getDepth(),
                                         nchannel);

      for (int channel = 0; channel < nchannel; ++channel) {
        Stack stackView;
        Stack newStackView;
        C_Stack::view(m_stack, &stackView, channel);
        C_Stack::view(newStack, &newStackView, channel);

        ZIntPoint startPoint = cuboid.getFirstCorner() - m_offset;
        C_Stack::crop(&stackView, startPoint.getX(), startPoint.getY(),
                      startPoint.getZ(),
                      cuboid.getWidth(), cuboid.getHeight(),
                      cuboid.getDepth(), &newStackView);
#ifdef _DEBUG_2
        newStackView.text = NULL;
        C_Stack::write(GET_DATA_DIR + "/test.tif", &newStackView);
#endif
      }
      m_offset = cuboid.getFirstCorner();
      setData(newStack);
    } else {
      deprecate(MC_STACK);
    }
  }
}

void ZStack::downsampleMax(int xintv, int yintv, int zintv)
{
  if (xintv == 0 && yintv == 0 && zintv == 0) {
    return;
  }

  int w = width();
  int h = height();
  int d = depth();
  int swidth = w / (xintv + 1) + (w % (xintv + 1) > 0);
  int sheight = h / (yintv + 1) + (h % (yintv + 1) > 0);
  int sdepth = d / (zintv + 1) + (d % (zintv + 1) > 0);

  m_offset.setX(m_offset.getX() / (xintv + 1));
  m_offset.setY(m_offset.getY() / (yintv + 1));
  m_offset.setZ(m_offset.getZ() / (zintv + 1));

  if (isVirtual()) {
    m_stack->width = swidth;
    m_stack->height = sheight;
    m_stack->depth = sdepth;
  } else {
    Stack dst;
    Stack src;
    Mc_Stack *result = C_Stack::make(
          kind(), swidth, sheight, sdepth, channelNumber());
    Mc_Stack *original = m_stack;

    for (int c = 0; c < channelNumber(); ++c) {
      C_Stack::view(result, &dst, c);
      C_Stack::view(original, &src, c);
      C_Stack::downsampleMax(&src, xintv, yintv, zintv, &dst);
    }

    setData(result);
  }
}

void ZStack::pushDsIntv(int dx, int dy, int dz)
{
  m_dsIntv.setX((m_dsIntv.getX() + 1) * (dx + 1) - 1);
  m_dsIntv.setY((m_dsIntv.getY() + 1) * (dy + 1) - 1);
  m_dsIntv.setZ((m_dsIntv.getZ() + 1) * (dz + 1) - 1);
}

void ZStack::pushDsIntv(const ZIntPoint &dsIntv)
{
  pushDsIntv(dsIntv.getX(), dsIntv.getY(), dsIntv.getZ());
}

void ZStack::downsampleMin(int xintv, int yintv, int zintv)
{
  if (xintv == 0 && yintv == 0 && zintv == 0) {
    return;
  }

  int w = width();
  int h = height();
  int d = depth();
  int swidth = w / (xintv + 1) + (w % (xintv + 1) > 0);
  int sheight = h / (yintv + 1) + (h % (yintv + 1) > 0);
  int sdepth = d / (zintv + 1) + (d % (zintv + 1) > 0);

  m_offset.setX(m_offset.getX() / (xintv + 1));
  m_offset.setY(m_offset.getY() / (yintv + 1));
  m_offset.setZ(m_offset.getZ() / (zintv + 1));

  if (isVirtual()) {
    m_stack->width = swidth;
    m_stack->height = sheight;
    m_stack->depth = sdepth;
  } else {
    Stack dst;
    Stack src;
    Mc_Stack *result = C_Stack::make(GREY, swidth, sheight, sdepth, 1);
    Mc_Stack *original = m_stack;

    for (int c = 0; c < channelNumber(); ++c) {
      C_Stack::view(result, &dst, c);
      C_Stack::view(original, &src, c);
      C_Stack::downsampleMin(&src, xintv, yintv, zintv, &dst);
    }

    setData(result);
  }
}

void ZStack::downsampleMean(int xintv, int yintv, int zintv)
{
  if (xintv == 0 && yintv == 0 && zintv == 0) {
    return;
  }

  int w = width();
  int h = height();
  int d = depth();
  int swidth = w / (xintv + 1) + (w % (xintv + 1) > 0);
  int sheight = h / (yintv + 1) + (h % (yintv + 1) > 0);
  int sdepth = d / (zintv + 1) + (d % (zintv + 1) > 0);

  m_offset.setX(m_offset.getX() / (xintv + 1));
  m_offset.setY(m_offset.getY() / (yintv + 1));
  m_offset.setZ(m_offset.getZ() / (zintv + 1));

  if (isVirtual()) {
    m_stack->width = swidth;
    m_stack->height = sheight;
    m_stack->depth = sdepth;
  } else {
    Stack dst;
    Stack src;
    Mc_Stack *result = C_Stack::make(GREY, swidth, sheight, sdepth, 1);
    Mc_Stack *original = m_stack;

    for (int c = 0; c < channelNumber(); ++c) {
      C_Stack::view(result, &dst, c);
      C_Stack::view(original, &src, c);
      C_Stack::downsampleMean(&src, xintv, yintv, zintv, &dst);
    }

    setData(result);
  }
}


void ZStack::downsampleMinIgnoreZero(int xintv, int yintv, int zintv)
{
  if (xintv == 0 && yintv == 0 && zintv == 0) {
    return;
  }

  int w = width();
  int h = height();
  int d = depth();
  int swidth = w / (xintv + 1) + (w % (xintv + 1) > 0);
  int sheight = h / (yintv + 1) + (h % (yintv + 1) > 0);
  int sdepth = d / (zintv + 1) + (d % (zintv + 1) > 0);

  m_offset.setX(m_offset.getX() / (xintv + 1));
  m_offset.setY(m_offset.getY() / (yintv + 1));
  m_offset.setZ(m_offset.getZ() / (zintv + 1));

  if (isVirtual()) {
    m_stack->width = swidth;
    m_stack->height = sheight;
    m_stack->depth = sdepth;
  } else {
    Stack dst;
    Stack src;
    Mc_Stack *result = C_Stack::make(GREY, swidth, sheight, sdepth, 1);
    Mc_Stack *original = m_stack;

    for (int c = 0; c < channelNumber(); ++c) {
      C_Stack::view(result, &dst, c);
      C_Stack::view(original, &src, c);
      C_Stack::downsampleMinIgnoreZero(&src, xintv, yintv, zintv, &dst);
    }

    setData(result);
  }
}

void ZStack::swapData(ZStack *stack)
{
  std::swap(m_stack, stack->m_stack);
  std::swap(m_dealloc, stack->m_dealloc);
  std::swap(m_source, stack->m_source);
//  std::swap(m_preferredZScale, stack->m_preferredZScale);
//  std::swap(m_resolution, stack->m_resolution);
  std::swap(m_offset, stack->m_offset);

  deprecateDependent(MC_STACK);
  stack->deprecateDependent(MC_STACK);
}

bool ZStack::equals(const ZStack &stack2) const
{
  if (isVirtual() != stack2.isVirtual()) {
    return false;
  }

  if (!getBoundBox().equals(stack2.getBoundBox())) {
    return false;
  }

  if (kind() != stack2.kind()) {
    return false;
  }

  if (channelNumber() != stack2.channelNumber()) {
    return false;
  }

  const uint8_t *array1 = array8();
  const uint8_t *array2 = stack2.array8();

  if (array1 != NULL && array2 != NULL) {
    size_t byteNumber = getByteNumber();
    for (size_t i = 0; i < byteNumber; ++i) {
      if (array1[i] != array2[i]) {
        return false;
      }
    }
  }

  return true;
}
