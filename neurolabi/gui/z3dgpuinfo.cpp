#include "z3dgpuinfo.h"

#include "z3dgl.h"

#include <QStringList>
#include <QProcess>

#include "logging/zqslog.h"

uint64_t getDedicatedVideoMemoryMB();

Z3DGpuInfo& Z3DGpuInfo::instance()
{
  static Z3DGpuInfo gpuInfo;
  return gpuInfo;
}

Z3DGpuInfo::Z3DGpuInfo()
{
  detectGpuInfo();
}

int Z3DGpuInfo::glslMajorVersion() const
{
  if (isSupported()) {
    return m_glslMajorVersion;
  }
  LOG(FATAL) << "Current GPU card not supported. This function call should not happen.";
  return -1;
}

int Z3DGpuInfo::glslMinorVersion() const
{
  if (isSupported()) {
    return m_glslMinorVersion;
  }
  LOG(FATAL) << "Current GPU card not supported. This function call should not happen.";
  return -1;
}

int Z3DGpuInfo::glslReleaseVersion() const
{
  if (isSupported()) {
    return m_glslReleaseVersion;
  }
  LOG(FATAL) << "Current GPU card not supported. This function call should not happen.";
  return -1;
}

Z3DGpuInfo::GpuVendor Z3DGpuInfo::gpuVendor() const
{
  return m_gpuVendor;
}

bool Z3DGpuInfo::isExtensionSupported(const QString& extension) const
{
  return m_glExtensionsString.contains(extension, Qt::CaseInsensitive);
}

QString Z3DGpuInfo::glVersionString() const
{
  return m_glVersionString;
}

QString Z3DGpuInfo::glVendorString() const
{
  return m_glVendorString;
}

QString Z3DGpuInfo::glRendererString() const
{
  return m_glRendererString;
}

QString Z3DGpuInfo::glShadingLanguageVersionString() const
{
  return m_glslVersionString;
}

void Z3DGpuInfo::getDataScaleForTexture(uint64_t width, uint64_t height, uint64_t depth,
                                        double& widthScale, double& heightScale, double& depthScale) const
{
  bool scaleZ = depth > std::pow(textureSizeLimit() * 1.0, 1 / 3.0);
  double scale = 1.0;
  uint64_t dataSize = width * height * depth;
  if (dataSize > textureSizeLimit()) {
    if (scaleZ)
      scale = std::pow((textureSizeLimit() * 1.0) / dataSize, 1 / 3.0);
    else
      scale = std::sqrt((textureSizeLimit() * 1.0) / dataSize);
  }
  uint64_t resHeight = height * scale;
  uint64_t resWidth = width * scale;
  uint64_t resDepth = scaleZ ? (depth * scale) : double(depth);
  widthScale = scale;
  heightScale = scale;
  depthScale = scaleZ ? scale : 1.0;

  uint64_t maxTexSize = depth > 1 ? max3DTextureSize() : maxTextureSize();
  if (resHeight > maxTexSize) {
    heightScale *= static_cast<double>(maxTexSize) / resHeight;
  }
  if (resWidth > maxTexSize) {
    widthScale *= static_cast<double>(maxTexSize) / resWidth;
  }
  if (resDepth > maxTexSize) {
    depthScale *= static_cast<double>(maxTexSize) / resDepth;
  }
}

bool Z3DGpuInfo::needScaleDataForTexture(uint64_t width, uint64_t height, uint64_t depth)
{
  double s1 = 1.0;
  double s2 = 1.0;
  double s3 = 1.0;
  getDataScaleForTexture(width, height, depth, s1, s2, s3);
  return s1 != 1.0 || s2 != 1.0 || s3 != 1.0;
}

QString Z3DGpuInfo::glExtensionsString() const
{
  return m_glExtensionsString;
}

bool Z3DGpuInfo::isFrameBufferObjectSupported() const
{
  return GLVersionGE(3, 0) || isExtensionSupported("GL_EXT_framebuffer_object");
}

bool Z3DGpuInfo::isNonPowerOfTwoTextureSupported() const
{
  return GLVersionGE(2, 0) || isExtensionSupported("GL_ARB_texture_non_power_of_two");
}

bool Z3DGpuInfo::isGeometryShaderSupported() const
{
  return GLVersionGE(3, 2) ||
         isExtensionSupported("GL_ARB_geometry_shader4") ||
         isExtensionSupported("GL_EXT_geometry_shader4");
}

bool Z3DGpuInfo::isTessellationShaderSupported() const
{
  return GLVersionGE(4, 0);
}

bool Z3DGpuInfo::isTextureFilterAnisotropicSupported() const
{
  return isExtensionSupported("GL_EXT_texture_filter_anisotropic");
}

bool Z3DGpuInfo::isTextureRectangleSupported() const
{
  return GLVersionGE(3, 1) || isExtensionSupported("GL_ARB_texture_rectangle");
}

bool Z3DGpuInfo::isImagingSupported() const
{
  return isExtensionSupported("GL_ARB_imaging");
}

bool Z3DGpuInfo::isColorBufferFloatSupported() const
{
  return GLVersionGE(3, 0) || isExtensionSupported("GL_ARB_color_buffer_float");
}

bool Z3DGpuInfo::isDepthBufferFloatSupported() const
{
  return GLVersionGE(3, 0) || isExtensionSupported("GL_ARB_depth_buffer_float");
}

bool Z3DGpuInfo::isTextureFloatSupported() const
{
  return GLVersionGE(3, 0) || isExtensionSupported("GL_ARB_texture_float");
}

bool Z3DGpuInfo::isTextureRGSupported() const
{
  return GLVersionGE(3, 0) || isExtensionSupported("GL_ARB_texture_rg");
}

bool Z3DGpuInfo::isVAOSupported() const
{
  return GLVersionGE(3, 0) || isExtensionSupported("GL_ARB_vertex_array_object") ||
         isExtensionSupported("GL_APPLE_vertex_array_object");
}

QStringList Z3DGpuInfo::gpuInfo() const
{
  QStringList info;
  if (!isSupported()) {
    info << QString("Current GPU card is not supported. Reason: %1").
      arg(m_notSupportedReason);
    info << "3D functions will be disabled.";
    return info;
  }

#if defined(__APPLE__) && !defined(SANITIZE_THREAD)
  QProcess dispInfo;
  dispInfo.start("system_profiler", QStringList() << "SPDisplaysDataType");

  if (dispInfo.waitForFinished(-1)) {
    info << dispInfo.readAllStandardOutput();
  } else {
    info << dispInfo.readAllStandardError();
  }
#endif

  info << QString("OpenGL Vendor:                 %1").arg(m_glVendorString);
  info << QString("OpenGL Renderer:               %1").arg(m_glRendererString);
  info << QString("OpenGL Version:                %1").arg(m_glVersionString);
  info << QString("OpenGL SL Version:             %1").arg(m_glslVersionString);
  info << QString("Context Core Profile Bit: %1").arg(m_contextCoreProfileBit);
  info << QString("Context Compatibility Profile Bit: %1").arg(m_contextCompatibilityProfileBit);
  info << QString("Context Flag Forward Compatible Bit: %1").arg(m_contextFlagForwardCompatibleBit);
  info << QString("Context Flag Debug Bit: %1").arg(m_contextFlagDebugBit);
  info << QString("Context Flag Robust Access Bit: %1").arg(m_contextFlagRobustAccessBit);
  info << QString("OpenGL Extensions: %1").arg(m_glExtensionsString);
  info << QString("Max Viewport Dimensions:       %1").arg(m_maxViewportDims);
  info << QString("Max Renderbuffer Size:         %1").arg(m_maxRenderbufferSize);
  info << QString("Max Texture Size:              %1 (use %2)").arg(m_maxTexureSize).arg(maxTextureSize());
  info << QString("Max 3D Texture Size:           %1 (use %2)").arg(m_max3DTextureSize).arg(max3DTextureSize());
  info << QString("Max Color Attachments:         %1").arg(m_maxColorAttachments);
  info << QString("Max Draw Buffer:               %1").arg(m_maxDrawBuffer);
  if (isGeometryShaderSupported() && m_maxGeometryOutputVertices > 0) {
    info << QString("Max GS Output Vertices:        %1").
      arg(m_maxGeometryOutputVertices);
  }
  info << QString("Max VS Texture Image Units:    %1").
    arg(m_maxVertexTextureImageUnits);
  if (isGeometryShaderSupported() && m_maxGeometryTextureImageUnits > 0) {
    info << QString("Max GS Texture Image Units:    %1").arg(m_maxGeometryTextureImageUnits);
  }
  info << QString("Max FS Texture Image Units:    %1").arg(m_maxTextureImageUnits);
  info << QString("VS+GS+FS Texture Image Units:  %1").arg(m_maxCombinedTextureImageUnits);
  //info << QString("Max Texture Coordinates:       %1").arg(m_maxTextureCoords);
  info << QString("Max Array Texture Layers:      %1").arg(m_maxArrayTextureLayers);

  info << QString("Total Graphics Memory Size:    %1 MB").arg(dedicatedVideoMemoryMB());

  info << QString("Smooth Point Size Range:       (%1, %2)").arg(m_minSmoothPointSize).arg(m_maxSmoothPointSize);
  info << QString("Smooth Point Size Granularity: %1").arg(m_smoothPointSizeGranularity);
  //info << QString("Aliased Point Size Range:      (%1, %2)").arg(m_minAliasedPointSize).arg(m_maxAliasedPointSize);

  info << QString("Smooth Line Width Range:       (%1, %2)").
    arg(m_minSmoothLineWidth).arg(m_maxSmoothLineWidth);
  info << QString("Smooth Line Width Granularity: %1").
    arg(m_smoothLineWidthGranularity);
  info << QString("Aliased Line Width Range:      (%1, %2)").
    arg(m_minAliasedLineWidth).arg(m_maxAliasedLineWidth);

  return info;
}

void Z3DGpuInfo::logGpuInfo() const
{
  QStringList info = gpuInfo();
  for (int i = 0; i < info.size(); ++i) {
    LINFO() << info[i];
  }

  LINFO() << "";
}

bool Z3DGpuInfo::isWeightedAverageSupported() const
{
  return Z3DGpuInfo::instance().isTextureRGSupported() && Z3DGpuInfo::instance().isTextureRectangleSupported() &&
         Z3DGpuInfo::instance().isTextureFloatSupported() &&
         Z3DGpuInfo::instance().isColorBufferFloatSupported() &&
         Z3DGpuInfo::instance().maxColorAttachments() >= 2;
}

bool Z3DGpuInfo::isWeightedBlendedSupported() const
{
  return Z3DGpuInfo::instance().isTextureRectangleSupported() &&
         Z3DGpuInfo::instance().isTextureFloatSupported() &&
         Z3DGpuInfo::instance().isColorBufferFloatSupported() &&
         Z3DGpuInfo::instance().maxColorAttachments() >= 2;
}

bool Z3DGpuInfo::isDualDepthPeelingSupported() const
{
#if defined(__APPLE__)
  return false;
#else
  return Z3DGpuInfo::instance().isTextureRGSupported() && Z3DGpuInfo::instance().isTextureRectangleSupported() &&
         Z3DGpuInfo::instance().isTextureFloatSupported() &&
         Z3DGpuInfo::instance().isColorBufferFloatSupported() &&
         Z3DGpuInfo::instance().maxColorAttachments() >= 8;
#endif
}

bool Z3DGpuInfo::isLinkedListSupported() const
{
  return GLVersionGE(4, 2);
}

void Z3DGpuInfo::detectGpuInfo()
{
  // reinterpret_cast allowed (AliasedType is the (possibly cv-qualified) signed or unsigned variant of DynamicType)
  m_glVersionString = QString(reinterpret_cast<const char*>(glGetString(GL_VERSION)));
  m_glVendorString = QString(reinterpret_cast<const char*>(glGetString(GL_VENDOR)));
  m_glRendererString = QString(reinterpret_cast<const char*>(glGetString(GL_RENDERER)));

  if (GLVersionGE(3, 0)) {
    GLint contextFlags;
    glGetIntegerv(GL_CONTEXT_FLAGS, &contextFlags);
    m_contextFlagForwardCompatibleBit = contextFlags & GLint(GL_CONTEXT_FLAG_FORWARD_COMPATIBLE_BIT);
    m_contextFlagDebugBit = contextFlags & GLint(GL_CONTEXT_FLAG_DEBUG_BIT);
    m_contextFlagRobustAccessBit = contextFlags & GLint(GL_CONTEXT_FLAG_ROBUST_ACCESS_BIT);
    if (GLVersionGE(3, 2)) {
      GLint contextProfileMask;
      glGetIntegerv(GL_CONTEXT_PROFILE_MASK, &contextProfileMask);
      m_contextCoreProfileBit = contextProfileMask & GLint(GL_CONTEXT_CORE_PROFILE_BIT);
      m_contextCompatibilityProfileBit = contextProfileMask & GLint(GL_CONTEXT_COMPATIBILITY_PROFILE_BIT);
    }
    if (!m_contextFlagForwardCompatibleBit) {
      m_glExtensionsString = QString(reinterpret_cast<const char*>(glGetString(GL_EXTENSIONS)));
    }
  } else {
    m_glExtensionsString = QString(reinterpret_cast<const char*>(glGetString(GL_EXTENSIONS)));
  }

  if (GLVersionGE(2, 1)) {
    if (!isFrameBufferObjectSupported()) {
      m_isSupported = false;
      m_notSupportedReason = "Frame Buffer Object (FBO) is not supported by current openGL context.";
      return;
    }
    if (!isNonPowerOfTwoTextureSupported()) { // not necessary, NPOT texture is supported since opengl 2.0
      m_isSupported = false;
      m_notSupportedReason = "Non power of two texture is not supported by current openGL context.";
      return;
    }
    if (gpuVendor() == GpuVendor::AMD && isNonPowerOfTwoTextureSupported() &&
               (m_glRendererString.contains("RADEON X", Qt::CaseInsensitive) ||
                m_glRendererString.contains("RADEON 9",
                                            Qt::CaseInsensitive))) { //from http://www.opengl.org/wiki/NPOT_Texture
      m_isSupported = false;
      m_notSupportedReason = "The R300 and R400-based cards (Radeon 9500+ and X500+) are incapable of generic NPOT usage. You can use NPOTs, "
        "but only if the texture has no mipmaps.";
      return;
    }
    if (gpuVendor() == GpuVendor::NVIDIA && isNonPowerOfTwoTextureSupported() &&
               m_glRendererString.contains("GeForce FX",
                                           Qt::CaseInsensitive)) { //from http://www.opengl.org/wiki/NPOT_Texture
      m_isSupported = false;
      m_notSupportedReason = "NV30-based cards (GeForce FX of any kind) are incapable of NPOTs at all, despite implementing OpenGL 2.0 "
        "(which requires NPOT). It will do software rendering if you try to use it. ";
      return;
    }
    m_isSupported = true;

    // Prevent segfault
    const char* glslVS = reinterpret_cast<const char*>(glGetString(GL_SHADING_LANGUAGE_VERSION));
    m_glslVersionString = glslVS ? QString(glslVS) : "";

    if (!parseVersionString(m_glVersionString, m_glMajorVersion, m_glMinorVersion, m_glReleaseVersion)) {
      LOG(ERROR) << "Malformed OpenGL version string: " << m_glVersionString;
    }

    // GPU Vendor
    if (m_glVendorString.contains("NVIDIA", Qt::CaseInsensitive)) {
      m_gpuVendor = GpuVendor::NVIDIA;
    } else if (m_glVendorString.contains("ATI", Qt::CaseInsensitive)) {
      m_gpuVendor = GpuVendor::AMD;
    } else if (m_glVendorString.contains("INTEL", Qt::CaseInsensitive)) {
      m_gpuVendor = GpuVendor::INTEL;
    } else {
      m_gpuVendor = GpuVendor::UNKNOWN;
    }

    // Shaders
    if (!parseVersionString(m_glslVersionString, m_glslMajorVersion, m_glslMinorVersion,
                            m_glslReleaseVersion)) {
      LOG(ERROR) << "Malformed GLSL version string: " << m_glslVersionString;
      m_isSupported = false;
      m_notSupportedReason = QString("Malformed GLSL version string: %1").arg(m_glslVersionString);
    }

    m_maxGeometryOutputVertices = -1;
    if (isGeometryShaderSupported())
      glGetIntegerv(GL_MAX_GEOMETRY_OUTPUT_VERTICES_EXT, &m_maxGeometryOutputVertices);

    if (GLVersionGE(3, 0) || isExtensionSupported("GL_EXT_texture_array")) {
      glGetIntegerv(GL_MAX_ARRAY_TEXTURE_LAYERS, &m_maxArrayTextureLayers);
    } else {
      m_maxArrayTextureLayers = 0;
    }

    //
    glGetIntegerv(GL_MAX_VIEWPORT_DIMS, &m_maxViewportDims);
    glGetIntegerv(GL_MAX_RENDERBUFFER_SIZE, &m_maxRenderbufferSize);
    // Texturing
    glGetIntegerv(GL_MAX_TEXTURE_SIZE, &m_maxTexureSize);
    glGetIntegerv(GL_MAX_3D_TEXTURE_SIZE, &m_max3DTextureSize);
    // http://www.opengl.org/wiki/Textures_-_more
    glGetIntegerv(GL_MAX_TEXTURE_IMAGE_UNITS, &m_maxTextureImageUnits);
    glGetIntegerv(GL_MAX_VERTEX_TEXTURE_IMAGE_UNITS, &m_maxVertexTextureImageUnits);
    m_maxGeometryTextureImageUnits = -1;
    if (isGeometryShaderSupported())
      glGetIntegerv(GL_MAX_GEOMETRY_TEXTURE_IMAGE_UNITS, &m_maxGeometryTextureImageUnits);
    glGetIntegerv(GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS, &m_maxCombinedTextureImageUnits);
    if (!GLVersionGE(3, 1)) {
      glGetIntegerv(GL_MAX_TEXTURE_COORDS, &m_maxTextureCoords);
    }

    if (isTextureFilterAnisotropicSupported()) {
      glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &m_maxTextureAnisotropy);
    } else {
      m_maxTextureAnisotropy = 1.0;
    }

    glGetIntegerv(GL_MAX_COLOR_ATTACHMENTS_EXT, &m_maxColorAttachments);
    glGetIntegerv(GL_MAX_DRAW_BUFFERS, &m_maxDrawBuffer);

    // Point
    GLfloat range[2];
    glGetFloatv(GL_SMOOTH_POINT_SIZE_RANGE, range);
    glGetFloatv(GL_SMOOTH_POINT_SIZE_GRANULARITY, &m_smoothPointSizeGranularity);
    m_minSmoothPointSize = range[0];
    m_maxSmoothPointSize = range[1];
    // can not find in opengl doc
    //glGetFloatv(GL_ALIASED_POINT_SIZE_RANGE, range);
    //m_minAliasedPointSize = range[0];
    //m_maxAliasedPointSize = range[1];

    // Line
    glGetFloatv(GL_SMOOTH_LINE_WIDTH_RANGE, range);
    glGetFloatv(GL_SMOOTH_LINE_WIDTH_GRANULARITY, &m_smoothLineWidthGranularity);
    m_minSmoothLineWidth = range[0];
    m_maxSmoothLineWidth = range[1];
    glGetFloatv(GL_ALIASED_LINE_WIDTH_RANGE, range);
    m_minAliasedLineWidth = range[0];
    m_maxAliasedLineWidth = range[1];

    detectDedicatedVideoMemory();
  } else {
    m_isSupported = false;
    m_notSupportedReason =
      "Minimum OpenGL version required is 2.1, while current openGL version is: \"" + m_glVersionString + "\"";
  }
}

// format "2.1[.1] otherstring"
bool Z3DGpuInfo::parseVersionString(const QString& versionString, int& major, int& minor, int& release)
{
  major = -1;
  minor = -1;
  release = -1;

  if (versionString.isEmpty())
    return false;

  QString str = versionString.mid(0, versionString.indexOf(" "));
  QStringList list = str.split(".");
  if (list.size() < 2 || list.size() > 3)
    return false;

  bool ok;
  major = list[0].toInt(&ok);
  if (!ok) {
    major = -1;
    return false;
  }

  minor = list[1].toInt(&ok);
  if (!ok) {
    major = -1;
    minor = -1;
    return false;
  }

  if (list.size() > 2) {
    release = list[2].toInt(&ok);
    if (!ok) {
      major = -1;
      minor = -1;
      release = -1;
      return false;
    }
  } else
    release = 0;

  return true;
}

void Z3DGpuInfo::detectDedicatedVideoMemory()
{
  m_dedicatedVideoMemoryMB = 0;
  if (m_gpuVendor == GpuVendor::NVIDIA) {
    if (isExtensionSupported("GL_NVX_gpu_memory_info")) {
      int retVal;
      //glGetIntegerv(GL_GPU_MEMORY_INFO_DEDICATED_VIDMEM_NVX, &retVal);
      glGetIntegerv(GL_GPU_MEMORY_INFO_TOTAL_AVAILABLE_MEMORY_NVX, &retVal);
      m_dedicatedVideoMemoryMB = retVal / 1024;
    }
  } else if (m_gpuVendor == GpuVendor::AMD) {
    if (isExtensionSupported("GL_ATI_meminfo")) {
      int retVal[4];
      glGetIntegerv(GL_TEXTURE_FREE_MEMORY_ATI, retVal);
      m_dedicatedVideoMemoryMB = retVal[0] / 1024;
    }
  }
  if (m_dedicatedVideoMemoryMB == 0) {
    m_dedicatedVideoMemoryMB = getDedicatedVideoMemoryMB();
  }
  if (m_dedicatedVideoMemoryMB == 0) {
    LOG(ERROR) << "Can not detect dedicated video memory, use 256";
    m_dedicatedVideoMemoryMB = 256;
  }
}
