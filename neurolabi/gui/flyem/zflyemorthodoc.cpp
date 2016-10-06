#include "zflyemorthodoc.h"

#include <QElapsedTimer>

#include "dvid/zdvidsynapseensenmble.h"
#include "zstackobjectsourcefactory.h"
#include "zcrosshair.h"
#include "neutubeconfig.h"
#include "dvid/zdvidsynapseensenmble.h"

ZFlyEmOrthoDoc::ZFlyEmOrthoDoc(QObject *parent) :
  ZFlyEmProofDoc(parent)
{
  init();
}

void ZFlyEmOrthoDoc::init()
{
  setTag(NeuTube::Document::FLYEM_ORTHO);
  m_width = 256;
  m_height = 256;
  m_depth = 256;

  setRoutineCheck(false);

  ZCrossHair *crossHair = new ZCrossHair;
  crossHair->setCenter(m_width / 2, m_height / 2, m_depth / 2);
  crossHair->setSource(ZStackObjectSourceFactory::MakeCrossHairSource());
  addObject(crossHair);
}

ZCrossHair* ZFlyEmOrthoDoc::getCrossHair() const
{
  return getObject<ZCrossHair>(ZStackObjectSourceFactory::MakeCrossHairSource());
}

void ZFlyEmOrthoDoc::initSynapseEnsemble(NeuTube::EAxis axis)
{
  ZDvidSynapseEnsemble *se = new ZDvidSynapseEnsemble;
  se->setSliceAxis(axis);
  se->setSource(ZStackObjectSourceFactory::MakeDvidSynapseEnsembleSource(axis));
  se->setReady(true);
  addObject(se);
}

void ZFlyEmOrthoDoc::initSynapseEnsemble()
{
  initSynapseEnsemble(NeuTube::X_AXIS);
  initSynapseEnsemble(NeuTube::Y_AXIS);
  initSynapseEnsemble(NeuTube::Z_AXIS);
}

void ZFlyEmOrthoDoc::initTodoList(NeuTube::EAxis axis)
{
  ZFlyEmToDoList *td = new ZFlyEmToDoList;
  td->setSliceAxis(axis);
  td->setSource(ZStackObjectSourceFactory::MakeTodoListEnsembleSource(axis));
  addObject(td);
}

void ZFlyEmOrthoDoc::initTodoList()
{
  initTodoList(NeuTube::X_AXIS);
  initTodoList(NeuTube::Y_AXIS);
  initTodoList(NeuTube::Z_AXIS);
}

void ZFlyEmOrthoDoc::updateStack(const ZIntPoint &center)
{
  if (m_dvidReader.isReady()) {
    ZIntCuboid box;
    box.setFirstCorner(center - ZIntPoint(m_width / 2, m_height / 2, m_depth / 2));
    box.setSize(m_width, m_height, m_depth);
//    m_dvidReader.readGrayScale(box);
    ZStack *stack = m_dvidReader.readGrayScale(box);
    loadStack(stack);

    ZDvidUrl dvidUrl(m_dvidTarget);
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
        synapse.loadJsonObject(synapseJson, FlyEM::LOAD_NO_PARTNER);
        for (QList<ZDvidSynapseEnsemble*>::iterator iter = seList.begin();
             iter != seList.end(); ++iter) {
          ZDvidSynapseEnsemble *se = *iter;
          se->addSynapseUnsync(synapse, ZDvidSynapseEnsemble::DATA_LOCAL);
        }
      }
    }

    /*
    getDvidSynapseEnsemble(NeuTube::X_AXIS)->setRange(box);
    getDvidSynapseEnsemble(NeuTube::Y_AXIS)->setRange(box);
    getDvidSynapseEnsemble(NeuTube::Z_AXIS)->setRange(box);
    */



    QList<ZFlyEmToDoList*> todoList = getObjectList<ZFlyEmToDoList>();
    for (QList<ZFlyEmToDoList*>::iterator iter = todoList.begin();
         iter != todoList.end(); ++iter) {
      ZFlyEmToDoList *obj = *iter;
      obj->setRange(box);
    }
  }
}

ZDvidSynapseEnsemble* ZFlyEmOrthoDoc::getDvidSynapseEnsemble(
    NeuTube::EAxis axis) const
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
    addDvidLabelSlice(NeuTube::X_AXIS);
    addDvidLabelSlice(NeuTube::Y_AXIS);
    addDvidLabelSlice(NeuTube::Z_AXIS);
  }
}
