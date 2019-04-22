#include "z3dmeshfilter.h"

#include "zmesh.h"
#include "zrandom.h"
#include <QFileInfo>
#include <QPushButton>

class Z3DMeshFilter::SelectionStateMachine
{
public:
  void onEvent(QMouseEvent* e, Z3DMeshFilter* filter)
  {
    bool atStart = ((std::abs(e->x() - m_startCoord.x) < 2) &&
                    (std::abs(m_startCoord.y - e->y()) < 2));

    State nextState = m_state;
    switch (e->type()) {
      case QEvent::MouseButtonPress:
        if (m_state != FINISHED) {
          // Defend against a missed QEvent::MouseButtonRelease.
          m_state = NOT_STARTED;
        }
        m_startCoord = glm::ivec2(e->x(), e->y());
        nextState = CLICK;
        break;

      case QEvent::MouseMove:
        if (m_state == CLICK) {
          if (!atStart) {
            nextState = (e->modifiers() == Qt::ControlModifier) ? DRAG : NO_ACTION;
          }
        } else if (m_state == DRAG) {
          if (atStart) {
            nextState = CLICK;
          }
          if (e->modifiers() != Qt::ControlModifier) {
            nextState = NO_ACTION;
          }
        }
        break;

      case QEvent::MouseButtonRelease:
        if (m_state == CLICK) {
          // Defend against missed QEvent::MouseMove with Qt::NoModifier.
          if (!atStart) {
            m_state = (e->modifiers() == Qt::ControlModifier) ? DRAG : NO_ACTION;
          }
        }
        if ((m_state == CLICK) || (m_state == DRAG)) {
          m_clicked = (m_state == CLICK);
          m_appended = (e->modifiers() == Qt::ControlModifier);
          m_endCoord = m_clicked ? glm::ivec2(m_startCoord.x + 1, m_startCoord.y + 1) : glm::ivec2(e->x(), e->y());
        } else {
          m_clicked = m_appended = false;
          m_endCoord = m_startCoord;
        }
        nextState = FINISHED;
        break;

      default:
        break;

    }

    emitSignal(e, nextState, filter);
    m_state = nextState;
  }

  bool finished(bool& clicked, bool& appended, glm::ivec2& rectP0, glm::ivec2& rectP1)
  {
    if (m_state == FINISHED) {
      clicked = m_clicked;
      appended = m_appended;
      rectP0 = m_startCoord;
      rectP1 = m_clicked ? glm::ivec2(m_startCoord.x + 1, m_startCoord.y + 1) : m_endCoord;
      return true;
    } else {
      return false;
    }
  }

private:
  enum State { NOT_STARTED, CLICK, DRAG, FINISHED, NO_ACTION };

  void emitSignal(QMouseEvent* e, State nextState, Z3DMeshFilter* filter)
  {
    if ((m_state != DRAG) && (nextState == DRAG)) {
      emit filter->selectionRectStarted(m_startCoord.x, m_startCoord.y);
    } else if ((m_state == DRAG) && (nextState == DRAG)) {
      emit filter->selectionRectChanged(e->x(), e->y());
    } else if ((m_state == DRAG) && (nextState != DRAG)) {
      emit filter->selectionRectEnded();
    }
  }

  State m_state = NOT_STARTED;
  glm::ivec2 m_startCoord = glm::ivec2(-1, -1);
  glm::ivec2 m_endCoord = glm::ivec2(-1, -1);
  bool m_clicked = false;
  bool m_appended = false;
};

//

Z3DMeshFilter::Z3DMeshFilter(Z3DGlobalParameters& globalParas, QObject* parent)
  : Z3DGeometryFilter(globalParas, parent)
  , m_meshRenderer(m_rendererBase)
  , m_colorMode("Color Mode")
  , m_singleColorForAllMesh("Mesh Color", glm::vec4(ZRandom::instance().randReal<float>(),
                                                    ZRandom::instance().randReal<float>(),
                                                    ZRandom::instance().randReal<float>(),
                                                    1.f))
  , m_preserveSourceColors(false)
  , m_showSourceColors(true)
  , m_selectMeshEvent("Select Mesh", false)
  , m_selectionStateMachine(new SelectionStateMachine)
  , m_dataIsInvalid(false)
{
  setControlName("Mesh");
  m_singleColorForAllMesh.setStyle("COLOR");
  connect(&m_singleColorForAllMesh, &ZVec4Parameter::valueChanged,
          this, &Z3DMeshFilter::prepareColor);

  // Color Mode
  m_colorMode.addOptions("Mesh Color", "Single Color", "Mesh Source", "Indexed Color");
  m_colorMode.select("Mesh Source");

  connect(&m_colorMode, &ZStringIntOptionParameter::valueChanged,
          this, &Z3DMeshFilter::processColorModeChange);
//  connect(&m_colorMode, &ZStringIntOptionParameter::valueChanged,
//          this, &Z3DMeshFilter::adjustWidgets);

  addParameter(m_colorMode);

  addParameter(m_singleColorForAllMesh);

  m_selectMeshEvent.listenTo(
        "select mesh", Qt::LeftButton, Qt::NoModifier, QEvent::MouseButtonPress);
  m_selectMeshEvent.listenTo(
        "select mesh", Qt::LeftButton, Qt::NoModifier, QEvent::MouseButtonRelease);
  m_selectMeshEvent.listenTo(
        "select mesh", Qt::LeftButton, Qt::NoModifier, QEvent::MouseButtonDblClick);
  m_selectMeshEvent.listenTo(
        "select mesh", Qt::LeftButton, Qt::ControlModifier, QEvent::MouseButtonDblClick);
  m_selectMeshEvent.listenTo(
        "append select mesh", Qt::LeftButton, Qt::ControlModifier, QEvent::MouseButtonPress);
  m_selectMeshEvent.listenTo(
        "rect select mesh", Qt::LeftButton, Qt::ControlModifier, QEvent::MouseMove);
  m_selectMeshEvent.listenTo(
        "append select mesh", Qt::LeftButton, Qt::ControlModifier, QEvent::MouseButtonRelease);

  // If the user starts dragging a selection rectangle, using Qt::ControlModifier, it is best
  // to cancel this interaction if the user changes the modifier mid drag.  But the result
  // of each modifier change must be registered explicilty here.  It is feasible only to register
  // the most likely change, to Qt::NoModifier.
  m_selectMeshEvent.listenTo(
        "rect select mesh", Qt::LeftButton, Qt::NoModifier, QEvent::MouseMove);

  connect(&m_selectMeshEvent, &ZEventListenerParameter::mouseEventTriggered,
          this, &Z3DMeshFilter::selectMesh);
  addEventListener(m_selectMeshEvent);

  adjustWidgets();

  addParameter(m_meshRenderer.wireframeModePara());
  addParameter(m_meshRenderer.wireframeColorPara());

  addParameter(m_meshRenderer.useTwoSidedLightingPara());

  //Use queued connection to work in the right order with updateSettingsDockWidget in Z3DWindow
  connect(this, SIGNAL(clearingParamGarbage()), this, SLOT(dumpParamGarbage()),
          Qt::QueuedConnection);
}

void Z3DMeshFilter::process(Z3DEye)
{
  if (m_dataIsInvalid) {
    prepareData();
  }
}

void Z3DMeshFilter::setColorMode(const std::string &mode)
{
  m_colorMode.select(mode.c_str());
}

void Z3DMeshFilter::processColorModeChange()
{
  if (m_colorMode.get() == "Mesh Source") {
    updateSourceColorMapper();
  }
  prepareColor();
  adjustWidgets();
}

void Z3DMeshFilter::dumpParamGarbage()
{
  LINFO() << "Dumping garbage";
  m_paramGarbage.clear();
}

void Z3DMeshFilter::emitDumpParaGarbage()
{
  emit clearingParamGarbage();
}

void Z3DMeshFilter::enablePreservingSourceColors(bool on)
{
  m_preserveSourceColors = on;
}

bool Z3DMeshFilter::preservingSourceColorsEnabled() const
{
  return m_preserveSourceColors;
}

void Z3DMeshFilter::showSourceColors(bool show)
{
  m_showSourceColors = show;
}

bool Z3DMeshFilter::showingSourceColors() const
{
  return m_showSourceColors;
}

void Z3DMeshFilter::setData(const std::vector<ZMesh*>& meshList)
{
  m_meshBoundboxMapper.clear();
  m_origMeshList = meshList;
  //LOG(INFO) << className() << " read " << m_origMeshList.size() << " meshes.";
  getVisibleData();
  m_dataIsInvalid = true;
  invalidateResult();

  updateBoundBox();
}

void Z3DMeshFilter::setData(const QList<ZMesh*>& meshList)
{
  m_meshBoundboxMapper.clear();
  m_origMeshList.clear();
  for (auto mesh : meshList)
    m_origMeshList.push_back(mesh);
  //LOG(INFO) << className() << " read " << m_origMeshList.size() << " meshes.";
  getVisibleData();
  m_dataIsInvalid = true;
  invalidateResult();

  updateBoundBox();
}

bool Z3DMeshFilter::isReady(Z3DEye eye) const
{
  return Z3DGeometryFilter::isReady(eye) && isVisible() && !m_origMeshList.empty();
}

std::shared_ptr<ZWidgetsGroup> Z3DMeshFilter::widgetsGroup()
{
  if (!m_widgetsGroup) {
    m_widgetsGroup = std::make_shared<ZWidgetsGroup>(getControlName().c_str(), 1);
    m_widgetsGroup->addChild(m_visible, 1);
    m_widgetsGroup->addChild(m_stayOnTop, 1);
    m_widgetsGroup->addChild(m_meshRenderer.useTwoSidedLightingPara(), 1);
    m_widgetsGroup->addChild(m_colorMode, 2);
    m_widgetsGroup->addChild(m_singleColorForAllMesh, 2);

    for (const auto& kv : m_sourceColorMapper) {
      m_widgetsGroup->addChild(*kv.second, 2);
    }

    m_widgetsGroup->addChild(m_meshRenderer.wireframeModePara(), 3);
    m_widgetsGroup->addChild(m_meshRenderer.wireframeColorPara(), 3);

    std::vector<ZParameter*> paras = m_rendererBase.parameters();
    for (auto para : paras) {
      if (para->name() == "Coord Transform") {
        m_widgetsGroup->addChild(*para, 4);
        //        QPushButton *pb = new QPushButton("Apply Transform");
        //        connect(pb, &QPushButton::clicked, this, &Z3DMeshFilter::onApplyTransform);
        //        m_widgetsGroup->addChild(*pb, 2);
      }
      else if (para->name() == "Opacity")
        m_widgetsGroup->addChild(*para, 1);
      else if (para->name() != "Size Scale")
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

void Z3DMeshFilter::renderOpaque(Z3DEye eye)
{
  m_rendererBase.render(eye, m_meshRenderer);
  renderBoundBox(eye);
}

void Z3DMeshFilter::renderTransparent(Z3DEye eye)
{
  m_rendererBase.render(eye, m_meshRenderer);
  renderBoundBox(eye);
}

void Z3DMeshFilter::renderPicking(Z3DEye eye)
{
  if (pickingEnabled()) {
    if (!m_pickingObjectsRegistered)
      registerPickingObjects();
    m_rendererBase.renderPicking(eye, m_meshRenderer);
  }
}

namespace {
  glm::vec4 randomColorRGB()
  {
    // Remember the last hue used, so the next one can continue to rotate through
    // the range of hues.
    static qreal hPrev = 0;

    qreal h = hPrev + 0.234 + 0.3 * ZRandom::instance().randReal<float>();
    h = (h > 1) ? h - 1 : h;
    hPrev = h;

    qreal s = 0.5 + 0.5 * ZRandom::instance().randReal<float>();
    qreal l = 0.5 + 0.5 * ZRandom::instance().randReal<float>();

    QColor hsl = QColor::fromHslF(h, s, l);
    QColor rgb = hsl.toRgb();
    return glm::vec4(rgb.redF(), rgb.greenF(), rgb.blueF(), 1.0);
  }
}

void Z3DMeshFilter::removeOldColorParameter(
    const std::set<QString, QStringNaturalCompare> &allSources)
{
#ifdef _DEBUG_
  printColorMapper();
#endif
  if (!m_preserveSourceColors) {
    // remove not in use sources
    for (auto it = m_sourceColorMapper.begin(); it != m_sourceColorMapper.end(); ) {
      if (allSources.find(it->first) == allSources.end()) {
        removeParameter(*it->second);
        m_paramGarbage.push_back(it->second);
        it = m_sourceColorMapper.erase(it);
      } else {
        ++it;
      }
    }
  }
#ifdef _DEBUG_
  std::cout << "After removing:";
  printColorMapper();
#endif
}

void Z3DMeshFilter::printColorMapper()
{
  std::cout << "Current color mapper: ";
  for (const auto &kv : m_sourceColorMapper) {
    std::cout << " " << kv.first.toStdString();
  }
  std::cout << std::endl;
}

void Z3DMeshFilter::addSourceColor(const QString &source)
{
  QString guiname = QString("Source: %1").arg(source);

  auto color = randomColorRGB();

  Q_ASSERT(m_sourceColorMapper.count(source) == 0);

  m_sourceColorMapper.insert(
        std::make_pair(source,
                       std::make_shared<ZVec4Parameter>(guiname, color)));

  m_sourceColorMapper[source]->setStyle("COLOR");
  connect(m_sourceColorMapper[source].get(), &ZVec4Parameter::valueChanged,
          this, &Z3DMeshFilter::prepareColor);
  addParameter(*m_sourceColorMapper[source]);
}

void Z3DMeshFilter::addNewColorParameter(
    const std::set<QString, QStringNaturalCompare> &allSources)
{
#ifdef _DEBUG_
  std::cout << "All sources:";
  for (const QString &source : allSources) {
    std::cout << " " << source.toStdString();
  }
  std::cout << std::endl;

  printColorMapper();
#endif
  // create color parameters for new sources
  std::set<QString, QStringNaturalCompare> newSources;
  std::set_difference(allSources.begin(), allSources.end(),
                      m_sourceColorMapper.begin(), m_sourceColorMapper.end(),
                      std::inserter(newSources, newSources.end()),
                      QStringKeyNaturalLess());
  for (const auto& kv : newSources) {
    addSourceColor(kv);
  }
#ifdef _DEBUG_
  std::cout << "After update:";
  printColorMapper();
#endif
}

bool Z3DMeshFilter::sourceChanged(
    const std::set<QString, QStringNaturalCompare> &allSources) const
{
  return m_sourceColorMapper.size() != allSources.size() ||
      !std::equal(m_sourceColorMapper.begin(), m_sourceColorMapper.end(),
                  allSources.begin(), _KeyEqual());
}

void Z3DMeshFilter::removeSourceColorWidget()
{
  if (m_widgetsGroup) {
    for (auto& kv : m_sourceColorMapper) {
      m_widgetsGroup->removeChild(*kv.second);
    }
  }
}

void Z3DMeshFilter::updateWidgetGroup()
{
  m_widgetsGroup->emitWidgetsGroupChangedSignal();
  emitDumpParaGarbage();
}

void Z3DMeshFilter::addSourceColorWidget()
{
  if (m_widgetsGroup) {
    for (const auto& kv : m_sourceColorMapper) {
      m_widgetsGroup->addChild(*kv.second, 2);
    }
    updateWidgetGroup();
  }
}

void Z3DMeshFilter::updateSourceColorMapper()
{
  if (m_colorMode.get() == "Mesh Source") {
    if (m_showSourceColors) {
      std::set<QString, QStringNaturalCompare> allSources;
      for (auto mesh : m_origMeshList) {
        allSources.insert(mesh->getSource().c_str());
      }
      // do nothing if sources don't change
      if (sourceChanged(allSources)) {
        // remove old source color parameters from widget, will add new ones later
        removeSourceColorWidget();

        removeOldColorParameter(allSources);
        addNewColorParameter(allSources);

        // update widget group
        addSourceColorWidget();
      }
    } else {
      for (auto mesh : m_origMeshList) {
        auto kv = mesh->getSource().c_str();
        QString guiname = QString("Source: %1").arg(kv);
        auto color = randomColorRGB();
        m_sourceColorMapper.insert(
              std::make_pair(kv, std::make_shared<ZVec4Parameter>(guiname, color)));

        m_sourceColorMapper[kv]->setStyle("COLOR");
      }
    }
  }
}

void Z3DMeshFilter::prepareData()
{
  if (!m_dataIsInvalid)
    return;

  deregisterPickingObjects();

  initializeCutRange();
  initializeRotationCenter();

  updateSourceColorMapper();

  m_meshRenderer.setData(&m_meshList);
  prepareColor();
  adjustWidgets();
  m_dataIsInvalid = false;
}

void Z3DMeshFilter::registerPickingObjects()
{
  if (pickingEnabled()) {
    if (!m_pickingObjectsRegistered) {
      m_registeredMeshList.clear();
      for (ZMesh* mesh : m_meshList) {
        pickingManager().registerObject(mesh);
        m_registeredMeshList.push_back(mesh);
      }
//      m_registeredMeshList = m_meshList;
      m_meshPickingColors.clear();
      for (ZMesh* mesh : m_registeredMeshList) {
        glm::col4 pickingColor = pickingManager().colorOfObject(mesh);
#ifdef _DEBUG_2
        std::cout << "Mesh picking color: " << pickingColor << std::endl;
#endif
        glm::vec4 fPickingColor(
              pickingColor[0] / 255.f, pickingColor[1] / 255.f,
              pickingColor[2] / 255.f,  pickingColor[3] / 255.f);
        m_meshPickingColors.push_back(fPickingColor);
      }
      m_meshRenderer.setDataPickingColors(&m_meshPickingColors);
    }

    m_pickingObjectsRegistered = true;
  }
}

void Z3DMeshFilter::deregisterPickingObjects()
{
  if (m_pickingObjectsRegistered) {
    for (auto mesh : m_registeredMeshList) {
      pickingManager().deregisterObject(mesh);
    }
    m_registeredMeshList.clear();
  }

  m_pickingObjectsRegistered = false;
}

ZBBox<glm::dvec3> Z3DMeshFilter::meshBound(ZMesh* p)
{
  std::map<ZMesh*, ZBBox<glm::dvec3>>::const_iterator it = m_meshBoundboxMapper.find(p);
  if (it != m_meshBoundboxMapper.end()) {
    ZBBox<glm::dvec3> result = it->second;
    return result;
  } else {
    ZBBox<glm::dvec3> result = p->boundBox(coordTransform());
    m_meshBoundboxMapper[p] = result;
    return result;
  }
}

void Z3DMeshFilter::updateNotTransformedBoundBoxImpl()
{
  m_notTransformedBoundBox.reset();
  for (size_t i = 0; i < m_origMeshList.size(); ++i) {
    m_notTransformedBoundBox.expand(m_origMeshList[i]->boundBox());
  }
}

void Z3DMeshFilter::addSelectionLines()
{
  for (const auto& p : m_meshList) {
    if (p->isVisible() && p->isSelected()) {
      appendBoundboxLines(meshBound(p), m_selectionLines);
    }
  }
}

void Z3DMeshFilter::prepareColor()
{
  m_meshColors.clear();

  if (m_colorMode.isSelected("Single Color")) {
    for (size_t i = 0; i < m_meshList.size(); ++i) {
      m_meshColors.push_back(m_singleColorForAllMesh.get());
    }
    m_meshRenderer.setDataColors(&m_meshColors);
    m_meshRenderer.setColorSource("CustomColor");
  } else if (m_colorMode.isSelected("Mesh Source")) {
    for (size_t i=0; i<m_meshList.size(); i++) {
      //LOG(INFO) << m_meshList[i]->getSource().c_str() << m_sourceColorMapper[m_meshList[i]->getSource().c_str()].get();
      glm::vec4 color = m_sourceColorMapper[m_meshList[i]->getSource().c_str()]->get();
      m_meshColors.push_back(color);
    }
    m_meshRenderer.setDataColors(&m_meshColors);
    m_meshRenderer.setColorSource("CustomColor");
  } else if (m_colorMode.isSelected("Mesh Color")) {
    m_meshRenderer.setColorSource("MeshColor");
  } else if (m_colorMode.isSelected("Indexed Color")) {
    glm::vec4 defaultColor =
        m_indexedColors.size() > 1 ? m_indexedColors[0] : glm::vec4(1, 1, 1, 1);
    for (size_t i = 0; i < m_meshList.size(); ++i) {
      glm::vec4 color = defaultColor;
      size_t index = m_indexedColors.size();
      if (m_meshIDToColorIndexFunc != nullptr) {
        index = m_meshIDToColorIndexFunc(m_meshList[i]->getLabel());
      } else {
        auto it = m_meshIDToColorIndex.find(m_meshList[i]->getLabel());
        if (it != m_meshIDToColorIndex.end()) {
          index = it->second;
        }
      }
      if (m_indexedColors.size() > index) {
        color = m_indexedColors[index];
      }
      m_meshColors.push_back(color);
    }
    m_meshRenderer.setDataColors(&m_meshColors);
    m_meshRenderer.setColorSource("CustomColor");
  }
}

void Z3DMeshFilter::adjustWidgets()
{
  m_singleColorForAllMesh.setVisible(m_colorMode.isSelected("Single Color"));

  if (m_showSourceColors) {
    for (auto& kv : m_sourceColorMapper) {
      kv.second->setVisible(m_colorMode.isSelected("Mesh Source"));
    }
  }
}

bool Z3DMeshFilter::hitObject(int x, int y)
{
  const void* obj = pickingManager().objectAtWidgetPos(glm::ivec2(x, y));
  if (obj != NULL) {
    return true;
  }

  return false;
}

std::vector<bool> Z3DMeshFilter::hitObject(
    const std::vector<std::pair<int, int> > &ptArray)
{
  std::vector<bool> hitArray(ptArray.size(), false);

  if (pickingEnabled()) {
    std::vector<const void*> objArray =
        pickingManager().objectAtWidgetPos(ptArray);
    for (size_t i = 0; i < objArray.size(); ++i) {
      if (objArray[i] != NULL) {
        hitArray[i] = true;
      }
    }
  }

  return hitArray;
}

void Z3DMeshFilter::setColorIndexing(const std::vector<glm::vec4> &indexedColors,
                                     const std::map<uint64_t, std::size_t> &meshIdToColorIndex)
{
  m_meshIDToColorIndexFunc = nullptr;
  m_indexedColors = indexedColors;
  m_meshIDToColorIndex = meshIdToColorIndex;
  updateMeshVisibleState();
}

void Z3DMeshFilter::setColorIndexing(const std::vector<glm::vec4> &indexedColors,
                                     std::function<std::size_t(uint64_t)> meshIdToColorIndexFunc)
{
  m_indexedColors = indexedColors;
  m_meshIDToColorIndexFunc = meshIdToColorIndexFunc;
  updateMeshVisibleState();
}

ZMesh* Z3DMeshFilter::hitMesh(int x, int y)
{
  const void* obj = pickingManager().objectAtWidgetPos(glm::ivec2(x, y));
  ZMesh *hitMesh = NULL;
  if (obj != NULL) {
    // Check if any point was selected...
    for (auto m : m_meshList) {
      if (m == obj) {
        hitMesh = m;
        break;
      }
    }
  }

  return hitMesh;
}

void Z3DMeshFilter::selectMesh(QMouseEvent* e, int, int)
{
  if (m_meshList.empty() || !pickingEnabled()) {
    return;
  }

#ifdef _DEBUG_
  std::cout << "Selecting graph in " << this << std::endl;
  std::cout << "Original mesh count: " << m_origMeshList.size() << std::endl;
  std::cout << "Mesh count: " << m_meshList.size() << std::endl;
#endif

  e->ignore();

  m_selectionStateMachine->onEvent(e, this);

  bool clicked = false;
  bool appended = false;
  glm::ivec2 p0, p1;
  if (m_selectionStateMachine->finished(clicked, appended, p0, p1)) {
    if (clicked || appended) {
      std::set<const void*> objs = pickingManager().objectsInWidgetRect(p0, p1);
      bool accept = false;
      for (auto m : m_meshList) {
        if (objs.find(m) != objs.end()) {
          emit meshSelected(m, appended);
          accept = true;
        }
      }
      if (clicked && (objs.size() == 1) && (objs.count(nullptr) == 1)) {
        // In this case, the user clicked on empty space, so the selection should be cleared.
        emit meshSelected(nullptr, false);
        accept = true;
      }
      if (accept) {
        e->accept();
      }
    }
  }
}

void Z3DMeshFilter::onApplyTransform()
{
  LOG(INFO) << m_rendererBase.coordTransform();
}

void Z3DMeshFilter::updateMeshVisibleState()
{
  getVisibleData();
  m_dataIsInvalid = true;
  invalidateResult();
}

void Z3DMeshFilter::getVisibleData()
{
  m_meshList.clear();
  for (size_t i=0; i<m_origMeshList.size(); ++i) {
    if (m_origMeshList[i]->isVisible())
      m_meshList.push_back(m_origMeshList[i]);
  }
}
