#include "z3dshader.h"

#include "z3dgl.h"
#include "qt/core/zexception.h"
#include <vector>

Z3DShader::Z3DShader(Z3DShader::Type type)
  : m_type(type)
  , m_compiled(false)
  , m_id(0)
  , m_context()
{
  if (m_type == Z3DShader::Type::Vertex) {
    m_id = glCreateShader(GL_VERTEX_SHADER);
  } else if (m_type == Z3DShader::Type::Fragment) {
    m_id = glCreateShader(GL_FRAGMENT_SHADER);
  } else if (m_type == Z3DShader::Type::Geometry) {
    m_id = glCreateShader(GL_GEOMETRY_SHADER);
  } else if (m_type == Z3DShader::Type::TessellationControl) {
    m_id = glCreateShader(GL_TESS_CONTROL_SHADER);
  } else if (m_type == Z3DShader::Type::TessellationEvaluation) {
    m_id = glCreateShader(GL_TESS_EVALUATION_SHADER);
  } else if (m_type == Z3DShader::Type::Compute) {
    m_id = glCreateShader(GL_COMPUTE_SHADER);
  }
  if (!m_id) {
    throw ZGLException("Z3DShader: Could not create shader");
  }
}

Z3DShader::~Z3DShader()
{
  glDeleteShader(m_id);
}

void Z3DShader::compileSourceCode(const char* source)
{
  if (m_id) {
    GLint length = std::strlen(source);
    glShaderSource(m_id, 1, &source, &length);

    glCompileShader(m_id);
    GLint value = 0;
    glGetShaderiv(m_id, GL_COMPILE_STATUS, &value);
    m_compiled = (value != 0);

    if (!m_compiled) {
      const char* types[] = {
        "Fragment",
        "Vertex",
        "Geometry",
        "Tessellation Control",
        "Tessellation Evaluation",
        "Compute",
        ""
      };
      const char* type = nullptr;
      switch (m_type) {
        case Z3DShader::Type::Fragment:
          type = types[0];
          break;
        case Z3DShader::Type::Vertex:
          type = types[1];
          break;
        case Z3DShader::Type::Geometry:
          type = types[2];
          break;
        case Z3DShader::Type::TessellationControl:
          type = types[3];
          break;
        case Z3DShader::Type::TessellationEvaluation:
          type = types[4];
          break;
        case Z3DShader::Type::Compute:
          type = types[5];
          break;
        default:
          type = types[6];
          break;
      }
      // Get info and source code lengths
      GLint infoLogLength = 0;
      GLint sourceCodeLength = 0;
      std::vector<char> logBuffer;
      std::vector<char> sourceCodeBuffer;
      // Get the compilation info log
      glGetShaderiv(m_id, GL_INFO_LOG_LENGTH, &infoLogLength);
      if (infoLogLength > 1) {
        GLint temp;
        logBuffer.resize(infoLogLength);
        glGetShaderInfoLog(m_id, infoLogLength, &temp, logBuffer.data());
      }
      // Get the source code
      glGetShaderiv(m_id, GL_SHADER_SOURCE_LENGTH, &sourceCodeLength);
      if (sourceCodeLength > 1) {
        GLint temp;
        sourceCodeBuffer.resize(sourceCodeLength);
        glGetShaderSource(m_id, sourceCodeLength, &temp, sourceCodeBuffer.data());
      }
      QString log = QString("Z3DShader::compileSourceCode(%1): %2").arg(type).arg(
        logBuffer.empty() ? "failed" : logBuffer.data());
      // Dump the source code if we got it
      if (!sourceCodeBuffer.empty()) {
        log += QString("\n*** source code ***\n");
        log += source;
        log += QString("\n***");
      }
      throw ZGLException(log);
    }
  } else {
    throw ZGLException("Z3DShader: Share is not created yet");
  }
}

void Z3DShader::compileSourceCode(const QString& source)
{
  compileSourceCode(source.toUtf8().constData());
}

QByteArray Z3DShader::sourceCode() const
{
  if (!m_id)
    return QByteArray();
  GLint size = 0;
  glGetShaderiv(m_id, GL_SHADER_SOURCE_LENGTH, &size);
  if (size <= 0)
    return QByteArray();
  GLint len = 0;
  std::vector<char> source(size);
  glGetShaderSource(m_id, size, &len, source.data());
  QByteArray src(source.data());
  return src;
}
