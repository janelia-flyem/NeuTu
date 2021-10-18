/*
 * Copyright (C) 2005-2012 University of Muenster, Germany.
 * Visualization and Computer Graphics Group <http://viscg.uni-muenster.de>
 * For a list of authors please refer to the file "CREDITS.txt".
 * Copyright (C) 2012-2013 Korea Institiute of Science and Technologhy, Seoul.
 * Linqing Feng, Jinny Kim's lab <http://jinny.kist.re.kr>
 *
 * This file is derived from code of the free Voreen software package.
 * you can redistribute it and/or modify it under the terms of the GNU General
 * Public License version 2 as published by the Free Software Foundation.
 *
 * You should have received a copy of the GNU General Public License in the file
 * "LICENSE.txt" along with this file. If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef Z3DFILTER_H
#define Z3DFILTER_H

#include <QObject>
#include <map>
#include <set>
#include <vector>
#include <QString>

#include "z3dgl.h"
#include "z3dcanvaseventlistener.h"
#include "zflags.h"
#include "logging/zloggable.h"
#include "zwidgetmessage.h"

class Z3DInputPortBase;
class Z3DOutputPortBase;
class Z3DInteractionHandler;
class ZParameter;
class ZEventListenerParameter;
class Z3DRenderOutputPort;
class Z3DRenderTarget;
class Z3DTexture;
class Z3DShaderProgram;
class ZVertexArrayObject;
class QMouseEvent;

class Z3DFilter : public QObject, public Z3DCanvasEventListener,
    public ZLoggable
{
Q_OBJECT

  friend class Z3DNetworkEvaluator;

public:
  // specifies the invalidation status of the filter.
  // The networkEvaluator use this value to mark filters that has to be executed
  enum class State
  {
    Valid = 0,
    MonoViewResultInvalid = 1,
    LeftEyeResultInvalid = 1 << 1,
    RightEyeResultInvalid = 1 << 2,
    StereoResultInvalid = LeftEyeResultInvalid | RightEyeResultInvalid,
    AllResultInvalid = MonoViewResultInvalid | StereoResultInvalid
  };

  explicit Z3DFilter(QObject* parent = nullptr);

  QString className() const
  { return metaObject()->className(); }

  void setName(const QString& name)
  { m_name = name; }

  QString name() const
  { return m_name; }

  // returns all parameters
  const std::vector<ZParameter*>& parameters() const
  { return m_parameters; }

  std::vector<ZParameter*>& parameters()
  { return m_parameters; }

  // returns first parameter with the given name. return nullptr if not found
  ZParameter* parameter(const QString& name) const;

  virtual void invalidate(State inv = State::AllResultInvalid);

  // returns the port with the given name, or nullptr if such a port does not exist.
  Z3DInputPortBase* inputPort(const QString& name) const;

  Z3DOutputPortBase* outputPort(const QString& name) const;

  // return all inputports or outputports as vector
  const std::vector<Z3DInputPortBase*>& inputPorts() const
  { return m_inputPorts; }

  const std::vector<Z3DOutputPortBase*>& outputPorts() const
  { return m_outputPorts; }

  virtual void onEvent(QEvent* e, int w, int h) override;

  const std::vector<ZEventListenerParameter*> eventListeners() const
  { return m_eventListeners; }

  const std::vector<Z3DInteractionHandler*>& interactionHandlers() const
  { return m_interactionHandlers; }

  // removes all port connections
  void disconnectAllPorts();

  inline void invalidateResult()
  { invalidate(State::AllResultInvalid); }

signals:

  // emit this only if resize starts from current filter.
  void requestUpstreamSizeChange(Z3DFilter*);
  void messageGenerated(ZWidgetMessage);

protected:
  // mark that the output of current filter for certain eye is valid.
  // if process function (e.g. prepare data) is not related to stereo view or mono view, you should rewrite this
  // function in subclass and set the invalidstate to VALID to avoid being executed again for
  // a different eye parameter
  // this function will be called by networkevaluator after process(eye) is called
  virtual void setValid(Z3DEye eye);

  // return true if the output of current filter for certain eye is valid.
  // will be used by networkevalutor to decide whether is neccessary to call process(eye)
  virtual bool isValid(Z3DEye eye) const;

  // returns true if filter is ready to do rendering
  // The default implementation checks, whether the filter has been initialized and
  // all input ports and output ports are ready. This is not always necessary since not all
  // input or output ports are needed depending on rendering context.
  virtual bool isReady(Z3DEye eye) const;

  // this is the place to do rendering related work
  // the networkevaluator will sets its invalidation level to VALID after calling this
  // input is current camera (eye), can be left or right in stereo case
  virtual void process(Z3DEye eye) = 0;

  void addPort(Z3DInputPortBase& port);

  void addPort(Z3DOutputPortBase& port);

  void removePort(Z3DInputPortBase& port);

  void removePort(Z3DOutputPortBase& port);

  void addParameter(ZParameter& para, State inv = State::AllResultInvalid);

  void removeParameter(const ZParameter& para);

  // listen to some events
  void addEventListener(ZEventListenerParameter& para);

  // react to interaction
  void addInteractionHandler(Z3DInteractionHandler& handler);

  virtual void enterInteractionMode()
  {}

  virtual void exitInteractionMode()
  {}

  bool isInInteractionMode() const;

  void toggleInteractionMode(bool interactionMode, void* source);

  void addPrivateRenderPort(Z3DRenderOutputPort& port);

  void addPrivateRenderTarget(Z3DRenderTarget& target);

  static void renderScreenQuad(const ZVertexArrayObject& vao, const Z3DShaderProgram& shader);

  // 1. for each outport, get all expected size from all connected inports, and use the maximum one
  //    as the new size of the outport
  // 2. update private port size
  // 3. Once we get the newsize of all outports, we calculate a expected size for each inport and set it.
  //    default choice for inport expected size is the maximum new outport size
  // reimplement this if you want different behavior
  virtual void updateSize();

  void recordMousePosition(QMouseEvent *e);
  bool stayingMouse(QMouseEvent* e) const;

//  static bool StayingMouse(QMouseEvent* e, int x, int y) const;

protected:
  // used for the detection of duplicate port names.
  std::map<QString, Z3DInputPortBase*> m_inputPortMap;
  std::map<QString, Z3DOutputPortBase*> m_outputPortMap;

  State m_state;
  std::set<void*> m_interactionModeSources;

  QString m_name;

  // all parameters that can change the render behavior
  std::vector<ZParameter*> m_parameters;
  std::set<QString> m_parameterNames;

  // input the filter expects.
  std::vector<Z3DInputPortBase*> m_inputPorts;
  // output the filter generates.
  std::vector<Z3DOutputPortBase*> m_outputPorts;
  // private port for intermediate rendering
  std::vector<Z3DRenderOutputPort*> m_privateRenderPorts;
  std::vector<Z3DRenderTarget*> m_privateRenderTargets;

  std::vector<ZEventListenerParameter*> m_eventListeners;
  std::vector<Z3DInteractionHandler*> m_interactionHandlers;

  int m_mouseX = 0;
  int m_mouseY = 0;

  // used for cycle prevention during invalidation propagation
  bool m_invalidationVisited;
};

DECLARE_OPERATORS_FOR_ENUM(Z3DFilter::State)

#endif // Z3DFILTER_H
