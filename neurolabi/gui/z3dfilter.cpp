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

#include "z3dfilter.h"

#include <QWidget>

#include "logging/zqslog.h"
#include "logging/zlog.h"
#include "logging/utilities.h"

#include "neutubeconfig.h"
#include "zsysteminfo.h"
#include "z3dport.h"
#include "z3dinteractionhandler.h"
#include "z3dshaderprogram.h"
#include "zeventlistenerparameter.h"
#include "widgets/zparameter.h"
#include "z3drenderport.h"
#include "zvertexarrayobject.h"
//#include "z3dview.h"

Z3DFilter::Z3DFilter(QObject* parent)
  : QObject(parent)
  , m_state(State::AllResultInvalid)
  , m_invalidationVisited(false)
{
  setLogger(neutu::LogMessageF);
}

ZParameter* Z3DFilter::parameter(const QString& name) const
{
  for (ZParameter* para : m_parameters) {
    if (para->name() == name)
      return para;
  }
  return nullptr;
}

void Z3DFilter::invalidate(State inv)
{
  set_flag(m_state, inv);

  if (inv == State::Valid)
    return;

  if (!m_invalidationVisited) {
    m_invalidationVisited = true;

    for (auto port : m_outputPorts)
      port->invalidate();

    m_invalidationVisited = false;
  }
}

Z3DInputPortBase* Z3DFilter::inputPort(const QString& name) const
{
  for (auto port : m_inputPorts) {
    if (port->name() == name)
      return port;
  }

  return nullptr;
}

Z3DOutputPortBase* Z3DFilter::outputPort(const QString& name) const
{
  for (auto port : m_outputPorts) {
    if (port->name() == name)
      return port;
  }

  return nullptr;
}

void Z3DFilter::onEvent(QEvent* e, int w, int h)
{
  e->ignore();

  //LOG(WARNING) << e << " " << className();
  // propagate to interaction handlers
  for (size_t i = 0; i < m_interactionHandlers.size() && !e->isAccepted(); ++i) {
    m_interactionHandlers[i]->onEvent(e, w, h);
  }

  // propagate to event listeners
  for (size_t i = 0; (i < m_eventListeners.size()) && !e->isAccepted(); ++i)
    m_eventListeners[i]->sendEvent(e, w, h);
}

void Z3DFilter::disconnectAllPorts()
{
  for (auto port : m_inputPorts) {
    port->disconnectAll();
  }

  for (auto port : m_outputPorts) {
    port->disconnectAll();
  }
}

void Z3DFilter::setValid(Z3DEye eye)
{
  if (eye == Z3DEye::Mono)
    reset_flag(m_state, State::MonoViewResultInvalid);
  else if (eye == Z3DEye::Left)
    reset_flag(m_state, State::LeftEyeResultInvalid);
  else
    reset_flag(m_state, State::RightEyeResultInvalid);

  for (auto port : m_inputPorts)
    port->setValid();
}

bool Z3DFilter::isValid(Z3DEye eye) const
{
  if (eye == Z3DEye::Mono)
    return !is_flag_set(m_state, State::MonoViewResultInvalid);
  else if (eye == Z3DEye::Left)
    return !is_flag_set(m_state, State::LeftEyeResultInvalid);
  else
    return !is_flag_set(m_state, State::RightEyeResultInvalid);
}

bool Z3DFilter::isReady(Z3DEye /*unused*/) const
{
  for (auto port : m_inputPorts)
    if (!port->isReady())
      return false;

  for (auto port : m_outputPorts)
    if (!port->isReady())
      return false;

  return true;
}

void Z3DFilter::addPort(Z3DInputPortBase& port)
{
  m_inputPorts.push_back(&port);

  std::map<QString, Z3DInputPortBase*>::const_iterator it = m_inputPortMap.find(port.name());
  if (it == m_inputPortMap.end())
    m_inputPortMap.emplace(port.name(), &port);
  else {
    LOG(FATAL) << className() << " port " << port.name() << " has already been inserted!";
  }
}

void Z3DFilter::addPort(Z3DOutputPortBase& port)
{
  m_outputPorts.push_back(&port);
  std::map<QString, Z3DOutputPortBase*>::const_iterator it = m_outputPortMap.find(port.name());
  if (it == m_outputPortMap.end())
    m_outputPortMap.emplace(port.name(), &port);
  else {
    LOG(FATAL) << className() << " port " << port.name() << " has already been inserted!";
  }
}

void Z3DFilter::removePort(Z3DInputPortBase& port)
{
  m_inputPorts.erase(std::find(m_inputPorts.begin(), m_inputPorts.end(), &port));

  std::map<QString, Z3DInputPortBase*>::iterator inIt = m_inputPortMap.find(port.name());
  if (inIt != m_inputPortMap.end())
    m_inputPortMap.erase(inIt);
  else {
    LOG(FATAL) << className() << " port " << port.name() << " was not found!";
  }
}

void Z3DFilter::removePort(Z3DOutputPortBase& port)
{
  m_outputPorts.erase(std::find(m_outputPorts.begin(), m_outputPorts.end(), &port));

  std::map<QString, Z3DOutputPortBase*>::iterator outIt = m_outputPortMap.find(port.name());
  if (outIt != m_outputPortMap.end())
    m_outputPortMap.erase(outIt);
  else {
    LOG(FATAL) << className() << " port " << port.name() << " was not found!";
  }
}

void Z3DFilter::addParameter(ZParameter& para, State inv)
{
  ZOUT(LTRACE(), 5) << "Adding" << &para << para.name();
  if (m_parameterNames.find(para.name()) != m_parameterNames.end()) {
    LOG(FATAL) << "Duplicated para name " << para.name();
  }
  m_parameters.push_back(&para);
  m_parameterNames.insert(para.name());
  if (inv != State::Valid) {
    connect(&para, &ZParameter::valueChanged, this, &Z3DFilter::invalidateResult);
  }
}

void Z3DFilter::removeParameter(const ZParameter &para)
{
  ZOUT(LTRACE(), 5) << "Removing" << &para << para.name();
  if (!parameter(para.name())) {
    LOG(ERROR) << className() << " parameter " << para.name() << " cannot be removed, it does not exist";
  } else {
    para.disconnect(this);
    m_parameters.erase(std::find(m_parameters.begin(), m_parameters.end(), &para));
    m_parameterNames.erase(para.name());
    ZOUT(LTRACE(), 5) << "Removed";
  }
}

void Z3DFilter::addEventListener(ZEventListenerParameter& para)
{
  addParameter(para);
  m_eventListeners.push_back(&para);
}

void Z3DFilter::addInteractionHandler(Z3DInteractionHandler& handler)
{
  m_interactionHandlers.push_back(&handler);
}

bool Z3DFilter::isInInteractionMode() const
{
  return (!m_interactionModeSources.empty());
}

void Z3DFilter::toggleInteractionMode(bool interactionMode, void* source)
{
  if (interactionMode) {
    if (m_interactionModeSources.find(source) == m_interactionModeSources.end()) {

      m_interactionModeSources.insert(source);

      if (m_interactionModeSources.size() == 1)
        enterInteractionMode();
    }
  } else {
    if (m_interactionModeSources.find(source) != m_interactionModeSources.end()) {

      m_interactionModeSources.erase(source);

      if (m_interactionModeSources.empty())
        exitInteractionMode();
    }
  }
}

void Z3DFilter::addPrivateRenderPort(Z3DRenderOutputPort& port)
{
  m_privateRenderPorts.push_back(&port);

  std::map<QString, Z3DOutputPortBase*>::const_iterator it = m_outputPortMap.find(port.name());
  if (it == m_outputPortMap.end())
    m_outputPortMap.emplace(port.name(), &port);
  else {
    LOG(FATAL) << className() << " port " << port.name() << " has already been inserted!";
  }
}

void Z3DFilter::addPrivateRenderTarget(Z3DRenderTarget& target)
{
  m_privateRenderTargets.push_back(&target);
}

void Z3DFilter::renderScreenQuad(const ZVertexArrayObject& vao, const Z3DShaderProgram& shader)
{
  if (!shader.isLinked())
    return;

  glDepthFunc(GL_ALWAYS);

  vao.bind();

  const GLfloat vertices[] = {-1.f, 1.f, 0.f, //top left corner
                              -1.f, -1.f, 0.f, //bottom left corner
                              1.f, 1.f, 0.f, //top right corner
                              1.f, -1.f, 0.f}; // bottom right rocner
  GLint attr_vertex = shader.vertexAttributeLocation();

  GLuint bufObjects[1];
  glGenBuffers(1, bufObjects);

  glEnableVertexAttribArray(attr_vertex);
  glBindBuffer(GL_ARRAY_BUFFER, bufObjects[0]);
  glBufferData(GL_ARRAY_BUFFER, 3 * 4 * sizeof(GLfloat), vertices, GL_STATIC_DRAW);
  glVertexAttribPointer(attr_vertex, 3, GL_FLOAT, GL_FALSE, 0, nullptr);

  glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

  glBindBuffer(GL_ARRAY_BUFFER, 0);
  glDeleteBuffers(1, bufObjects);

  glDisableVertexAttribArray(attr_vertex);

  vao.release();

  glDepthFunc(GL_LESS);
}

void Z3DFilter::updateSize()
{
  // 1. update outport size
  glm::uvec2 maxOutportSize(0, 0);
  for (auto port : m_outputPorts) {
    glm::uvec2 outportSize = port->expectedSize();
    if (outportSize.x > 0 && outportSize != port->size()) {
      port->resize(outportSize);
    }

    maxOutportSize = glm::max(maxOutportSize, port->size());
  }

  // 2. update private ports
  for (auto port : m_privateRenderPorts) {
    port->resize(maxOutportSize);
  }
  for (auto target : m_privateRenderTargets) {
    target->resize(maxOutportSize);
  }

  // 3. update inport expected size
  for (auto port : m_inputPorts) {
    port->setExpectedSize(maxOutportSize);
  }

  invalidate();
}

void Z3DFilter::recordMousePosition(QMouseEvent *e)
{
  m_mouseX = e->x();
  m_mouseY = e->y();
}

bool Z3DFilter::stayingMouse(QMouseEvent *e) const
{
  return std::abs(e->x() - m_mouseX) < 2 && std::abs(e->y() - m_mouseY) < 2;
}
