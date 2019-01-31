#include "z3dglobalparameters.h"

#include <QWindow>

#include "widgets/zwidgetsgroup.h"
#include "z3dcanvas.h"
#include "z3dgpuinfo.h"

Z3DGlobalParameters::Z3DGlobalParameters(Z3DCanvas& canvas)
  : geometriesMultisampleMode("Multisample Anti-Aliasing")
  , transparencyMethod("Transparency")
  , weightedBlendedDepthScale("Weighted Blended Depth Scale", 1.f, 1e-3f, 1e3f)
  , lightCount("Light Count", 5, 1, 5)
  , sceneAmbient("Scene Ambient", glm::vec4(0.2f, 0.2f, 0.2f, 1.f))
  , fogMode("Fog Mode")
  , fogTopColor("Fog Top Color", glm::vec3(.9f, .9f, .9f))
  , fogBottomColor("Fog Bottom Color", glm::vec3(.9f, .9f, .9f))
  , fogRange("Fog Range", glm::ivec2(100, 50000), 1, 1e5)
  , fogDensity("Fog Denstiy", 1.f, 0.0001f, 10.f)
  , camera("Camera", Z3DCamera())
  , pickingManager()
  , interactionHandler("Interaction Handler", &camera)
  , m_canvas(canvas)
{
  geometriesMultisampleMode.addOptions("None", "2x2");
  geometriesMultisampleMode.select("2x2");
  addParameter(geometriesMultisampleMode);

  transparencyMethod.addOption("Blend Delayed");
  transparencyMethod.addOption("Blend No Depth Mask");
  transparencyMethod.select("Blend Delayed");

  if (Z3DGpuInfo::instance().isWeightedAverageSupported()) {
    transparencyMethod.addOption("Weighted Average");
    transparencyMethod.select("Weighted Average");
  }

  if (Z3DGpuInfo::instance().isWeightedBlendedSupported()) {
    transparencyMethod.addOption("Weighted Blended");
  }

  if (Z3DGpuInfo::instance().isDualDepthPeelingSupported()) {
    transparencyMethod.addOption("Dual Depth Peeling");
  }
  //weightedBlendedDepthScale.setStyle("SPINBOX");

  //  if (Z3DGpuInfoInstance.isLinkedListSupported())
  //    m_transparencyMethod.addOption("Linked List");

  addParameter(transparencyMethod);
  addParameter(weightedBlendedDepthScale);

  addParameter(camera);

  // lights
  QString lightname = "Key Light";
  QString name = lightname + " Position";
  lightPositions.emplace_back(
    std::make_unique<ZVec4Parameter>(name, glm::vec4(0.1116f, 0.7660f, 0.6330f, 0.0f), glm::vec4(-1.0f),
                                     glm::vec4(1.f)));
  name = lightname + " Ambient";
  lightAmbients.emplace_back(std::make_unique<ZVec4Parameter>(name, glm::vec4(0.1f, 0.1f, 0.1f, 1.0f)));
  name = lightname + " Diffuse";
  lightDiffuses.emplace_back(std::make_unique<ZVec4Parameter>(name, glm::vec4(0.75f, 0.75f, 0.75f, 1.0f)));
  name = lightname + " Specular";
  lightSpeculars.emplace_back(std::make_unique<ZVec4Parameter>(name, glm::vec4(0.85f, 0.85f, 0.85f, 1.0f)));
  name = lightname + " Attenuation";
  lightAttenuations.emplace_back(
    std::make_unique<ZVec3Parameter>(name, glm::vec3(1.f, 0.f, 0.f), glm::vec3(0.f), glm::vec3(100.f)));
  name = lightname + " Spot Cutoff";
  lightSpotCutoff.emplace_back(std::make_unique<ZFloatParameter>(name, 180.f, 0.f, 180.f));
  name = lightname + " Spot Exponent";
  lightSpotExponent.emplace_back(std::make_unique<ZFloatParameter>(name, 1.f, 0.f, 50.f));
  name = lightname + " Spot Direction";
  lightSpotDirection.emplace_back(
    std::make_unique<ZVec3Parameter>(name, glm::vec3(-0.1116f, -0.7660f, -0.6330f), glm::vec3(-1.f), glm::vec3(1.f)));

  lightname = "Head Light";
  name = lightname + " Position";
  lightPositions.emplace_back(
    std::make_unique<ZVec4Parameter>(name, glm::vec4(0.f, 0.f, 1.f, 0.0f), glm::vec4(-1.0f), glm::vec4(1.f)));
  name = lightname + " Ambient";
  lightAmbients.emplace_back(
    std::make_unique<ZVec4Parameter>(name, glm::vec4(0.1 * 0.333f, 0.1 * 0.333f, 0.1 * 0.333f, 1.0f)));
  name = lightname + " Diffuse";
  lightDiffuses.emplace_back(
    std::make_unique<ZVec4Parameter>(name, glm::vec4(0.75 * 0.333f, 0.75 * 0.333f, 0.75 * 0.333f, 1.0f)));
  name = lightname + " Specular";
  lightSpeculars.emplace_back(std::make_unique<ZVec4Parameter>(name, glm::vec4(0.f, 0.f, 0.f, 1.0f)));
  name = lightname + " Attenuation";
  lightAttenuations.emplace_back(
    std::make_unique<ZVec3Parameter>(name, glm::vec3(1.f, 0.f, 0.f), glm::vec3(0.f), glm::vec3(100.f)));
  name = lightname + " Spot Cutoff";
  lightSpotCutoff.emplace_back(std::make_unique<ZFloatParameter>(name, 180.f, 0.f, 180.f));
  name = lightname + " Spot Exponent";
  lightSpotExponent.emplace_back(std::make_unique<ZFloatParameter>(name, 1.f, 0.f, 50.f));
  name = lightname + " Spot Direction";
  lightSpotDirection.emplace_back(
    std::make_unique<ZVec3Parameter>(name, glm::vec3(0.f, 0.f, -1.f), glm::vec3(-1.f), glm::vec3(1.f)));

  lightname = "Fill Light";
  name = lightname + " Position";
  lightPositions.emplace_back(
    std::make_unique<ZVec4Parameter>(name, glm::vec4(-0.0449f, -0.9659f, 0.2549f, 0.0f), glm::vec4(-1.0f),
                                     glm::vec4(1.f)));
  name = lightname + " Ambient";
  lightAmbients.emplace_back(
    std::make_unique<ZVec4Parameter>(name, glm::vec4(0.1 * 0.333f, 0.1 * 0.333f, 0.1 * 0.333f, 1.0f)));
  name = lightname + " Diffuse";
  lightDiffuses.emplace_back(
    std::make_unique<ZVec4Parameter>(name, glm::vec4(0.75 * 0.333f, 0.75 * 0.333f, 0.75 * 0.333f, 1.0f)));
  name = lightname + " Specular";
  lightSpeculars.emplace_back(
    std::make_unique<ZVec4Parameter>(name, glm::vec4(0.85 * 0.333f, 0.85 * 0.333f, 0.85 * 0.333f, 1.0f)));
  name = lightname + " Attenuation";
  lightAttenuations.emplace_back(
    std::make_unique<ZVec3Parameter>(name, glm::vec3(1.f, 0.f, 0.f), glm::vec3(0.f), glm::vec3(100.f)));
  name = lightname + " Spot Cutoff";
  lightSpotCutoff.emplace_back(std::make_unique<ZFloatParameter>(name, 180.f, 0.f, 180.f));
  name = lightname + " Spot Exponent";
  lightSpotExponent.emplace_back(std::make_unique<ZFloatParameter>(name, 1.f, 0.f, 50.f));
  name = lightname + " Spot Direction";
  lightSpotDirection.emplace_back(
    std::make_unique<ZVec3Parameter>(name, glm::vec3(0.0449f, 0.9659f, -0.2549f), glm::vec3(-1.f), glm::vec3(1.f)));

  lightname = "Back Light 1";
  name = lightname + " Position";
  lightPositions.emplace_back(
    std::make_unique<ZVec4Parameter>(name, glm::vec4(0.9397f, 0.f, -0.3420f, 0.0f), glm::vec4(-1.0f), glm::vec4(1.f)));
  name = lightname + " Ambient";
  lightAmbients.emplace_back(
    std::make_unique<ZVec4Parameter>(name, glm::vec4(0.1 * 0.27f, 0.1 * 0.27f, 0.1 * 0.27f, 1.0f)));
  name = lightname + " Diffuse";
  lightDiffuses.emplace_back(
    std::make_unique<ZVec4Parameter>(name, glm::vec4(0.75 * 0.27f, 0.75 * 0.27f, 0.75 * 0.27f, 1.0f)));
  name = lightname + " Specular";
  lightSpeculars.emplace_back(
    std::make_unique<ZVec4Parameter>(name, glm::vec4(0.85 * 0.27f, 0.85 * 0.27f, 0.85 * 0.27f, 1.0f)));
  name = lightname + " Attenuation";
  lightAttenuations.emplace_back(
    std::make_unique<ZVec3Parameter>(name, glm::vec3(1.f, 0.f, 0.f), glm::vec3(0.f), glm::vec3(100.f)));
  name = lightname + " Spot Cutoff";
  lightSpotCutoff.emplace_back(std::make_unique<ZFloatParameter>(name, 180.f, 0.f, 180.f));
  name = lightname + " Spot Exponent";
  lightSpotExponent.emplace_back(std::make_unique<ZFloatParameter>(name, 1.f, 0.f, 50.f));
  name = lightname + " Spot Direction";
  lightSpotDirection.emplace_back(
    std::make_unique<ZVec3Parameter>(name, glm::vec3(-0.9397f, 0.f, 0.3420f), glm::vec3(-1.f), glm::vec3(1.f)));

  lightname = "Back Light 2";
  name = lightname + " Position";
  lightPositions.emplace_back(
    std::make_unique<ZVec4Parameter>(name, glm::vec4(-0.9397f, 0.f, -0.3420f, 0.0f), glm::vec4(-1.0f), glm::vec4(1.f)));
  name = lightname + " Ambient";
  lightAmbients.emplace_back(
    std::make_unique<ZVec4Parameter>(name, glm::vec4(0.1 * 0.27f, 0.1 * 0.27f, 0.1 * 0.27f, 1.0f)));
  name = lightname + " Diffuse";
  lightDiffuses.emplace_back(
    std::make_unique<ZVec4Parameter>(name, glm::vec4(0.75 * 0.27f, 0.75 * 0.27f, 0.75 * 0.27f, 1.0f)));
  name = lightname + " Specular";
  lightSpeculars.emplace_back(
    std::make_unique<ZVec4Parameter>(name, glm::vec4(0.85 * 0.27f, 0.85 * 0.27f, 0.85 * 0.27f, 1.0f)));
  name = lightname + " Attenuation";
  lightAttenuations.emplace_back(
    std::make_unique<ZVec3Parameter>(name, glm::vec3(1.f, 0.f, 0.f), glm::vec3(0.f), glm::vec3(100.f)));
  name = lightname + " Spot Cutoff";
  lightSpotCutoff.emplace_back(std::make_unique<ZFloatParameter>(name, 180.f, 0.f, 180.f));
  name = lightname + " Spot Exponent";
  lightSpotExponent.emplace_back(std::make_unique<ZFloatParameter>(name, 1.f, 0.f, 50.f));
  name = lightname + " Spot Direction";
  lightSpotDirection.emplace_back(
    std::make_unique<ZVec3Parameter>(name, glm::vec3(0.9397f, 0.f, 0.3420f), glm::vec3(-1.f), glm::vec3(1.f)));

  addParameter(lightCount);

  m_lightPositionArray.resize(lightPositions.size());
  m_lightAmbientArray.resize(lightPositions.size());
  m_lightDiffuseArray.resize(lightPositions.size());
  m_lightSpecularArray.resize(lightPositions.size());
  m_lightAttenuationArray.resize(lightPositions.size());
  m_lightSpotCutoffArray.resize(lightPositions.size());
  m_lightSpotExponentArray.resize(lightPositions.size());
  m_lightSpotDirectionArray.resize(lightPositions.size());
  updateLightsArray();

  for (size_t i = 0; i < lightPositions.size(); ++i) {
    lightAmbients[i]->setStyle("COLOR");
    lightDiffuses[i]->setStyle("COLOR");
    lightSpeculars[i]->setStyle("COLOR");
    addParameter(*lightPositions[i]);
    connect(lightPositions[i].get(), &ZVec4Parameter::valueChanged, this, &Z3DGlobalParameters::updateLightsArray);
    addParameter(*lightAmbients[i]);
    connect(lightAmbients[i].get(), &ZVec4Parameter::valueChanged, this, &Z3DGlobalParameters::updateLightsArray);
    addParameter(*lightDiffuses[i]);
    connect(lightDiffuses[i].get(), &ZVec4Parameter::valueChanged, this, &Z3DGlobalParameters::updateLightsArray);
    addParameter(*lightSpeculars[i]);
    connect(lightSpeculars[i].get(), &ZVec4Parameter::valueChanged, this, &Z3DGlobalParameters::updateLightsArray);
    addParameter(*lightAttenuations[i]);
    connect(lightAttenuations[i].get(), &ZVec3Parameter::valueChanged, this, &Z3DGlobalParameters::updateLightsArray);
    addParameter(*lightSpotCutoff[i]);
    connect(lightSpotCutoff[i].get(), &ZFloatParameter::valueChanged, this, &Z3DGlobalParameters::updateLightsArray);
    addParameter(*lightSpotExponent[i]);
    connect(lightSpotExponent[i].get(), &ZFloatParameter::valueChanged, this, &Z3DGlobalParameters::updateLightsArray);
    addParameter(*lightSpotDirection[i]);
    connect(lightSpotDirection[i].get(), &ZVec3Parameter::valueChanged, this, &Z3DGlobalParameters::updateLightsArray);
  }

  sceneAmbient.setStyle("COLOR");
  addParameter(sceneAmbient);

  // fog
  fogMode.addOptions("None", "Linear", "Exponential", "Squared Exponential");
  fogMode.select("None");
  addParameter(fogMode);
  fogTopColor.setStyle("COLOR");
  fogBottomColor.setStyle("COLOR");
  fogRange.setSingleStep(1);
  fogDensity.setSingleStep(0.00001);
  fogDensity.setDecimal(5);
  addParameter(fogTopColor);
  addParameter(fogBottomColor);
  addParameter(fogRange);
  addParameter(fogDensity);

  m_widgetsGrp = std::make_shared<ZWidgetsGroup>("Global", 1);
  m_widgetsGrp->addChild(geometriesMultisampleMode, 1);
  m_widgetsGrp->addChild(transparencyMethod, 1);
  m_widgetsGrp->addChild(weightedBlendedDepthScale, 1);
  m_widgetsGrp->addChild(camera, 1);
  m_widgetsGrpNoCamera = std::make_shared<ZWidgetsGroup>("Lighting", 1);
  m_widgetsGrpNoCamera->addChild(geometriesMultisampleMode, 1);
  m_widgetsGrpNoCamera->addChild(transparencyMethod, 1);
  m_widgetsGrpNoCamera->addChild(weightedBlendedDepthScale, 1);

  for (size_t i = 4; i < m_parameters.size(); ++i) {
    m_widgetsGrp->addChild(*m_parameters[i], 1);
    m_widgetsGrpNoCamera->addChild(*m_parameters[i], 1);
  }

  updateDevicePixelRatio();
  connect(m_canvas.parentWidget()->windowHandle(), SIGNAL(screenChanged(QScreen*)),
          this, SLOT(updateDevicePixelRatio()));
  connect(&canvas, SIGNAL(canvasSizeChanged(size_t,size_t)),
          this, SLOT(updateDevicePixelRatio()));
}

std::shared_ptr<ZWidgetsGroup> Z3DGlobalParameters::widgetsGroup(bool includeCamera)
{
  return includeCamera ? m_widgetsGrp : m_widgetsGrpNoCamera;
}

void Z3DGlobalParameters::updateDevicePixelRatio()
{
  pickingManager.setDevicePixelRatio(m_canvas.devicePixelRatioF());
}

void Z3DGlobalParameters::updateLightsArray()
{
  for (size_t i = 0; i < lightPositions.size(); ++i) {
    m_lightPositionArray[i] = lightPositions[i]->get();
    m_lightAmbientArray[i] = lightAmbients[i]->get();
    m_lightDiffuseArray[i] = lightDiffuses[i]->get();
    m_lightSpecularArray[i] = lightSpeculars[i]->get();
    m_lightAttenuationArray[i] = lightAttenuations[i]->get();
    m_lightSpotCutoffArray[i] = lightSpotCutoff[i]->get();
    m_lightSpotExponentArray[i] = lightSpotExponent[i]->get();
    m_lightSpotDirectionArray[i] = lightSpotDirection[i]->get();
  }
}
