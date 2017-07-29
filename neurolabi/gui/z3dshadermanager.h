#ifndef Z3DSHADERMANAGER_H
#define Z3DSHADERMANAGER_H


#include "z3dcontext.h"
#include <QString>
#include <map>
#include <memory>

class Z3DShader;

// throw ZGLException if compile error
class Z3DShaderManager
{
public:
  static Z3DShaderManager& instance();

  // return reference because it is always valid
  Z3DShader& shader(const QString& fn, const QString& header, const Z3DContext& context);

private:
  struct ShaderKey
  {
    ShaderKey(const QString& fn, const QString& hd, const Z3DContext& ct)
      : filename(fn), header(hd), context(ct)
    {}

    QString filename;
    QString header;
    Z3DContext context;

    bool operator<(const ShaderKey& rhs) const;
  };

  std::map<ShaderKey, std::unique_ptr<Z3DShader>> m_shaders;
};

#endif // Z3DSHADERMANAGER_H
