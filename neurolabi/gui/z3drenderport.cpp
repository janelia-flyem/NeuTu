#include "z3drenderport.h"

#include "logging/zqslog.h"
#include "z3dgl.h"
#include "z3dgpuinfo.h"
#include "z3drendertarget.h"

Z3DRenderOutputPort::Z3DRenderOutputPort(const QString& name, Z3DFilter* filter,
                                         GLint internalColorFormat, GLint internalDepthFormat)
  : Z3DOutputPortBase(name, filter)
  , m_resultIsValid(false)
  , m_internalColorFormat(internalColorFormat)
  , m_internalDepthFormat(internalDepthFormat)
  , m_multisample(false)
  , m_sample(4)
  , m_renderTarget(m_internalColorFormat, m_internalDepthFormat,
                   m_size, m_multisample, m_sample)
{
}

void Z3DRenderOutputPort::invalidate()
{
  m_resultIsValid = false;
  Z3DOutputPortBase::invalidate();
}

void Z3DRenderOutputPort::clearTarget() const
{
  if (!isBound())
    LOG(ERROR) << "RenderTarget is not bound, can not clear.";
  else
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void Z3DRenderOutputPort::resize(const glm::uvec2& newsize)
{
  if (m_renderTarget.resize(newsize)) {
    m_resultIsValid = false;
    m_size = newsize;
  }
}

void Z3DRenderOutputPort::changeColorFormat(GLint internalColorFormat)
{
  m_internalColorFormat = internalColorFormat;
  m_renderTarget.changeColorAttachmentFormat(m_internalColorFormat);
  invalidate();
}

void Z3DRenderOutputPort::chagneDepthFormat(GLint internalDepthFormat)
{
  m_internalDepthFormat = internalDepthFormat;
  m_renderTarget.changeDepthAttachmentFormat(m_internalDepthFormat);
  invalidate();
}

bool Z3DRenderOutputPort::canConnectTo(const Z3DInputPortBase* inport) const
{
  if (dynamic_cast<const Z3DRenderInputPort*>(inport)) {
    return Z3DOutputPortBase::canConnectTo(inport);
  } else {
    return false;
  }
}

//void Z3DRenderOutputPort::setMultisample(bool multisample, int nsample)
//{
//  if (!isInitialized() || (multisample == m_multisample && nsample == m_sample))
//    return;
//  m_multisample = multisample;
//  m_sample = nsample;
//  changeFormat(m_internalColorFormat, m_internalDepthFormat);    // use same format, just replace rendertarget
//}

//-----------------------------------------------------------------------------------

Z3DRenderInputPort::Z3DRenderInputPort(const QString& name, bool allowMultipleConnections,
                                       Z3DFilter* filter,
                                       Z3DFilter::State invalidationState)
  : Z3DInputPortBase(name, allowMultipleConnections, filter, invalidationState)
{
}

size_t Z3DRenderInputPort::numValidInputs() const
{
  size_t res = 0;
  for (size_t i = 0; i < m_connectedOutputPorts.size(); ++i) {
    const Z3DRenderOutputPort* p = static_cast<const Z3DRenderOutputPort*>(m_connectedOutputPorts[i]);
    if (p->hasValidData())
      ++res;
  }
  return res;
}

glm::uvec2 Z3DRenderInputPort::size(size_t idx) const
{
  if (renderTarget(idx))
    return renderTarget(idx)->size();
  else
    return glm::uvec2(0);
}

const Z3DTexture* Z3DRenderInputPort::colorTexture(size_t idx) const
{
  if (renderTarget(idx))
    return renderTarget(idx)->attachment(GL_COLOR_ATTACHMENT0);

  return nullptr;
}

const Z3DTexture* Z3DRenderInputPort::depthTexture(size_t idx) const
{
  if (renderTarget(idx))
    return renderTarget(idx)->attachment(GL_DEPTH_ATTACHMENT);

  return nullptr;
}

const Z3DRenderTarget* Z3DRenderInputPort::renderTarget(size_t idx) const
{
  if (idx >= numValidInputs())
    return nullptr;
  size_t res = 0;
  for (size_t i = 0; i < m_connectedOutputPorts.size(); ++i) {
    const Z3DRenderOutputPort* p = static_cast<const Z3DRenderOutputPort*>(m_connectedOutputPorts[i]);
    if (p->hasValidData())
      ++res;
    if (idx == res - 1)
      return &p->renderTarget();
  }
  return nullptr;
}
