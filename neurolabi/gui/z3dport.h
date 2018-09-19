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

#ifndef Z3DPORT_H
#define Z3DPORT_H

#include "z3dfilter.h"
//#include "QsLog.h"
#include <vector>

class Z3DOutputPortBase;

class Z3DInputPortBase
{
public:
  Z3DInputPortBase(const QString& name, bool allowMultipleConnections,
                   Z3DFilter* filter,
                   Z3DFilter::State invalidationState = Z3DFilter::State::AllResultInvalid);

  virtual ~Z3DInputPortBase();

  // return the filter this port belongs to.
  Z3DFilter* filter() const
  { return m_filter; }

  QString name() const
  { return m_name; }

  // invalidate filter with the given State and set hasChanged=true.
  void invalidate();

  // has the data in this port changed since the last process() call?
  bool hasChanged() const
  { return m_hasChanged; }

  // mark the port as valid.
  void setValid()
  { m_hasChanged = false; }

  const std::vector<Z3DOutputPortBase*> connected() const
  { return m_connectedOutputPorts; }

  size_t numConnections() const
  { return m_connectedOutputPorts.size(); }

  bool isConnected() const
  { return !m_connectedOutputPorts.empty(); }

  bool isConnectedTo(const Z3DOutputPortBase* port) const;

  // return true if the port is connected and contains valid data.
  virtual bool isReady() const = 0;

  bool connect(Z3DOutputPortBase* outport);

  void disconnect(Z3DOutputPortBase* outport);

  void disconnectAll();

  void setExpectedSize(const glm::uvec2& size)
  { m_expectedSize = size; }

  glm::uvec2 expectedSize() const
  { return m_expectedSize; }

protected:
  friend class Z3DOutputPortBase;

  QString m_name;
  bool m_allowMultipleConnections;
  Z3DFilter* m_filter;

  // how changes from this port affect its filter
  Z3DFilter::State m_invalidationState;

  std::vector<Z3DOutputPortBase*> m_connectedOutputPorts;
  bool m_hasChanged;

  glm::uvec2 m_expectedSize;
};

class Z3DOutputPortBase
{
public:
  Z3DOutputPortBase(const QString& name, Z3DFilter* filter);

  virtual ~Z3DOutputPortBase();

  // return the filter this port belongs to.
  Z3DFilter* filter() const
  { return m_filter; }

  QString name() const
  { return m_name; }

  const std::vector<Z3DInputPortBase*> connected() const
  { return m_connectedInputPorts; }

  // test if this outport can connect to a given inport.
  virtual bool canConnectTo(const Z3DInputPortBase* inport) const;

  // invalidate all connected inports.
  virtual void invalidate();

  size_t numConnections() const
  { return m_connectedInputPorts.size(); }

  bool isConnected() const
  { return !m_connectedInputPorts.empty(); }

  bool isConnectedTo(const Z3DInputPortBase* port) const;

  // returns whether the port is ready to be used by its owning filter.
  // return true if the port is connected.
  virtual bool isReady() const
  { return isConnected(); }

  // return true if this output port contains valid data
  virtual bool hasValidData() const = 0;

  bool connect(Z3DInputPortBase* inport);

  void disconnect(Z3DInputPortBase* inport);

  void disconnectAll();

  glm::uvec2 size() const
  { return m_size; }

  // return the maximum of expectesize of all connected inports.
  // If no inport connected, return (0, 0)
  glm::uvec2 expectedSize() const;

  virtual void resize(const glm::uvec2& newsize)
  { m_size = newsize; }

protected:
  QString m_name;
  Z3DFilter* m_filter;

  std::vector<Z3DInputPortBase*> m_connectedInputPorts;

  glm::uvec2 m_size;
};

template<typename T>
class Z3DFilterInputPort : public Z3DInputPortBase
{
public:
  Z3DFilterInputPort(const QString& name, bool allowMultipleConnections,
                     Z3DFilter* filter,
                     Z3DFilter::State invalidationState = Z3DFilter::State::AllResultInvalid)
    : Z3DInputPortBase(name, allowMultipleConnections, filter, invalidationState)
  {}

  std::vector<T*> connectedFilters() const
  {
    std::vector<T*> filters;
    for (size_t i = 0; i < m_connectedOutputPorts.size(); ++i) {
      T* p = static_cast<T*>(m_connectedOutputPorts[i]->filter());
      filters.push_back(p);
    }
    return filters;
  }

  T* firstConnectedFilter() const
  {
    if (isConnected())
      return static_cast<T*>(m_connectedOutputPorts[0]->filter());
    else
      return 0;
  }

  virtual bool isReady() const override
  { return isConnected(); }
};

template<typename T>
class Z3DFilterOutputPort : public Z3DOutputPortBase
{
public:
  Z3DFilterOutputPort(const QString& name, Z3DFilter* filter)
    : Z3DOutputPortBase(name, filter)
  {}

  virtual bool canConnectTo(const Z3DInputPortBase* inport) const override
  {
    if (dynamic_cast<const Z3DFilterInputPort<T>*>(inport)) {
      return Z3DOutputPortBase::canConnectTo(inport);
    } else {
      return false;
    }
  }

  // data is filter itself, so it is always valid
  virtual bool hasValidData() const override
  { return true; }
};


#endif // Z3DPORT_H
