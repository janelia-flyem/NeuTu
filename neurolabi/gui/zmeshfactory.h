#ifndef ZMESHFACTORY_H
#define ZMESHFACTORY_H

#include <memory>
#include <vector>

class ZMesh;
class ZObject3dScan;
class ZIntPoint;
class ZObject3dScanArray;

class ZMeshFactory
{
public:
  ZMeshFactory();

  void setDsIntv(int dsIntv);
  void setSmooth(int smooth);
  void setOffsetAdjust(bool on);

  static ZMesh* MakeMesh(const ZObject3dScan &obj);
  static ZMesh* MakeMesh(
      const ZObject3dScan &obj, int dsIntv, int smooth, bool offsetAdjust);
  static ZMesh* MakeFaceMesh(const ZObject3dScan &obj, int dsIntv = 0);

//  static ZMesh* MakeFaceMesh(const ZObject3dScan &obj, int dsIntv, int smooth);
//  static ZMesh* MakeMesh(const ZObject3dScanArray &objArray);

  ZMesh* makeMesh(const ZObject3dScan &obj);
  ZMesh* makeMesh(const ZObject3dScanArray &objArray);
  ZMesh* makeMesh(const std::vector<std::shared_ptr<ZObject3dScan>> &objArray);
//  static ZMesh* MakeMesh(const ZObject3dScan &obj, const ZIntPoint &dsIntv, int smooth);

private:
  template<typename InputIterator>
  ZMesh* makeMesh(const InputIterator &first, const InputIterator &last);

private:
  int m_dsIntv = 0;
  int m_smooth = 3;
  bool m_offsetAdjust = true;
};

#endif // ZMESHFACTORY_H
