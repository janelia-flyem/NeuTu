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

#include "z3dnetworkevaluator.h"

#include <boost/graph/topological_sort.hpp>
#include <algorithm>
#include <queue>
#include <set>

#include "z3dgl.h"
#include "z3dcanvaspainter.h"
#include "z3dfilter.h"
#include "z3dmeshfilter.h"
#include "z3drendertarget.h"
#include "z3dtexture.h"
#include "logging/zqslog.h"
#include "zrandom.h"
#include "zutils.h"

//#define PROFILE3DRENDERERS

Z3DNetworkEvaluator::Z3DNetworkEvaluator(Z3DCanvasPainter& canvasPainter, QObject* parent)
  : QObject(parent)
  , m_locked(false)
  , m_processPending(false)
  , m_canvasPainter(canvasPainter)
{
#if defined(_DEBUG_)
  m_filterWrappers.emplace_back(std::make_unique<Z3DCheckOpenGLStateFilterWrapper>());
#endif
#if defined(PROFILE3DRENDERERS)
  m_filterWrappers.emplace_back(std::make_unique<Z3DProfileFilterWrapper>());
#endif

  updateNetwork();

  m_canvasPainter.canvas().setNetworkEvaluator(this);
}

Z3DNetworkEvaluator::~Z3DNetworkEvaluator()
{
  m_canvasPainter.canvas().setNetworkEvaluator(nullptr);
}

void Z3DNetworkEvaluator::process(bool stereo)
{
  if (m_locked) {
    LOG(INFO) << "locked. Scheduling.";
    //m_processPending = true;
    return;
  }

  lock();

//  for (size_t i = 0; i < m_renderingOrder.size(); ++i) {
//    Z3DMeshFilter* meshFilter = qobject_cast<Z3DMeshFilter*>(m_renderingOrder[i]);
//    if (meshFilter && !meshFilter->isFixed()) {
//      if (ZRandomInstance.randReal<float>() < 0.001f) {
//        meshFilter->setVisible(true);
//        meshFilter->setGlow(true);
//        meshFilter->setStayOnTop(true);
//      } else {
//        meshFilter->setVisible(false);
//      }
//    }
//  }

  m_canvasPainter.canvas().getGLFocus();

  // notify filter wrappers
  for (size_t j = 0; j < m_filterWrappers.size(); ++j)
    m_filterWrappers[j]->beforeNetworkProcess();
  CHECK_GL_ERROR

  // Iterate over filters in rendering order
  for (size_t i = 0; i < m_renderingOrder.size(); ++i) {
    Z3DFilter* currentFilter = m_renderingOrder[i];

    Z3DEye eye = stereo ? Z3DEye::Left : Z3DEye::Mono;

#ifdef _DEBUG_2
    std::cout << currentFilter->className().toStdString() << ":"
              << currentFilter->isValid(eye)
              << " " << currentFilter->isReady(eye) << std::endl;
#endif

    // execute the filter, if it needs processing and is ready
    if (!currentFilter->isValid(eye) && currentFilter->isReady(eye)) {
      // notify filter wrappers
      for (size_t j = 0; j < m_filterWrappers.size(); ++j)
        m_filterWrappers[j]->beforeFilterProcess(currentFilter);
      CHECK_GL_ERROR

      {
        currentFilter->process(eye);
        currentFilter->setValid(eye);
        CHECK_GL_ERROR
      }

      // notify filter wrappers
      for (size_t j = 0; j < m_filterWrappers.size(); ++j)
        m_filterWrappers[j]->afterFilterProcess(currentFilter);
      CHECK_GL_ERROR
    }

    if (stereo && !currentFilter->isValid(Z3DEye::Right) && currentFilter->isReady(Z3DEye::Right)) {
      // notify filter wrappers
      for (size_t j = 0; j < m_filterWrappers.size(); ++j)
        m_filterWrappers[j]->beforeFilterProcess(currentFilter);
      CHECK_GL_ERROR

      {
        currentFilter->process(Z3DEye::Right);
        currentFilter->setValid(Z3DEye::Right);
        CHECK_GL_ERROR
      }

      // notify filter wrappers
      for (size_t j = 0; j < m_filterWrappers.size(); ++j)
        m_filterWrappers[j]->afterFilterProcess(currentFilter);
      CHECK_GL_ERROR
    }
  }

  // notify filter wrappers
  for (size_t j = 0; j < m_filterWrappers.size(); ++j)
    m_filterWrappers[j]->afterNetworkProcess();
  CHECK_GL_ERROR

  unlock();

  // make sure that canvases are repainted, if their update has been blocked by the locked evaluator
  if (m_processPending) {
    m_processPending = false;
    m_canvasPainter.invalidate();
  }
}

void Z3DNetworkEvaluator::updateNetwork()
{
  m_renderingOrder.clear();
  m_filterToVertexMapper.clear();
  m_filterGraph.clear();
  m_reverseSortedFilters.clear();

  std::queue<Z3DFilter*> filterQueue;

  filterQueue.push(&m_canvasPainter);
  Vertex v = boost::add_vertex(VertexInfo(&m_canvasPainter), m_filterGraph);
  m_filterToVertexMapper[&m_canvasPainter] = v;

  // build graph of all connected filters
  while (!filterQueue.empty()) {
    Z3DFilter* filter = filterQueue.front();
    const std::vector<Z3DInputPortBase*> inports = filter->inputPorts();
    for (size_t i = 0; i < inports.size(); ++i) {
      const std::vector<Z3DOutputPortBase*> connected = inports[i]->connected();
      for (size_t j = 0; j < connected.size(); ++j) {
        Z3DFilter* outFilter = connected[j]->filter();
        if (m_filterToVertexMapper.find(outFilter) == m_filterToVertexMapper.end()) {
          filterQueue.push(outFilter);
          v = boost::add_vertex(VertexInfo(outFilter), m_filterGraph);
          m_filterToVertexMapper[outFilter] = v;
        }
        boost::add_edge(m_filterToVertexMapper[outFilter],
                        m_filterToVertexMapper[filter],
                        EdgeInfo(connected[j], inports[i]),
                        m_filterGraph);
      }
    }

    filterQueue.pop();
  }

  // sort to get rendering order
  std::vector<Vertex> sorted;
  boost::topological_sort(m_filterGraph, std::back_inserter(sorted));
  for (auto it = sorted.rbegin(); it != sorted.rend(); ++it) {
    m_renderingOrder.push_back(m_filterGraph[*it].filter);
  }

  LOG(INFO) << "Rendering Order: ";
  for (size_t i = 0; i < m_renderingOrder.size(); ++i) {
    LOG(INFO) << "  " << i << ": " << m_renderingOrder[i]->className();
  }
  LOG(INFO) << "";

  // update reverse sorted filters
  m_reverseSortedFilters = m_renderingOrder;
  std::reverse(m_reverseSortedFilters.begin(), m_reverseSortedFilters.end());

  // update size
  sizeChangedFromFilter();
  for (auto filter : m_reverseSortedFilters) {
    QObject::disconnect(filter, &Z3DFilter::requestUpstreamSizeChange, 0, 0);
    connect(filter, &Z3DFilter::requestUpstreamSizeChange,
            this, &Z3DNetworkEvaluator::sizeChangedFromFilter);
  }
}

void Z3DNetworkEvaluator::sizeChangedFromFilter(Z3DFilter* rp)
{
  if (rp) {
    bool started = false;
    for (auto filter : m_reverseSortedFilters) {
      if (started)
        filter->updateSize();
      else {
        if (rp == filter)
          started = true;
      }
    }
  } else {
    for (auto filter : m_reverseSortedFilters) {
      filter->updateSize();
    }
  }
}

// ----------------------------------------------------------------------------

void Z3DCheckOpenGLStateFilterWrapper::afterFilterProcess(const Z3DFilter* p)
{
  checkState(p);
}

void Z3DCheckOpenGLStateFilterWrapper::beforeNetworkProcess()
{
  checkState();
}

void Z3DCheckOpenGLStateFilterWrapper::checkState(const Z3DFilter* p)
{

  if (!checkGLState(GL_BLEND, false)) {
    glDisable(GL_BLEND);
    warn(p, "GL_BLEND was enabled");
  }

  if (!checkGLState(GL_BLEND_SRC, GL_ONE) || !checkGLState(GL_BLEND_DST, GL_ZERO)) {
    glBlendFunc(GL_ONE, GL_ZERO);
//    warn(p, "Modified BlendFunc"); //Remove warning produced by rect display with an unknown reason
  }

  if (!checkGLState(GL_DEPTH_TEST, false)) {
    glDisable(GL_DEPTH_TEST);
    warn(p, "GL_DEPTH_TEST was enabled");
  }

  if (!checkGLState(GL_CULL_FACE, false)) {
    glDisable(GL_CULL_FACE);
    warn(p, "GL_CULL_FACE was enabled");
  }

  if (!checkGLState(GL_COLOR_CLEAR_VALUE, glm::vec4(0.f))) {
    glClearColor(0.f, 0.f, 0.f, 0.f);
    warn(p, "glClearColor() was not set to all zeroes");
  }

  if (!checkGLState(GL_DEPTH_CLEAR_VALUE, 1.f)) {
    glClearDepth(1.0);
    warn(p, "glClearDepth() was not set to 1.0");
  }

  if (!checkGLState(GL_LINE_WIDTH, 1.f)) {
    glLineWidth(1.f);
    warn(p, "glLineWidth() was not set to 1.0");
  }

  if (!checkGLState(GL_ACTIVE_TEXTURE, GL_TEXTURE0)) {
    glActiveTexture(GL_TEXTURE0);
    warn(p, "glActiveTexture was not set to GL_TEXTURE0");
  }

#if !defined(_USE_CORE_PROFILE_) && defined(_SUPPORT_FIXED_PIPELINE_)
  if (!checkGLState(GL_MATRIX_MODE, GL_MODELVIEW)) {
    glMatrixMode(GL_MODELVIEW);
    warn(p, "glMatrixMode was not set to GL_MODELVIEW");
  }

  if (!checkGLState(GL_TEXTURE_1D, false)) {
    glDisable(GL_TEXTURE_1D);
    warn(p, "GL_TEXTURE_1D was enabled");
  }

  if (!checkGLState(GL_TEXTURE_2D, false)) {
    glDisable(GL_TEXTURE_2D);
    warn(p, "GL_TEXTURE_2D was enabled");
  }

  if (!checkGLState(GL_TEXTURE_3D, false)) {
    glDisable(GL_TEXTURE_3D);
    warn(p, "GL_TEXTURE_3D was enabled");
  }
#endif

  GLint id;
  glGetIntegerv(GL_CURRENT_PROGRAM, &id);
  if (id != 0) {
    glUseProgram(0);
    warn(p, "A shader was active");
  }

   // can not check this as we are drawing to QOpenglWidget's (Qt5) fbo which is not 0
#if 0
  if (Z3DRenderTarget::currentBoundDrawFBO() != 0) {
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    warn(p, "A render target was bound (releaseTarget() missing?)");
  }
#endif

  if (!checkGLState(GL_DEPTH_FUNC, GL_LESS)) {
    glDepthFunc(GL_LESS);
    warn(p, "glDepthFunc was not set to GL_LESS");
  }

  if (!checkGLState(GL_CULL_FACE_MODE, GL_BACK)) {
    glCullFace(GL_BACK);
    warn(p, "glCullFace was not set to GL_BACK");
  }
}

void Z3DCheckOpenGLStateFilterWrapper::warn(const Z3DFilter* p, const char* message)
{
  if (p) {
    LOG(WARNING) << "Invalid OpenGL state after processing " << p->className() << " : " << message;
  } else {
    LOG(WARNING) << "Invalid OpenGL state before network processing: " << message;
  }
}


void Z3DProfileFilterWrapper::beforeFilterProcess(const Z3DFilter* /*unused*/)
{
  m_benchTimer.start();
}

void Z3DProfileFilterWrapper::afterFilterProcess(const Z3DFilter* p)
{
  m_benchTimer.stop();
  LOG(INFO) << "Filter " << p->className() << " took time: " << m_benchTimer.time() << " seconds.";
}

void Z3DProfileFilterWrapper::beforeNetworkProcess()
{
  m_benchTimer.reset();
}

void Z3DProfileFilterWrapper::afterNetworkProcess()
{
  LOG(INFO) << "Network took time: " << m_benchTimer.total() << " seconds.";
}
