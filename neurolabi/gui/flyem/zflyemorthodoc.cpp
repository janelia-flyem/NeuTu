#include "zflyemorthodoc.h"

#include <QElapsedTimer>

#include "zqslog.h"
#include "dvid/zdvidsynapseensenmble.h"
#include "zstackobjectsourcefactory.h"
#include "zcrosshair.h"
#include "neutubeconfig.h"
#include "dvid/zdvidsynapseensenmble.h"

ZFlyEmOrthoDoc::ZFlyEmOrthoDoc(QObject *parent) :
  ZFlyEmProofDoc(parent)
{
  init(256, 256, 256);
}

ZFlyEmOrthoDoc::ZFlyEmOrthoDoc(int width, int height, int depth, QObject *parent) :
  ZFlyEmProofDoc(parent)
{
  init(width, height, depth);
}

void ZFlyEmOrthoDoc::init(int width, int height, int depth)
{
  setTag(neutube::Document::ETag::FLYEM_ORTHO);
  m_width = width;
  m_height = height;
  m_depth = depth;

  setRoutineCheck(false);

  ZCrossHair *crossHair = new ZCrossHair;
  crossHair->setCenter(m_width / 2, m_height / 2, m_depth / 2);
  crossHair->setSource(ZStackObjectSourceFactory::MakeCrossHairSource());
  addObject(crossHair);
}

void ZFlyEmOrthoDoc::setSize(int width, int height, int depth)
{
  m_width = width;
  m_height = height;
  m_depth = depth;
}

ZCrossHair* ZFlyEmOrthoDoc::getCrossHair() const
{
  return getObject<ZCrossHair>(ZStackObjectSourceFactory::MakeCrossHairSource());
}

void ZFlyEmOrthoDoc::setCrossHairCenter(double x, double y, neutube::EAxis axis)
{
  ZPoint center = getCrossHair()->getCenter();
  //Transform to the view space
  center.shiftSliceAxis(axis);
  center.setX(x);
  center.setY(y);
  //Transform back to the world space
  center.shiftSliceAxisInverse(axis);

  getCrossHair()->setCenter(center);
}

void ZFlyEmOrthoDoc::setCrossHairCenter(const ZIntPoint &center)
{
  getCrossHair()->setCenter(center);
}

void ZFlyEmOrthoDoc::initSynapseEnsemble(neutube::EAxis axis)
{
  ZDvidSynapseEnsemble *se = new ZDvidSynapseEnsemble;
  se->setSliceAxis(axis);
  se->setSource(ZStackObjectSourceFactory::MakeDvidSynapseEnsembleSource(axis));
  se->setResolution(m_grayScaleInfo.getVoxelResolution());
  se->setReady(true);
  addObject(se);
}

void ZFlyEmOrthoDoc::initSynapseEnsemble()
{
  initSynapseEnsemble(neutube::EAxis::X);
  initSynapseEnsemble(neutube::EAxis::Y);
  initSynapseEnsemble(neutube::EAxis::Z);
}

void ZFlyEmOrthoDoc::initTodoList(neutube::EAxis axis)
{
  ZFlyEmToDoList *td = new ZFlyEmToDoList;
  td->setSliceAxis(axis);
  td->setSource(ZStackObjectSourceFactory::MakeTodoListEnsembleSource(axis));
  addObject(td);
}

void ZFlyEmOrthoDoc::initTodoList()
{
  initTodoList(neutube::EAxis::X);
  initTodoList(neutube::EAxis::Y);
  initTodoList(neutube::EAxis::Z);
}

void ZFlyEmOrthoDoc::updateStack(const ZIntPoint &center)
{
  if (m_grayscaleReader.isReady()) {
    ZIntCuboid box;
    box.setFirstCorner(center - ZIntPoint(m_width / 2, m_height / 2, m_depth / 2));
    box.setSize(m_width, m_height, m_depth);
//    m_dvidReader.readGrayScale(box);
    ZStack *stack = m_grayscaleReader.readGrayScale(box);
    loadStack(stack);

    ZDvidUrl dvidUrl(getDvidTarget());
    QElapsedTimer timer;
    timer.start();
    ZJsonArray obj = getDvidReader().readJsonArray(dvidUrl.getSynapseUrl(box));
    LINFO() << "Synapse reading time: " << timer.elapsed();

    QList<ZDvidSynapseEnsemble*> seList = getObjectList<ZDvidSynapseEnsemble>();
    for (QList<ZDvidSynapseEnsemble*>::iterator iter = seList.begin();
         iter != seList.end(); ++iter) {
      ZDvidSynapseEnsemble *se = *iter;
      se->setRange(box);
//      se->setReady(true);
    }

    for (size_t i = 0; i < obj.size(); ++i) {
      ZJsonObject synapseJson(obj.at(i), ZJsonValue::SET_INCREASE_REF_COUNT);
      if (synapseJson.hasKey("Pos")) {
        ZDvidSynapse synapse;
        synapse.loadJsonObject(synapseJson, flyem::EDvidAnnotationLoadMode::NO_PARTNER);
        for (QList<ZDvidSynapseEnsemble*>::iterator iter = seList.begin();
             iter != seList.end(); ++iter) {
          ZDvidSynapseEnsemble *se = *iter;
          se->addSynapseUnsync(synapse, ZDvidSynapseEnsemble::DATA_LOCAL);
        }
      }
    }

    QList<ZFlyEmToDoList*> todoList = getObjectList<ZFlyEmToDoList>();
    for (QList<ZFlyEmToDoList*>::iterator iter = todoList.begin();
         iter != todoList.end(); ++iter) {
      ZFlyEmToDoList *obj = *iter;
      obj->setRange(box);
    }
  }
}

ZDvidSynapseEnsemble* ZFlyEmOrthoDoc::getDvidSynapseEnsemble(
    neutube::EAxis axis) const
{
  ZOUT(LTRACE(), 5) << "Get dvid synapses";
  QList<ZStackObject*> teList =
      getObjectList(ZStackObject::TYPE_DVID_SYNAPE_ENSEMBLE);
  for (QList<ZStackObject*>::iterator iter = teList.begin();
       iter != teList.end(); ++iter) {
    ZDvidSynapseEnsemble *te = dynamic_cast<ZDvidSynapseEnsemble*>(*iter);
    if (te->getSource() ==
        ZStackObjectSourceFactory::MakeDvidSynapseEnsembleSource(axis)) {
      return te;
    }
  }

  return NULL;
}

void ZFlyEmOrthoDoc::prepareDvidData()
{
  if (m_dvidReader.isReady()) {
    initSynapseEnsemble();
    initTodoList();
    addDvidLabelSlice(neutube::EAxis::X);
    addDvidLabelSlice(neutube::EAxis::Y);
    addDvidLabelSlice(neutube::EAxis::Z);
  }
}
