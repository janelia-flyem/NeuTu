#ifndef ZMARCHINGCUBE_H
#define ZMARCHINGCUBE_H

class ZStack;
class ZMesh;

class ZMarchingCube
{
public:
  ZMarchingCube();

  static ZMesh* March(const ZStack &stack, ZMesh *out = nullptr);

};

#endif // ZMARCHINGCUBE_H
