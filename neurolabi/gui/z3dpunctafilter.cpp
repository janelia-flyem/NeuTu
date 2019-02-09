#include "z3dpunctafilter.h"

#include <algorithm>
#include <iostream>

#include "neutubeconfig.h"
#include "zrandom.h"
#include "zpunctumcolorscheme.h"
#include "z3dfiltersetting.h"
#include "zjsonparser.h"

Z3DPunctaFilter::Z3DPunctaFilter(Z3DGlobalParameters& globalParas, QObject* parent)
  : Z3DGeometryFilter(globalParas, parent)
  , m_monoEyeOutport("Image", this)
  , m_leftEyeOutport("LeftEyeImage", this)
  , m_rightEyeOutport("RightEyeImage", this)
  , m_monoEyeOutport2("Image2", this)
  , m_leftEyeOutport2("LeftEyeImage2", this)
  , m_rightEyeOutport2("RightEyeImage2", this)
  , m_sphereRenderer(m_rendererBase)
  , m_colorMode("Color Mode")
  , m_singleColorForAllPuncta("Puncta Color", glm::vec4(ZRandom::instance().randReal<float>(),
                                                        ZRandom::instance().randReal<float>(),
                                                        ZRandom::instance().randReal<float>(),
                                                        1.f))
  , m_colorMapScore("Score Color Map", -1., 1., QColor(255, 255, 0), QColor(0, 0, 255))
  , m_colorMapMeanIntensity("Mean Intensity Color Map", 0., 1., QColor(255, 0, 0), QColor(0, 0, 0))
  , m_colorMapMaxIntensity("Max Intensity Color Map", 0., 1., QColor(255, 0, 0), QColor(0, 0, 0))
  , m_useSameSizeForAllPuncta("Use Same Size", false)
  //  , m_glowSphereRenderer(m_rendererBase)
  //  , m_textureGlowRenderer(m_rendererBase)
  //  , m_randomGlow("Random Glow", false)
  //  , m_glowPercentage("Glow Percentage", 0.2f, 0.f, 1.f)
  //  , m_textureCopyRenderer(m_rendererBase)
  , m_selectPunctumEvent("Select Puncta", false)
{
  addPrivateRenderPort(m_monoEyeOutport);
  addPrivateRenderPort(m_leftEyeOutport);
  addPrivateRenderPort(m_rightEyeOutport);
  addPrivateRenderPort(m_monoEyeOutport2);
  addPrivateRenderPort(m_leftEyeOutport2);
  addPrivateRenderPort(m_rightEyeOutport2);

  //m_textureCopyRenderer.setDiscardTransparent(true);

  m_singleColorForAllPuncta.setStyle("COLOR");
  connect(&m_singleColorForAllPuncta, &ZVec4Parameter::valueChanged,
          this, &Z3DPunctaFilter::prepareColor);
  connect(&m_colorMapScore, &ZColorMapParameter::valueChanged, this,
          &Z3DPunctaFilter::prepareColor);
  connect(&m_colorMapMeanIntensity, &ZColorMapParameter::valueChanged,
          this, &Z3DPunctaFilter::prepareColor);
  connect(&m_colorMapMaxIntensity, &ZColorMapParameter::valueChanged,
          this, &Z3DPunctaFilter::prepareColor);

  // Color Mode
  m_colorMode.addOptions("Single Color", "Random Color", "Point Source",
                         "Original Point Color", "Colormap Score");
  m_colorMode.select("Point Source");
  m_currentColorMode = m_colorMode.get();

//  connect(&m_colorMode, &ZStringIntOptionParameter::valueChanged,
//          this, &Z3DPunctaFilter::prepareColor);
  connect(&m_colorMode, &ZStringIntOptionParameter::valueChanged,
          this, &Z3DPunctaFilter::updateColorMode);

  connect(&m_useSameSizeForAllPuncta, &ZBoolParameter::valueChanged,
          this, &Z3DPunctaFilter::changePunctaSize);

  addParameter(m_colorMode);

  addParameter(m_singleColorForAllPuncta);
  addParameter(m_colorMapScore);
  addParameter(m_colorMapMeanIntensity);
  addParameter(m_colorMapMaxIntensity);

  addParameter(m_useSameSizeForAllPuncta);

  addParameter(m_sphereRenderer.useDynamicMaterialPara());

  m_selectPunctumEvent.listenTo("select punctum", Qt::LeftButton, Qt::NoModifier, QEvent::MouseButtonPress);
  m_selectPunctumEvent.listenTo("select punctum", Qt::LeftButton, Qt::NoModifier, QEvent::MouseButtonRelease);
  m_selectPunctumEvent.listenTo("select punctum", Qt::LeftButton,
                                Qt::NoModifier, QEvent::MouseButtonDblClick);
  m_selectPunctumEvent.listenTo("select punctum", Qt::LeftButton,
                                Qt::ControlModifier, QEvent::MouseButtonDblClick);
  m_selectPunctumEvent.listenTo("append select punctum", Qt::LeftButton, Qt::ControlModifier, QEvent::MouseButtonPress);
  m_selectPunctumEvent.listenTo("append select punctum", Qt::LeftButton, Qt::ControlModifier,
                                QEvent::MouseButtonRelease);
  connect(&m_selectPunctumEvent, &ZEventListenerParameter::mouseEventTriggered, this, &Z3DPunctaFilter::selectPuncta);
  addEventListener(m_selectPunctumEvent);

  adjustWidgets();
}

void Z3DPunctaFilter::process(Z3DEye)
{
  if (m_dataIsInvalid) {
    prepareData();
  }
}

void Z3DPunctaFilter::setData(const std::vector<ZPunctum*>& punctaList)
{
  ZOUT(LTRACE(), 5) << "Set puncta:" << punctaList.size();

  m_origPunctaList = punctaList;
  updateData();
}

void Z3DPunctaFilter::setData(const QList<ZPunctum *> &punctaList)
{
  ZOUT(LTRACE(), 5) << "Set puncta:" << punctaList.size();

  m_origPunctaList.clear();
  m_origPunctaList.insert(m_origPunctaList.end(), punctaList.begin(),
                          punctaList.end());
  updateData();
}

bool Z3DPunctaFilter::isReady(Z3DEye eye) const
{
  return Z3DGeometryFilter::isReady(eye) && isVisible() && !m_origPunctaList.empty();
}

std::shared_ptr<ZWidgetsGroup> Z3DPunctaFilter::widgetsGroup()
{
  if (!m_widgetsGroup) {
    m_widgetsGroup = std::make_shared<ZWidgetsGroup>("Puncta", 1);
    m_widgetsGroup->addChild(m_visible, 1);
    m_widgetsGroup->addChild(m_stayOnTop, 1);
    m_widgetsGroup->addChild(m_colorMode, 1);
    m_widgetsGroup->addChild(m_singleColorForAllPuncta, 1);
    m_widgetsGroup->addChild(m_colorMapScore, 1);
    m_widgetsGroup->addChild(m_colorMapMeanIntensity, 1);
    m_widgetsGroup->addChild(m_colorMapMaxIntensity, 1);

    for (const auto& kv : m_sourceColorMapper) {
      m_widgetsGroup->addChild(*kv.second, 2);
    }

    m_widgetsGroup->addChild(m_useSameSizeForAllPuncta, 3);
    m_widgetsGroup->addChild(m_sphereRenderer.useDynamicMaterialPara(), 7);

    const std::vector<ZParameter*>& paras = m_rendererBase.parameters();
    for (auto para : paras) {
      if (para->name() == "Coord Transform")
        m_widgetsGroup->addChild(*para, 3);
      else if (para->name() == "Size Scale")
        m_widgetsGroup->addChild(*para, 3);
      else if (para->name() == "Rendering Method")
        m_widgetsGroup->addChild(*para, 4);
      else if (para->name() == "Opacity")
        m_widgetsGroup->addChild(*para, 5);
      else
        m_widgetsGroup->addChild(*para, 7);
    }

    m_widgetsGroup->addChild(m_xCut, 5);
    m_widgetsGroup->addChild(m_yCut, 5);
    m_widgetsGroup->addChild(m_zCut, 5);
    m_widgetsGroup->addChild(m_boundBoxMode, 5);
    m_widgetsGroup->addChild(m_boundBoxLineWidth, 5);
    m_widgetsGroup->addChild(m_boundBoxLineColor, 5);
    m_widgetsGroup->addChild(m_selectionLineWidth, 7);
    m_widgetsGroup->addChild(m_selectionLineColor, 7);
    //m_widgetsGroup->setBasicAdvancedCutoff(5);
  }
  return m_widgetsGroup;
}

void Z3DPunctaFilter::renderOpaque(Z3DEye eye)
{
  m_rendererBase.render(eye, m_sphereRenderer);
  renderBoundBox(eye);
}

void Z3DPunctaFilter::renderTransparent(Z3DEye eye)
{
  m_rendererBase.render(eye, m_sphereRenderer);
  renderBoundBox(eye);
}

void Z3DPunctaFilter::configure(const ZJsonObject &obj)
{
  Z3DGeometryFilter::configure(obj);
  if (obj.hasKey(Z3DFilterSetting::COLOR_MODE_KEY)) {
    setColorMode(ZJsonParser::stringValue(obj[Z3DFilterSetting::COLOR_MODE_KEY]));
  }

  if (obj.hasKey(Z3DFilterSetting::VISIBLE_KEY)) {
    setVisible(ZJsonParser::booleanValue(obj[Z3DFilterSetting::VISIBLE_KEY]));
  }
}

void Z3DPunctaFilter::renderPicking(Z3DEye eye)
{
  if (!m_pickingObjectsRegistered)
      registerPickingObjects();
  m_rendererBase.renderPicking(eye, m_sphereRenderer);
}

void Z3DPunctaFilter::registerPickingObjects()
{
  if (!m_pickingObjectsRegistered) {
    for (auto punctum : m_punctaList) {
      pickingManager().registerObject(punctum);
    }
    m_registeredPunctaList = m_punctaList;
    m_pointPickingColors.clear();
    for (auto punctum : m_punctaList) {
      glm::col4 pickingColor = pickingManager().colorOfObject(punctum);
      glm::vec4 fPickingColor(pickingColor[0] / 255.f, pickingColor[1] / 255.f, pickingColor[2] / 255.f,
          pickingColor[3] / 255.f);
      m_pointPickingColors.push_back(fPickingColor);
    }
    m_sphereRenderer.setDataPickingColors(&m_pointPickingColors);
  }

  m_pickingObjectsRegistered = true;
}

void Z3DPunctaFilter::deregisterPickingObjects()
{
  if (m_pickingObjectsRegistered) {
    for (auto punctum : m_registeredPunctaList) {
      pickingManager().deregisterObject(punctum);
    }
    m_registeredPunctaList.clear();
  }

  m_pickingObjectsRegistered = false;
}

void Z3DPunctaFilter::prepareData()
{
  if (!m_dataIsInvalid)
    return;

  deregisterPickingObjects();

  // convert puncta to format that glsl can use
  m_specularAndShininess.clear();
  m_pointAndRadius.clear();
  for (auto punctum : m_punctaList) {
    if (m_useSameSizeForAllPuncta.get())
      m_pointAndRadius.emplace_back(punctum->x(), punctum->y(), punctum->z(), 2.f);
    else
      m_pointAndRadius.emplace_back(punctum->x(), punctum->y(), punctum->z(),
                                    punctum->radius());
    m_specularAndShininess.emplace_back(punctum->maxIntensity() / 255.f,
                                        punctum->maxIntensity() / 255.f,
                                        punctum->maxIntensity() / 255.f,
                                        punctum->maxIntensity() / 2.f);
  }

  initializeCutRange();
  initializeRotationCenter();

  ZPunctumColorScheme colorScheme;
  colorScheme.setColorScheme(ZColorScheme::PUNCTUM_TYPE_COLOR);

  std::map<QString, int, QStringNaturalCompare> sourceTypeMapper;
  for (ZPunctum *punctum : m_origPunctaList) {
    sourceTypeMapper[punctum->getSource().c_str()] = punctum->getTypeFromSource();
  }
  // do nothing if sources don't change
  if (m_sourceColorMapper.size() != sourceTypeMapper.size() ||
      !std::equal(m_sourceColorMapper.begin(), m_sourceColorMapper.end(),
                  sourceTypeMapper.begin(), _KeyEqual())) {
    // remove old source color parameters from widget, will add new ones later
    if (m_widgetsGroup) {
      for (auto& kv : m_sourceColorMapper) {
        m_widgetsGroup->removeChild(*kv.second);
      }
    }

    // remove not in use sources
    for (auto it = m_sourceColorMapper.begin(); it != m_sourceColorMapper.end(); ) {
      if (sourceTypeMapper.find(it->first) == sourceTypeMapper.end()) {
        removeParameter(*it->second);
        it = m_sourceColorMapper.erase(it);
      } else {
        ++it;
      }
    }

    // create color parameters for new sources
    std::map<QString, int, QStringNaturalCompare> newSources;
    std::set_difference(sourceTypeMapper.begin(), sourceTypeMapper.end(),
                        m_sourceColorMapper.begin(), m_sourceColorMapper.end(),
                        std::inserter(newSources, newSources.end()),
                        QStringKeyNaturalLess());
#ifdef _DEBUG_
    std::cout << "sourceTypeMapper" << std::endl;
    for (std::map<QString, int, QStringNaturalCompare>::const_iterator iter = sourceTypeMapper.begin();
         iter != sourceTypeMapper.end(); ++iter) {
      qDebug() << " " << iter->first << iter->second;
    }
    std::cout << "m_sourceColorMapper" << std::endl;
    for (std::map<QString, std::unique_ptr<ZVec4Parameter>, QStringNaturalCompare>::const_iterator iter = m_sourceColorMapper.begin();
         iter != m_sourceColorMapper.end(); ++iter) {
      qDebug() << " " << iter->first;
    }
    std::cout << "newSources" << std::endl;
    for ( std::map<QString, int, QStringNaturalCompare>::const_iterator iter = newSources.begin();
         iter != newSources.end(); ++iter) {
      qDebug() << " " << iter->first << iter->second;
    }
#endif

    for (const auto& kv : newSources) {
      QString guiname = QString("Source: %1").arg(kv.first);

      if (m_sourceColorMapper.count(kv.first) > 0) {
        LOG(FATAL) << "Parameter check failed:" << kv.first;
      }

      if (kv.second >= 0) {
        QColor color = colorScheme.getColor(kv.second);
        m_sourceColorMapper.insert(
              std::make_pair(kv.first,
                             std::make_unique<ZVec4Parameter>(
                               guiname,
                               glm::vec4(color.redF(),
                                         color.greenF(),
                                         color.blueF(),
                                         color.alphaF()))));
      } else {
        m_sourceColorMapper.insert(
              std::make_pair(kv.first,
                             std::make_unique<ZVec4Parameter>(
                               guiname,
                               glm::vec4(ZRandom::instance().randReal<float>(),
                                         ZRandom::instance().randReal<float>(),
                                         ZRandom::instance().randReal<float>(),
                                         1.f))));
      }
      m_sourceColorMapper[kv.first]->setStyle("COLOR");
      connect(m_sourceColorMapper[kv.first].get(), &ZVec4Parameter::valueChanged,
              this, &Z3DPunctaFilter::prepareColor);
      addParameter(*m_sourceColorMapper[kv.first]);
    }

    // update widget group
    if (m_widgetsGroup) {
      for (const auto& kv : m_sourceColorMapper) {
        m_widgetsGroup->addChild(*kv.second, 2);
      }
      m_widgetUpdateToDate = false;
//      m_widgetsGroup->emitWidgetsGroupChangedSignal();
    }
  }

  m_sphereRenderer.setData(&m_pointAndRadius, &m_specularAndShininess);
  prepareColor();
  adjustWidgets();
  m_dataIsInvalid = false;
}

void Z3DPunctaFilter::punctumBound(const ZPunctum& p, ZBBox<glm::dvec3>& result) const
{
  double radius = p.radius() * m_rendererBase.sizeScale();
  if (m_useSameSizeForAllPuncta.get())
    radius = 2.0 * m_rendererBase.sizeScale();
  glm::dvec3 cent = glm::dvec3(glm::applyMatrix(coordTransform(), glm::vec3(p.x(), p.y(), p.z())));
  result.setMinCorner(cent - radius);
  result.setMaxCorner(cent + radius);
}

void Z3DPunctaFilter::notTransformedPunctumBound(const ZPunctum& p, ZBBox<glm::dvec3>& result) const
{
  double radius = p.radius() * m_rendererBase.sizeScale();
  if (m_useSameSizeForAllPuncta.get())
    radius = 2.0 * m_rendererBase.sizeScale();
  glm::dvec3 cent(p.x(), p.y(), p.z());
  result.setMinCorner(cent - radius);
  result.setMaxCorner(cent + radius);
}

void Z3DPunctaFilter::updateNotTransformedBoundBoxImpl()
{
  m_notTransformedBoundBox.reset();
  ZBBox<glm::dvec3> boundBox;
  for (const auto& p : m_origPunctaList) {
    notTransformedPunctumBound(*p, boundBox);
    m_notTransformedBoundBox.expand(boundBox);
  }
}

void Z3DPunctaFilter::addSelectionLines()
{
  ZBBox<glm::dvec3> boundBox;
  for (const auto& p : m_punctaList) {
    if (p->isVisible() && p->isSelected()) {
      punctumBound(*p, boundBox);
      appendBoundboxLines(boundBox, m_selectionLines);
    }
  }
}

void Z3DPunctaFilter::prepareColor()
{
  m_pointColors.clear();

  if (m_colorMode.isSelected("Original Point Color")) {
    for (size_t i=0; i<m_punctaList.size(); i++) {
      glm::vec4 color(m_punctaList[i]->color().redF(), m_punctaList[i]->color().greenF(), m_punctaList[i]->color().blueF(), m_punctaList[i]->color().alphaF());
      m_pointColors.push_back(color);
    }
  } else if (m_colorMode.isSelected("Point Source")) {
    for (size_t i=0; i<m_punctaList.size(); i++) {
      glm::vec4 color = m_sourceColorMapper[m_punctaList[i]->getSource().c_str()]->get();
      m_pointColors.push_back(color);
    }
  } else if (m_colorMode.isSelected("Random Color")) {
    for (size_t i = 0; i < m_punctaList.size(); ++i) {
      glm::vec4 color(ZRandom::instance().randReal<float>(),
                      ZRandom::instance().randReal<float>(),
                      ZRandom::instance().randReal<float>(),
                      1.0f);
      m_pointColors.push_back(color);
    }
  } else if (m_colorMode.isSelected("Single Color")) {
    for (size_t i=0; i<m_punctaList.size(); i++) {
      m_pointColors.push_back(m_singleColorForAllPuncta.get());
    }
  } else if (m_colorMode.isSelected("Colormap Score")) {
    for (auto punctum : m_punctaList) {
      m_pointColors.push_back(m_colorMapScore.get().mappedFColor(punctum->score()));
    }
  } else if (m_colorMode.isSelected("Colormap Mean Intensity")) {
    for (auto punctum : m_punctaList) {
      m_pointColors.push_back(m_colorMapMeanIntensity.get().mappedFColor(punctum->meanIntensity()));
    }
  } else if (m_colorMode.isSelected("Colormap Max Intensity")) {
    for (auto punctum : m_punctaList) {
      m_pointColors.push_back(m_colorMapMaxIntensity.get().mappedFColor(punctum->maxIntensity()));
    }
  }

  m_sphereRenderer.setDataColors(&m_pointColors);
}

void Z3DPunctaFilter::updateColorMode()
{
  prepareColor();

  m_prevColorMode = m_currentColorMode;
  m_currentColorMode = m_colorMode.get();
  adjustWidgets();
}

bool Z3DPunctaFilter::widgetGroupUpdateNeeded() const
{
  if (m_widgetsGroup) {
    if (m_widgetUpdateToDate == false) {
      if (((m_prevColorMode == "Point Source" ||
            m_currentColorMode == "Point Source") &&
           m_prevColorMode != m_currentColorMode) ||
          m_prevColorMode.isEmpty()) {
        return true;
      }
    }
  }

  return false;
}

void Z3DPunctaFilter::adjustWidgets()
{
  if (widgetGroupUpdateNeeded()) {
    m_widgetsGroup->emitWidgetsGroupChangedSignal();
    m_prevColorMode = m_currentColorMode;
    m_widgetUpdateToDate = true;
  }

  m_singleColorForAllPuncta.setVisible(m_colorMode.isSelected("Single Color"));
  m_colorMapScore.setVisible(m_colorMode.isSelected("Colormap Score"));
  m_colorMapMeanIntensity.setVisible(m_colorMode.isSelected("Colormap Mean Intensity"));
  m_colorMapMaxIntensity.setVisible(m_colorMode.isSelected("Colormap Max Intensity"));

  for (auto& kv : m_sourceColorMapper) {
    kv.second->setVisible(m_colorMode.isSelected("Point Source"));
  }
}

void Z3DPunctaFilter::selectPuncta(QMouseEvent *e, int, int)
{
  if (m_punctaList.empty())
    return;

  e->ignore();
  // Mouse button pressend
  // can not accept the event in button press, because we don't know if it is a selection or interaction
  if (e->type() == QEvent::MouseButtonPress) {
    m_startCoord.x = e->x();
    m_startCoord.y = e->y();

    const void* obj = pickingManager().objectAtWidgetPos(glm::ivec2(e->x(), e->y()));
    if (!obj) {
      return;
    }

    // Check if any point was selected...
    for (auto p : m_punctaList) {
      if (p == obj) {
        m_pressedPunctum = p;
        break;
      }
    }
    return;
  }

  if (e->type() == QEvent::MouseButtonRelease) {
    if (std::abs(e->x() - m_startCoord.x) < 2 &&
        std::abs(m_startCoord.y - e->y()) < 2) { //moving distinguishment
      if (e->modifiers() == Qt::ControlModifier)
        emit punctumSelected(m_pressedPunctum, true);
      else
        emit punctumSelected(m_pressedPunctum, false);
      if (m_pressedPunctum)
        e->accept();
    }
    m_pressedPunctum = nullptr;
  }
}

void Z3DPunctaFilter::updateData()
{
  double minMeanInten = std::numeric_limits<double>::max();
  double maxMeanInten = std::numeric_limits<double>::lowest();
  double minMaxInten = std::numeric_limits<double>::max();
  double maxMaxInten = std::numeric_limits<double>::lowest();
  for (const auto& p : m_origPunctaList) {
    minMeanInten = std::min(minMeanInten, p->meanIntensity());
    maxMeanInten = std::max(maxMeanInten, p->meanIntensity());
    minMaxInten = std::min(minMaxInten, p->maxIntensity());
    maxMaxInten = std::max(maxMaxInten, p->maxIntensity());
  }
  //todo: set correct range for colormap

  getVisibleData();
  m_dataIsInvalid = true;
  invalidateResult();

  updateBoundBox();
}

void Z3DPunctaFilter::getVisibleData()
{
  m_punctaList.clear();
  for (size_t i=0; i<m_origPunctaList.size(); ++i) {
    if (m_origPunctaList[i]->isVisible())
      m_punctaList.push_back(m_origPunctaList[i]);
  }
}

void Z3DPunctaFilter::updatePunctumVisibleState()
{
  getVisibleData();
  m_dataIsInvalid = true;
  invalidateResult();
}

void Z3DPunctaFilter::changePunctaSize()
{
  for (size_t i=0; i<m_pointAndRadius.size(); i++) {
    if (m_useSameSizeForAllPuncta.get())
      m_pointAndRadius.at(i).w = 2.f;
    else
      m_pointAndRadius.at(i).w = m_punctaList[i]->radius();
  }
  m_sphereRenderer.setData(&m_pointAndRadius, &m_specularAndShininess);
  updateBoundBox();
}
