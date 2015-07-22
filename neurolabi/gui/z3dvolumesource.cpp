#include "z3dvolumesource.h"

#include "zstack.hxx"
#include "zstackdoc.h"
#include "tz_stack_attribute.h"
#include "z3dgpuinfo.h"
#include "zsparseobject.h"
#include "neutubeconfig.h"
#include "zsparsestack.h"

const size_t Z3DVolumeSource::m_nChannelSupport = 10;

Z3DVolumeSource::Z3DVolumeSource(ZStackDoc *doc, size_t maxVoxelNumber)
  : Z3DProcessor()
  , m_stackOutputPort("Stack")
  , m_xScale("X Scale", 1.0f, 0.1f, 50.f)
  , m_yScale("Y Scale", 1.0f, 0.1f, 50.f)
  , m_zScale("Z Scale", 1.0f, 0.1f, 500.f)
  , m_isVolumeDownsampled("Volume Is Downsampled", false)
  , m_isSubVolume("Is Subvolume", false)
  , m_zoomInViewSize("Zoom In View Size", 256, 128, 512)
  , m_doc(doc)
  , m_widgetsGroup(NULL)
{
  if (maxVoxelNumber == 0) {
    int currentAvailableTexMem = Z3DGpuInfoInstance.getAvailableTextureMemory();
    if (currentAvailableTexMem != -1 && currentAvailableTexMem <= 256000)
      m_maxVoxelNumber = 256 * 256 * 256 * 2;
    else
      m_maxVoxelNumber = 512 * 512 * 512 * 1;
  } else {
    m_maxVoxelNumber = maxVoxelNumber;
  }

  for (size_t i=0; i<m_nChannelSupport; i++) {
    QString name = QString("Volume%1").arg(i+1);
    m_outputPorts.push_back(new Z3DOutputPort<Z3DVolume>(name));
    addPort(m_outputPorts[i]);
  }
  addPort(m_stackOutputPort);

  loadData();

  addParameter(m_xScale);
  addParameter(m_yScale);
  addParameter(m_zScale);
  m_isVolumeDownsampled.setEnabled(false);
  addParameter(m_isVolumeDownsampled);
  m_isSubVolume.setEnabled(false);
  addParameter(m_isSubVolume);
  m_zoomInViewSize.setTracking(false);
  m_zoomInViewSize.setSingleStep(32);
  addParameter(m_zoomInViewSize);
  connect(&m_xScale, SIGNAL(valueChanged()), this, SLOT(changeXScale()));
  connect(&m_yScale, SIGNAL(valueChanged()), this, SLOT(changeYScale()));
  connect(&m_zScale, SIGNAL(valueChanged()), this, SLOT(changeZScale()));
  connect(&m_zoomInViewSize, SIGNAL(valueChanged()), this, SLOT(changeZoomInViewSize()));
}

Z3DVolumeSource::~Z3DVolumeSource()
{
  for (size_t i=0; i<m_outputPorts.size(); i++)
    delete m_outputPorts[i];
}

void Z3DVolumeSource::loadData()
{
  if (m_doc != NULL) {
    if (m_doc->hasStackData()) {
      if (m_doc->hasPlayer(ZStackObjectRole::ROLE_3DPAINT)) {
        readVolumesWithObject();
      } else {
        readVolumes();
      }
    } else if (m_doc->hasStack() && m_doc->hasSparseObject()) {
      if (m_doc->hasPlayer(ZStackObjectRole::ROLE_3DPAINT)) {
        readSparseVolumeWithObject();
      } else {
        readSparseVolume();
      }
    } else if (m_doc->hasStack()) {
      if (m_doc->hasSparseStack()) {
        readSparseStack();
      }
    }
  }
}

void Z3DVolumeSource::process(Z3DEye) {}

void Z3DVolumeSource::initialize()
{
  Z3DProcessor::initialize();
  sendData();
  CHECK_GL_ERROR;
}

void Z3DVolumeSource::deinitialize()
{
  clearVolume();
  clearZoomInVolume();
  CHECK_GL_ERROR;
  Z3DProcessor::deinitialize();
}

void Z3DVolumeSource::readVolumes()
{
  if (m_doc == NULL) {
    return;
  }

  clearVolume();
  int nchannel = m_doc->hasStackData() ?
        m_doc->getStack()->channelNumber() : 0;
  if (nchannel > 0) {
    for (int i=0; i<nchannel; i++) {
      Stack *stack = m_doc->getStack()->c_stack(i);

      //Under deveopment
      ZPoint offset(m_doc->getStack()->getOffset().getX() * m_xScale.get(),
                    m_doc->getStack()->getOffset().getY() * m_yScale.get(),
                    m_doc->getStack()->getOffset().getZ() * m_zScale.get());
      if (m_doc->getStack()->getVoxelNumber() * nchannel > m_maxVoxelNumber) { //Downsample big stack
        m_isVolumeDownsampled.set(true);
        double scale = std::sqrt((m_maxVoxelNumber*1.0) /
                                 (m_doc->getStack()->getVoxelNumber() * nchannel));
        int height = (int)(stack->height * scale);
        int width = (int)(stack->width * scale);
        int depth = stack->depth;
        double widthScale = 1.0;
        double heightScale = 1.0;
        double depthScale = 1.0;
        int maxTextureSize = 100;
        if (stack->depth > 1)
          maxTextureSize = Z3DGpuInfoInstance.getMax3DTextureSize();
        else
          maxTextureSize = Z3DGpuInfoInstance.getMaxTextureSize();

        if (maxTextureSize > 1024) {
          maxTextureSize = 1024;
        }

        if (height > maxTextureSize) {
          heightScale = (double)maxTextureSize / height;
          height = std::floor(height * heightScale);
        }
        if (width > maxTextureSize) {
          widthScale = (double)maxTextureSize / width;
          width = std::floor(width * widthScale);
        }
        if (depth > maxTextureSize) {
          depthScale = (double)maxTextureSize / depth;
          depth = std::floor(depth * depthScale);
        }

        widthScale *= scale;
        heightScale *= scale;

        Stack *stack2 = Resize_Stack(stack, width, height, depth);
        Translate_Stack(stack2, GREY, 1);

        if (m_doc->getStack()->isBinary()) {
          size_t volume = Stack_Voxel_Number(stack2);
          for (size_t voxelIndex = 0; voxelIndex < volume; ++voxelIndex) {
            if (stack2->array[voxelIndex] == 1) {
              stack2->array[voxelIndex] = 255;
            }
          }
        }

        Z3DVolume *vh = new Z3DVolume(
              stack2, glm::vec3(1.f/widthScale, 1.f/heightScale, 1.f/depthScale),
              glm::vec3(m_xScale.get(), m_yScale.get(), m_zScale.get()),
              glm::vec3(offset.x(), offset.y(),
                        offset.z())
                                      /*glm::vec3(.0)*/);

        m_volumes.push_back(vh);
      } else { //small stack
        double widthScale = 1.0;
        double heightScale = 1.0;
        double depthScale = 1.0;
        int height = C_Stack::height(stack);
        int width = C_Stack::width(stack);
        int depth = C_Stack::depth(stack);
        int maxTextureSize = 100;
        if (stack->depth > 1)
          maxTextureSize = Z3DGpuInfoInstance.getMax3DTextureSize();
        else
          maxTextureSize = Z3DGpuInfoInstance.getMaxTextureSize();

        if (height > maxTextureSize) {
          heightScale = (double)maxTextureSize / height;
          height = std::floor(height * heightScale);
        }
        if (width > maxTextureSize) {
          widthScale = (double)maxTextureSize / width;
          width = std::floor(width * widthScale);
        }
        if (depth > maxTextureSize) {
          depthScale = (double)maxTextureSize / depth;
          depth = std::floor(depth * depthScale);
        }
        Stack *stack2;
        if (widthScale != 1.0 || heightScale != 1.0 || depthScale != 1.0) {
          stack2 = C_Stack::resize(stack, width, height, depth);
        } else {
          stack2 = Copy_Stack(stack);
        }

        if (stack->kind == GREY && m_doc->getStack()->isBinary()) {
          size_t volume = m_doc->getStack()->getVoxelNumber();
          for (size_t voxelIndex = 0; voxelIndex < volume; ++voxelIndex) {
            if (stack2->array[voxelIndex] == 1) {
              stack2->array[voxelIndex] = 255;
            }
          }
        }

        Translate_Stack(stack2, GREY, 1);

        Z3DVolume *vh = new Z3DVolume(stack2,
                                      glm::vec3(1.f/widthScale, 1.f/heightScale, 1.f/depthScale),
                                      glm::vec3(m_xScale.get(),
                                                m_yScale.get(),
                                                m_zScale.get()),
                                      glm::vec3(offset.x(),
                                                offset.y(),
                                                offset.z()));

        m_volumes.push_back(vh);

      }
    } //for each cannel

    std::vector<ZVec3Parameter*>& chCols = m_doc->getStack()->channelColors();
    for (int i=0; i<nchannel; i++) {
      m_volumes[i]->setVolColor(chCols[i]->get());
    }
  }
}

void Z3DVolumeSource::readSparseStack()
{
  if (m_doc == NULL) {
    return;
  }

  if (!m_doc->hasSparseStack()) {
    return;
  }

  ZSparseStack *spStack = m_doc->getSparseStack();
  if (spStack->getBoundBox().isEmpty()) {
    return;
  }

  const ZStack *stackData = spStack->getStack();

  if (stackData == NULL) {
    return;
  }

  clearVolume();


  int nchannel = stackData->channelNumber();
  const ZIntPoint dsIntv = spStack->getDownsampleInterval();

  double widthScale = 1.0;
  double heightScale = 1.0;
  double depthScale = 1.0;
  if (dsIntv.getX() > 0 || dsIntv.getY() > 0 || dsIntv.getZ() > 0) {
    widthScale /= dsIntv.getX() + 1;
    heightScale /= dsIntv.getY() + 1;
    depthScale /= dsIntv.getZ() + 1;
  }
  int width = stackData->width();
  int height = stackData->height();
  int depth = stackData->depth();

  std::vector<Stack*> stackArray;

  if (nchannel > 0) {
    int maxTextureSize = 100;
    if (stackData->depth() > 1) {
      maxTextureSize = Z3DGpuInfoInstance.getMax3DTextureSize();
    } else {
      maxTextureSize = Z3DGpuInfoInstance.getMaxTextureSize();
    }

    if (height > maxTextureSize) {
      double alpha = (double) maxTextureSize / height;
      heightScale *= alpha;
      height = (int) (height * alpha);
    }
    if (width > maxTextureSize) {
      double alpha = (double) maxTextureSize / width;
      widthScale *= alpha;
      width = (int) (width * alpha);
    }
    if (depth > maxTextureSize) {
      double alpha = (double) maxTextureSize / depth;
      depthScale *= alpha;
      depth = (int) (depth * alpha);
    }

    ZIntPoint dsIntv2 = misc::getDsIntvFor3DVolume(
          ZIntCuboid(0, 0, 0, width - 1, height - 1, depth - 1));
    width /= dsIntv2.getX() + 1;
    height /= dsIntv2.getY() + 1;
    depth /= dsIntv2.getZ() + 1;


    /*
    ZIntPoint misc::getDsIntvFor3DVolume(const ZIntCuboid &box);

    size_t volume = (size_t) width * height *depth * nchannel;
    if (volume > m_maxVoxelNumber) {
      //Downsample big stack
      //m_isVolumeDownsampled.set(true);
      double scale = std::sqrt((double) (m_maxVoxelNumber) / volume);
      height = (int)(height * scale);
      width = (int)(width * scale);

      widthScale *= scale;
      heightScale *= scale;
    }
    */

    for (int i=0; i<nchannel; i++) {
      const Stack *stack = stackData->c_stack(i);
      Stack *stack2 = NULL;
      if (C_Stack::width(stack) != width || C_Stack::height(stack) != height ||
          C_Stack::depth(stack) != depth) {
        m_isVolumeDownsampled.set(true);

        int xIntv = C_Stack::width(stack) / width;
        int yIntv = C_Stack::height(stack) / height;
        int zIntv = C_Stack::depth(stack) / depth;

        stack2 = Downsample_Stack_Max(stack, xIntv, yIntv, zIntv, NULL);

        widthScale = 1.0 / ((xIntv + 1) * (dsIntv.getX() + 1));
        heightScale = 1.0 / ((yIntv + 1) * (dsIntv.getY() + 1));
        depthScale = 1.0 / ((zIntv + 1) * (dsIntv.getZ() + 1));

//        stack2 = Resize_Stack(stack, width, height, depth);
      } else {
        stack2 = C_Stack::clone(stack);
      }

      Translate_Stack(stack2, GREY, 1);

      if (C_Stack::isBinary(stack2)) {
        size_t volume = C_Stack::voxelNumber(stack2);
        for (size_t voxelIndex = 0; voxelIndex < volume; ++voxelIndex) {
          if (stack2->array[voxelIndex] == 1) {
            stack2->array[voxelIndex] = 255;
          }
        }
      }

      stackArray.push_back(stack2);
    } //for each cannel

    /**********************/
    int offset[3];
    offset[0] = -stackData->getOffset().getX() * (dsIntv.getX() + 1);
    offset[1] = -stackData->getOffset().getY() * (dsIntv.getY() + 1);
    offset[2] = -stackData->getOffset().getZ() * (dsIntv.getZ() + 1);

    QList<ZDocPlayer*> playerList =
        m_doc->getPlayerList(ZStackObjectRole::ROLE_3DPAINT);
    foreach (const ZDocPlayer *player, playerList) {
      //player->paintStack(colorStack);
      if (player->getLabel() > 0 && player->getLabel() < 10) {
        if (player->getLabel() >= (int) stackArray.size()) {
          stackArray.push_back(C_Stack::make(
                                 GREY, stackData->width(),
                                 stackData->height(),
                                 stackData->depth()));
          C_Stack::setZero(stackArray.back());
          //stackArray.resize(stackArray.size() + 1);
        }
        player->labelStack(stackArray[player->getLabel()], offset, 255,
            dsIntv.getX(), dsIntv.getY(), dsIntv.getZ());
      }
    }

    for (size_t i = 0; i < stackArray.size(); ++i) {
      Z3DVolume *vh = new Z3DVolume(
            stackArray[i],
            glm::vec3(1.f/widthScale, 1.f/heightScale, 1.f/depthScale),
            glm::vec3(m_xScale.get(), m_yScale.get(), m_zScale.get()),
            glm::vec3(-offset[0] * m_xScale.get(),
                      -offset[1] * m_yScale.get(),
                      -offset[2] * m_zScale.get())
            /*glm::vec3(.0)*/);

      m_volumes.push_back(vh);
    }

    m_volumes[0]->setVolColor(glm::vec3(1.f,1.f,1.f));
    ZLabelColorTable colorTable;
    for (size_t i = 1; i < stackArray.size(); ++i) {
      QColor color = colorTable.getColor(i);
      m_volumes[i]->setVolColor(
            glm::vec3(color.redF(), color.greenF(), color.blueF()));
    }
    /**********************/
  }
}

void Z3DVolumeSource::readVolumesWithObject()
{
  if (m_doc == NULL) {
    return;
  }

  clearVolume();

  std::vector<Stack*> stackArray;
  //int nchannel = 1;

  stackArray.push_back(C_Stack::clone(m_doc->getStack()->c_stack(0)));

#if 0
  ZStack *colorStack = new ZStack(GREY, m_doc->getStack()->width(),
                                  m_doc->getStack()->height(),
                                  m_doc->getStack()->depth(), 3);
  colorStack->setOffset(m_doc->getStackOffset());
  colorStack->initChannelColors();



  C_Stack::copyValue(m_doc->getStack()->c_stack(0),
                     colorStack->c_stack(0));
  colorStack->setChannelColor(0, 1, 1, 1);
#endif

  int offset[3];
  offset[0] = m_doc->getStackOffset().getX();
  offset[1] = m_doc->getStackOffset().getY();
  offset[2] = m_doc->getStackOffset().getZ();

  ZLabelColorTable colorTable;
  std::vector<QColor> colorArray(m_nChannelSupport);

  colorArray[0] = QColor(255, 255, 255);
  for (size_t i = 1; i < m_nChannelSupport; ++i) {
    colorArray[i] = colorTable.getColor(i);
  }

#if 0
  C_Stack::setZero(colorStack->c_stack(1));
  QColor color = colorTable.getColor(1);
  colorStack->setChannelColor(1, color.redF(), color.greenF(), color.blueF());

  C_Stack::setZero(colorStack->c_stack(2));
  color = colorTable.getColor(2);
  colorStack->setChannelColor(2, color.redF(), color.greenF(), color.blueF());
#endif

  //C_Stack::copyValue(m_doc->getStack()->c_stack(0),
  //                   colorStack->c_stack(1));
  //C_Stack::copyValue(m_doc->getStack()->c_stack(0),
  //                   colorStack->c_stack(2));

  QList<ZDocPlayer*> playerList =
      m_doc->getPlayerList(ZStackObjectRole::ROLE_3DPAINT);
  foreach (const ZDocPlayer *player, playerList) {
    //player->paintStack(colorStack);
    if (player->getLabel() > 0 && player->getLabel() < 10) {
      if (player->getLabel() >= (int) stackArray.size()) {
        stackArray.push_back(C_Stack::make(
                               GREY, m_doc->getStack()->width(),
                               m_doc->getStack()->height(),
                               m_doc->getStack()->depth()));
        C_Stack::setZero(stackArray.back());
        //stackArray.resize(stackArray.size() + 1);
      }
      player->labelStack(stackArray[player->getLabel()], offset, 255);

      //player->labelStack(colorStack->c_stack(player->getLabel()), offset, 255);
    }
  }

  /*
  QList<ZObject3d*> &objList = m_doc->getObj3dList();
  foreach(ZObject3d *obj, objList) {
    obj->drawStack(colorStack);
  }
  */

  int nchannel = stackArray.size();

  if (nchannel > 0) {
    for (int i=0; i<nchannel; i++) {
      //Stack *stack = colorStack->c_stack(i);
      Stack *stack = stackArray[i];

      //Under deveopment
      ZPoint offset = ZPoint(
            m_doc->getStack()->getOffset().getX() * m_xScale.get(),
            m_doc->getStack()->getOffset().getY() * m_yScale.get(),
            m_doc->getStack()->getOffset().getZ() * m_zScale.get());
      if (m_doc->getStack()->getVoxelNumber() * nchannel > m_maxVoxelNumber) { //Downsample big stack
        m_isVolumeDownsampled.set(true);
        double scale = std::sqrt((m_maxVoxelNumber*1.0) /
                                 (m_doc->getStack()->getVoxelNumber() * nchannel));
        int height = (int)(stack->height * scale);
        int width = (int)(stack->width * scale);
        int depth = stack->depth;
        double widthScale = 1.0;
        double heightScale = 1.0;
        double depthScale = 1.0;
        int maxTextureSize = 100;
        if (stack->depth > 1)
          maxTextureSize = Z3DGpuInfoInstance.getMax3DTextureSize();
        else
          maxTextureSize = Z3DGpuInfoInstance.getMaxTextureSize();

        if (height > maxTextureSize) {
          heightScale = (double)maxTextureSize / height;
          height = std::floor(height * heightScale);
        }
        if (width > maxTextureSize) {
          widthScale = (double)maxTextureSize / width;
          width = std::floor(width * widthScale);
        }
        if (depth > maxTextureSize) {
          depthScale = (double)maxTextureSize / depth;
          depth = std::floor(depth * depthScale);
        }

        widthScale *= scale;
        heightScale *= scale;

        Stack *stack2 = Resize_Stack(stack, width, height, depth);
        Translate_Stack(stack2, GREY, 1);

        if (m_doc->getStack()->isBinary()) {
          size_t volume = Stack_Voxel_Number(stack2);
          for (size_t voxelIndex = 0; voxelIndex < volume; ++voxelIndex) {
            if (stack2->array[voxelIndex] == 1) {
              stack2->array[voxelIndex] = 255;
            }
          }
        }

        Z3DVolume *vh = new Z3DVolume(
              stack2, glm::vec3(1.f/widthScale, 1.f/heightScale, 1.f/depthScale),
              glm::vec3(m_xScale.get(), m_yScale.get(), m_zScale.get()),
              glm::vec3(offset.x(), offset.y(),
                        offset.z())
                                      /*glm::vec3(.0)*/);

        m_volumes.push_back(vh);
      } else { //small stack
        double widthScale = 1.0;
        double heightScale = 1.0;
        double depthScale = 1.0;
        int height = C_Stack::height(stack);
        int width = C_Stack::width(stack);
        int depth = C_Stack::depth(stack);
        int maxTextureSize = 100;
        if (stack->depth > 1)
          maxTextureSize = Z3DGpuInfoInstance.getMax3DTextureSize();
        else
          maxTextureSize = Z3DGpuInfoInstance.getMaxTextureSize();

        if (height > maxTextureSize) {
          heightScale = (double)maxTextureSize / height;
          height = std::floor(height * heightScale);
        }
        if (width > maxTextureSize) {
          widthScale = (double)maxTextureSize / width;
          width = std::floor(width * widthScale);
        }
        if (depth > maxTextureSize) {
          depthScale = (double)maxTextureSize / depth;
          depth = std::floor(depth * depthScale);
        }
        Stack *stack2;
        if (widthScale != 1.0 || heightScale != 1.0)
          stack2 = C_Stack::resize(stack, width, height, depth);
        else
          stack2 = Copy_Stack(stack);

        if (stack->kind == GREY && m_doc->getStack()->isBinary()) {
          size_t volume = m_doc->getStack()->getVoxelNumber();
          for (size_t voxelIndex = 0; voxelIndex < volume; ++voxelIndex) {
            if (stack2->array[voxelIndex] == 1) {
              stack2->array[voxelIndex] = 255;
            }
          }
        }

        Translate_Stack(stack2, GREY, 1);

        Z3DVolume *vh = new Z3DVolume(stack2,
                                      glm::vec3(1.f/widthScale, 1.f/heightScale, 1.f/depthScale),
                                      glm::vec3(m_xScale.get(),
                                                m_yScale.get(),
                                                m_zScale.get()),
                                      glm::vec3(offset.x(),
                                                offset.y(),
                                                offset.z()));

        m_volumes.push_back(vh);

      }
    } //for each cannel

    //std::vector<ZVec3Parameter*>& chCols = colorStack->channelColors();
    for (int i=0; i<nchannel; i++) {
      QColor &color = colorArray[i];
      m_volumes[i]->setVolColor(glm::vec3(color.redF(), color.greenF(),
                                          color.blueF()));
      //m_volumes[i]->setVolColor(chCols[i]->get());
    }
  }

  for (std::vector<Stack*>::iterator iter = stackArray.begin();
       iter != stackArray.end(); ++iter) {
    C_Stack::kill(*iter);
  }

  //delete colorStack;
}

void Z3DVolumeSource::readSparseVolume()
{
  if (m_doc == NULL) {
    return;
  }

  if (!m_doc->hasStack() || m_doc->getSparseObjectList().isEmpty()) {
    return;
  }

  clearVolume();
  //int nchannel = 1;

  ZSparseObject obj = *(m_doc->getSparseObjectList().front());
  QColor color = obj.getColor();
  int nchannel = 1;
  if (color.green() > 0) {
    nchannel = 2;
  }
  if (color.blue() > 0) {
    nchannel = 3;
  }


  ZIntPoint dsIntv =
      misc::getDsIntvFor3DVolume(m_doc->getStack()->getBoundBox());


  int xIntv = dsIntv.getX();
  int yIntv = dsIntv.getY();
  int zIntv = dsIntv.getZ();
  /*
  int xIntv = 0;
  int yIntv = 0;
  int zIntv = 0;

  if (m_doc->getStack()->getVoxelNumber() * nchannel > m_maxVoxelNumber) { //Downsample big stack
    //m_isVolumeDownsampled.set(true);
    xIntv = 1;
    yIntv = 1;
    zIntv = 1;
  } else if (m_doc->getStack()->getVoxelNumber() * nchannel > m_maxVoxelNumber * 3) {
    xIntv = 2;
    yIntv = 2;
    zIntv = 2;
  }
*/
  int height = m_doc->getStack()->width();
  int width = m_doc->getStack()->height();
  int depth = m_doc->getStack()->depth();

  int maxTextureSize = 100;
  if (depth > 1) {
    maxTextureSize = Z3DGpuInfoInstance.getMax3DTextureSize();
  } else {
    maxTextureSize = Z3DGpuInfoInstance.getMaxTextureSize();
  }

  if (height > maxTextureSize) {
    yIntv += height / maxTextureSize;
  }
  if (width > maxTextureSize) {
    xIntv += width / maxTextureSize;
  }
  if (depth > maxTextureSize) {
    zIntv += depth / maxTextureSize;
  }


  obj.downsampleMax(xIntv, yIntv, zIntv);
  int offset[3];

  int rgb[3];
  rgb[0] = color.red();
  rgb[1] = color.green();
  rgb[2] = color.blue();

  for (int i = 0; i < nchannel; ++i) {
    Stack *stack2 = obj.toStack(offset, rgb[i]);

    ZPoint finalOffset;
    finalOffset.set(offset[0]* m_xScale.get() * (xIntv + 1),
        offset[1] * m_yScale.get() * (yIntv + 1),
        offset[2] * m_zScale.get() * (zIntv + 1));

#ifdef _DEBUG_2
    C_Stack::write(GET_TEST_DATA_DIR + "/test.tif", stack2);
#endif

    Z3DVolume *vh = new Z3DVolume(
          stack2, glm::vec3(xIntv + 1, yIntv + 1, zIntv + 1),
          glm::vec3(m_xScale.get(), m_yScale.get(), m_zScale.get()),
          glm::vec3(finalOffset.x(), finalOffset.y(), finalOffset.z()));

    m_volumes.push_back(vh);
  }


  m_volumes[0]->setVolColor(glm::vec3(1.f,0.f,0.f));
  if (m_volumes.size() > 1) {
    m_volumes[1]->setVolColor(glm::vec3(0.f,1.f,0.f));
  }
  if (m_volumes.size() > 2) {
    m_volumes[2]->setVolColor(glm::vec3(0.f,0.f,1.f));
  }
}

void Z3DVolumeSource::readSparseVolumeWithObject()
{
  if (m_doc == NULL) {
    return;
  }

  if (!m_doc->hasStack() || m_doc->getSparseObjectList().isEmpty()) {
    return;
  }

  clearVolume();
  //int nchannel = 1;

  ZSparseObject obj = *(m_doc->getSparseObjectList().front());
  QColor color = obj.getColor();
  int nchannel = 3;

  ZIntPoint dsIntv =
      misc::getDsIntvFor3DVolume(m_doc->getStack()->getBoundBox());


  int xIntv = dsIntv.getX();
  int yIntv = dsIntv.getY();
  int zIntv = dsIntv.getZ();
  /*
  if (m_doc->getStack()->getVoxelNumber() * nchannel > m_maxVoxelNumber) { //Downsample big stack
    //m_isVolumeDownsampled.set(true);
    xIntv = 1;
    yIntv = 1;
    zIntv = 1;
  }
  */

  int height = m_doc->getStack()->width();
  int width = m_doc->getStack()->height();
  int depth = m_doc->getStack()->depth();

  int maxTextureSize = 100;
  if (depth > 1) {
    maxTextureSize = Z3DGpuInfoInstance.getMax3DTextureSize();
  } else {
    maxTextureSize = Z3DGpuInfoInstance.getMaxTextureSize();
  }

  if (height > maxTextureSize) {
    yIntv += height / maxTextureSize;
  }
  if (width > maxTextureSize) {
    xIntv += width / maxTextureSize;
  }
  if (depth > maxTextureSize) {
    zIntv += depth / maxTextureSize;
  }


  obj.downsampleMax(xIntv, yIntv, zIntv);
  int offset[3];

  int rgb[3];
  rgb[0] = color.red();
  rgb[1] = color.green();
  rgb[2] = color.blue();

  std::vector<Stack*> stackArray(3);

  for (int i = 0; i < nchannel; ++i) {
    stackArray[i] = obj.toStack(offset, rgb[i]);

    ZPoint finalOffset;
    finalOffset.set(offset[0]* m_xScale.get() * (xIntv + 1),
        offset[1] * m_yScale.get() * (yIntv + 1),
        offset[2] * m_zScale.get() * (zIntv + 1));

#ifdef _DEBUG_2
    C_Stack::write(GET_TEST_DATA_DIR + "/test.tif", stack2);
#endif

    Z3DVolume *vh = new Z3DVolume(
          stackArray[i], glm::vec3(xIntv + 1, yIntv + 1, zIntv + 1),
          glm::vec3(m_xScale.get(), m_yScale.get(), m_zScale.get()),
          glm::vec3(finalOffset.x(), finalOffset.y(), finalOffset.z()));

    m_volumes.push_back(vh);
  }

  int originalOffset[3];
  originalOffset[0] = offset[0] * (xIntv + 1);
  originalOffset[1] = offset[1] * (yIntv + 1);
  originalOffset[2] = offset[2] * (zIntv + 1);

  QList<ZObject3d*>objList = m_doc->getObj3dList();
  foreach(ZObject3d *obj, objList) {
    obj->drawStack(stackArray, originalOffset, xIntv, yIntv, zIntv);
  }


  m_volumes[0]->setVolColor(glm::vec3(1.f,0.f,0.f));
  if (m_volumes.size() > 1) {
    m_volumes[1]->setVolColor(glm::vec3(0.f,1.f,0.f));
  }
  if (m_volumes.size() > 2) {
    m_volumes[2]->setVolColor(glm::vec3(0.f,0.f,1.f));
  }
}


void Z3DVolumeSource::readSubVolumes(int left, int top, int front, int width,
                                     int height, int depth)
{
  if (m_doc == NULL) {
    return;
  }

  clearZoomInVolume();
  int nchannel = m_doc->hasStackData() ? m_doc->getStack()->channelNumber() : 0;
  if (nchannel > 0) {
    glm::vec3 scaleSpacing = glm::vec3(m_xScale.get(), m_yScale.get(), m_zScale.get());
    glm::vec3 downsampleSpacing = glm::vec3(1.f, 1.f, 1.f);
    glm::vec3 offset = glm::vec3(left, top, front) * scaleSpacing + getVolume(0)->getOffset();
    for (int i=0; i<nchannel; i++) {
      Stack *stack = m_doc->getStack()->c_stack(i);
      Stack *subStack = Crop_Stack(stack, left, top, front, width, height, depth, NULL);
      if (subStack->kind == GREY) {
        Z3DVolume *vh = new Z3DVolume(subStack, downsampleSpacing, scaleSpacing, offset,
                                      getVolume(0)->getPhysicalToWorldMatrix());
        vh->setParentVolumeDimensions(glm::uvec3(stack->width, stack->height, stack->depth));
        vh->setParentVolumeOffset(getVolume(0)->getOffset());
        m_zoomInVolumes.push_back(vh);
      } else {
        Translate_Stack(subStack, GREY, 1);
        Z3DVolume *vh = new Z3DVolume(subStack, downsampleSpacing, scaleSpacing, offset,
                                      getVolume(0)->getPhysicalToWorldMatrix());
        vh->setParentVolumeDimensions(glm::uvec3(stack->width, stack->height, stack->depth));
        vh->setParentVolumeOffset(getVolume(0)->getOffset());
        m_zoomInVolumes.push_back(vh);
      }
    }

    std::vector<ZVec3Parameter*>& chCols = m_doc->getStack()->channelColors();
    for (int i=0; i<nchannel; i++) {
      m_zoomInVolumes[i]->setVolColor(chCols[i]->get());
    }
  }
}

void Z3DVolumeSource::sendData()
{
  for (size_t i=0; i<m_volumes.size(); i++) {
    if (i < m_nChannelSupport) {
      m_outputPorts[i]->setData(m_volumes[i], false);
    }
  }
  for (size_t i=m_volumes.size(); i<m_outputPorts.size(); i++) {
    m_outputPorts[i]->setData(NULL);
  }
  if (m_volumes.size() > 0) {
    m_stackOutputPort.setData(m_doc->getStack());
  }
}

void Z3DVolumeSource::sendZoomInVolumeData()
{
  if (m_doc == NULL) {
    return;
  }

  for (size_t i=0; i<m_zoomInVolumes.size(); i++) {
    if (i < m_nChannelSupport) {
      m_outputPorts[i]->setData(m_zoomInVolumes[i], false);
    }
  }
  for (size_t i=m_zoomInVolumes.size(); i<m_outputPorts.size(); i++) {
    m_outputPorts[i]->setData(NULL);
  }
  if (m_volumes.size() > 0) {
    m_stackOutputPort.setData(m_doc->getStack());
  }
}

void Z3DVolumeSource::changeXScale()
{
  if (m_volumes.empty())
    return;
  for (size_t i=0; i<m_volumes.size(); i++) {
    m_volumes[i]->setScaleSpacing(glm::vec3(m_xScale.get(), m_yScale.get(), m_zScale.get()));
  }
  for (size_t i=0; i<m_zoomInVolumes.size(); i++) {
    m_zoomInVolumes[i]->setScaleSpacing(glm::vec3(m_xScale.get(), m_yScale.get(), m_zScale.get()));
  }
  emit xScaleChanged();
}

void Z3DVolumeSource::changeYScale()
{
  if (m_volumes.empty())
    return;
  for (size_t i=0; i<m_volumes.size(); i++) {
    m_volumes[i]->setScaleSpacing(glm::vec3(m_xScale.get(), m_yScale.get(), m_zScale.get()));
  }
  for (size_t i=0; i<m_zoomInVolumes.size(); i++) {
    m_zoomInVolumes[i]->setScaleSpacing(glm::vec3(m_xScale.get(), m_yScale.get(), m_zScale.get()));
  }
  emit yScaleChanged();
}

void Z3DVolumeSource::changeZScale()
{
  if (m_volumes.empty())
    return;
  for (size_t i=0; i<m_volumes.size(); i++) {
    m_volumes[i]->setScaleSpacing(glm::vec3(m_xScale.get(), m_yScale.get(), m_zScale.get()));
  }
  for (size_t i=0; i<m_zoomInVolumes.size(); i++) {
    m_zoomInVolumes[i]->setScaleSpacing(glm::vec3(m_xScale.get(), m_yScale.get(), m_zScale.get()));
  }
  emit zScaleChanged();
}

void Z3DVolumeSource::changeZoomInViewSize()
{
  if (m_zoomInVolumes.empty())
    return;
  exitZoomInView();
  openZoomInView(m_zoomInPos);
}

void Z3DVolumeSource::exitZoomInView()
{
  if (m_zoomInVolumes.empty())
    return;

  // copy transform matrix from sub volume, in case it is changed
  for (size_t i=0; i<m_volumes.size(); i++) {
    m_volumes[i]->setPhysicalToWorldMatrix(m_zoomInVolumes[i]->getPhysicalToWorldMatrix());
  }
  clearZoomInVolume();
  sendData();
  m_isSubVolume.set(false);
  m_isVolumeDownsampled.set(true);
}

void Z3DVolumeSource::clearVolume()
{
  for (size_t i=0; i<m_volumes.size(); i++) {
    delete m_volumes[i];
  }
  m_volumes.clear();
}

void Z3DVolumeSource::clearZoomInVolume()
{
  for (size_t i=0; i<m_zoomInVolumes.size(); i++) {
    delete m_zoomInVolumes[i];
  }
  m_zoomInVolumes.clear();
}

void Z3DVolumeSource::reloadVolume()
{
  if (!isInitialized())
    return;

  clearVolume();
  clearZoomInVolume();
  loadData();
  sendData();
}

bool Z3DVolumeSource::openZoomInView(const glm::ivec3& volPos)
{
  if (m_doc == NULL) {
    return false;
  }

  if (!m_isVolumeDownsampled.get())
    return false;
  if (!m_doc->hasStackData())
    return false;
  if (!volumeNeedDownsample())
    return false;
  if (getVolume(0) == NULL)
    return false;
  glm::ivec3 voldim = glm::ivec3(getVolume(0)->getCubeSize());
  if (!(volPos[0] >= 0 && volPos[0] < voldim.x  &&
        volPos[1] >= 0 && volPos[1] < voldim.y &&
        volPos[2] >= 0 && volPos[2] < voldim.z)) {
    return false;
  }

  glm::vec3 offset = getVolume(0)->getOffset();
  m_zoomInPos = volPos;
  if (m_zoomInViewSize.get() % 2 != 0)
    m_zoomInViewSize.set(m_zoomInViewSize.get()+1);
  int halfsize = m_zoomInViewSize.get() / 2;
  int left = std::max(volPos[0]-halfsize+1, 0);
  int right = std::min(volPos[0]+halfsize, m_doc->getStack()->width()-1);
  int width = right - left + 1;
  int up = std::max(volPos[1]-halfsize+1, 0);
  int down = std::min(volPos[1]+halfsize, m_doc->getStack()->height()-1);
  int height = down - up + 1;
  int front = 0;
  int depth = m_doc->getStack()->depth();
  m_zoomInBound.clear();
  m_zoomInBound.push_back(left*m_xScale.get() + offset.x);
  m_zoomInBound.push_back(right*m_xScale.get() + offset.x);
  m_zoomInBound.push_back(up*m_yScale.get() + offset.y);
  m_zoomInBound.push_back(down*m_yScale.get() + offset.y);
  m_zoomInBound.push_back(front*m_zScale.get() + offset.z);
  m_zoomInBound.push_back(depth*m_zScale.get() + offset.z);
  readSubVolumes(left, up, front, width, height, depth);
  sendZoomInVolumeData();

  m_isSubVolume.set(true);
  m_isVolumeDownsampled.set(false);
  return true;
}

Z3DVolume *Z3DVolumeSource::getVolume(size_t index)
{
  if (index < m_volumes.size())
    return m_volumes[index];
  else
    return NULL;
}

bool Z3DVolumeSource::isEmpty()
{
  return m_volumes.empty();
}

bool Z3DVolumeSource::volumeNeedDownsample() const
{
  if (m_doc == NULL) {
    return false;
  }

  if (m_doc->getStack()->getVoxelNumber() * m_doc->getStack()->channelNumber() <= m_maxVoxelNumber)
    return false;
  else
    return true;
}

bool Z3DVolumeSource::isVolumeDownsampled() const
{
  return m_isVolumeDownsampled.get();
}

ZWidgetsGroup *Z3DVolumeSource::getWidgetsGroup()
{
  if (!m_widgetsGroup) {
    m_widgetsGroup = new ZWidgetsGroup("Volume Source", NULL, 1);
    new ZWidgetsGroup(&m_zScale, m_widgetsGroup, 3);
    new ZWidgetsGroup(&m_xScale, m_widgetsGroup, 3);
    new ZWidgetsGroup(&m_yScale, m_widgetsGroup, 3);
    new ZWidgetsGroup(&m_isVolumeDownsampled, m_widgetsGroup, 3);
    new ZWidgetsGroup(&m_isSubVolume, m_widgetsGroup, 3);
    new ZWidgetsGroup(&m_zoomInViewSize, m_widgetsGroup, 3);
    m_widgetsGroup->setBasicAdvancedCutoff(4);
  }
  return m_widgetsGroup;
}
