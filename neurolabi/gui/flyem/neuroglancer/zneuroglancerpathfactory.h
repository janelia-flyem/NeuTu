#ifndef ZNEUROGLANCERPATHFACTORY_H
#define ZNEUROGLANCERPATHFACTORY_H

#include <memory>

#include <QString>
#include <QList>

#include "geometry/zpoint.h"

class ZDvidTarget;
class ZIntPoint;
class ZFlyEmBookmark;
class ZDvidEnv;
class ZResolution;
class ZNeuroglancerLayerSpec;

class ZNeuroglancerPathFactory
{
public:
  ZNeuroglancerPathFactory();

  static QString MakePath(
      const ZDvidTarget &target, const ZIntPoint &voxelSize,
      const ZPoint &position,
      const QList<ZFlyEmBookmark*> bookmarkList = QList<ZFlyEmBookmark*>());

  static QString MakePath(
      const ZDvidEnv &target, const ZIntPoint &voxelSize,
      const ZPoint &position,
      const QList<ZFlyEmBookmark*> bookmarkList = QList<ZFlyEmBookmark*>());

  static QString MakePath(
      const ZDvidEnv &env, const ZResolution &voxelSize,
      const ZPoint &position, double scale,
      const QList<std::shared_ptr<ZNeuroglancerLayerSpec>> &additionalLayers =
      QList<std::shared_ptr<ZNeuroglancerLayerSpec>>());

};

#endif // ZNEUROGLANCERPATHFACTORY_H
