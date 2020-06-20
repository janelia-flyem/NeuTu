#include "z3ddef.h"

#include "common/utilities.h"

static const char *VolumeName = "Volume";

static const char *VolumeRenderingModeNameArray[] = {
  "", "Direct Volume Rendering", "Maximum Intensity Projection",
  "MIP Opaque", "Local MIP", "Local MIP Opaque", "ISO Surface", "X Ray"
};


const char* neutu3d::GetVolumeName()
{
  return VolumeName;
}

const char* neutu3d::GetVolumeRenderingModeName(EVolumeRenderingMode mode)
{
  return VolumeRenderingModeNameArray[neutu::EnumValue(mode)];
}

std::string neutu3d::GetWindowKeyString(neutu3d::EWindowType type)
{
  switch (type) {
  case neutu3d::EWindowType::GENERAL:
    return "window3d_general";
  case neutu3d::EWindowType::COARSE_BODY:
    return "window3d_coarse_body";
  case neutu3d::EWindowType::BODY:
    return "window3d_body";
  case neutu3d::EWindowType::SKELETON:
    return "window3d_skeleton";
  case neutu3d::EWindowType::NEU3:
    return "window3d_neu3_skeleton";
  case neutu3d::EWindowType::MESH:
    return "window3d_mesh";
  }

  return "unknown";
}
