#ifndef ZMESHFACTORY_H
#define ZMESHFACTORY_H

class ZMesh;
class ZObject3dScan;

class ZMeshFactory
{
public:
  ZMeshFactory();

  ZMesh* MakeMesh(const ZObject3dScan &obj, int dsIntv);
};

#endif // ZMESHFACTORY_H
