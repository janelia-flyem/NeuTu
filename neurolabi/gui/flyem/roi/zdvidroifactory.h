#ifndef ZDVIDROIFACTORY_H
#define ZDVIDROIFACTORY_H

#include <memory>

#include "zabstractroifactory.h"

class ZDvidTarget;
class ZDvidReader;

class ZDvidRoiFactory : public ZAbstractRoiFactory
{
public:
  ZDvidRoiFactory();

  enum class ESource {
    ROI, MESH
  };

  void setSource(ESource source);
  void setDvidTarget(const ZDvidTarget &target);

  ZMesh* makeRoiMesh(const std::string &name) const override;

private:
  std::shared_ptr<ZDvidReader> m_reader;
  ESource m_source = ESource::ROI;

};

#endif // ZDVIDROIFACTORY_H
