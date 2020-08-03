#include "utilities.h"

#include <algorithm>

#include "common/utilities.h"

std::string neutu::ToString(neutu::data3d::ETarget target)
{
  switch (target) {
  case neutu::data3d::ETarget::TARGET_NONE:
    return "NULL";
  case neutu::data3d::ETarget::MASK_CANVAS:
    return "mask canvas";
  case neutu::data3d::ETarget::PIXEL_OBJECT_CANVAS:
    return "object canvas";
  case neutu::data3d::ETarget::WIDGET:
    return "widget";
  case neutu::data3d::ETarget::TILE_CANVAS:
    return "tile canvas";
  case neutu::data3d::ETarget::ONLY_3D:
    return "3D";
  case neutu::data3d::ETarget::DYNAMIC_OBJECT_CANVAS:
    return "dynamic object canvas";
  case neutu::data3d::ETarget::CANVAS_3D:
    return "3D canvas";
  case neutu::data3d::ETarget::STACK_CANVAS:
    return "stack canvas";
  case neutu::data3d::ETarget::HD_OBJECT_CANVAS:
    return "widget canvas";
  case neutu::data3d::ETarget::ROAMING_OBJECT_CANVAS:
    return "active decoration canvas";
  case neutu::data3d::ETarget::NONBLOCKING_OBJECT_CANVAS:
    return "non-blocking canvas";
  }

  return std::to_string(neutu::EnumValue(target));
}

std::vector<neutu::data3d::ETarget> neutu::data3d::GetTargetList()
{
  return
    {
          ETarget::STACK_CANVAS,
          ETarget::TILE_CANVAS,
          ETarget::MASK_CANVAS,
          ETarget::PIXEL_OBJECT_CANVAS,
          ETarget::HD_OBJECT_CANVAS,
          ETarget::DYNAMIC_OBJECT_CANVAS,
          ETarget::ROAMING_OBJECT_CANVAS,
          ETarget::WIDGET,
          ETarget::CANVAS_3D,
          ETarget::ONLY_3D
    };
}


std::vector<neutu::data3d::ETarget> neutu::data3d::GetTargetList(
    std::function<std::vector<ETarget>()> getFullList,
    std::function<bool(ETarget)> pred)
{
  std::vector<neutu::data3d::ETarget> objList;
  std::vector<neutu::data3d::ETarget> fullList = getFullList();
  std::copy_if(
        fullList.begin(), fullList.end(), std::back_inserter(objList), pred);

  return objList;
}

std::vector<neutu::data3d::ETarget> neutu::data3d::GetTargetList(
    std::function<std::vector<ETarget>()> getFullList,
    const std::set<ETarget> & excluded)
{
  std::vector<neutu::data3d::ETarget> fullList = getFullList();
  fullList.erase(
        std::remove_if(fullList.begin(), fullList.end(), [&](ETarget target) {
          return excluded.count(target) > 0; }), fullList.end());

  return fullList;
}

std::vector<neutu::data3d::ETarget> neutu::data3d::GetTarget2dList()
{
  return {
    ETarget::STACK_CANVAS,
    ETarget::TILE_CANVAS,
    ETarget::MASK_CANVAS,
    ETarget::DYNAMIC_OBJECT_CANVAS,
    ETarget::PIXEL_OBJECT_CANVAS,
    ETarget::HD_OBJECT_CANVAS,
    ETarget::ROAMING_OBJECT_CANVAS,
    ETarget::NONBLOCKING_OBJECT_CANVAS,
    ETarget::WIDGET,
  };
}

std::vector<neutu::data3d::ETarget> neutu::data3d::GetTarget2dList(
    const std::set<ETarget> & excluded)
{
  return GetTargetList([](){ return GetTarget2dList(); }, excluded);
}

std::vector<neutu::data3d::ETarget> neutu::data3d::GetTarget2dObjectCanvasList()
{
  return {
    ETarget::TILE_CANVAS,
    ETarget::PIXEL_OBJECT_CANVAS,
    ETarget::HD_OBJECT_CANVAS,
    ETarget::DYNAMIC_OBJECT_CANVAS,
    ETarget::ROAMING_OBJECT_CANVAS,
    ETarget::NONBLOCKING_OBJECT_CANVAS
  };
}

std::vector<neutu::data3d::ETarget> neutu::data3d::GetTarget2dObjectCanvasList(
    const std::set<ETarget> & excluded)
{
  return GetTargetList([](){ return GetTarget2dObjectCanvasList(); }, excluded);
}

std::vector<neutu::data3d::ETarget>
neutu::data3d::GetTargetSettled2dObjectCanvasList()
{
  return
    {
          ETarget::TILE_CANVAS,
          ETarget::PIXEL_OBJECT_CANVAS,
          ETarget::HD_OBJECT_CANVAS,
          ETarget::DYNAMIC_OBJECT_CANVAS,
          ETarget::NONBLOCKING_OBJECT_CANVAS
    };
}

bool neutu::data3d::IsSettled2dObjectCanvas(ETarget target)
{
  return target == ETarget::TILE_CANVAS ||
      target == ETarget::PIXEL_OBJECT_CANVAS ||
      target == ETarget::HD_OBJECT_CANVAS ||
      target == ETarget::DYNAMIC_OBJECT_CANVAS ||
      target == ETarget::NONBLOCKING_OBJECT_CANVAS;
}

bool neutu::data3d::IsNonblocking(ETarget target)
{
  return target == ETarget::NONBLOCKING_OBJECT_CANVAS;
}

