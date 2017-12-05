#ifndef ZMESHFACTORY_H
#define ZMESHFACTORY_H

class ZMesh;
class ZObject3dScan;

class ZMeshFactory
{
public:
  ZMeshFactory();

  static ZMesh* MakeMesh(const ZObject3dScan &obj, int dsIntv = 0);
};

#endif // ZMESHFACTORY_H
