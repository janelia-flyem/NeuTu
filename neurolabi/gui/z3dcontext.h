#ifndef Z3DCONTEXT_H
#define Z3DCONTEXT_H

class QOpenGLContextGroup;

class Z3DContext
{
public:
  Z3DContext();

  Z3DContext(const Z3DContext&) = default;

  Z3DContext& operator=(const Z3DContext&) = default;

  bool operator<(const Z3DContext& rhs) const;

  bool operator==(const Z3DContext& rhs) const;

  bool operator!=(const Z3DContext& rhs) const;

private:
  QOpenGLContextGroup* m_context;
};

#endif // Z3DCONTEXT_H
