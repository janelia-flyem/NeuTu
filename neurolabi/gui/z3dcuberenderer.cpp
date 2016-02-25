// cube renderer for rendering triangles, quads, and cubes
// Yang Yu, 11/11/2015

#include "zglew.h"
#include "z3dcuberenderer.h"
#include "z3dgpuinfo.h"
#include "zglmutils.h"

//
Z3DCubeRenderer::Z3DCubeRenderer(QObject *parent)
    : Z3DPrimitiveRenderer(parent)
    , m_cubeShaderGrp()
    , m_dataChanged(false)
    , m_pickingDataChanged(false)
    , m_oneBatchNumber(4e6)
{
    setNeedLighting(true);
    setUseDisplayList(true);

    //
    oit2DComposeProgram = NULL;
    m_vao = 0;
    m_fbo = 0;
    m_renderbuffer = 0;
    m_texture = 0;

    //
    nCubes = 0;

    //
    m_cubes.clear();

    //
    m_cubeList.clear();

    //
    m_screen << QVector3D(-1.0f, -1.0f, 0.0f) // (a-b-c)
             << QVector3D( 1.0f, -1.0f, 0.0f)
             << QVector3D( 1.0f,  1.0f, 0.0f)
             << QVector3D( 1.0f,  1.0f, 0.0f) // (c-d-a)
             << QVector3D(-1.0f,  1.0f, 0.0f)
             << QVector3D(-1.0f, -1.0f, 0.0f);
}

Z3DCubeRenderer::~Z3DCubeRenderer()
{
}

void Z3DCubeRenderer::addCube(double l, double x, double y, double z, glm::vec4 color, std::vector<bool> v)
{
    addCube(l,l,l,x,y,z,color,v);
}

void Z3DCubeRenderer::addCube(double sx, double sy, double sz, double tx, double ty, double tz, glm::vec4 color, std::vector<bool> v)
{
    Cube cube;

    //
    cube.setVisible(v);
    cube.init(sx,sy,sz,tx,ty,tz);
    cube.setFaceColor(color);

    //
    m_cubes.push_back(cube);

    //
    m_dataChanged = true;
}

void Z3DCubeRenderer::addCube(Z3DCube *zcube)
{
    Cube cube;

    //
    cube.setVisible(zcube->b_visible);

    if(zcube->initByNodes)
    {
        cube.init(zcube->nodes);
    }
    else
    {
        cube.init(zcube->length, zcube->length, zcube->length, zcube->x, zcube->y, zcube->z);
    }

    // cube.setFaceColor(zcube->color);
    m_color = zcube->color;

    //
    m_cubes.push_back(cube);

    //
    m_dataChanged = true;
}

void Z3DCubeRenderer::addCubes(ZCubeArray cubes)
{
    CubeArrayType cubeArray;
    std::vector<Z3DCube> z3dcubes = cubes.getCubeArray();

    qDebug()<<"#### add cubes "<<cubes.size();

    //
    for (size_t i = 0; i < cubes.size(); ++i)
    {
        Z3DCube *zcube = &(z3dcubes[i]);

        Cube cube;

        if(zcube->b_visible.size()>0)
        {
            //
            cube.setVisible(zcube->b_visible);

            if(zcube->initByNodes)
            {
                cube.init(zcube->nodes);
            }
            else
            {
                cube.init(zcube->length, zcube->length, zcube->length, zcube->x, zcube->y, zcube->z);
            }

            if(i==0)
                m_color = zcube->color;

            //
            cubeArray.push_back(cube);
        }
    }

    qDebug()<<"#### cube array is added "<<cubeArray.size();

    //
    m_cubeList.push_back(cubeArray);
    m_colorList.push_back(m_color);


    qDebug()<<"#### render objects "<<m_cubeList.size();

    //
    m_dataChanged = true;
}

void Z3DCubeRenderer::clearData()
{
    m_cubeList.clear();
    m_colorList.clear();
}

void Z3DCubeRenderer::compile()
{
    m_dataChanged = true;
    m_cubeShaderGrp.rebuild(generateHeader());
}

void Z3DCubeRenderer::initialize()
{
    Z3DPrimitiveRenderer::initialize();
    QStringList cubeShaders, screenShaders;
    cubeShaders << "cube_wboit.vert" << "lighting.frag" << "cube_wboit.frag";
    m_cubeShaderGrp.init(QStringList(), generateHeader(), m_rendererBase, cubeShaders);
    m_cubeShaderGrp.addAllSupportedPostShaders();

    //
    oit2DComposeProgram = new QGLShaderProgram;
    oit2DComposeProgram->addShaderFromSourceFile(QGLShader::Vertex, ":/Resources/shader/cube_wboit_compose.vert");
    oit2DComposeProgram->addShaderFromSourceFile(QGLShader::Fragment, ":/Resources/shader/cube_wboit_compose.frag");

    //
    glGenFramebuffers(1, &m_fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, m_fbo);

    glGenTextures(1, &m_texture);
    glBindTexture(GL_TEXTURE_2D, m_texture);

    glGenRenderbuffers(1, &m_renderbuffer);
    glBindRenderbuffer(GL_RENDERBUFFER, m_renderbuffer);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    //
    m_initialized = true;
}

void Z3DCubeRenderer::deinitialize()
{
    if (!m_VAOs.empty())
    {
        glDeleteVertexArrays(m_VAOs.size(), &m_VAOs[0]);
    }
    m_VAOs.clear();

    if (!m_pickingVAOs.empty())
    {
        glDeleteVertexArrays(m_pickingVAOs.size(), &m_pickingVAOs[0]);
    }
    m_pickingVAOs.clear();

    if (!m_VBOs.empty())
    {
        glDeleteVertexArrays(m_VBOs.size(), &m_VBOs[0]);
    }
    m_VBOs.clear();

    if (!m_pickingVBOs.empty())
    {
        glDeleteVertexArrays(m_pickingVBOs.size(), &m_pickingVBOs[0]);
    }
    m_pickingVBOs.clear();

    //
    if (oit2DComposeProgram)
    {
        oit2DComposeProgram->release();
        delete oit2DComposeProgram;
        oit2DComposeProgram = NULL;
    }

    if(m_vao)
    {
        glDeleteBuffers(1, &m_vao);
    }

    if(m_fbo)
    {
        glDeleteBuffers(1, &m_fbo);
    }

    if(m_renderbuffer)
    {
        glDeleteBuffers(1, &m_renderbuffer);
    }

    if(m_texture)
    {
        glDeleteBuffers(1, &m_texture);
    }

    m_cubeShaderGrp.removeAllShaders();
    CHECK_GL_ERROR;
    Z3DPrimitiveRenderer::deinitialize();
}

QString Z3DCubeRenderer::generateHeader()
{
    QString headerSource = Z3DPrimitiveRenderer::generateHeader();
    return headerSource;
}

void Z3DCubeRenderer::renderUsingOpengl()
{
    // deprecated
}

void Z3DCubeRenderer::renderPickingUsingOpengl()
{
    // deprecated
}

void Z3DCubeRenderer::render(Z3DEye eye)
{
    //
    //m_cubeShaderGrp.bind();
    //m_rendererBase->setMaterialSpecular(glm::vec4(.1f, .1f, .1f, .1f));



    //
    nObjects = m_cubeList.size();

    qDebug()<<"**** "<<nObjects<<" to be rendered";

    if (m_hardwareSupportVAO)
    {
        if (m_dataChanged)
        {
            for(size_t i=0; i<nObjects; ++i)
            {
                renderSingleObj(eye, i);
            }
        }
        m_dataChanged = false;
    }
    else
    {
        // w/o vao defined
    }

    //
    //m_cubeShaderGrp.release();
}

void Z3DCubeRenderer::renderSingleObj(Z3DEye eye, int index)
{
    if (!m_initialized)
        return;

    //
    m_cubeShaderGrp.bind();
    Z3DShaderProgram &oit3DTransparentizeShader = m_cubeShaderGrp.get();

    //
    m_rendererBase->setMaterialSpecular(glm::vec4(.1f, .1f, .1f, .1f));
    m_rendererBase->setGlobalShaderParameters(oit3DTransparentizeShader, eye);

    //
    double w, h;
    w = m_rendererBase->getViewport().z;
    h = m_rendererBase->getViewport().w;

    //
    nCubes = m_cubeList[index].size();

    qDebug()<<"******** render "<<nCubes;

    //

    m_VAOs.resize(nCubes);
    glGenVertexArrays(m_VAOs.size(), &m_VAOs[0]);

    m_VBOs.resize(nCubes);
    glGenBuffers( m_VBOs.size(), &m_VBOs[0]);

    //glBindVertexArray(m_VAO);
    // oit pass
    //
    oit3DTransparentizeShader.setUniformValue("lighting_enabled", m_needLighting);
    oit3DTransparentizeShader.setUniformValue("pos_scale", getCoordScales());
    oit3DTransparentizeShader.setUniformValue("vColor", m_colorList[index]);

    //
    GLint loc_position = oit3DTransparentizeShader.attributeLocation("vPosition");
    GLint loc_normal = oit3DTransparentizeShader.attributeLocation("vNormal");

    //
    m_cubes = m_cubeList[index];

    //
    for (size_t i=0; i<nCubes; ++i)
    {
        glBindVertexArray(m_VAOs[i]);
        glBindBuffer( GL_ARRAY_BUFFER, m_VBOs[i] );

        size_t size_position = sizeof(glm::vec3)*m_cubes[i].positions.size();
        size_t size_normal = sizeof(float)*m_cubes[i].normalIndices.size();
        //size_t size_normal = sizeof(glm::vec3)*m_cubes[i].normals.size();
        //size_t size_color = sizeof(glm::vec4)*m_cubes[i].colors.size();

        glBufferData( GL_ARRAY_BUFFER, size_position + size_normal, NULL, GL_STATIC_DRAW );
        glBufferSubData( GL_ARRAY_BUFFER, 0, size_position, &(m_cubes[i].positions[0]) );
        glBufferSubData( GL_ARRAY_BUFFER, size_position, size_normal, &(m_cubes[i].normalIndices[0]) );
        //glBufferSubData( GL_ARRAY_BUFFER, size_position + size_normal, size_color, &(m_cubes[i].colors[0]) );

        //
        glEnableVertexAttribArray( loc_position );
        glVertexAttribPointer( loc_position, 3, GL_FLOAT, GL_FALSE, 0, (GLvoid*)(0) );

        glEnableVertexAttribArray( loc_normal );
        glVertexAttribPointer( loc_normal, 1, GL_FLOAT, GL_FALSE, 0, (GLvoid*)(size_position) );

        //glEnableVertexAttribArray( loc_color );
        //glVertexAttribPointer( loc_color, 4, GL_FLOAT, GL_FALSE, 0, (GLvoid*)(size_position + size_normal));

        glBindBuffer( GL_ARRAY_BUFFER, 0);
        //glBindVertexArray(0);
    }
    glBindVertexArray(0);


    // compose pass
    // vao
    glGenVertexArrays(1, &m_vao);
    glBindVertexArray( m_vao );
    //oit2DComposeProgram->setAttributeArray("composeVertPos", m_screen.constData());
    //oit2DComposeProgram->enableAttributeArray("composeVertPos");

    glGenBuffers(1, &m_vbo);
    glBindBuffer( GL_ARRAY_BUFFER, m_vbo);
    glBufferData( GL_ARRAY_BUFFER, m_screen.size()*sizeof(QVector3D), m_screen.constData(), GL_STATIC_DRAW);

    GLint posLoc = glGetAttribLocation(oit2DComposeProgram->programId(), "composeVertPos");
    glEnableVertexAttribArray(posLoc);
    glVertexAttribPointer( posLoc, 3, GL_FLOAT, GL_FALSE, 0, (GLvoid*)(0));

    glBindVertexArray(0);

    // fbo
    glGetIntegerv(GL_DRAW_FRAMEBUFFER_BINDING, &m_preFBO);

    glBindFramebuffer(GL_FRAMEBUFFER, m_fbo);

    glBindTexture(GL_TEXTURE_2D, m_texture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB32F, w, h, 0, GL_RGB, GL_FLOAT, 0);

    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_texture, 0);
    glBindTexture(GL_TEXTURE_2D, 0);

    glBindRenderbuffer(GL_RENDERBUFFER, m_renderbuffer);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, w, h);

    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, m_renderbuffer);
    glBindRenderbuffer(GL_RENDERBUFFER, 0);

    glBindFramebuffer(GL_FRAMEBUFFER, m_preFBO);

    //
    if (!oit2DComposeProgram->link())
    {
        qWarning() << oit2DComposeProgram->log() << endl;
    }

    // render
    // oit pass
    glDepthMask(GL_FALSE);
    glEnable(GL_BLEND);

    glBlendFuncSeparate(GL_ONE, GL_ONE, GL_ZERO, GL_ONE_MINUS_SRC_ALPHA);

    for(size_t i=0; i<nCubes; i++)
    {
        glBindVertexArray( m_VAOs[i] );
        glDrawArrays( GL_TRIANGLES, 0, m_cubes[i].positions.size() );
        glBindVertexArray(0);
    }

    glDepthMask(GL_TRUE);
    glDisable(GL_BLEND);

    // compose pass
    if (!oit2DComposeProgram->bind())
    {
        qWarning() << oit2DComposeProgram->log() << endl;
    }

    //
    glBlendFunc(GL_ONE_MINUS_SRC_ALPHA, GL_SRC_ALPHA);

    glBindFramebuffer(GL_FRAMEBUFFER, m_fbo);

    //
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, m_texture);

    GLuint cloc_accum = glGetUniformLocation(oit2DComposeProgram->programId(), "accumTexture");
    glUniform1i(cloc_accum, 0);

    GLuint cloc_revealage = glGetUniformLocation(oit2DComposeProgram->programId(), "revealageTexture");
    glUniform1i(cloc_revealage, 1);

    //    GLenum DrawBuffers[1] = {GL_COLOR_ATTACHMENT0};
    //    glDrawBuffers(1, DrawBuffers); // "1" is the size of DrawBuffers

    glBindVertexArray( m_vao );
    glDrawArrays( GL_TRIANGLES, 0, m_screen.size());
    glBindVertexArray(0);

    glBindFramebuffer(GL_FRAMEBUFFER, m_preFBO);

    glDisable(GL_BLEND);

    //
    m_cubeShaderGrp.release();
}


void Z3DCubeRenderer::renderPicking(Z3DEye eye)
{
}

bool Z3DCubeRenderer::isEmpty()
{
    return m_cubeList.empty();
}
