#include "z3ddef.h"



static const char *VolumeName = "Volume";

static const char *VolumeRenderingModeNameArray[] = {
  "", "Direct Volume Rendering", "Maximum Intensity Projection",
  "MIP Opaque", "Local MIP", "Local MIP Opaque", "ISO Surface", "X Ray"
};


const char* NeuTube3D::GetVolumeName()
{
  return VolumeName;
}

const char* NeuTube3D::GetVolumeRenderingModeName(EVolumeRenderingMode mode)
{
  return VolumeRenderingModeNameArray[mode];
}

std::string NeuTube3D::GetWindowKeyString(NeuTube3D::EWindowType type)
{
  switch (type) {
  case NeuTube3D::TYPE_GENERAL:
    return "window3d_general";
  case NeuTube3D::TYPE_COARSE_BODY:
    return "window3d_coarse_body";
  case NeuTube3D::TYPE_BODY:
    return "window3d_body";
  case NeuTube3D::TYPE_SKELETON:
    return "window3d_skeleton";
  }

  return "unknown";
}
