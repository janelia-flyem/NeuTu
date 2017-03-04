#ifndef ZSTACKMULTISCALEWATERSHED_H
#define ZSTACKMULTISCALEWATERSHED_H
#include <QList>
#include <vector>


class ZStack;
class ZSwcTree;

class ZStackMultiScaleWatershed
{
public:
  ZStackMultiScaleWatershed();
  ~ZStackMultiScaleWatershed();
public:
  ZStack* run(ZStack *src,QList<ZSwcTree*>& trees,int scale);
  void test();
private:
  void getSeeds(std::vector<ZStack*>& seeds,QList<ZSwcTree*>& trees);
  ZStack* upSampleAndRecoverEdge(ZStack* sampled_watershed,ZStack* src);

private:
  int _scale;
};

#endif // ZSTACKMULTISCALEWATERSHED_H
