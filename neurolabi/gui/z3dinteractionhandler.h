#ifndef Z3DINTERACTIONHANDLER_H
#define Z3DINTERACTIONHANDLER_H

#include <QObject>
#include <memory>

#include "zeventlistenerparameter.h"
#include "zglmutils.h"


class Z3DInteractionHandler : public QObject
{
Q_OBJECT
public:
  explicit Z3DInteractionHandler(const QString& name, QObject* parent = nullptr);

  void setName(const QString& name)
  { m_name = name; }

  inline QString name() const
  { return m_name; }

  // Default to 1.0. Set it to a different value to emphasize or de-emphasize the action triggered by
  // mouse wheel motion.
  void setMouseWheelMotionFactor(float f)
  { m_mouseWheelMotionFactor = f; }

  float mouseWheelMotionFactor() const
  { return m_mouseWheelMotionFactor; }

  void setEnabled(bool enabled);

  void setVisible(bool state);

  void setSharing(bool sharing);

  void onEvent(QEvent* e, int w, int h);

signals:
  void cameraMoved();

  void mousePressed();

  void mouseReleased();

protected:
  enum class State
  {
    None,
    PreRotate,
    Rotate,
    Roll,
    Dolly,
    Zoom,
    Shift
  };

  void setState(State state)
  { m_lastState = m_state; m_state = state; }

  void addEventListener(ZEventListenerParameter* eventListener)
  { m_eventListeners.emplace_back(eventListener); }

  bool isStateToggledOn(State state) const;
  bool isStateToggledOff(State state) const;

  void updateLastState() {
    m_lastState = m_state;
  }

protected:
  QString m_name;
  std::vector<std::unique_ptr<ZEventListenerParameter>> m_eventListeners;
  State m_state = State::None;
  State m_lastState = State::None;

  float m_mouseWheelMotionFactor;
};

class Z3DCameraParameter;

class Z3DTrackballInteractionHandler : public Z3DInteractionHandler
{
Q_OBJECT
public:
  Z3DTrackballInteractionHandler(const QString& name, Z3DCameraParameter* camera, QObject* parent = nullptr);

  Z3DCameraParameter* camera() const
  { return m_camera; }

  void setCamera(Z3DCameraParameter* camera)
  { m_camera = camera; }

  // dolly in on wheel up and out on wheel down when true (default), otherwise when false
  void setMouseWheelUpDollyIn(bool b)
  { m_mouseWheelUpDollyIn = b; }

  // roll left on wheel up and right on wheel down when true (default), otherwise when false
  void setMouseWheelUpRollLeft(bool b)
  { m_mouseWheelUpRollLeft = b; }

  // more sensitive (bigger motion) at bigger mouse motion factor
  void setMouseMotionFactor(float f)
  { m_mouseMotionFactor = f; }

  float mouseMotionFactor() const
  { return m_mouseMotionFactor; }

  // angle per key press
  void setKeyPressAngle(float angle)
  { m_keyPressAngle = angle; }

  float keyPressAngle() const
  { return m_keyPressAngle; }

  // distance per key press in pixels
  void setKeyPressDistance(int d)
  { m_keyPressDistance = d; }

  int keyPressDistance() const
  { return m_keyPressDistance; }

  void setMoveObjects(bool v)
  { m_moveObjects = v; }

  bool isMovingObjects() const
  { return m_moveObjects; }

signals:

  void objectsMoved(double x, double y, double z);
  void cameraRotated();

protected:
  void rotateEvent(QMouseEvent* e, int w, int h);

  void mouseDollyEvent(QMouseEvent* e, int w, int h);

  void dollyEvent(QWheelEvent* e, int w, int h);

  void shiftEvent(QMouseEvent* e, int w, int h);

  void mouseRollEvent(QMouseEvent* e, int w, int h);

  void wheelRollEvent(QWheelEvent* e, int w, int h);

  void keyRotateEvent(QKeyEvent* e, int w, int h);

  void keyShiftEvent(QKeyEvent* e, int w, int h);

  void keyDollyEvent(QKeyEvent* e, int w, int h);

  void keyRollEvent(QKeyEvent* e, int w, int h);

  void mousePressEvent(QMouseEvent* e, int w, int h);

  void mouseReleaseEvent(QMouseEvent* e, int w, int h);

  void mouseMoveEvent(QMouseEvent* e, int w, int h);

  void wheelEvent(QWheelEvent* e, int w, int h);

  // convert screen space move to world space move and move camera eye and center
  void shift(const glm::ivec2& mouseStart, const glm::ivec2& mouseEnd, int w, int h);

  // use mouse move and mouseMotionFactor to calculate angle then rotate camera
  void rotate(const glm::ivec2& mouseStart, const glm::ivec2& mouseEnd, int w, int h);

  void roll(const glm::ivec2& mouseStart, const glm::ivec2& mouseEnd, int w, int h);

  void dolly(const glm::ivec2& mouseStart, const glm::ivec2& mouseEnd, int w, int h, float centerDistStart);

protected:
  ZEventListenerParameter* m_rotateEvent;
  ZEventListenerParameter* m_shiftEvent;
  ZEventListenerParameter* m_mouseDollyEvent;
  ZEventListenerParameter* m_wheelDollyEvent;
  ZEventListenerParameter* m_rollEvent;
  ZEventListenerParameter* m_keyRotateEvent;
  ZEventListenerParameter* m_keyShiftEvent;
  ZEventListenerParameter* m_keyDollyEvent;
  ZEventListenerParameter* m_keyRollEvent;

  Z3DCameraParameter* m_camera;
  glm::ivec2 m_lastMousePosition;
  float m_lastCenterDistance;

  bool m_mouseWheelUpDollyIn;
  bool m_mouseWheelUpRollLeft;

  // sensitivity of the interactor to mouse motion.
  float m_mouseMotionFactor;
  // sensitivity of the interactor to key press
  float m_keyPressAngle;
  int m_keyPressDistance;

  bool m_moveObjects;

  int m_delta;
};

#endif // Z3DINTERACTIONHANDLER_H
