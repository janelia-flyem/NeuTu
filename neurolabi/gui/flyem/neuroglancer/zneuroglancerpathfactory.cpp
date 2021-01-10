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
  ZDvidEnv env;
  env.setMainTarget(target);
  return MakePath(env, voxelSize, position, bookmarkList);
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

//    gpath.addLayer(ZNeuroglancerLayerSpecFactory::MakeSkeletonLayer(target));
  }

  std::shared_ptr<ZNeuroglancerAnnotationLayerSpec> annotLayer =
      ZNeuroglancerLayerSpecFactory::MakePointAnnotationLayer(
        target, ZDvidData::ERole::BOOKMARK, segLayer);
  annotLayer->setVoxelSize(voxelSize);
  gpath.addLayer(
        std::dynamic_pointer_cast<ZNeuroglancerLayerSpec>(annotLayer), true);

  for (const ZFlyEmBookmark *bookmark : bookmarkList) {
    annotLayer->addAnnotation(make_bookmark_annotation_json(*bookmark));
  }

  if (target.hasSynapse()) {
    std::shared_ptr<ZNeuroglancerAnnotationLayerSpec> synapseLayer =
        ZNeuroglancerLayerSpecFactory::MakePointAnnotationLayer(
          target, ZDvidData::ERole::SYNAPSE, segLayer);
    synapseLayer->setVoxelSize(voxelSize);
    gpath.addLayer(
          std::dynamic_pointer_cast<ZNeuroglancerLayerSpec>(synapseLayer), false);
  }



  return QString(QUrl(gpath.getPath().c_str()).toEncoded());
}

QString ZNeuroglancerPathFactory::MakePath(
    const ZDvidEnv &env, const ZResolution &voxelSize,
    const ZPoint &position, double scale,
    const QList<std::shared_ptr<ZNeuroglancerLayerSpec>> &additionalLayers)
{
  ZNeuroglancerPath gpath;

  ZNeuroglancerNavigation nav;
  nav.setVoxelSize(voxelSize);
  nav.setPosition(position);
  nav.setZoomScale2D(scale);
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

//    gpath.addLayer(ZNeuroglancerLayerSpecFactory::MakeSkeletonLayer(target));
  }

  std::shared_ptr<ZNeuroglancerAnnotationLayerSpec> annotLayer =
      ZNeuroglancerLayerSpecFactory::MakePointAnnotationLayer(
        target, ZDvidData::ERole::BOOKMARK, segLayer);
  gpath.addLayer(
        std::dynamic_pointer_cast<ZNeuroglancerLayerSpec>(annotLayer), true);
  annotLayer->setShader(
        "#uicontrol vec3 falseSplitColor color(default=\"#F08040\")\n"
        "#uicontrol vec3 falseMergeColor color(default=\"#F040F0\")\n"
        "#uicontrol vec3 checkedColor color(default=\"green\")\n"
        "#uicontrol vec3 borderColor color(default=\"black\")\n"
        "#uicontrol float radius slider(min=3, max=20, step=1, default=10)\n"
        "#uicontrol float opacity slider(min=0, max=1, step=0.1, default=1)\n"
        "#uicontrol float opacity3D slider(min=0, max=1, step=0.1, default=0.2)\n"
        "void main() {\n"
        "  setPointMarkerSize(radius);\n"
        "  float finalOpacity = PROJECTION_VIEW ? opacity3D : opacity;\n"
        "  setPointMarkerBorderColor(vec4(borderColor, finalOpacity));\n"
        "  if (prop_rendering_attribute() == 1) {\n"
        "    setColor(vec4(checkedColor, finalOpacity));\n"
        "  } else if (prop_rendering_attribute() == 2) {\n"
        "    setColor(vec4(falseSplitColor, finalOpacity));\n"
        "  } else if (prop_rendering_attribute() == 3)  {\n"
        "    setColor(vec4(falseMergeColor, finalOpacity));\n"
        "  } else {\n"
        "    setColor(vec4(1, 0, 0, finalOpacity));\n"
        "  }\n"
        "}");


  if (target.hasSynapse()) {
    std::shared_ptr<ZNeuroglancerAnnotationLayerSpec> synapseLayer =
        ZNeuroglancerLayerSpecFactory::MakePointAnnotationLayer(
          target, ZDvidData::ERole::SYNAPSE, segLayer);
    synapseLayer->setShader(
          "#uicontrol float radius slider(min=3, max=20, step=1, default=5)\n"
          "#uicontrol float opacity slider(min=0, max=1, step=0.1, default=1)\n"
          "#uicontrol float opacity3D slider(min=0, max=1, step=0.1, default=0.2)\n"
          "void main() {\n"
          "  setPointMarkerSize(radius);\n"
          "  float finalOpacity = PROJECTION_VIEW ? opacity3D : opacity;\n"
          "  setColor(vec4(defaultColor(), finalOpacity));\n"
          "}");

    gpath.addLayer(
          std::dynamic_pointer_cast<ZNeuroglancerLayerSpec>(synapseLayer), false);
  }

  for (auto layer : additionalLayers) {
    gpath.addLayer(layer);
  }

#ifdef _DEBUG_
  std::cout << "Layer spec:" << std::endl;
  std::cout << gpath.toJsonObject().dumpString(2) << std::endl;
#endif

  return QString(QUrl(gpath.getPath().c_str()).toEncoded());
}
