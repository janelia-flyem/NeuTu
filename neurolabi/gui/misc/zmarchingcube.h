#ifndef ZMARCHINGCUBE_H
#define ZMARCHINGCUBE_H

class ZStack;
class ZMesh;

class ZMarchingCube
{
public:
  ZMarchingCube();

  void setOffsetAdjust(bool on);
  void setSmooth(int smooth);
  void setResultHost(ZMesh *mesh);

//  static ZMesh* March(const ZStack &stack, ZMesh *out = nullptr);
  static ZMesh* March(const ZStack &stack, int smooth, bool offsetAdjust, ZMesh *out);

  ZMesh* march(const ZStack &stack);

private:
  bool m_offsetAdjust = false;
  int m_smooth = 3;
  ZMesh *m_result = nullptr;
};

#endif // ZMARCHINGCUBE_H
