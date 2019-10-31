#ifndef ZNEUROGLANCERPATHFACTORY_H
#define ZNEUROGLANCERPATHFACTORY_H

#include <QString>
#include <QList>

class ZDvidTarget;
class ZIntPoint;
class ZPoint;
class ZFlyEmBookmark;
class ZDvidEnv;

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


};

#endif // ZNEUROGLANCERPATHFACTORY_H
