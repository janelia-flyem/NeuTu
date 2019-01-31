#include "z3ddef.h"

#include "common/neutube_def.h"

static const char *VolumeName = "Volume";

static const char *VolumeRenderingModeNameArray[] = {
  "", "Direct Volume Rendering", "Maximum Intensity Projection",
  "MIP Opaque", "Local MIP", "Local MIP Opaque", "ISO Surface", "X Ray"
};


const char* neutube3d::GetVolumeName()
{
  return VolumeName;
}

const char* neutube3d::GetVolumeRenderingModeName(EVolumeRenderingMode mode)
{
  return VolumeRenderingModeNameArray[neutube::EnumValue(mode)];
}

std::string neutube3d::GetWindowKeyString(neutube3d::EWindowType type)
{
  switch (type) {
  case neutube3d::EWindowType::GENERAL:
    return "window3d_general";
  case neutube3d::EWindowType::COARSE_BODY:
    return "window3d_coarse_body";
  case neutube3d::EWindowType::BODY:
    return "window3d_body";
  case neutube3d::EWindowType::SKELETON:
    return "window3d_skeleton";
  case neutube3d::EWindowType::NEU3:
    return "window3d_neu3_skeleton";
  case neutube3d::EWindowType::MESH:
    return "window3d_mesh";
  }

  return "unknown";
}
