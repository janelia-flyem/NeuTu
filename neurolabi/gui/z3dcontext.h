#ifndef Z3DCONTEXT_H
#define Z3DCONTEXT_H

class QOpenGLContext;
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

  static void logCurrentContext();

private:
  QOpenGLContext* m_context;
};

class Z3DContextGroup
{
public:
  Z3DContextGroup();

  Z3DContextGroup(const Z3DContextGroup&) = default;

  Z3DContextGroup& operator=(const Z3DContextGroup&) = default;

  bool operator<(const Z3DContextGroup& rhs) const;

  bool operator==(const Z3DContextGroup& rhs) const;

  bool operator!=(const Z3DContextGroup& rhs) const;

private:
  QOpenGLContextGroup* m_contextGroup;
};

#endif // Z3DCONTEXT_H
