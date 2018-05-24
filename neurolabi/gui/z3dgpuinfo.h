#ifndef Z3DGPUINFO_H
#define Z3DGPUINFO_H

#include "zutils.h"

// This class provides information about the GPU
// If the openGL version is too low or certain critical extensions are not supported,
// isSupported() will return false, and notSupportedReason() will return the reason.
// If isSupported() return false, other functions (except gl*Version functions
// which will still be correct.) will return uninitialized values.

class Z3DGpuInfo
{
public:
  static Z3DGpuInfo& instance();

  enum class GpuVendor
  {
    NVIDIA,
    AMD,
    INTEL,
    UNKNOWN
  };

  Z3DGpuInfo();

  bool isSupported() const
  { return m_isSupported; }

  QString notSupportedReason() const
  { return m_notSupportedReason; }

  int glMajorVersion() const
  { return m_glMajorVersion; }

  int glMinorVersion() const
  { return m_glMinorVersion; }

  int glReleaseVersion() const
  { return m_glReleaseVersion; }

  int glslMajorVersion() const;

  int glslMinorVersion() const;

  int glslReleaseVersion() const;

  GpuVendor gpuVendor() const;

  bool isExtensionSupported(const QString& extension) const;

  QString glVersionString() const;

  QString glVendorString() const;

  QString glRendererString() const;

  QString glExtensionsString() const;

  QString glShadingLanguageVersionString() const;

  // directX 10 resource limit
  // 1D 8192 2D 8192 3D 2048
  // directX 11 resource limit
  // 1D 16384 2D 16384 3D 2048
  int maxTextureSize() const
  { return std::min(m_maxViewportDims, std::min(16384, m_maxTexureSize)); }

  // Returns the maximal side length of 3D textures.
  // directx limit?
  int max3DTextureSize() const
  { return std::min(2048, m_max3DTextureSize); }

  // Return a value such as 16 or 32. That is the number of image samplers that your GPU supports in the fragment shader.
  // the maximum supported texture image units that can be used to access texture maps from the fragment shader.
  // The value must be at least 16
  int maxTextureImageUnits() const
  { return m_maxTextureImageUnits; }

  // The following is for the vertex shader (available since GL 2.0). This might return 0 for certain GPUs.
  // the maximum supported texture image units that can be used to access texture maps from the vertex shader.
  // The value may be at least 16.
  int maxVertexTextureImageUnits() const
  { return m_maxVertexTextureImageUnits; }

  // The following is for the geometry shader (available since GL 3.2)
  // the maximum supported texture image units that can be used to access texture maps from the geometry shader.
  // The value must be at least 16.
  int maxGeometryTextureImageUnits() const
  { return m_maxGeometryTextureImageUnits; }

  // The following is VS + GS + FS (available since GL 2.0)
  // the maximum supported texture image units that can be used to access texture maps from the vertex shader and the
  // fragment processor combined. If both the vertex shader and the fragment processing stage access the same texture image unit,
  // then that counts as using two texture image units against this limit. The value must be at least 48
  int maxCombinedTextureImageUnits() const
  { return m_maxCombinedTextureImageUnits; }

  // and the following is the number of texture coordinates available which usually is 8
  //int maxTextureCoordinates() const { return m_maxTextureCoords; }
  // The value indicates the maximum number of layers allowed in an array texture, and must be at least 256.
  int maxArrayTextureLayers() const
  { return m_maxArrayTextureLayers; }

  uint64_t dedicatedVideoMemoryMB() const
  { return m_dedicatedVideoMemoryMB; }

  // directX 10 resource limit
  // 128 MB
  // directX 11 resource limit
  //min(max(128, 0.25f * (amount of dedicated VRAM)), 2048) MB
  //D3D11_REQ_RESOURCE_SIZE_IN_MEGABYTES_EXPRESSION_A_TERM (128)
  //D3D11_REQ_RESOURCE_SIZE_IN_MEGABYTES_EXPRESSION_B_TERM (0.25f)
  //D3D11_REQ_RESOURCE_SIZE_IN_MEGABYTES_EXPRESSION_C_TERM (2048)
  uint64_t textureSizeLimit() const
  { return std::min(std::max<uint64_t>(128, 0.25 * dedicatedVideoMemoryMB()), 2048_u64) * 1024 * 1024 / 2; }

  // get the required scales to fit uint8_t data of size (width, height, depth) to texture limit
  void getDataScaleForTexture(uint64_t width, uint64_t height, uint64_t depth,
                              double& widthScale, double& heightScale, double& depthScale) const;

  bool needScaleDataForTexture(uint64_t width, uint64_t height, uint64_t depth);

  bool isFrameBufferObjectSupported() const;

  bool isNonPowerOfTwoTextureSupported() const;

  bool isGeometryShaderSupported() const;

  bool isTessellationShaderSupported() const;

  bool isTextureFilterAnisotropicSupported() const;

  bool isTextureRectangleSupported() const;

  // for glBlendEquation
  bool isImagingSupported() const;

  bool isColorBufferFloatSupported() const;

  bool isDepthBufferFloatSupported() const;

  bool isTextureFloatSupported() const;

  bool isTextureRGSupported() const;

  bool isVAOSupported() const;

  float maxTextureAnisotropy() const
  { return m_maxTextureAnisotropy; }

  // Returns the maximal number of color attachments for a FBO
  int maxColorAttachments() const
  { return m_maxColorAttachments; }

  int maxDrawBuffer() const
  { return m_maxDrawBuffer; }

  float minSmoothPointSize() const
  { return m_minSmoothPointSize; }

  float maxSmoothPointSize() const
  { return m_maxSmoothPointSize; }

  float smoothPointSizeGranularity() const
  { return m_smoothPointSizeGranularity; }
  //float minAliasedPointSize() const { return m_minAliasedPointSize; }
  //float maxAliasedPointSize() const { return m_maxAliasedPointSize; }

  float minSmoothLineWidth() const
  { return m_minSmoothLineWidth; }

  float maxSmoothLineWidth() const
  { return m_maxSmoothLineWidth; }

  float smoothLineWidthGranularity() const
  { return m_smoothLineWidthGranularity; }

  float minAliasedLineWidth() const
  { return m_minAliasedLineWidth; }

  float maxAliasedLineWidth() const
  { return m_maxAliasedLineWidth; }

  // log useful gpu info
  void logGpuInfo() const;

  QStringList gpuInfo() const;

  // check avalibility of some special effect
  bool isWeightedAverageSupported() const;

  bool isWeightedBlendedSupported() const;

  bool isDualDepthPeelingSupported() const;

  bool isLinkedListSupported() const;

protected:
  void detectGpuInfo();

  bool parseVersionString(const QString& versionString, int& major, int& minor, int& release);

  void detectDedicatedVideoMemory();

private:
  bool m_isSupported = false;   //whether current graphic card is supported
  QString m_notSupportedReason;  // Reason why current gpu card are not supported

  int m_glMajorVersion;
  int m_glMinorVersion;
  int m_glReleaseVersion;
  int m_glslMajorVersion;
  int m_glslMinorVersion;
  int m_glslReleaseVersion;

  QString m_glVersionString;
  QString m_glExtensionsString;
  QString m_glVendorString;
  QString m_glRendererString;
  QString m_glslVersionString;
  GpuVendor m_gpuVendor;

  int m_maxViewportDims;
  int m_maxRenderbufferSize;
  int m_maxTexureSize;
  int m_max3DTextureSize;
  float m_maxTextureAnisotropy;
  int m_maxColorAttachments;
  int m_maxDrawBuffer;
  int m_maxGeometryOutputVertices;
  int m_maxArrayTextureLayers;

  // Return a value such as 16 or 32. That is the number of image samplers that your GPU supports in the fragment shader.
  int m_maxTextureImageUnits;
  // The following is for the vertex shader (available since GL 2.0). This might return 0 for certain GPUs.
  int m_maxVertexTextureImageUnits;
  // The following is for the geometry shader (available since GL 3.2)
  int m_maxGeometryTextureImageUnits;
  // The following is VS + GS + FS (available since GL 2.0)
  int m_maxCombinedTextureImageUnits;
  // and the following is the number of texture coordinates available which usually is 8
  int m_maxTextureCoords;

  float m_minSmoothPointSize;
  float m_maxSmoothPointSize;
  float m_smoothPointSizeGranularity;
  //float m_minAliasedPointSize;
  //float m_maxAliasedPointSize;

  float m_minSmoothLineWidth;
  float m_maxSmoothLineWidth;
  float m_smoothLineWidthGranularity;
  float m_minAliasedLineWidth;
  float m_maxAliasedLineWidth;

  uint64_t m_dedicatedVideoMemoryMB;

  bool m_contextCoreProfileBit = false;
  bool m_contextCompatibilityProfileBit = false;
  bool m_contextFlagForwardCompatibleBit = false;
  bool m_contextFlagDebugBit = false;
  bool m_contextFlagRobustAccessBit = false;
};

#endif // Z3DGPUINFO_H
