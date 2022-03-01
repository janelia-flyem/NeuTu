#include "flyemsparsevolbodymeshfactory.h"

#include "common/debug.h"
#include "zmeshfactory.h"
#include "flyembodysource.h"
#include "dvid/zdvidbodyhelper.h"

FlyEmSparsevolBodyMeshFactory::FlyEmSparsevolBodyMeshFactory()
{

}

void FlyEmSparsevolBodyMeshFactory::setBodySource(
    std::shared_ptr<FlyEmBodySource> source)
{
  m_bodySource = source;
}

void FlyEmSparsevolBodyMeshFactory::setBodySource(FlyEmBodySource* source)
{
  setBodySource(std::shared_ptr<FlyEmBodySource>(source));
}

FlyEmBodyMesh FlyEmSparsevolBodyMeshFactory::make_(const FlyEmBodyConfig &config)
{
  ZMesh *bodyMesh = nullptr;

  if (m_bodySource) {
    if (config.isHybrid()) {
      HLDEBUG("mesh factory") << "Make hybrid mesh from sparsevol" << std::endl;
      ZDvidBodyHelper helper(m_bodySource);
      if (m_bodySource->getCoarseSparsevolScale() > 0) {
        if (config.usingCoarseLevel() ||
            config.getLocalDsLevel() >= m_bodySource->getCoarseSparsevolScale()) {
          helper.setCoarse(true);
        }
      }
      helper.setZoom(config.getDsLevel());
      helper.setRange(config.getRange());
      helper.setLowresZoom(config.getDsLevel());
      helper.setZoom(config.getLocalDsLevel());
      std::vector<std::shared_ptr<ZObject3dScan>> objArray =
          helper.readHybridBody(config.getDecodedBodyId());
      ZMeshFactory mf;
      bodyMesh = mf.makeMesh(objArray);
    } else {
      ZObject3dScan *body = nullptr;
      if (config.usingCoarseLevel()) {
         HLDEBUG("mesh factory") << "Make mesh from coarse sparsevol" << std::endl;
        body = m_bodySource->getCoarseSparsevol(
              config.getBodyId(), config.getRange());
      } else {
         HLDEBUG("mesh factory") << "Make mesh from sparsevol @L"
                                 << config.getDsLevel() << std::endl;
        body = m_bodySource->getSparsevol(
              config.getBodyId(), config.getDsLevel(), config.getRange());
      }

      if (body) {
        bodyMesh = ZMeshFactory::MakeMesh(*body);
        delete body;
      }
    }
  }

  return {bodyMesh, config};
}
