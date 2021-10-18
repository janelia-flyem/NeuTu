#ifndef ZABSTRACTROIFACTORY_H
#define ZABSTRACTROIFACTORY_H

#include <string>

class ZMesh;

class ZAbstractRoiFactory
{
public:
  ZAbstractRoiFactory();
  virtual ~ZAbstractRoiFactory();

  virtual ZMesh* makeRoiMesh(const std::string &name) const = 0;


};

#endif // ZABASTRACTROIFACTORY_H
