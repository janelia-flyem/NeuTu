#include "flyemsynapseensemble.h"

#include "flyemsynapseblockgrid.h"
#include "flyemsynapsechunk.h"
#include "flyemsynapsedvidsource.h"

FlyEmSynapseEnsemble::FlyEmSynapseEnsemble()
{
  m_type = GetType();
  setTarget(neutu::data3d::ETarget::NONBLOCKING_OBJECT_CANVAS);
  m_blockGrid = std::shared_ptr<
      ZIntPointAnnotationBlockGrid<ZDvidSynapse, FlyEmSynapseChunk>>(
        new FlyEmSynapseBlockGrid);
}

void FlyEmSynapseEnsemble::setDvidTarget(const ZDvidTarget &target)
{
  _setDvidTarget<FlyEmSynapseDvidSource>(target);
  /*
  FlyEmSynapseDvidSource *source = new FlyEmSynapseDvidSource;
  source->setDvidTarget(target);
  m_blockGrid->setSource(std::shared_ptr<FlyEmSynapseSource>(source));
  */
}
