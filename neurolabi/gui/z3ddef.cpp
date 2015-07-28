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
