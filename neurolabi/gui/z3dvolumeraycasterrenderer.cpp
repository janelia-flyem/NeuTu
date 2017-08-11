#include "z3dvolumeraycasterrenderer.h"

#include "z3dtexture.h"
#include "z3dvolume.h"

Z3DVolumeRaycasterRenderer::Z3DVolumeRaycasterRenderer(Z3DRendererBase& rendererBase)
  : Z3DPrimitiveRenderer(rendererBase)
  , m_samplingRate("Sampling Rate", 2.f, 0.01f, 20.f)
  , m_isoValue("ISO Value", 0.5f, 0.0f, 1.0f)
  , m_localMIPThreshold("Local MIP Threshold", 0.8f, 0.01f, 1.f)
  , m_compositingMode("Compositing")
  , m_is2DImage(false)
  , m_entryTexCoordTexture(nullptr)
  , m_entryEyeCoordTexture(nullptr)
  , m_exitTexCoordTexture(nullptr)
  , m_exitEyeCoordTexture(nullptr)
  , m_opaque(false)
  , m_alpha(1.0)
  , m_VAO(1)
{
  // compositing modes
  m_compositingMode.addOptions("Direct Volume Rendering", "Maximum Intensity Projection",
                               "MIP Opaque", "Local MIP", "Local MIP Opaque", "ISO Surface", "X Ray");
  m_compositingMode.select("MIP Opaque");

  connect(&m_compositingMode, &ZStringIntOptionParameter::valueChanged, this, &Z3DVolumeRaycasterRenderer::adjustWidgets);
  connect(&m_compositingMode, &ZStringIntOptionParameter::valueChanged, this, &Z3DVolumeRaycasterRenderer::compile);
  //connect(&m_gradientMode, SIGNAL(valueChanged()), this, SLOT(compile()));

  adjustWidgets();

  //  m_raycasterShader.bindFragDataLocation(0, "FragData0");
  //  m_raycasterShader.loadFromSourceFile("pass.vert", "volume_raycaster.frag",
  //                                       m_rendererBase.generateHeader() + generateHeader());
  //  m_2dImageShader.bindFragDataLocation(0, "FragData0");
  //  m_2dImageShader.loadFromSourceFile("transform_with_2dtexture.vert", "image2d_with_transfun.frag",
  //                                     m_rendererBase.generateHeader() + generateHeader());
  //  m_volumeSliceWithTransferfunShader.bindFragDataLocation(0, "FragData0");
  //  m_volumeSliceWithTransferfunShader.loadFromSourceFile("transform_with_3dtexture.vert", "volume_slice_with_transfun.frag",
  //                                                        m_rendererBase.generateHeader() + generateHeader());

  m_scRaycasterShader.bindFragDataLocation(0, "FragData0");
  m_scRaycasterShader.loadFromSourceFile("pass.vert", "volume_raycaster_single_channel.frag",
                                         m_rendererBase.generateHeader() + generateHeader());
  m_sc2dImageShader.bindFragDataLocation(0, "FragData0");
  m_sc2dImageShader.loadFromSourceFile("transform_with_2dtexture.vert", "image2d_with_transfun_single_channel.frag",
                                       m_rendererBase.generateHeader() + generateHeader());
  m_scVolumeSliceWithTransferfunShader.bindFragDataLocation(0, "FragData0");
  m_scVolumeSliceWithTransferfunShader.loadFromSourceFile("transform_with_3dtexture.vert",
                                                          "volume_slice_with_transfun_single_channel.frag",
                                                          m_rendererBase.generateHeader() + generateHeader());
  m_mergeChannelShader.bindFragDataLocation(0, "FragData0");
  m_mergeChannelShader.loadFromSourceFile("pass.vert", "image2d_array_compositor.frag",
                                          m_rendererBase.generateHeader() + generateHeader());
  CHECK_GL_ERROR
}

void Z3DVolumeRaycasterRenderer::setChannels(const std::vector<std::unique_ptr<Z3DVolume> >& volsIn)
{
  std::vector<Z3DVolume*> vols;
  for (size_t i = 0; i < volsIn.size(); ++i) {
    vols.push_back(volsIn[i].get());
  }

  if (m_volumes != vols) {
    bool numChannelsChanged = m_volumes.size() != vols.size();
    if (numChannelsChanged) {
      m_volumeUniformNames.clear();
      m_transferFuncUniformNames.clear();
      m_channelVisibleParas.clear();
      m_transferFuncParas.clear();
      m_texFilterModeParas.clear();
    }

    m_volumes = vols;

    if (numChannelsChanged) {
      for (size_t i = 0; i < m_volumes.size(); ++i) {
        m_volumeUniformNames.push_back(QString("volume_%1").arg(i + 1));
        m_volumeDimensionNames.push_back(QString("volume_dimensions_%1").arg(i + 1));
        m_transferFuncUniformNames.push_back(QString("transfer_function_%1").arg(i + 1));
        m_channelVisibleParas.emplace_back(
              std::make_unique<ZBoolParameter>(QString("Show Channel %1").arg(i + 1), true));
        connect(m_channelVisibleParas[i].get(), &ZBoolParameter::valueChanged, this, &Z3DVolumeRaycasterRenderer::compile);
        m_transferFuncParas.emplace_back(
              std::make_unique<Z3DTransferFunctionParameter>(QString("Transfer Function %1").arg(i + 1)));
        m_texFilterModeParas.emplace_back(
              std::make_unique<ZStringIntOptionParameter>(QString("Texture Filtering %1").arg(i + 1)));
        m_texFilterModeParas[i]->addOptionsWithData(qMakePair(QString("Nearest"), static_cast<int>(GL_NEAREST)),
                                                    qMakePair(QString("Linear"), static_cast<int>(GL_LINEAR)));
        m_texFilterModeParas[i]->select("Linear");
      }
    }

    for (size_t i = 0; i < m_volumes.size(); ++i) {
      m_transferFuncParas[i]->setVolume(m_volumes[i]);
    }

    m_is2DImage = !m_volumes.empty() && m_volumes[0]->is2DData();

    if (numChannelsChanged) {
      compile();
      resetTransferFunctions();
    }
  }
}

void Z3DVolumeRaycasterRenderer::addQuad(const ZMesh &quad)
{
  if (quad.empty() ||
      (quad.numVertices() != 4 && quad.numVertices() != 6) ||
      (quad.numVertices() != quad.num2DTextureCoordinates() &&
       quad.numVertices() != quad.num3DTextureCoordinates())) {
    LOG(ERROR) << "Input quad should be 2D slice with either 2D or 3D texture coordinates";
    return;
  }
  m_quads.push_back(quad);
  m_entryTexCoordTexture = nullptr;
  m_entryEyeCoordTexture = nullptr;
  m_exitTexCoordTexture = nullptr;
  m_exitEyeCoordTexture = nullptr;
}

void Z3DVolumeRaycasterRenderer::setEntryExitInfo(const Z3DTexture* entryTexCoordTexture,
                                                  const Z3DTexture* entryEyeCoordTexture,
                                                  const Z3DTexture* exitTexCoordTexture,
                                                  const Z3DTexture* exitEyeCoordTexture)
{
  m_entryTexCoordTexture = entryTexCoordTexture;
  m_entryEyeCoordTexture = entryEyeCoordTexture;
  m_exitTexCoordTexture = exitTexCoordTexture;
  m_exitEyeCoordTexture = exitEyeCoordTexture;
  m_quads.clear();
}

void Z3DVolumeRaycasterRenderer::adjustWidgets()
{
  m_isoValue.setVisible(m_compositingMode.isSelected("ISO Surface"));
  m_localMIPThreshold.setVisible(m_compositingMode.isSelected("Local MIP") ||
                                 m_compositingMode.isSelected("Local MIP Opaque"));
}

void Z3DVolumeRaycasterRenderer::bindVolumesAndTransferFuncs(Z3DShaderProgram &shader)
{
  shader.setLogUniformLocationError(false);

  size_t idx = 0;
  for (size_t i = 0; i < m_volumes.size(); ++i) {
    if (!m_channelVisibleParas[i]->get())
      continue;

    // volumes
    shader.bindTexture(m_volumeUniformNames[idx], m_volumes[i]->texture(), m_texFilterModeParas[i]->associatedData(),
                       m_texFilterModeParas[i]->associatedData());
    shader.setUniform(m_volumeDimensionNames[idx], glm::vec3(m_volumes[i]->dimensions()));

    // transfer functions
    shader.bindTexture(m_transferFuncUniformNames[idx++], m_transferFuncParas[i]->get().texture());

    CHECK_GL_ERROR
  }

  shader.setLogUniformLocationError(true);
}

void Z3DVolumeRaycasterRenderer::bindVolumeAndTransferFunc(Z3DShaderProgram& shader, size_t idx)
{
  shader.setLogUniformLocationError(false);

  shader.bindTexture(m_volumeUniformNames[0], m_volumes[idx]->texture(), m_texFilterModeParas[idx]->associatedData(),
      m_texFilterModeParas[idx]->associatedData());
  shader.setUniform(m_volumeDimensionNames[0], glm::vec3(m_volumes[idx]->dimensions()));

  // transfer functions
  shader.bindTexture(m_transferFuncUniformNames[0], m_transferFuncParas[idx]->get().texture());

  CHECK_GL_ERROR

  shader.setLogUniformLocationError(true);
}

void Z3DVolumeRaycasterRenderer::compile()
{
  m_scRaycasterShader.setHeaderAndRebuild(m_rendererBase.generateHeader() + generateHeader());
  m_sc2dImageShader.setHeaderAndRebuild(m_rendererBase.generateHeader() + generateHeader());
  m_scVolumeSliceWithTransferfunShader.setHeaderAndRebuild(m_rendererBase.generateHeader() + generateHeader());
  m_mergeChannelShader.setHeaderAndRebuild(m_rendererBase.generateHeader() + generateHeader());
}

QString Z3DVolumeRaycasterRenderer::generateHeader()
{
  QString headerSource;

  if (hasVisibleRendering()) {
    size_t numVisibleChannels = 0;
    for (size_t i = 0; i < m_volumes.size(); ++i) {
      if (m_channelVisibleParas[i]->get()) {
        ++numVisibleChannels;
      }
    }
    headerSource += QString("#define NUM_VOLUMES %1\n").arg(numVisibleChannels);
  } else {
    headerSource += QString("#define NUM_VOLUMES 0\n");
    headerSource += "#define DISABLE_TEXTURE_COORD_OUTPUT\n";
  }

  //  if (!m_gradientMode.isSelected("None"))
  //    headerSource += "#define USE_GRADIENTS\n";

  if (m_compositingMode.isSelected("Direct Volume Rendering")) {
    headerSource += "#define COMPOSITING(result, color, currentRayLength, rayDepth) ";
    headerSource += "compositeDVR(result, color, currentRayLength, rayDepth);\n";
  } else if (m_compositingMode.isSelected("ISO Surface")) {
    headerSource += "#define ISO\n";
    headerSource += "#define COMPOSITING(result, color, currentRayLength, rayDepth) ";
    headerSource += "compositeISO(result, color, currentRayLength, rayDepth, iso_value);\n";
  } else if (m_compositingMode.isSelected("Maximum Intensity Projection")) {
    headerSource += "#define MIP\n";
  } else if (m_compositingMode.isSelected("Local MIP")) {
    headerSource += "#define MIP\n";
    headerSource += "#define LOCAL_MIP\n";
  } else if (m_compositingMode.isSelected("X Ray")) {
    headerSource += "#define COMPOSITING(result, color, currentRayLength, rayDepth) ";
    headerSource += "compositeXRay(result, color, currentRayLength, rayDepth);\n";
  } else if (m_compositingMode.isSelected("MIP Opaque")) {
    headerSource += "#define MIP\n";
    headerSource += "#define RESULT_OPAQUE\n";
  } else if (m_compositingMode.isSelected("Local MIP Opaque")) {
    headerSource += "#define MIP\n";
    headerSource += "#define LOCAL_MIP\n";
    headerSource += "#define RESULT_OPAQUE\n";
  }

  if (!m_quads.empty() ||
      m_compositingMode.isSelected("Maximum Intensity Projection") ||
      m_compositingMode.isSelected("Local MIP") ||
      m_compositingMode.isSelected("MIP Opaque") ||
      m_compositingMode.isSelected("Local MIP Opaque")) {
    headerSource += "#define MAX_PROJ_MERGE\n";
  }

  return headerSource;
}

void Z3DVolumeRaycasterRenderer::render(Z3DEye eye)
{
  if (!hasVisibleRendering())
    return;

  if (m_quads.empty()) {
    if (m_entryTexCoordTexture == nullptr || m_entryEyeCoordTexture == nullptr ||
        m_exitTexCoordTexture == nullptr || m_exitEyeCoordTexture == nullptr)
      return;
  } else {
    for (size_t i = 0; i < m_quads.size(); ++i) {
      if (m_is2DImage && m_quads[i].numVertices() != m_quads[i].num2DTextureCoordinates())
        return;
      if (!m_is2DImage && m_quads[i].numVertices() != m_quads[i].num3DTextureCoordinates())
        return;
    }
  }

  std::vector<size_t> visibleIdxs;
  for (size_t i = 0; i < m_volumes.size(); ++i) {
    if (m_channelVisibleParas[i]->get()) {
      visibleIdxs.push_back(i);
    }
  }

  if (!m_quads.empty()) { // 2d image or slice from 3d volume
    if (m_is2DImage) {   // image is 2D
      m_sc2dImageShader.bind();
      m_rendererBase.setGlobalShaderParameters(m_sc2dImageShader, eye);

      if (visibleIdxs.size() == 1) {
        bindVolumeAndTransferFunc(m_sc2dImageShader, visibleIdxs[0]);

        for (size_t i = 0; i < m_quads.size(); ++i)
          renderTriangleList(m_VAO, m_sc2dImageShader, m_quads[i]);

      } else {
        for (size_t j = 0; j < visibleIdxs.size(); ++j) {
          m_layerTarget->attachSlice(j);
          m_layerTarget->bind();
          m_layerTarget->clear();
          bindVolumeAndTransferFunc(m_sc2dImageShader, visibleIdxs[j]);

          for (size_t i = 0; i < m_quads.size(); ++i)
            renderTriangleList(m_VAO, m_sc2dImageShader, m_quads[i]);

          m_layerTarget->release();
        }
      }

      m_sc2dImageShader.release();
    } else {   // image is 3D, but a 2D slice will be shown
      m_scVolumeSliceWithTransferfunShader.bind();
      m_rendererBase.setGlobalShaderParameters(m_scVolumeSliceWithTransferfunShader, eye);

      if (visibleIdxs.size() == 1) {
        bindVolumeAndTransferFunc(m_scVolumeSliceWithTransferfunShader, visibleIdxs[0]);

        for (size_t i = 0; i < m_quads.size(); ++i)
          renderTriangleList(m_VAO, m_scVolumeSliceWithTransferfunShader, m_quads[i]);
      } else {
        for (size_t j = 0; j < visibleIdxs.size(); ++j) {
          m_layerTarget->attachSlice(j);
          m_layerTarget->bind();
          m_layerTarget->clear();

          bindVolumeAndTransferFunc(m_scVolumeSliceWithTransferfunShader, visibleIdxs[j]);

          for (size_t i = 0; i < m_quads.size(); ++i)
            renderTriangleList(m_VAO, m_scVolumeSliceWithTransferfunShader, m_quads[i]);

          m_layerTarget->release();
        }
      }

      m_scVolumeSliceWithTransferfunShader.release();
    }
  } else {  // 3d volume raycasting
    m_scRaycasterShader.bind();

    m_rendererBase.setGlobalShaderParameters(m_scRaycasterShader, eye);

    float n = m_rendererBase.camera().nearDist();
    float f = m_rendererBase.camera().farDist();
    //http://www.opengl.org/archives/resources/faq/technical/depthbuffer.htm
    // zw = a/ze + b;  ze = a/(zw - b);  a = f*n/(f-n);  b = 0.5*(f+n)/(f-n) + 0.5;
    float a = f * n / (f - n);
    float b = 0.5f * (f + n) / (f - n) + 0.5f;
    m_scRaycasterShader.setUniform("ze_to_zw_b", b);
    m_scRaycasterShader.setUniform("ze_to_zw_a", a);

    // entry exit points
    m_scRaycasterShader.bindTexture("ray_entry_tex_coord", m_entryTexCoordTexture);
    m_scRaycasterShader.bindTexture("ray_entry_eye_coord", m_entryEyeCoordTexture);
    m_scRaycasterShader.bindTexture("ray_exit_tex_coord", m_exitTexCoordTexture);
    m_scRaycasterShader.bindTexture("ray_exit_eye_coord", m_exitEyeCoordTexture);

    if (m_compositingMode.get() == "ISO Surface")
      m_scRaycasterShader.setUniform("iso_value", m_isoValue.get());

    if (m_compositingMode.get() == "Local MIP" || m_compositingMode.get() == "Local MIP Opaque")
      m_scRaycasterShader.setUniform("local_MIP_threshold", m_localMIPThreshold.get());

    m_scRaycasterShader.setUniform("sampling_rate", m_samplingRate.get());

    if (visibleIdxs.size() == 1) {
      bindVolumeAndTransferFunc(m_scRaycasterShader, visibleIdxs[0]);
      renderScreenQuad(m_VAO, m_scRaycasterShader);
    } else {
      for (size_t i = 0; i < visibleIdxs.size(); ++i) {
        m_layerTarget->attachSlice(i);
        m_layerTarget->bind();
        m_layerTarget->clear();

        bindVolumeAndTransferFunc(m_scRaycasterShader, visibleIdxs[i]);
        renderScreenQuad(m_VAO, m_scRaycasterShader);

        m_layerTarget->release();
      }
    }

    m_scRaycasterShader.release();
  }

  if (visibleIdxs.size() > 1) {
    m_mergeChannelShader.bind();
    m_mergeChannelShader.bindTexture("color_texture", m_layerTarget->attachment(GL_COLOR_ATTACHMENT0));
    m_mergeChannelShader.bindTexture("depth_texture", m_layerTarget->attachment(GL_DEPTH_ATTACHMENT));
    renderScreenQuad(m_VAO, m_mergeChannelShader);
    m_mergeChannelShader.release();
  }

  CHECK_GL_ERROR
}

void Z3DVolumeRaycasterRenderer::renderPicking(Z3DEye)
{
}

void Z3DVolumeRaycasterRenderer::resetTransferFunctions()
{
#if 1
  for (size_t i=0; i<m_transferFuncParas.size(); ++i) {
    if (m_opaque) {
      m_transferFuncParas[i]->get().reset(
            0.0, 1.0, glm::vec4(0.f),
            glm::vec4(m_volumes[i]->volColor(), 1.0));
      m_transferFuncParas[i]->get().addKey(
            ZColorMapKey(0.001, glm::vec4(0.01f, 0.01f, 0.01f,0.0)));
      m_transferFuncParas[i]->get().addKey(
            ZColorMapKey(0.01, glm::vec4(0.01f, 0.01f, 0.01f,1.0)));
    } else {
      m_transferFuncParas[i]->get().reset(
            0.0, 1.0, glm::vec4(0.f),
            glm::vec4(m_volumes[i]->volColor(), 1.f));
      //m_transferFuncParas[i]->get().addKey(ZColorMapKey(0.1, glm::vec4(m_volumes[i]->getVolColor(), 1.f) *
      //                                                  glm::vec4(.1f,.1f,.1f,0.f)));
    }
  }
#else
  if (m_nChannel == 1) {
    m_transferFunc1.get().reset(0.0, 1.0, glm::col4(0, 0, 0, 0), glm::col4(255, 255, 255, 255));
    m_transferFunc1.get().addKey(ZColorMapKey(0.1, glm::col4(25,25,25,0)));
  } else {
    m_transferFunc1.get().reset(0.0, 1.0, glm::col4(0, 0, 0, 0), glm::col4(255, 0, 0, 255));
    m_transferFunc2.get().reset(0.0, 1.0, glm::col4(0, 0, 0, 0), glm::col4(0, 255, 0, 255));
    m_transferFunc3.get().reset(0.0, 1.0, glm::col4(0, 0, 0, 0), glm::col4(0, 0, 255, 255));
    m_transferFunc1.get().addKey(ZColorMapKey(0.1, glm::col4(25,0,0,0)));
    m_transferFunc2.get().addKey(ZColorMapKey(0.1, glm::col4(0,25,0,0)));
    m_transferFunc3.get().addKey(ZColorMapKey(0.1, glm::col4(0,0,25,0)));
  }
  m_transferFunc4.get().reset(0.0, 1.0, glm::col4(0, 0, 0, 0), glm::col4(255, 255, 255, 255));
#endif
}

void Z3DVolumeRaycasterRenderer::translate(double dx, double dy, double dz)
{
  for (auto vol : m_volumes) {
    if (vol) {
      vol->translate(dx, dy, dz);
    }
  }
}

bool Z3DVolumeRaycasterRenderer::hasVisibleRendering() const
{
  for (size_t i = 0; i < m_volumes.size(); ++i) {
    if (m_channelVisibleParas[i]->get()) {
      return true;
    }
  }
  return false;
}
