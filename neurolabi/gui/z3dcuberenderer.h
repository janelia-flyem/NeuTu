// cube renderer for rendering triangles, quads, and cubes
// Yang Yu, 11/11/2015

#ifndef Z3DCUBERENDERER_H
#define Z3DCUBERENDERER_H

#include "z3dprimitiverenderer.h"
#include "z3dshadergroup.h"
#include "zcubearray.h"

typedef std::vector<Cube> CubeArrayType;

/// Z3DCubeRenderer
class Z3DCubeRenderer : public Z3DPrimitiveRenderer
{
    Q_OBJECT
public:
    explicit Z3DCubeRenderer(QObject *parent = 0);
    virtual ~Z3DCubeRenderer();

    void addCube(double sx, double sy, double sz, double tx, double ty, double tz, glm::vec4 color, std::vector<bool> v);
    void addCube(double l, double x, double y, double z, glm::vec4 color, std::vector<bool> v);
    void addCube(Z3DCube *zcube);
    void addCubes(ZCubeArray cubes);
    void setColor(glm::vec4 color);
    void clearData();
    bool isEmpty();

protected:
    virtual void compile();
    virtual void initialize();
    virtual void deinitialize();
    virtual QString generateHeader();

    virtual void renderUsingOpengl();
    virtual void renderPickingUsingOpengl();

    virtual void render(Z3DEye eye);
    virtual void renderPicking(Z3DEye eye);

    Z3DShaderGroup m_cubeShaderGrp;
    QGLShaderProgram *oit2DComposeProgram;

private:
    std::vector<glm::vec4> m_pointAndRadius;
    std::vector<glm::vec4> m_specularAndShininess;
    std::vector<glm::vec4> m_pointColors;
    std::vector<glm::vec4> m_pointPickingColors;
    std::vector<GLfloat> m_allFlags;
    std::vector<GLuint> m_indexs;

    QVector<QVector3D> m_screen;
    GLuint m_vao, m_vbo;
    GLint m_preFBO;
    GLuint m_fbo;
    GLuint m_renderbuffer;
    GLuint m_accumTexture, m_revealageTexture;

    GLuint m_vaoSurf, m_vboSurf;

    bool m_dataChanged;
    bool m_pickingDataChanged;
    size_t m_oneBatchNumber;

    size_t nObjects; // multiple ROIs
    std::vector<CubeArrayType> m_cubeList;
    std::vector<glm::vec4> m_colorList;

    std::vector<glm::vec3> positions;
    std::vector<float> normalIndices;

    size_t size_positions;
    size_t size_normalIndices;

    glm::vec4 m_color; // uniform
};

#endif // Z3DCUBERENDERER_H
