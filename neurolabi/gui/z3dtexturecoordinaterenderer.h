#ifndef Z3DTEXTURECOORDINATERENDERER_H
#define Z3DTEXTURECOORDINATERENDERER_H

#include "z3dprimitiverenderer.h"
#include "z3dshaderprogram.h"

class ZMesh;

// render 3d texture coordinates as color
class Z3DTextureCoordinateRenderer : public Z3DPrimitiveRenderer
{
Q_OBJECT
public:
  explicit Z3DTextureCoordinateRenderer(Z3DRendererBase& rendererBase);

  // triangle list should contains vertexs and 3d texture coordinates
  void setTriangleList(const ZMesh* mesh)
  {
    m_mesh = mesh;
    m_dataChanged = true;
  }
  // todo: add function to set data (vertex, texture coordinate, triangle type, indexes) separately

protected:
  virtual void compile() override;

  virtual void render(Z3DEye eye) override;

protected:
  const ZMesh* m_mesh;

  Z3DShaderProgram m_renderTextureCoordinateShader;

  ZVertexBufferObject m_VBOs;
  ZVertexArrayObject m_VAO;
  bool m_dataChanged;
};

#endif // Z3DTEXTURECOORDINATERENDERER_H
