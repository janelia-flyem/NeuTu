#ifndef Z3DDEF_H
#define Z3DDEF_H

namespace NeuTube3D {

enum EVolumeRenderingMode {
  VR_AUTO = 0, VR_ALPHA_BLENDING, VR_MIP, VR_MIP_OPAQUE, VR_LOCAL_MIP,
  VR_LOCAL_MIP_OPAQUE, VR_ISO_SURFACE, VR_XRAY
};

enum ESwcGeometryPrimitive {
  SWC_NORMAL, SWC_SPHERE, SWC_LINE, SWC_CYLINDER
};

const char* GetVolumeName();
const char* GetVolumeRenderingModeName(EVolumeRenderingMode mode);

}

#endif // Z3DDEF_H
