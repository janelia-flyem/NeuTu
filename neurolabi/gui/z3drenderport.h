#pragma once

#include "z3dport.h"
#include "z3drendertarget.h"
#include "z3dtexture.h"
#include <typeinfo>

class Z3DRenderOutputPort : public Z3DOutputPortBase
{
public:
  Z3DRenderOutputPort(const QString& name, Z3DFilter* filter,
                      GLint internalColorFormat = GLint(GL_RGBA16),
                      GLint internalDepthFormat = GLint(GL_DEPTH_COMPONENT24));

  virtual void invalidate() override;

  void bindTarget()
  {
    m_renderTarget.bind();
    m_resultIsValid = true;
  }

  void releaseTarget()
  { m_renderTarget.release(); }

  GLint internalDepthFormat() const
  { return m_internalDepthFormat; }

  GLint internalColorFormat() const
  { return m_internalColorFormat; }

  // Clears the contents of an activated outport's RenderTarget,
  void clearTarget() const;

  virtual bool hasValidData() const override
  { return m_resultIsValid; }

  // Returns true, if the associated RenderTarget is currently bound.
  bool isBound() const
  { return m_renderTarget.isBound(); }

  const Z3DRenderTarget& renderTarget() const
  { return m_renderTarget; }

  Z3DRenderTarget& renderTarget()
  { return m_renderTarget; }

  const Z3DTexture* colorTexture() const
  { return m_renderTarget.attachment(GL_COLOR_ATTACHMENT0); }

  const Z3DTexture* depthTexture() const
  { return m_renderTarget.attachment(GL_DEPTH_ATTACHMENT); }

  Z3DTexture* colorTexture()
  { return m_renderTarget.attachment(GL_COLOR_ATTACHMENT0); }

  Z3DTexture* depthTexture()
  { return m_renderTarget.attachment(GL_DEPTH_ATTACHMENT); }

  // Resizes the associated RenderTarget to the passed dimensions.
  virtual void resize(const glm::uvec2& newsize) override;

  // change RenderTarget with the given format.
  void changeColorFormat(GLint internalColorFormat);

  void chagneDepthFormat(GLint internalDepthFormat);

  virtual bool canConnectTo(const Z3DInputPortBase* inport) const override;

  //void setMultisample(bool multisample, int nsample = 4);

private:
  bool m_resultIsValid;

  GLint m_internalColorFormat;
  GLint m_internalDepthFormat;

  bool m_multisample;
  int m_sample;

  Z3DRenderTarget m_renderTarget;
};

class Z3DRenderInputPort : public Z3DInputPortBase
{
public:
  Z3DRenderInputPort(const QString& name, bool allowMultipleConnections,
                     Z3DFilter* filter,
                     Z3DFilter::State invalidationState = Z3DFilter::State::AllResultInvalid);

  virtual bool isReady() const override
  { return numValidInputs() > 0; }

  // go through all connected output render ports and count how many have valid rendering
  size_t numValidInputs() const;

  // once we have the number of valid inputs, we can use a index as parameter to query data from input
  // idx range from 0 to numValidInputs() - 1
  glm::uvec2 size(size_t idx = 0) const;

  const Z3DTexture* colorTexture(size_t idx = 0) const;

  const Z3DTexture* depthTexture(size_t idx = 0) const;

private:
  const Z3DRenderTarget* renderTarget(size_t idx) const;
};
