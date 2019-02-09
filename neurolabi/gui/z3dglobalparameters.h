#ifndef Z3DGLOBALPARAMETERS_H
#define Z3DGLOBALPARAMETERS_H

#include "widgets/znumericparameter.h"
#include "z3dcameraparameter.h"
#include "z3dpickingmanager.h"
#include "widgets/zoptionparameter.h"
#include "z3dinteractionhandler.h"
#include <vector>

class Z3DCanvas;

class ZWidgetsGroup;

class Z3DGlobalParameters : public QObject
{
Q_OBJECT
public:
  Z3DGlobalParameters(Z3DCanvas& canvas);

  const std::vector<ZParameter*>& parameters() const
  { return m_parameters; }

  std::shared_ptr<ZWidgetsGroup> widgetsGroup(bool includeCamera);

  // count is lightCount
  const glm::vec4* lightPositionArray() const
  { return m_lightPositionArray.data(); }

  const glm::vec4* lightAmbientArray() const
  { return m_lightAmbientArray.data(); }

  const glm::vec4* lightDiffuseArray() const
  { return m_lightDiffuseArray.data(); }

  const glm::vec4* lightSpecularArray() const
  { return m_lightSpecularArray.data(); }

  const glm::vec3* lightAttenuationArray() const
  { return m_lightAttenuationArray.data(); }

  const float* lightSpotCutoffArray() const
  { return m_lightSpotCutoffArray.data(); }

  const float* lightSpotExponentArray() const
  { return m_lightSpotExponentArray.data(); }

  const glm::vec3* lightSpotDirectionArray() const
  { return m_lightSpotDirectionArray.data(); }

  // must call
  void setPickingTarget(Z3DRenderTarget& rt)
  { pickingManager.setRenderTarget(rt); }

private:
  void updateLightsArray();

  void addParameter(ZParameter& para)
  { m_parameters.push_back(&para); }

private slots:
  void updateDevicePixelRatio();

public:
  ZStringIntOptionParameter geometriesMultisampleMode;
  ZStringIntOptionParameter transparencyMethod;
  ZFloatParameter weightedBlendedDepthScale;
  ZIntParameter lightCount;
  std::vector<std::unique_ptr<ZVec4Parameter>> lightPositions;
  std::vector<std::unique_ptr<ZVec4Parameter>> lightAmbients;
  std::vector<std::unique_ptr<ZVec4Parameter>> lightDiffuses;
  std::vector<std::unique_ptr<ZVec4Parameter>> lightSpeculars;
  // The light source's attenuation factors (x = constant, y = linear, z = quadratic)
  std::vector<std::unique_ptr<ZVec3Parameter>> lightAttenuations;
  std::vector<std::unique_ptr<ZFloatParameter>> lightSpotCutoff;
  std::vector<std::unique_ptr<ZFloatParameter>> lightSpotExponent;
  std::vector<std::unique_ptr<ZVec3Parameter>> lightSpotDirection;
  ZVec4Parameter sceneAmbient;

  // fog
  ZStringIntOptionParameter fogMode;
  ZVec3Parameter fogTopColor;
  ZVec3Parameter fogBottomColor;
  ZIntSpanParameter fogRange;
  ZFloatParameter fogDensity;

  Z3DCameraParameter camera;
  Z3DPickingManager pickingManager;
  // must add to network
  Z3DTrackballInteractionHandler interactionHandler;

private:
  std::vector<ZParameter*> m_parameters;

  std::shared_ptr<ZWidgetsGroup> m_widgetsGrp;
  std::shared_ptr<ZWidgetsGroup> m_widgetsGrpNoCamera;

  std::vector<glm::vec4> m_lightPositionArray;
  std::vector<glm::vec4> m_lightAmbientArray;
  std::vector<glm::vec4> m_lightDiffuseArray;
  std::vector<glm::vec4> m_lightSpecularArray;
  std::vector<glm::vec3> m_lightAttenuationArray;
  std::vector<float> m_lightSpotCutoffArray;
  std::vector<float> m_lightSpotExponentArray;
  std::vector<glm::vec3> m_lightSpotDirectionArray;

  Z3DCanvas& m_canvas;
};

#endif // Z3DGLOBALPARAMETERS_H
