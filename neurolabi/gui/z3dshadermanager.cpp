#include "z3dshadermanager.h"

#include <QTextStream>
#include <QFile>

#include "zsysteminfo.h"
#include "qt/core/zexception.h"
#include "logging/zqslog.h"
#include "z3dshader.h"
#include "zutils.h"

Z3DShaderManager& Z3DShaderManager::instance()
{
  static Z3DShaderManager sm;
  return sm;
}

Z3DShader& Z3DShaderManager::shader(const QString& fn, const QString& header, const Z3DContextGroup& context)
{
  ShaderKey key(fn, header, context);
  auto lb = m_shaders.lower_bound(key);
  if (lb == m_shaders.end() || m_shaders.key_comp()(key, lb->first)) {
    QString filename = ZSystemInfo::instance().shaderPath(fn);
    Z3DShader::Type type;
    if (filename.endsWith(".vert", Qt::CaseInsensitive)) {
      type = Z3DShader::Type::Vertex;
    } else if (filename.endsWith(".geom", Qt::CaseInsensitive)) {
      type = Z3DShader::Type::Geometry;
    } else if (filename.endsWith(".frag", Qt::CaseInsensitive)) {
      type = Z3DShader::Type::Fragment;
    } else {
      throw ZGLException(
        QString("Not supported file extension: %1. Use .vert, .geom or .frag as shader extension").arg(filename));
    }

    QFile file(filename);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
      throw ZGLException(
        QString("Can not open vertex shader file: %1.  Error String: %2").arg(filename).arg(file.errorString()));
    }
    QTextStream fileStream(&file);
    fileStream.setCodec("UTF-8");
    QString src = header + fileStream.readAll();

    CHECK(context == Z3DContextGroup());
    auto shdr = std::make_unique<Z3DShader>(type);
    shdr->compileSourceCode(src);
    Z3DShader* res = shdr.get();
    m_shaders.emplace_hint(lb, key, std::move(shdr));
    return *res;
  }
  return *(lb->second);
}

bool Z3DShaderManager::ShaderKey::operator<(const Z3DShaderManager::ShaderKey& rhs) const
{
  return std::tie(filename, context, header) < std::tie(rhs.filename, rhs.context, rhs.header);
}
