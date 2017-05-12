#ifndef ZSTACKMULTISCALEWATERSHED_H
#define ZSTACKMULTISCALEWATERSHED_H

#if defined(_QT_GUI_USED_)
#include <QList>
#endif

#include <vector>

class ZStack;
class ZSwcTree;

class ZStackMultiScaleWatershed
{
public:
  ZStackMultiScaleWatershed();
  ~ZStackMultiScaleWatershed();
public:
#if defined(_QT_GUI_USED_)
  ZStack* run(ZStack *src,QList<ZSwcTree*>& trees,int scale);
#endif


private:
#if defined(_QT_GUI_USED_)
  void getSeeds(std::vector<ZStack*>& seeds,QList<ZSwcTree*>& trees);
#endif
  ZStack* upSampleAndRecoverEdge(ZStack* sampled_watershed,ZStack* src);

private:
  int _scale;
};

#endif // ZSTACKMULTISCALEWATERSHED_H
