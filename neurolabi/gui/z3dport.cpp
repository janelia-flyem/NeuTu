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

#include "z3dport.h"

#include "logging/zqslog.h"

Z3DInputPortBase::Z3DInputPortBase(const QString& name, bool allowMultipleConnections, Z3DFilter* filter,
                                   Z3DFilter::State invalidationState)
  : m_name(name)
  , m_allowMultipleConnections(allowMultipleConnections)
  , m_filter(filter)
  , m_invalidationState(invalidationState)
  , m_expectedSize(0)
{
  CHECK(filter);
}

Z3DInputPortBase::~Z3DInputPortBase()
{
  disconnectAll();
}

void Z3DInputPortBase::invalidate()
{
  m_hasChanged = true;
  filter()->invalidate(m_invalidationState);
}

bool Z3DInputPortBase::isConnectedTo(const Z3DOutputPortBase* port) const
{
  return std::find(m_connectedOutputPorts.begin(), m_connectedOutputPorts.end(), port)
         != m_connectedOutputPorts.end();
}

bool Z3DInputPortBase::connect(Z3DOutputPortBase* outport)
{
  return outport->connect(this);
}

void Z3DInputPortBase::disconnect(Z3DOutputPortBase* outport)
{
  for (size_t i = 0; i < m_connectedOutputPorts.size(); ++i) {
    if (m_connectedOutputPorts[i] == outport) {
      m_connectedOutputPorts.erase(m_connectedOutputPorts.begin() + i);
      outport->disconnect(this);
      return;
    }
  }
}

void Z3DInputPortBase::disconnectAll()
{
  while (!m_connectedOutputPorts.empty()) {
    m_connectedOutputPorts[0]->disconnect(this);
  }
}

Z3DOutputPortBase::Z3DOutputPortBase(const QString& name, Z3DFilter* filter)
  : m_name(name)
  , m_filter(filter)
  , m_size(32, 32)
{
  CHECK(filter);
}

Z3DOutputPortBase::~Z3DOutputPortBase()
{
  disconnectAll();
}

bool Z3DOutputPortBase::canConnectTo(const Z3DInputPortBase* inport) const
{
  if (!inport)
    return false;

  if (isConnectedTo(inport))
    return false;

  if (!inport->m_allowMultipleConnections && inport->isConnected())
    return false;

  if (filter() == inport->filter())
    return false;

  return true;
}

void Z3DOutputPortBase::invalidate()
{
  for (size_t i = 0; i < m_connectedInputPorts.size(); ++i)
    m_connectedInputPorts[i]->invalidate();
}

bool Z3DOutputPortBase::isConnectedTo(const Z3DInputPortBase* port) const
{
  return std::find(m_connectedInputPorts.begin(), m_connectedInputPorts.end(), port)
         != m_connectedInputPorts.end();
}

bool Z3DOutputPortBase::connect(Z3DInputPortBase* inport)
{
  if (canConnectTo(inport)) {
    m_connectedInputPorts.push_back(inport);
    inport->m_connectedOutputPorts.push_back(this);
    inport->invalidate();
    return true;
  }
  LOG(ERROR) << "Inport " << inport->name() << " of " << inport->filter()->className()
             << " can not be connected to outport " << m_name << " of " << m_filter->className();
  return false;
}

void Z3DOutputPortBase::disconnect(Z3DInputPortBase* inport)
{
  for (size_t i = 0; i < m_connectedInputPorts.size(); ++i) {
    if (m_connectedInputPorts[i] == inport) {
      m_connectedInputPorts.erase(m_connectedInputPorts.begin() + i);
      inport->disconnect(this);
      invalidate();
      return;
    }
  }
}

void Z3DOutputPortBase::disconnectAll()
{
  while (!m_connectedInputPorts.empty()) {
    m_connectedInputPorts[0]->disconnect(this);
  }
}

glm::uvec2 Z3DOutputPortBase::expectedSize() const
{
  glm::uvec2 result(0, 0);
  for (size_t j = 0; j < m_connectedInputPorts.size(); ++j) {
    result = glm::max(result, m_connectedInputPorts[j]->expectedSize());
  }
  return result;
}







