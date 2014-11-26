#include "zflyembodymergeproject.h"

#include <QtConcurrentRun>

#include "zintpoint.h"
#include "zstackdvidgrayscalefactory.h"
#include "dvid/zdvidreader.h"
#include "zstackframe.h"
#include "zstackdoc.h"
#include "flyem/zflyembodymergeframe.h"
#include "neutubeconfig.h"
#include "zstackdocreader.h"
#include "flyem/zflyembodymergedoc.h"
#include "zarrayfactory.h"

ZFlyEmBodyMergeProject::ZFlyEmBodyMergeProject(QObject *parent) :
  QObject(parent), m_dataFrame(NULL)
{
}

ZFlyEmBodyMergeProject::~ZFlyEmBodyMergeProject()
{
  clear();
}

void ZFlyEmBodyMergeProject::clear()
{
  if (m_dataFrame != NULL) {
    m_dataFrame->hide();
    delete m_dataFrame;
    m_dataFrame = NULL;
  }
}

void ZFlyEmBodyMergeProject::test()
{
  ZStackDocReader *reader = new ZStackDocReader();
  reader->loadStack(
        (GET_TEST_DATA_DIR + "/benchmark/em_stack_slice.tif").c_str());
  emit newDocReady(reader);

  ZStack stack;
  stack.load(GET_TEST_DATA_DIR + "/benchmark/em_stack_slice_seg.tif");
  ZArray *array = ZArrayFactory::MakeArray(&stack);
  emit originalLabelUpdated(array);
}

void ZFlyEmBodyMergeProject::loadSlice(int x, int y, int z)
{
  QtConcurrent::run(
          this, &ZFlyEmBodyMergeProject::loadSliceFunc, x, y, z);
}

void ZFlyEmBodyMergeProject::loadSliceFunc(int x, int y, int z)
{
  ZDvidReader reader;
  if (reader.open(getDvidTarget())) {
    int width = 512;
    int height = 512;

    ZStack *stack = reader.readGrayScale(x - width / 2,
                                         y - height / 2,
                                         z, width, height, 1);
    ZStackDocReader *docReader = new ZStackDocReader;
    docReader->setStack(stack);
    emit newDocReady(docReader);
  }
}

void ZFlyEmBodyMergeProject::viewGrayscale(
    const ZIntPoint &offset, int width, int height)
{
  ZDvidReader reader;
  if (reader.open(getDvidTarget())) {
    ZStackFrame *frame = getDataFrame();
    if (frame != NULL) {
      ZStack *stack = reader.readGrayScale(
            offset.getX(), offset.getY(), offset.getZ(), width, height, 1);
      frame->loadStack(stack);
    }
  }
}

void ZFlyEmBodyMergeProject::loadGrayscaleFunc(int z, bool lowres)
{
#if 0
  emit progressAdvanced(0.1);
  ZDvidReader reader;
  const ZDvidTarget &target = m_project->getDvidTarget();
  if (z >= 0 && reader.open(target)) {
    if (m_project->getRoi(z) == NULL) {
      m_project->downloadRoi(z);
    }

    QString infoString = reader.readInfo("grayscale");

    qDebug() << infoString;

    ZDvidInfo dvidInfo;
    dvidInfo.setFromJsonString(infoString.toStdString());

    //int z = m_zDlg->getValue();
    //m_project->setZ(z);

    ZStack *stack = NULL;

    bool creatingLowres = false;
    std::string lowresPath =
        target.getLocalLowResGrayScalePath(m_xintv, m_xintv, 0, z);
    if (lowres) {
      m_project->setDsIntv(m_xintv, m_yintv, 0);
      prepareQuickLoad(z, true);
      if (QFileInfo(lowresPath.c_str()).exists() && !isPreparingQuickLoad(z)) {
        stack = new ZStack;
        stack->load(lowresPath);
      } else if (QDir(target.getLocalFolder().c_str()).exists()) {
        creatingLowres = true;
      }
    }

    if (!lowres || creatingLowres){
      if (!lowres) {
        m_project->setDsIntv(0, 0, 0);
      }

      ZIntCuboid boundBox = reader.readBoundBox(z);

      if (!boundBox.isEmpty()) {
        stack = reader.readGrayScale(boundBox.getFirstCorner().getX(),
                                     boundBox.getFirstCorner().getY(),
                                     z, boundBox.getWidth(),
                                     boundBox.getHeight(), 1);
      } else {
        stack = reader.readGrayScale(
              dvidInfo.getStartCoordinates().getX(),
              dvidInfo.getStartCoordinates().getY(),
              z, dvidInfo.getStackSize()[0],
            dvidInfo.getStackSize()[1], 1);
        if (stack != NULL) {
          boundBox = ZFlyEmRoiProject::estimateBoundBox(
                *stack, getDvidTarget().getBgValue());
          if (!boundBox.isEmpty()) {
            stack->crop(boundBox);
          }
          ZDvidWriter writer;
          if (writer.open(m_project->getDvidTarget())) {
            writer.writeBoundBox(boundBox, z);
          }
        }
      }

      if (stack != NULL) {
        if (creatingLowres) {
          emit messageDumped("Creating low res data ...", true);
          stack->downsampleMin(m_xintv, m_xintv, 0);
          stack->save(lowresPath);
          emit messageDumped(
                QString("Data saved into %1").arg(lowresPath.c_str()), true);
        }
      }
    }

    if (stack != NULL) {
      //advance(0.5);
      emit progressAdvanced(0.5);
      m_docReader.clear();
      m_docReader.setStack(stack);

      ZSwcTree *tree = m_project->getRoiSwc(
            z, FlyEm::GetFlyEmRoiMarkerRadius(stack->width(), stack->height()));
      if (tree != NULL) {
        m_docReader.addObject(tree, ZDocPlayer::ROLE_ROI);
      }
#ifdef _DEBUG_
      std::cout << "Object count in docreader: "
                << m_docReader.getObjectGroup().size() << std::endl;
      std::cout << "Swc count in docreader: "
                << m_docReader.getObjectGroup().getObjectList(ZStackObject::TYPE_SWC).size()
                << std::endl;
#endif
      emit newDocReady();
    } else {
      processLoadGrayscaleFailure();
    }
  } else {
    processLoadGrayscaleFailure();
  }
#endif
}

bool ZFlyEmBodyMergeProject::hasDataFrame() const
{
  return m_dataFrame != NULL;
}

void ZFlyEmBodyMergeProject::setDocData(ZStackDocReader &reader)
{
  if (m_dataFrame != NULL) {
    m_dataFrame->document()->reloadData(reader);
  }
}

void ZFlyEmBodyMergeProject::setDataFrame(ZStackFrame *frame)
{
  if (m_dataFrame != NULL) {
    frame->setObjectStyle(m_dataFrame->getObjectStyle());
    closeDataFrame();
  }

  connect(frame, SIGNAL(destroyed()), this, SLOT(shallowClear()));

  m_dataFrame = dynamic_cast<ZFlyEmBodyMergeFrame*>(frame);

  connect(this, SIGNAL(originalLabelUpdated(ZArray*)),
          m_dataFrame->getCompleteDocument(),
          SLOT(updateOriginalLabel(ZArray*)));
}

void ZFlyEmBodyMergeProject::closeDataFrame()
{
  if (m_dataFrame != NULL) {
    m_dataFrame->hide();
    delete m_dataFrame;
    m_dataFrame = NULL;
  }
}

void ZFlyEmBodyMergeProject::shallowClear()
{
  m_dataFrame = NULL;
}

void ZFlyEmBodyMergeProject::mergeBody()
{
  if (m_dataFrame != NULL) {
    m_dataFrame->getCompleteDocument()->mergeSelected();
  }
}
