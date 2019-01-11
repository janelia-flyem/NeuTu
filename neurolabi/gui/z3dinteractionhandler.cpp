#include "z3dinteractionhandler.h"

#include <boost/math/constants/constants.hpp>

#include "z3dcameraparameter.h"
#include "zqslog.h"
#include "logging/zlog.h"

Z3DInteractionHandler::Z3DInteractionHandler(const QString& name, QObject* parent)
  : QObject(parent)
  , m_name(name)
  , m_state(State::None)
  , m_mouseWheelMotionFactor(1.f)
{
}

void Z3DInteractionHandler::setEnabled(bool enabled)
{
  for (const auto& l : m_eventListeners)
    l->setEnabled(enabled);
}

void Z3DInteractionHandler::setVisible(bool state)
{
  for (const auto& l : m_eventListeners)
    l->setVisible(state);
}

void Z3DInteractionHandler::setSharing(bool sharing)
{
  for (const auto& l : m_eventListeners)
    l->setSharing(sharing);
}

void Z3DInteractionHandler::onEvent(QEvent* e, int w, int h)
{
  for (size_t j = 0; j < m_eventListeners.size() && !e->isAccepted(); ++j) {
    m_eventListeners[j]->sendEvent(e, w, h);
  }
}

bool Z3DInteractionHandler::isStateToggledOn(State state) const
{
  return (m_state == state) && (m_state != m_lastState);
}

bool Z3DInteractionHandler::isStateToggledOff(State state) const
{
  return (m_lastState == state) && (m_state != m_lastState);
}

//////////////////////////////////////////////////////////////////////////////////


Z3DTrackballInteractionHandler::Z3DTrackballInteractionHandler(const QString& name, Z3DCameraParameter* camera,
                                                               QObject* parent)
  : Z3DInteractionHandler(name, parent)
  , m_camera(camera)
  , m_mouseWheelUpDollyIn(true)
  , m_mouseWheelUpRollLeft(true)
  , m_mouseMotionFactor(10.f)
  , m_keyPressAngle(glm::radians(10.f))
  , m_keyPressDistance(10.f)
  , m_moveObjects(false)
  , m_delta(0)
{
  m_rotateEvent = new ZEventListenerParameter(name + " Rotate");
  m_rotateEvent->listenTo("rotate", Qt::LeftButton, Qt::NoModifier, QEvent::MouseButtonPress);
  m_rotateEvent->listenTo("rotate", Qt::LeftButton, Qt::NoModifier, QEvent::MouseButtonRelease);
  m_rotateEvent->listenTo("rotate", Qt::LeftButton, Qt::NoModifier, QEvent::MouseMove);
  connect(m_rotateEvent, &ZEventListenerParameter::mouseEventTriggered, this,
          &Z3DTrackballInteractionHandler::rotateEvent);
  addEventListener(m_rotateEvent);

  m_shiftEvent = new ZEventListenerParameter(name + " Shift");
  m_shiftEvent->listenTo("shift", Qt::LeftButton, Qt::ShiftModifier, QEvent::MouseButtonPress);
  m_shiftEvent->listenTo("shift", Qt::LeftButton, Qt::ShiftModifier, QEvent::MouseButtonRelease);
  m_shiftEvent->listenTo("shift", Qt::LeftButton, Qt::ShiftModifier, QEvent::MouseMove);
  connect(m_shiftEvent, &ZEventListenerParameter::mouseEventTriggered, this,
          &Z3DTrackballInteractionHandler::shiftEvent);
  addEventListener(m_shiftEvent);

  m_mouseDollyEvent = new ZEventListenerParameter(name + " Mouse Dolly");
  m_mouseDollyEvent->listenTo("mouse dolly", Qt::RightButton, Qt::NoModifier, QEvent::MouseButtonPress);
  m_mouseDollyEvent->listenTo("mouse dolly", Qt::RightButton, Qt::NoModifier, QEvent::MouseButtonRelease);
  m_mouseDollyEvent->listenTo("mouse dolly", Qt::RightButton, Qt::NoModifier, QEvent::MouseMove);
#ifdef __APPLE__
  m_mouseDollyEvent->listenTo("mouse dolly", Qt::RightButton, Qt::MetaModifier, QEvent::MouseButtonPress);
  m_mouseDollyEvent->listenTo("mouse dolly", Qt::RightButton, Qt::MetaModifier, QEvent::MouseButtonRelease);
  m_mouseDollyEvent->listenTo("mouse dolly", Qt::RightButton, Qt::MetaModifier, QEvent::MouseMove);
#endif
  connect(m_mouseDollyEvent, &ZEventListenerParameter::mouseEventTriggered, this,
          &Z3DTrackballInteractionHandler::mouseDollyEvent);
  addEventListener(m_mouseDollyEvent);

  m_wheelDollyEvent = new ZEventListenerParameter(name + " Wheel Dolly");
  m_wheelDollyEvent->listenTo("dolly", Qt::NoButton, Qt::NoModifier, QEvent::Wheel);
  connect(m_wheelDollyEvent, &ZEventListenerParameter::wheelEventTriggered, this,
          &Z3DTrackballInteractionHandler::dollyEvent);
  addEventListener(m_wheelDollyEvent);

  m_rollEvent = new ZEventListenerParameter(name + " Roll");
  m_rollEvent->listenTo("roll", Qt::LeftButton, Qt::AltModifier, QEvent::MouseButtonPress);
  m_rollEvent->listenTo("roll", Qt::LeftButton, Qt::AltModifier, QEvent::MouseButtonRelease);
  m_rollEvent->listenTo("roll", Qt::LeftButton, Qt::AltModifier, QEvent::MouseMove);
  m_rollEvent->listenTo("roll", Qt::NoButton, Qt::AltModifier, QEvent::Wheel);
  connect(m_rollEvent, &ZEventListenerParameter::mouseEventTriggered, this,
          &Z3DTrackballInteractionHandler::mouseRollEvent);
  connect(m_rollEvent, &ZEventListenerParameter::wheelEventTriggered, this,
          &Z3DTrackballInteractionHandler::wheelRollEvent);
  addEventListener(m_rollEvent);

  m_keyRotateEvent = new ZEventListenerParameter(name + " Key Rotate");
  m_keyRotateEvent->listenTo("rotate left", Qt::Key_Left, Qt::KeypadModifier, QEvent::KeyPress);
  m_keyRotateEvent->listenTo("rotate right", Qt::Key_Right, Qt::KeypadModifier, QEvent::KeyPress);
  m_keyRotateEvent->listenTo("rotate up", Qt::Key_Up, Qt::KeypadModifier, QEvent::KeyPress);
  m_keyRotateEvent->listenTo("rotate down", Qt::Key_Down, Qt::KeypadModifier, QEvent::KeyPress);
  connect(m_keyRotateEvent, &ZEventListenerParameter::keyEventTriggered, this,
          &Z3DTrackballInteractionHandler::keyRotateEvent);
  addEventListener(m_keyRotateEvent);

  m_keyShiftEvent = new ZEventListenerParameter(name + " Key Shift");
  m_keyShiftEvent->listenTo("shift left", Qt::Key_Left, Qt::ShiftModifier | Qt::KeypadModifier, QEvent::KeyPress);
  m_keyShiftEvent->listenTo("shift right", Qt::Key_Right, Qt::ShiftModifier | Qt::KeypadModifier, QEvent::KeyPress);
  m_keyShiftEvent->listenTo("shift up", Qt::Key_Up, Qt::ShiftModifier | Qt::KeypadModifier, QEvent::KeyPress);
  m_keyShiftEvent->listenTo("shift down", Qt::Key_Down, Qt::ShiftModifier | Qt::KeypadModifier, QEvent::KeyPress);
  connect(m_keyShiftEvent, &ZEventListenerParameter::keyEventTriggered, this,
          &Z3DTrackballInteractionHandler::keyShiftEvent);
  addEventListener(m_keyShiftEvent);

  m_keyDollyEvent = new ZEventListenerParameter(name + " Key Dolly");
  m_keyDollyEvent->listenTo("dolly in", Qt::Key_Equal, Qt::NoModifier, QEvent::KeyPress);
  m_keyDollyEvent->listenTo("dolly out", Qt::Key_Minus, Qt::NoModifier, QEvent::KeyPress);
  m_keyDollyEvent->listenTo("dolly in", Qt::Key_Up, Qt::ControlModifier | Qt::KeypadModifier, QEvent::KeyPress);
  m_keyDollyEvent->listenTo("dolly out", Qt::Key_Down, Qt::ControlModifier | Qt::KeypadModifier, QEvent::KeyPress);
  connect(m_keyDollyEvent, &ZEventListenerParameter::keyEventTriggered, this,
          &Z3DTrackballInteractionHandler::keyDollyEvent);
  addEventListener(m_keyDollyEvent);

  m_keyRollEvent = new ZEventListenerParameter(name + " Key Roll");
  m_keyRollEvent->listenTo("left", Qt::Key_Left, Qt::AltModifier | Qt::KeypadModifier, QEvent::KeyPress);
  m_keyRollEvent->listenTo("right", Qt::Key_Right, Qt::AltModifier | Qt::KeypadModifier, QEvent::KeyPress);
  connect(m_keyRollEvent, &ZEventListenerParameter::keyEventTriggered, this,
          &Z3DTrackballInteractionHandler::keyRollEvent);
  addEventListener(m_keyRollEvent);
}

void Z3DTrackballInteractionHandler::rotateEvent(QMouseEvent* e, int w, int h)
{
  if (e->type() == QEvent::MouseButtonPress) {
    setState(State::Rotate);
    mousePressEvent(e, w, h);
  } else if (e->type() == QEvent::MouseButtonRelease) {
    mouseReleaseEvent(e, w, h);
    if (isStateToggledOff(State::Rotate)) {
      KLOG << ZLog::Interact() << ZLog::Description("Stop rotating camera")
           << ZLog::Object(this);
    }
  } else if (e->type() == QEvent::MouseMove) {
    mouseMoveEvent(e, w, h);
    emit cameraMoved();
    emit cameraRotated();
    if (isStateToggledOn(State::Rotate)) {
      KLOG << ZLog::Interact() << ZLog::Description("Start rotating camera")
           << ZLog::Object(this);
    }

    updateLastState();
  }
}

void Z3DTrackballInteractionHandler::mouseDollyEvent(QMouseEvent* e, int w, int h)
{
  if (e->type() == QEvent::MouseButtonPress) {
    setState(State::Dolly);
    mousePressEvent(e, w, h);
  } else if (e->type() == QEvent::MouseButtonRelease) {
    mouseReleaseEvent(e, w, h);
  } else if (e->type() == QEvent::MouseMove) {
    mouseMoveEvent(e, w, h);
    emit cameraMoved();
  }
}

void Z3DTrackballInteractionHandler::dollyEvent(QWheelEvent* e, int w, int h)
{
  setState(State::Dolly);
  wheelEvent(e, w, h);
  emit cameraMoved();
}

void Z3DTrackballInteractionHandler::shiftEvent(QMouseEvent* e, int w, int h)
{
  if (e->type() == QEvent::MouseButtonPress) {
    setState(State::Shift);
    mousePressEvent(e, w, h);
  } else if (e->type() == QEvent::MouseButtonRelease) {
    mouseReleaseEvent(e, w, h);
  } else if (e->type() == QEvent::MouseMove) {
    mouseMoveEvent(e, w, h);
    emit cameraMoved();
  }
}

void Z3DTrackballInteractionHandler::mouseRollEvent(QMouseEvent* e, int w, int h)
{
  if (e->type() == QEvent::MouseButtonPress) {
    setState(State::Roll);
    mousePressEvent(e, w, h);
  } else if (e->type() == QEvent::MouseButtonRelease) {
    mouseReleaseEvent(e, w, h);
  } else if (e->type() == QEvent::MouseMove) {
    mouseMoveEvent(e, w, h);
    emit cameraMoved();
  }
}

void Z3DTrackballInteractionHandler::wheelRollEvent(QWheelEvent* e, int w, int h)
{
  setState(State::Roll);
  wheelEvent(e, w, h);
  emit cameraMoved();
}

void Z3DTrackballInteractionHandler::keyRotateEvent(QKeyEvent* e, int /*unused*/, int /*unused*/)
{
  bool accepted = false;
  if (e->key() == Qt::Key_Left) {
    m_camera->rotate(m_keyPressAngle, m_camera->get().vectorEyeToWorld(glm::vec3(0.f, 1.f, 0.f)));
    accepted = true;
  } else if (e->key() == Qt::Key_Right) {
    m_camera->rotate(-m_keyPressAngle, m_camera->get().vectorEyeToWorld(glm::vec3(0.f, 1.f, 0.f)));
    accepted = true;
  } else if (e->key() == Qt::Key_Up) {
    m_camera->rotate(-m_keyPressAngle, m_camera->get().vectorEyeToWorld(glm::vec3(1.f, 0.f, 0.f)));
    accepted = true;
  } else if (e->key() == Qt::Key_Down) {
    m_camera->rotate(m_keyPressAngle, m_camera->get().vectorEyeToWorld(glm::vec3(1.f, 0.f, 0.f)));
    accepted = true;
  }
  if (accepted) {
    e->accept();
    emit cameraMoved();
  }
}

void Z3DTrackballInteractionHandler::keyShiftEvent(QKeyEvent* e, int w, int h)
{
  bool accepted = false;
  glm::ivec2 center(w / 2, h / 2);
  if (e->key() == Qt::Key_Left) {
    shift(center, center + glm::ivec2(-m_keyPressDistance, 0), w, h);
    accepted = true;
  } else if (e->key() == Qt::Key_Right) {
    shift(center, center + glm::ivec2(m_keyPressDistance, 0), w, h);
    accepted = true;
  } else if (e->key() == Qt::Key_Up) {
    shift(center, center + glm::ivec2(0, m_keyPressDistance), w, h);
    accepted = true;
  } else if (e->key() == Qt::Key_Down) {
    shift(center, center + glm::ivec2(0, -m_keyPressDistance), w, h);
    accepted = true;
  }
  if (accepted) {
    e->accept();
    emit cameraMoved();
  }
}

void Z3DTrackballInteractionHandler::keyDollyEvent(QKeyEvent* e, int /*unused*/, int /*unused*/)
{
  bool accepted = false;
  float factor = m_mouseMotionFactor * 0.1f * m_mouseWheelMotionFactor;
  if (e->key() == Qt::Key_Up || e->key() == Qt::Key_Equal) {
    m_camera->dolly(std::pow(1.1f, factor));
    accepted = true;
  }
  else if (e->key() == Qt::Key_Down || e->key() == Qt::Key_Minus) {
    m_camera->dolly(std::pow(1.1f, -factor));
    accepted = true;
  }
  if (accepted) {
    e->accept();
    emit cameraMoved();
  }
}

void Z3DTrackballInteractionHandler::keyRollEvent(QKeyEvent* e, int /*unused*/, int /*unused*/)
{
  bool accepted = false;
  if (e->key() == Qt::Key_Left) {
    m_camera->roll(m_keyPressAngle);
    accepted = true;
  } else if (e->key() == Qt::Key_Right) {
    m_camera->roll(-m_keyPressAngle);
    accepted = true;
  }
  if (accepted) {
    e->accept();
    emit cameraMoved();
  }
}

void Z3DTrackballInteractionHandler::mousePressEvent(QMouseEvent* e, int /*unused*/, int h)
{
  emit mousePressed();
  m_lastMousePosition = glm::ivec2(e->x(), h - e->y());
  m_lastCenterDistance = m_camera->get().centerDist();
  e->ignore();
}

void Z3DTrackballInteractionHandler::mouseReleaseEvent(QMouseEvent* e, int /*unused*/, int /*unused*/)
{
  emit mouseReleased();
  setState(State::None);
  e->ignore();
}

void Z3DTrackballInteractionHandler::mouseMoveEvent(QMouseEvent* e, int w, int h)
{
  e->ignore();

  glm::ivec2 newMouse(e->x(), h - e->y());

  if (m_state == State::Rotate) {
    rotate(m_lastMousePosition, newMouse, w, h);
    e->accept();
    m_lastMousePosition = newMouse;
  } else if (m_state == State::Shift) {
    shift(m_lastMousePosition, newMouse, w, h);
    e->accept();
    m_lastMousePosition = newMouse;
  } else if (m_state == State::Roll) {
    roll(m_lastMousePosition, newMouse, w, h);
    e->accept();
    m_lastMousePosition = newMouse;
  } else if (m_state == State::Dolly) {
    dolly(m_lastMousePosition, newMouse, w, h, m_lastCenterDistance);
    e->accept();
  }
}

void Z3DTrackballInteractionHandler::wheelEvent(QWheelEvent* e, int /*unused*/, int /*unused*/)
{
  e->ignore();

#ifdef _NONEED_
  if (m_delta > 0 && e->delta() < 0)
    m_delta = e->delta();
  else if (m_delta < 0 && e->delta() > 0)
    m_delta = e->delta();
  else
    m_delta += e->delta();
  if (m_delta > -120 && m_delta < 120)
    return;
  if (m_state == DOLLY) {
    float factor = m_mouseMotionFactor * 0.2f * m_mouseWheelMotionFactor * std::abs(m_delta / 120);
    factor = std::min(8.f, factor);
    bool dollyIn = ( m_mouseWheelUpDollyIn && (m_delta > 0)) ||
        (!m_mouseWheelUpDollyIn && (m_delta < 0));
    if (!dollyIn)
      factor = -factor;
    m_camera->dolly(std::pow(1.1f, factor));
    e->accept();
  } else if (m_state == ROLL) {
    bool rollLeft = ( m_mouseWheelUpRollLeft && (m_delta > 0)) ||
        (!m_mouseWheelUpRollLeft && (m_delta < 0));
    if (rollLeft)
      m_camera->roll(m_keyPressAngle);
    else
      m_camera->roll(-m_keyPressAngle);
    e->accept();
  }
  setState(NONE);
  m_delta = 0;
#else
  if (e->delta() == 0)
    return;
  if (m_state == State::Dolly) {
    float factor = m_mouseMotionFactor * 0.2f * m_mouseWheelMotionFactor;
    bool dollyIn = (m_mouseWheelUpDollyIn && (e->delta() > 0)) ||
                   (!m_mouseWheelUpDollyIn && (e->delta() < 0));
    if (!dollyIn)
      factor = -factor;
    m_camera->dolly(std::pow(1.1f, factor));
    e->accept();
  } else if (m_state == State::Roll) {
    bool rollLeft = (m_mouseWheelUpRollLeft && (e->delta() > 0)) ||
                    (!m_mouseWheelUpRollLeft && (e->delta() < 0));
    if (rollLeft) {
      m_camera->roll(m_keyPressAngle);
    } else {
      m_camera->roll(-m_keyPressAngle);
    }
    e->accept();
  }
  setState(State::None);
#endif
}

void Z3DTrackballInteractionHandler::shift(const glm::ivec2& mouseStart, const glm::ivec2& mouseEnd, int w, int h)
{
  glm::ivec4 viewport(0, 0, w, h);
  float centerDepth = m_camera->get().worldToScreen(m_camera->get().center(), viewport).z;
  glm::vec3 startInWorld = m_camera->get().screenToWorld(glm::vec3(glm::vec2(mouseStart), centerDepth), viewport);
  glm::vec3 endInWorld = m_camera->get().screenToWorld(glm::vec3(glm::vec2(mouseEnd), centerDepth), viewport);
  glm::vec3 vec = endInWorld - startInWorld;
  if (m_moveObjects) {
    emit objectsMoved(vec.x, vec.y, vec.z);
  } else {
    // camera move in opposite direction
    m_camera->setCamera(m_camera->get().eye() - vec, m_camera->get().center() - vec);
  }
}

void Z3DTrackballInteractionHandler::rotate(const glm::ivec2& mouseStart, const glm::ivec2& mouseEnd, int w, int h)
{
  glm::ivec2 dPos = mouseEnd - mouseStart;

  double delta_elevation = -0.2 * boost::math::double_constants::pi / h;
  double delta_azimuth = -0.2 * boost::math::double_constants::pi / w;

  double rxf = dPos.x * delta_azimuth * m_mouseMotionFactor;
  double ryf = dPos.y * delta_elevation * m_mouseMotionFactor;

  m_camera->azimuth(rxf);
  m_camera->elevation(ryf);
}

void Z3DTrackballInteractionHandler::roll(const glm::ivec2& mouseStart, const glm::ivec2& mouseEnd, int w, int h)
{
  glm::dvec2 center(w / 2., h / 2.);
  double newAngle = std::atan2(mouseEnd.y - center.y, mouseEnd.x - center.x);
  double oldAngle = std::atan2(mouseStart.y - center.y, mouseStart.x - center.x);

  m_camera->roll(newAngle - oldAngle);
}

void
Z3DTrackballInteractionHandler::dolly(const glm::ivec2& mouseStart, const glm::ivec2& mouseEnd,
                                      int /*unused*/, int h, float centerDistStart)
{
  glm::ivec2 dPos = mouseEnd - mouseStart;

  double deltaFactor = 20.0 / h;
  float factor = dPos.y * deltaFactor;

  m_camera->dollyToCenterDistance(std::pow(1.1f, factor) * centerDistStart);
  //LOG(INFO) << centerDistStart << std::pow(1.1f, factor) * centerDistStart << factor
  // << dPos << deltaFactor << mouseStart << mouseEnd;
}
