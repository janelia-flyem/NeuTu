#include "z3dbackgroundrenderer.h"

#include "z3dgl.h"
#include "z3dgpuinfo.h"
#include "z3dshaderprogram.h"

Z3DBackgroundRenderer::Z3DBackgroundRenderer(Z3DRendererBase& rendererBase)
  : Z3DPrimitiveRenderer(rendererBase)
  , m_backgroundShaderGrp(rendererBase)
  , m_firstColor("First Color", glm::vec4(1.0f, 1.0f, 1.0f, 1.0f))
  , m_secondColor("Second Color", glm::vec4(0.2f, 0.2f, 0.2f, 1.0f))
  , m_gradientOrientation("Gradient Orientation")
  , m_mode("mode")
  , m_VAO(1)
  , m_region(0, 1, 0, 1)
{
  m_firstColor.setStyle("COLOR");
  m_secondColor.setStyle("COLOR");
  m_mode.addOptions("Uniform", "Gradient");
  m_mode.select("Gradient");
  m_gradientOrientation.addOptions("LeftToRight", "RightToLeft", "TopToBottom", "BottomToTop");
  m_gradientOrientation.select("BottomToTop");
  connect(&m_mode, &ZStringIntOptionParameter::valueChanged, this, &Z3DBackgroundRenderer::adjustWidgets);
  connect(&m_mode, &ZStringIntOptionParameter::valueChanged, this, &Z3DBackgroundRenderer::compile);
  connect(&m_gradientOrientation, &ZStringIntOptionParameter::valueChanged, this, &Z3DBackgroundRenderer::compile);

  QStringList allshaders;
  allshaders << "pass.vert" << "background_func.frag";
  QStringList normalShaders;
  normalShaders << "pass.vert" << "background.frag";
  m_backgroundShaderGrp.init(allshaders, m_rendererBase.generateHeader() + generateHeader(),
                             "", normalShaders);
  m_backgroundShaderGrp.addAllSupportedPostShaders();

  if (m_hardwareSupportVAO) {
    m_VAO.bind();
    const GLfloat vertices[] = {-1.f, 1.f, 1.0f - 1e-5f, //top left corner
                                -1.f, -1.f, 1.0f - 1e-5f, //bottom left corner
                                1.f, 1.f, 1.0f - 1e-5f, //top right corner
                                1.f, -1.f, 1.0f - 1e-5f}; // bottom right rocner
    GLint attr_vertex = m_backgroundShaderGrp.get().vertexAttributeLocation();

    glEnableVertexAttribArray(attr_vertex);
    m_VBO.bind(GL_ARRAY_BUFFER);
    glBufferData(GL_ARRAY_BUFFER, 3 * 4 * sizeof(GLfloat), vertices, GL_STATIC_DRAW);
    glVertexAttribPointer(attr_vertex, 3, GL_FLOAT, GL_FALSE, 0, nullptr);

    m_VBO.release(GL_ARRAY_BUFFER);

    m_VAO.release();
  }
  CHECK_GL_ERROR
}

void Z3DBackgroundRenderer::setRenderingRegion(double left, double right, double bottom, double top)
{
  m_region = glm::vec4(left, right - left, bottom, top - bottom);
}

void Z3DBackgroundRenderer::compile()
{
  m_backgroundShaderGrp.rebuild(m_rendererBase.generateHeader() + generateHeader());
}

QString Z3DBackgroundRenderer::generateHeader()
{
  QString headerSource;
  if (m_mode.get() == "Uniform") {
    headerSource += "#define UNIFORM\n";
  } else {
    if (m_gradientOrientation.isSelected("LeftToRight"))
      headerSource += "#define GRADIENT_LEFT_TO_RIGHT\n";
    else if (m_gradientOrientation.isSelected("RightToLeft"))
      headerSource += "#define GRADIENT_RIGHT_TO_LEFT\n";
    else if (m_gradientOrientation.isSelected("TopToBottom"))
      headerSource += "#define GRADIENT_TOP_TO_BOTTOM\n";
    else if (m_gradientOrientation.isSelected("BottomToTop"))
      headerSource += "#define GRADIENT_BOTTOM_TO_TOP\n";
  }
  return headerSource;
}

#ifndef _USE_CORE_PROFILE_
void Z3DBackgroundRenderer::renderUsingOpengl()
{
  glMatrixMode(GL_PROJECTION);
  glPushMatrix();
  glLoadIdentity();

  glMatrixMode(GL_MODELVIEW);
  glPushMatrix();
  glLoadIdentity();

  if (m_mode.get() == "Gradient") {
    if (m_gradientOrientation.isSelected("LeftToRight"))
      glRotatef(270.f, 0.0f, 0.0f, 1.0f);
    else if (m_gradientOrientation.isSelected("RightToLeft"))
      glRotatef(90.f, 0.0f, 0.0f, 1.0f);
    else if (m_gradientOrientation.isSelected("TopToBottom"))
      glRotatef(180.f, 0.0f, 0.0f, 1.0f);
    else if (m_gradientOrientation.isSelected("BottomToTop"))
      glRotatef(0.f, 0.0f, 0.0f, 1.0f);
  }

  glBegin(GL_QUADS);
  glColor4fv(&m_firstColor.get()[0]);
  glVertex3f(-1.0, -1.0, 1.0-1e-5);
  glVertex3f( 1.0, -1.0, 1.0-1e-5);
  if (m_mode.get() == "Gradient")
    glColor4fv(&m_secondColor.get()[0]);
  glVertex3f( 1.0, 1.0, 1.0-1e-5);
  glVertex3f(-1.0, 1.0, 1.0-1e-5);
  glEnd();

  glMatrixMode(GL_PROJECTION);
  glPopMatrix();

  glMatrixMode(GL_MODELVIEW);
  glPopMatrix();
  CHECK_GL_ERROR
}

void Z3DBackgroundRenderer::renderPickingUsingOpengl()
{
  // do nothing
}
#endif

void Z3DBackgroundRenderer::render(Z3DEye eye)
{
  m_backgroundShaderGrp.bind();
  Z3DShaderProgram& shader = m_backgroundShaderGrp.get();
  m_rendererBase.setGlobalShaderParameters(shader, eye);

  shader.setColor1Uniform(m_firstColor.get());
  if (m_mode.get() != "Uniform") {
    shader.setColor2Uniform(m_secondColor.get());
    shader.setRegionUniform(m_region);
  }

  if (m_hardwareSupportVAO) {
    m_VAO.bind();
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    m_VAO.release();
  } else {
    const GLfloat vertices[] = {-1.f, 1.f, 1.0f - 1e-5f, //top left corner
                                -1.f, -1.f, 1.0f - 1e-5f, //bottom left corner
                                1.f, 1.f, 1.0f - 1e-5f, //top right corner
                                1.f, -1.f, 1.0f - 1e-5f}; // bottom right rocner
    GLint attr_vertex = shader.vertexAttributeLocation();

    glEnableVertexAttribArray(attr_vertex);
    m_VBO.bind(GL_ARRAY_BUFFER);
    glBufferData(GL_ARRAY_BUFFER, 3 * 4 * sizeof(GLfloat), vertices, GL_STATIC_DRAW);
    glVertexAttribPointer(attr_vertex, 3, GL_FLOAT, GL_FALSE, 0, nullptr);

    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

    m_VBO.release(GL_ARRAY_BUFFER);

    glDisableVertexAttribArray(attr_vertex);
  }

  m_backgroundShaderGrp.release();
}

void Z3DBackgroundRenderer::renderPicking(Z3DEye /*unused*/)
{
  // do nothing
}

void Z3DBackgroundRenderer::adjustWidgets()
{
  if (m_mode.get() == "Gradient") {
    //m_firstColor.setName("First Color");
    m_firstColor.setVisible(true);
    m_secondColor.setVisible(true);
    m_gradientOrientation.setVisible(true);
  } else if (m_mode.get() == "Uniform") {
    //m_firstColor.setName("Color");
    m_firstColor.setVisible(true);
    m_secondColor.setVisible(false);
    m_gradientOrientation.setVisible(false);
  }
}
