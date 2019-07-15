#include "zneuroglancerpathfactory.h"

#include <string>

#include <QUrl>

#include "geometry/zintpoint.h"
#include "geometry/zpoint.h"

#include "dvid/zdvidtarget.h"
#include "dvid/zdvidenv.h"

#include "../zflyembookmark.h"

#include "zneuroglancerpath.h"
#include "zneuroglancerannotationlayerspec.h"
#include "zneuroglancerlayerspecfactory.h"

ZNeuroglancerPathFactory::ZNeuroglancerPathFactory()
{
}

namespace {

ZJsonObject make_bookmark_annotation_json(const ZFlyEmBookmark &bookmark)
{
  ZJsonObject obj;
  int pos[3];
  ZIntPoint center = bookmark.getCenter().toIntPoint();
  pos[0] = center.getX();
  pos[1] = center.getY();
  pos[2] = center.getZ();

  obj.setEntry("point", pos, 3);
  obj.setEntry("type", "point");
  obj.setEntry("description", bookmark.getComment().toStdString());
  obj.setEntry("id",
               std::to_string(center.getX()) + "_" +
               std::to_string(center.getY()) + "_" +
               std::to_string(center.getZ()));

  return obj;
}

}

QString ZNeuroglancerPathFactory::MakePath(
    const ZDvidTarget &target, const ZIntPoint &voxelSize,
    const ZPoint &position, const QList<ZFlyEmBookmark*> bookmarkList)
{
  ZNeuroglancerPath gpath;

  ZNeuroglancerNavigation nav;
  nav.setVoxelSize(voxelSize.getX(), voxelSize.getY(), voxelSize.getZ());
  nav.setCoordinates(position.getX(), position.getY(), position.getZ());
  gpath.setNavigation(nav);

  gpath.addLayer(
        ZNeuroglancerLayerSpecFactory::MakeGrayscaleLayer(target));

  std::string segLayer = "";
  if (target.hasSegmentation()) {
    gpath.addLayer(
          ZNeuroglancerLayerSpecFactory::MakeSegmentationLayer(target));
    segLayer = "segmentation";
  }

  std::shared_ptr<ZNeuroglancerAnnotationLayerSpec> annotLayer =
      ZNeuroglancerLayerSpecFactory::MakePointAnnotationLayer(
        target, segLayer);
  annotLayer->setVoxelSize(voxelSize.getX(), voxelSize.getY(), voxelSize.getZ());
  gpath.addLayer(
        std::dynamic_pointer_cast<ZNeuroglancerLayerSpec>(annotLayer), true);

  for (const ZFlyEmBookmark *bookmark : bookmarkList) {
    annotLayer->addAnnotation(make_bookmark_annotation_json(*bookmark));
  }

  return QString(QUrl(gpath.getPath().c_str()).toEncoded());
}

QString ZNeuroglancerPathFactory::MakePath(
    const ZDvidEnv &env, const ZIntPoint &voxelSize,
    const ZPoint &position, const QList<ZFlyEmBookmark*> bookmarkList)
{
  ZNeuroglancerPath gpath;

  ZNeuroglancerNavigation nav;
  nav.setVoxelSize(voxelSize.getX(), voxelSize.getY(), voxelSize.getZ());
  nav.setCoordinates(position.getX(), position.getY(), position.getZ());
  gpath.setNavigation(nav);

  ZDvidTarget grayscaleTarget = env.getMainGrayscaleTarget();
  if (grayscaleTarget.isValid()) {
    gpath.addLayer(
          ZNeuroglancerLayerSpecFactory::MakeGrayscaleLayer(grayscaleTarget));
  }

  ZDvidTarget target = env.getMainTarget();

  std::string segLayer = "";
  if (target.hasSegmentation()) {
    gpath.addLayer(
          ZNeuroglancerLayerSpecFactory::MakeSegmentationLayer(target));
    segLayer = "segmentation";
  }

  std::shared_ptr<ZNeuroglancerAnnotationLayerSpec> annotLayer =
      ZNeuroglancerLayerSpecFactory::MakePointAnnotationLayer(
        target, segLayer);
  annotLayer->setVoxelSize(voxelSize.getX(), voxelSize.getY(), voxelSize.getZ());
  gpath.addLayer(
        std::dynamic_pointer_cast<ZNeuroglancerLayerSpec>(annotLayer), true);

  for (const ZFlyEmBookmark *bookmark : bookmarkList) {
    annotLayer->addAnnotation(make_bookmark_annotation_json(*bookmark));
  }

  return QString(QUrl(gpath.getPath().c_str()).toEncoded());
}
