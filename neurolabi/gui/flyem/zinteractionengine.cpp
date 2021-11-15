#include "zinteractionengine.h"
#include <QMouseEvent>

#include "common/debug.h"
#include "z3dinteractionhandler.h"
#include "zstackoperator.h"
#include "zstackdockeyprocessor.h"
#include "data3d/displayconfig.h"

ZInteractionEngine::ZInteractionEngine(QObject *parent) :
  QObject(parent), m_objStyle(ZStackObject::EDisplayStyle::NORMAL),
  m_mouseLeftButtonPressed(false), m_mouseRightButtonPressed(false),
  m_cursorRadius(5), m_isStrokeOn(false),
  m_isKeyEventEnabled(true), m_interactionHandler(NULL)
{
  m_stroke.setWidth(10.0);
  m_stroke.setZ(0);

  m_rayMarker.setWidth(10.0);
  m_rayMarker.setZ(0);
  m_rayMarker.setFilled(false);
  m_rayMarker.useCosmeticPen(true);
  m_rayMarker.setColor(Qt::red);

  m_exploreMarker.setRadius(5.0);
  m_exploreMarker.setZ(0);
  m_exploreMarker.SetDefaultPenWidth(2.0);
  m_exploreMarker.useCosmeticPen(true);
  m_exploreMarker.addVisualEffect(neutu::display::Sphere::VE_CROSS_CENTER);
  m_exploreMarker.setColor(Qt::red);

  m_namedDecorationList.append(&m_stroke);
  m_namedDecorationList.append(&m_rayMarker);
  m_namedDecorationList.append(&m_exploreMarker);

  m_rect.setColor(255, 0, 0, 164);
  m_namedDecorationList.append(&m_rect);

  foreach (ZStackObject *drawable, m_namedDecorationList) {
    drawable->setVisible(false);
  }

  m_previousKey = Qt::Key_unknown;
  m_previousKeyModifiers = Qt::NoModifier;
  m_keyMode = KM_NORMAL;
}

ZInteractionEngine::~ZInteractionEngine()
{
  foreach (ZStackObject *drawable, m_unnamedDecorationList) {
    delete drawable;
  }
}

bool ZInteractionEngine::lockingMouseMoveEvent() const
{
  return isStateOn(STATE_DRAW_STROKE) || isStateOn(STATE_DRAW_RECT);
}

void ZInteractionEngine::processMouseMoveEvent(QMouseEvent *event)
{
  m_mouseMovePosition[0] = event->x();
  m_mouseMovePosition[1] = event->y();

  if (m_interactiveContext.strokeEditMode() ==
      ZInteractiveContext::STROKE_DRAW) {
#ifdef _DEBUG_0
    std::cout << OUTPUT_HIGHTLIGHT_2 << "Extend stroke: "
              << event->x() << ", " << event->y() << std::endl;
#endif
    if (m_mouseLeftButtonPressed == true){
      m_stroke.append(event->x(), event->y());
      event->accept();
    } else {
      m_stroke.set(event->x(), event->y());
      m_stroke.toggleLabel(event->modifiers() == Qt::ShiftModifier);
    }
#ifdef _DEBUG_2
    std::cout << "decorationUpdated emitted" << std::endl;
#endif

    emit decorationUpdated();
  }else if (m_interactiveContext.rectEditMode() ==
            ZInteractiveContext::RECT_DRAW) {
    if (m_mouseLeftButtonPressed == true) {
      m_rect.setMaxCorner(event->x(), event->y());
//      m_rect.makeValid();
    }

    emit decorationUpdated();
 } else if (m_interactiveContext.todoEditMode() ==
            ZInteractiveContext::TODO_ADD_ITEM) {
    m_rayMarker.set(event->x(), event->y());
    emit decorationUpdated();
  } else if (m_interactiveContext.exploreMode() == ZInteractiveContext::EXPLORE_LOCAL ||
             m_interactiveContext.exploreMode() == ZInteractiveContext::EXPLORE_EXTERNALLY ||
             m_interactiveContext.exploreMode() == ZInteractiveContext::EXPLORE_DETAIL) {
    m_exploreMarker.setCenter(event->x(), event->y(), 0);
    emit decorationUpdated();
  }

  if (m_mouseLeftButtonPressed) {
    suppressMouseRelease(true);
  }
}

bool ZInteractionEngine::processMouseReleaseEvent(
    QMouseEvent *event, int sliceIndex)
{
  bool processed = false;

  UNUSED_PARAMETER(sliceIndex);
  if (event->button() == Qt::LeftButton) {
    if (isStateOn(STATE_DRAW_STROKE)) {
      commitData();
      processed = true;
    } else if (isStateOn(STATE_DRAW_RECT)) {
      m_rect.makeValid();
      exitPaintRect();
      processed = true;
    }

    if (mouseReleaseSuppressed()) {
      suppressMouseRelease(false);
//      processed = true;
    } else {
      if (isStateOn(STATE_MARK)) {
        emit shootingTodo(event->x(), event->y());
        processed = true;
      } else if (isStateOn(STATE_LOCATE)) {
        if (event->modifiers() == Qt::NoModifier) {
          emit locating(event->x(), event->y());
        } else if (event->modifiers() == Qt::ShiftModifier) {
          emit showingDetail(event->x(), event->y());
        }
        processed = true;
      } else if (isStateOn(STATE_BROWSE)) {
        emit browsing(event->x(), event->y());
        processed = true;
      } else if (isStateOn(STATE_SHOW_DETAIL)) {
        emit showingDetail(event->x(), event->y());
        processed = true;
      }
    }
    m_mouseLeftButtonPressed = false;
  } else if (event->button() == Qt::RightButton) {
    exitEditMode();
    emit exitingEdit();
    m_mouseRightButtonPressed = false;
  }

  return processed;
}

void ZInteractionEngine::showContextMenu()
{
  emit showingContextMenu();
}

void ZInteractionEngine::commitData()
{
  saveStroke();
}

bool ZInteractionEngine::hasRectDecoration() const
{
  return m_rect.isValid();
}

void ZInteractionEngine::removeRectDecoration()
{
  m_rect.setSize(0, 0);
}

void ZInteractionEngine::setKeyProcessor(ZStackDocKeyProcessor *processor)
{
  m_keyProcessor = processor;
}

void ZInteractionEngine::processMousePressEvent(QMouseEvent *event,
                                                int sliceIndex)
{
  UNUSED_PARAMETER(sliceIndex);
  UNUSED_PARAMETER(event);

  if (event->button() == Qt::LeftButton) {
    m_mouseLeftButtonPressed = true;
    if (isStateOn(STATE_DRAW_RECT)) {
      m_rect.setMinCorner(event->x(), event->y());
      m_rect.setSize(0, 0);
    }
  } else if (event->button() == Qt::RightButton) {
    m_mouseRightButtonPressed = true;
  }
}

bool ZInteractionEngine::process(const ZStackOperator &op)
{
  bool processed = false;

  switch (op.getOperation()) {
  case ZStackOperator::OP_START_PAINT_STROKE:
    enterPaintStroke();
    processed = true;
    break;
  case ZStackOperator::OP_RECT_ROI_INIT:
    enterPaintRect();
    processed = true;
    break;
  case ZStackOperator::OP_EXIT_EDIT_MODE:
    exitEditMode();
    processed = true;
    break;
  case ZStackOperator::OP_ACTIVE_STROKE_CHANGE_LABEL:
    m_stroke.setLabel(op.getLabel());
    emit decorationUpdated();
    processed = true;
    break;
  case ZStackOperator::OP_SWC_SELECT_NODE_IN_ROI:
    if (hasRectDecoration()) {
      emit selectingSwcNodeInRoi(op.isShift());
    }
    break;
  case ZStackOperator::OP_FLYEM_TOD_ENTER_ADD_MODE:
    enterMarkTodo();
    processed = true;
    break;
  case ZStackOperator::OP_EXPLORE_LOCAL:
    enterLocateMode();
    processed = true;
    break;
  case ZStackOperator::OP_FLYEM_CROP_BODY:
    if (hasRectDecoration()) {
      emit croppingSwc();
      processed = true;
    }
    break;
  case ZStackOperator::OP_FLYEM_SPLIT_BODY_LOCAL:
    emit splittingBodyLocal();
    processed = true;
    break;
  case ZStackOperator::OP_FLYEM_SPLIT_BODY:
    emit splittingBody();
    processed = true;
    break;
  case ZStackOperator::OP_FLYEM_SPLIT_BODY_FULL:
    emit splittingFullBody();
    processed = true;
    break;
  case ZStackOperator::OP_OBJECT_DELETE_SELECTED:
    emit deletingSelected();
    processed = true;
    break;
  default:
    break;
  }

  return processed;
}

bool ZInteractionEngine::processKeyPressEvent(QKeyEvent *event)
{
  if (!m_isKeyEventEnabled) {
    return false;
  }

  bool processed = false;

  if (m_keyProcessor != NULL) {
    m_keyProcessor->processKeyEvent(event, m_interactiveContext);
    processed = process(m_keyProcessor->getOperator());
  }

  m_previousKey = event->key();
  m_previousKeyModifiers = event->modifiers();

  return processed;
}

void ZInteractionEngine::enterPaintStroke()
{
#ifdef _DEBUG_0
    std::cout << OUTPUT_HIGHTLIGHT_2 << "Activate stroke paint: "
              << m_mouseMovePosition[0] << ", "
              << m_mouseMovePosition[1] << std::endl;
#endif
  exitEditMode();

  m_interactiveContext.setStrokeEditMode(ZInteractiveContext::STROKE_DRAW);
  m_stroke.set(m_mouseMovePosition[0], m_mouseMovePosition[1]);
  m_stroke.setVisible(true);
  emit decorationUpdated();
}

void ZInteractionEngine::enableRayMarker()
{
  m_rayMarker.set(m_mouseMovePosition[0], m_mouseMovePosition[1]);
  m_rayMarker.setVisible(true);
}

void ZInteractionEngine::enterMarkTodo()
{
  exitEditMode();

  m_interactiveContext.setTodoEditMode(ZInteractiveContext::TODO_ADD_ITEM);
  enableRayMarker();

  emit decorationUpdated();
}

void ZInteractionEngine::enterMarkBookmark()
{
  exitEditMode();
  m_interactiveContext.setBookmarkEditMode(ZInteractiveContext::BOOKMARK_ADD);
  enableRayMarker();
}

void ZInteractionEngine::enterLocateMode()
{
  exitEditMode();
  m_interactiveContext.setExploreMode(ZInteractiveContext::EXPLORE_LOCAL);
  m_exploreMarker.setCenter(m_mouseMovePosition[0], m_mouseMovePosition[1], 0);
  m_exploreMarker.setColor(Qt::red);
  m_exploreMarker.setVisualEffect(neutu::display::Sphere::VE_DOT_CENTER);
  m_exploreMarker.setVisible(true);
  emit decorationUpdated();
}

void ZInteractionEngine::exitLocateMode()
{
  exitExplore();
}

void ZInteractionEngine::enterBrowseMode()
{
  exitEditMode();
  m_interactiveContext.setExploreMode(ZInteractiveContext::EXPLORE_EXTERNALLY);
  m_exploreMarker.setCenter(m_mouseMovePosition[0], m_mouseMovePosition[1], 0);
  m_exploreMarker.setColor(Qt::blue);
  m_exploreMarker.setVisualEffect(
        neutu::display::Sphere::VE_RECTANGLE_SHAPE |
        neutu::display::Sphere::VE_CROSS_CENTER);
  m_exploreMarker.setVisible(true);
  emit decorationUpdated();
}

void ZInteractionEngine::enterDetailMode()
{
  exitEditMode();
  m_interactiveContext.setExploreMode(ZInteractiveContext::EXPLORE_DETAIL);
  m_exploreMarker.setCenter(m_mouseMovePosition[0], m_mouseMovePosition[1], 0);
  m_exploreMarker.setVisible(true);
  emit decorationUpdated();
}

void ZInteractionEngine::exitBrowseMode()
{
  exitExplore();
}

void ZInteractionEngine::exitDetailMode()
{
  exitExplore();
}

void ZInteractionEngine::enterPaintRect()
{
  exitEditMode();

  m_rect.setVisible(true);
  m_interactiveContext.setRectEditMode(ZInteractiveContext::RECT_DRAW);
  emit decorationUpdated();
}

void ZInteractionEngine::exitPaintRect()
{
  if (m_interactiveContext.rectEditMode() != ZInteractiveContext::RECT_EDIT_OFF) {
    m_interactiveContext.setRectEditMode(ZInteractiveContext::RECT_EDIT_OFF);
    //  m_rect.setVisible(false);
    emit decorationUpdated();
  }
}

void ZInteractionEngine::exitSwcEdit()
{
  m_interactiveContext.setSwcEditMode(ZInteractiveContext::SWC_EDIT_OFF);
}

void ZInteractionEngine::exitPaintStroke()
{
  if (m_interactiveContext.strokeEditMode() !=
      ZInteractiveContext::STROKE_EDIT_OFF) {
    m_interactiveContext.setStrokeEditMode(ZInteractiveContext::STROKE_EDIT_OFF);
    m_stroke.setVisible(false);
    emit decorationUpdated();
  }
}

void ZInteractionEngine::exitMarkTodo()
{
  if (m_interactiveContext.todoEditMode() != ZInteractiveContext::TODO_EDIT_OFF) {
    m_interactiveContext.setTodoEditMode(ZInteractiveContext::TODO_EDIT_OFF);
    m_rayMarker.setVisible(false);
    emit decorationUpdated();
  }
}

void ZInteractionEngine::exitMarkBookmark()
{
  if (m_interactiveContext.bookmarkEditMode() !=
      ZInteractiveContext::BOOKMARK_EDIT_OFF) {
    m_interactiveContext.setBookmarkEditMode(
          ZInteractiveContext::BOOKMARK_EDIT_OFF);
    m_rayMarker.setVisible(false);
    emit decorationUpdated();
  }
}

void ZInteractionEngine::exitExplore()
{
  if (m_interactiveContext.exploreMode() != ZInteractiveContext::EXPLORE_OFF) {
    m_interactiveContext.setExploreMode(ZInteractiveContext::EXPLORE_OFF);
    m_exploreMarker.setVisible(false);

    emit decorationUpdated();
  }
}


void ZInteractionEngine::exitEditMode()
{
  exitPaintRect();
  exitSwcEdit();
  exitPaintStroke();
  exitMarkTodo();
  exitMarkBookmark();
  exitExplore();
}

QList<ZStackObject*> ZInteractionEngine::getDecorationList() const
{
  QList<ZStackObject*> decorationList;
  decorationList.append(m_namedDecorationList);
  decorationList.append(m_unnamedDecorationList);

  return decorationList;
}

void ZInteractionEngine::saveStroke()
{
  //if (m_dataBuffer != NULL) {
  if (m_stroke.isVisible() && !m_stroke.isEmpty()) {
    ZStroke2d *stroke = new ZStroke2d;
    *stroke = m_stroke;
#ifdef _DEBUG_0
  std::cout << OUTPUT_HIGHTLIGHT_2 << __FUNCTION__ << std::endl;
#endif
    //m_dataBuffer->addStroke(stroke);
    emit strokePainted(stroke);
  }
}

void ZInteractionEngine::set3DInteractionHandler(Z3DTrackballInteractionHandler *handler)
{
  m_interactionHandler = handler;
  connect(handler, SIGNAL(cameraRotated()), this, SIGNAL(cameraRotated()));
}

bool ZInteractionEngine::isStateOn(EState status) const
{
  switch (status) {
  case STATE_DRAW_LINE:
    return false;
  case STATE_DRAW_STROKE:
    return m_interactiveContext.strokeEditMode() ==
        ZInteractiveContext::STROKE_DRAW;
  case STATE_MARK:
    return m_interactiveContext.todoEditMode() ==
        ZInteractiveContext::TODO_ADD_ITEM;
  case STATE_DRAW_RECT:
    return m_interactiveContext.rectEditMode() ==
        ZInteractiveContext::RECT_DRAW;
  case STATE_LEFT_BUTTON_PRESSED:
    return m_mouseLeftButtonPressed;
  case STATE_RIGHT_BUTTON_PRESSED:
    return m_mouseRightButtonPressed;
  case STATE_MOVE_OBJECT:
    if (m_interactionHandler != NULL) {
      return m_interactionHandler->isMovingObjects();
    }
    break;
  case STATE_SWC_EXTEND:
    return m_interactiveContext.swcEditMode() ==
        ZInteractiveContext::SWC_EDIT_EXTEND;
  case STATE_SWC_SMART_EXTEND:
    return m_interactiveContext.swcEditMode() ==
        ZInteractiveContext::SWC_EDIT_SMART_EXTEND;
  case STATE_SWC_CONNECT:
    return m_interactiveContext.swcEditMode() ==
        ZInteractiveContext::SWC_EDIT_CONNECT;
  case STATE_SWC_ADD_NODE:
    return m_interactiveContext.swcEditMode() ==
        ZInteractiveContext::SWC_EDIT_ADD_NODE;
  case STATE_SWC_SELECTION:
    return m_keyMode == KM_SWC_SELECTION;
  case STATE_LOCATE:
    return m_interactiveContext.exploreMode() ==
        ZInteractiveContext::EXPLORE_LOCAL;
  case STATE_BROWSE:
    return m_interactiveContext.exploreMode() ==
        ZInteractiveContext::EXPLORE_EXTERNALLY;
  case STATE_SHOW_DETAIL:
    return m_interactiveContext.exploreMode() ==
        ZInteractiveContext::EXPLORE_DETAIL;
  }

  return false;
}

Qt::CursorShape ZInteractionEngine::getCursorShape() const
{
  if (isStateOn(STATE_DRAW_STROKE)) {
    return Qt::PointingHandCursor;
  } else if (isStateOn(STATE_MOVE_OBJECT)) {
    return Qt::ClosedHandCursor;
  } else if (isStateOn(STATE_SWC_EXTEND) || isStateOn(STATE_SWC_SMART_EXTEND)) {
    return Qt::PointingHandCursor;
  } else if (isStateOn(STATE_SWC_CONNECT)) {
    return Qt::SizeBDiagCursor;
  } else if (isStateOn(STATE_SWC_ADD_NODE)) {
    return Qt::PointingHandCursor;
  } else if (isStateOn(STATE_DRAW_RECT)) {
    return Qt::PointingHandCursor;
  }

  return Qt::ArrowCursor;
}

void ZInteractionEngine::suppressMouseRelease(bool s)
{
  m_mouseReleaseSuppressed = s;
}

bool ZInteractionEngine::mouseReleaseSuppressed() const
{
  return m_mouseReleaseSuppressed;
}

