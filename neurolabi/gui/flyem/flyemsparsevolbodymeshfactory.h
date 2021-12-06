#ifndef FLYEMSPARSEVOLBODYMESHFACTORY_H
#define FLYEMSPARSEVOLBODYMESHFACTORY_H

#include <memory>

#include "flyembodymeshfactory.h"

class FlyEmBodySource;

class FlyEmSparsevolBodyMeshFactory : public FlyEmBodyMeshFactory
{
public:
  FlyEmSparsevolBodyMeshFactory();

  void setBodySource(std::shared_ptr<FlyEmBodySource> source);
  void setBodySource(FlyEmBodySource* source);

protected:
  FlyEmBodyMesh _make(const FlyEmBodyConfig &config) override;

private:
  std::shared_ptr<FlyEmBodySource> m_bodySource;
};

#endif // FLYEMSPARSEVOLBODYMESHFACTORY_H
