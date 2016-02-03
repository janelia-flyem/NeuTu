#include "zflyemorthodoc.h"
#include "dvid/zdvidsynapseensenmble.h"
#include "zstackobjectsourcefactory.h"

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
}

void ZFlyEmOrthoDoc::initSynapseEnsemble(NeuTube::EAxis axis)
{
  ZDvidSynapseEnsemble *se = new ZDvidSynapseEnsemble;
  se->setSliceAxis(axis);
  se->setSource(ZStackObjectSourceFactory::MakeDvidSynapseEnsembleSource(axis));
  addObject(se);
}

void ZFlyEmOrthoDoc::initSynapseEnsemble()
{
  initSynapseEnsemble(NeuTube::X_AXIS);
  initSynapseEnsemble(NeuTube::Y_AXIS);
  initSynapseEnsemble(NeuTube::Z_AXIS);
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

    getDvidSynapseEnsemble(NeuTube::X_AXIS)->setRange(box);
    getDvidSynapseEnsemble(NeuTube::Y_AXIS)->setRange(box);
    getDvidSynapseEnsemble(NeuTube::Z_AXIS)->setRange(box);
  }
}

ZDvidSynapseEnsemble* ZFlyEmOrthoDoc::getDvidSynapseEnsemble(
    NeuTube::EAxis axis) const
{
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
    addDvidLabelSlice(NeuTube::X_AXIS);
    addDvidLabelSlice(NeuTube::Y_AXIS);
    addDvidLabelSlice(NeuTube::Z_AXIS);
  }
}
