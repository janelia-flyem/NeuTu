#ifndef ZDVIDGLOBAL_H
#define ZDVIDGLOBAL_H

#include <string>

#include "zdviddef.h"

class ZDvidInfo;
class ZDvidVersionDag;
class ZJsonObject;
class ZDvidTarget;

class ZDvidGlobal
{
public:
  ZDvidGlobal();

  static ZDvidGlobal& GetInstance() {
    static ZDvidGlobal g;

    return g;
  }

  struct Memo {
    static dvid::ENodeStatus ReadNodeStatus(const ZDvidTarget &target);
    static ZJsonObject ReadInfo(const ZDvidTarget &target);
    static ZJsonObject ReadDataInfoJson(
        const ZDvidTarget &target, const std::string &dataName);
    static ZDvidInfo ReadDataInfo(
        const ZDvidTarget &target, const std::string &dataName);
    static ZDvidInfo ReadGrayscaleInfo(const ZDvidTarget &target);
    static ZDvidInfo ReadSegmentationInfo(const ZDvidTarget &target);
    static ZDvidVersionDag ReadVersionDag(const ZDvidTarget &target);
    static ZJsonObject ReadRepoInfo(const ZDvidTarget &target);
    static int ReadMaxLabelZoom(const ZDvidTarget &target);
    static int ReadMaxGrayscaleZoom(const ZDvidTarget &target);
    static ZJsonObject ReadDataStatus(const ZDvidTarget &target);

  private:
    static int ReadMaxZoomFromDataName(
        const ZDvidTarget &target, std::function<std::string(int)> nameGetter);
  };

};

#endif // ZDVIDGLOBAL_H
