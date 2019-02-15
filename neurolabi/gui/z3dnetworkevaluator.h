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

#ifndef Z3DNETWORKEVALUATOR_H
#define Z3DNETWORKEVALUATOR_H

#include "z3dcanvas.h"
#include "logging/zbenchtimer.h"
#include <QObject>
#ifndef Q_MOC_RUN
#include <boost/graph/adjacency_list.hpp>
#endif
#include <vector>

class Z3DFilter;

class Z3DCanvasPainter;

class Z3DOutputPortBase;

class Z3DInputPortBase;

class Z3DFilterWrapper
{
public:
  virtual ~Z3DFilterWrapper() = default;

  virtual void beforeFilterProcess(const Z3DFilter* /*unused*/)
  {}

  virtual void afterFilterProcess(const Z3DFilter* /*unused*/)
  {}

  virtual void beforeNetworkProcess()
  {}

  virtual void afterNetworkProcess()
  {}
};

class Z3DNetworkEvaluator : public QObject
{
Q_OBJECT

public:
  explicit Z3DNetworkEvaluator(Z3DCanvasPainter& canvasPainter, QObject* parent = nullptr);

  ~Z3DNetworkEvaluator();

  // process the currently assigned network. The rendering order is determined internally
  // according the network topology and the invalidation levels of the filters.
  // stereo means run two passes for left and right eye
  void process(bool stereo = false);

  // call when network topology changed
  void updateNetwork();

protected:
  // Locks the evaluator. In this state, it does not perform
  // any operations, such as initializing or processing, on the filter network
  void lock()
  { m_locked = true; }

  void unlock()
  { m_locked = false; }

  // update size of all upstream filters. If input filter is nullptr, update all filters
  void sizeChangedFromFilter(Z3DFilter* rp = nullptr);

private:
  std::vector<Z3DFilter*> m_renderingOrder;

  std::vector<std::unique_ptr<Z3DFilterWrapper>> m_filterWrappers;

  bool m_locked;

  bool m_processPending;

  Z3DCanvasPainter& m_canvasPainter;

  struct VertexInfo
  {
    VertexInfo() : filter(nullptr)
    {}

    explicit VertexInfo(Z3DFilter* p) : filter(p)
    {}

    Z3DFilter* filter;
    //
  };

  struct EdgeInfo
  {
    EdgeInfo(Z3DOutputPortBase* out, Z3DInputPortBase* in) : outPort(out), inPort(in)
    {}

    Z3DOutputPortBase* outPort;
    Z3DInputPortBase* inPort;
  };

  using GraphT = boost::adjacency_list<boost::listS, boost::vecS, boost::bidirectionalS, VertexInfo, EdgeInfo>;
  using Vertex = boost::graph_traits<GraphT>::vertex_descriptor;
  using Edge = boost::graph_traits<GraphT>::edge_descriptor;
  std::map<Z3DFilter*, Vertex> m_filterToVertexMapper;
  GraphT m_filterGraph;

  std::vector<Z3DFilter*> m_reverseSortedFilters;
};

// check if OpenGL state conforms to default settings. Log a warning message if not.
class Z3DCheckOpenGLStateFilterWrapper : public Z3DFilterWrapper
{
public:
  void afterFilterProcess(const Z3DFilter* p) override;

  void beforeNetworkProcess() override;

private:
  void checkState(const Z3DFilter* p = nullptr);

  void warn(const Z3DFilter* p, const char* message);
};

// profile each filter and whole network
class Z3DProfileFilterWrapper : public Z3DFilterWrapper
{
  ZBenchTimer m_benchTimer;
public:
  void beforeFilterProcess(const Z3DFilter* /*unused*/) override;

  void afterFilterProcess(const Z3DFilter* p) override;

  void beforeNetworkProcess() override;

  void afterNetworkProcess() override;
};

#endif // Z3DNETWORKEVALUATOR_H
