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
    m_accumTexture = 0;
    m_revealageTexture = 0;

    //
    m_vaoSurf = 0;
    m_vboSurf = 0;

    clearData();

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
    // !obsolete
    Cube cube;

    //
    cube.setVisible(v);
    cube.init(sx,sy,sz,tx,ty,tz);
    cube.setFaceColor(color);

    //
    //m_cubes.push_back(cube);

    //
    m_dataChanged = true;
}

void Z3DCubeRenderer::addCube(Z3DCube *zcube)
{
    // !obsolete
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
    //m_cubes.push_back(cube);

    //
    m_dataChanged = true;
}

void Z3DCubeRenderer::addCubes(ZCubeArray cubes)
{
    // convert ZCubeArray to CubeArrayType
    std::vector<Z3DCube> z3dcubes = cubes.getCubeArray();

    //
    size_positions = 0;
    size_normalIndices = 0;
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

            //
            size_positions += sizeof(glm::vec3)*cube.positions.size();
            size_normalIndices += sizeof(float)*cube.normalIndices.size();

            for(size_t j=0; j<cube.positions.size(); j++)
            {
                positions.push_back(cube.positions[j]);
                normalIndices.push_back(cube.normalIndices[j]);
            }
        }
    }

    // QColor -> glm::vec4
    qreal r,g,b,a;
    cubes.getColor().getRgbF(&r, &g, &b, &a);
    setColor(glm::vec4(r,g,b,a));

    //
    m_dataChanged = true;
}

void Z3DCubeRenderer::clearData()
{
    positions.clear();
    normalIndices.clear();

    size_positions = 0;
    size_normalIndices = 0;
}

void Z3DCubeRenderer::setColor(glm::vec4 color)
{
    m_color = color;
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
    m_cubeShaderGrp.addWeightedBlendedShaders();

    //compile();

    //
//    oit2DComposeProgram = new QGLShaderProgram;
//    oit2DComposeProgram->addShaderFromSourceFile(QGLShader::Vertex, ":/Resources/shader/cube_wboit_compose.vert");
//    oit2DComposeProgram->addShaderFromSourceFile(QGLShader::Fragment, ":/Resources/shader/cube_wboit_compose.frag");

    //
//    glGenFramebuffers(1, &m_fbo);
//    glBindFramebuffer(GL_FRAMEBUFFER, m_fbo);

//    glGenTextures(1, &m_accumTexture);
//    glBindTexture(GL_TEXTURE_2D, m_accumTexture);

//    glGenTextures(1, &m_revealageTexture);
//    glBindTexture(GL_TEXTURE_2D, m_revealageTexture);

//    glGenRenderbuffers(1, &m_renderbuffer);
//    glBindRenderbuffer(GL_RENDERBUFFER, m_renderbuffer);

//    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    //
    m_initialized = true;
}

void Z3DCubeRenderer::deinitialize()
{
    //
    if (oit2DComposeProgram)
    {
        oit2DComposeProgram->release();
        delete oit2DComposeProgram;
        oit2DComposeProgram = NULL;
    }

    if(m_vao)
    {
        glDeleteVertexArrays(1, &m_vao);
    }

    if(m_vbo)
    {
        glDeleteBuffers(1, &m_vbo);
    }

    if(m_vaoSurf)
    {
        glDeleteVertexArrays(1, &m_vaoSurf);
    }

    if(m_vboSurf)
    {
        glDeleteBuffers(1, &m_vboSurf);
    }

    if(m_fbo)
    {
        glDeleteFramebuffers(1, &m_fbo);
    }

    if(m_renderbuffer)
    {
        glDeleteRenderbuffers(1, &m_renderbuffer);
    }

    if(m_accumTexture)
    {
        glDeleteTextures(1, &m_accumTexture);
    }

    if(m_revealageTexture)
    {
        glDeleteTextures(1, &m_revealageTexture);
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
    if (!m_initialized)
        return;

    //
    double w, h;
    w = m_rendererBase->getViewport().z;
    h = m_rendererBase->getViewport().w;

    // fbo
    glGetIntegerv(GL_DRAW_FRAMEBUFFER_BINDING, &m_preFBO); // try using old framebuffer

//    glBindFramebuffer(GL_FRAMEBUFFER, m_fbo);

//    glBindTexture(GL_TEXTURE_2D, m_accumTexture);
//    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
//    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
//    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
//    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
//    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB32F, w, h, 0, GL_RGBA, GL_FLOAT, 0);

//    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_accumTexture, 0);

//    glBindTexture(GL_TEXTURE_2D, m_revealageTexture);
//    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
//    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
//    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
//    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
//    glTexImage2D(GL_TEXTURE_2D, 0, GL_R32F, w, h, 0, GL_RED, GL_FLOAT, 0);

//    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, m_revealageTexture, 0);

//    glBindTexture(GL_TEXTURE_2D, 0);

//    glBindRenderbuffer(GL_RENDERBUFFER, m_renderbuffer);
//    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, w, h);

//    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, m_renderbuffer);

//    glBindRenderbuffer(GL_RENDERBUFFER, 0);

    // bind this off-screen framebuffer
    //glBindFramebuffer(GL_FRAMEBUFFER, m_fbo);

    //
    m_cubeShaderGrp.bind();
    Z3DShaderProgram &oit3DTransparentizeShader = m_cubeShaderGrp.get(Z3DRendererBase::Normal);
    m_rendererBase->setMaterialSpecular(glm::vec4(.1f, .1f, .1f, .1f));
    m_rendererBase->setGlobalShaderParameters(oit3DTransparentizeShader, eye);

    //
    oit3DTransparentizeShader.setUniformValue("lighting_enabled", m_needLighting);
    oit3DTransparentizeShader.setUniformValue("pos_scale", getCoordScales());
    oit3DTransparentizeShader.setUniformValue("uColor", m_color);

    // size of view
    //  double theta, neardist, w, h;
    //  theta = glm::degrees(m_rendererBase->getCamera().getFieldOfView());
    //  neardist = m_rendererBase->getCamera().getNearDist();
    //  h = neardist * glm::tan(theta);
    //  w = m_rendererBase->getCamera().getAspectRatio() * h;

    //
    if (m_hardwareSupportVAO) {
        if (m_dataChanged) {

            // oit pass
            GLint loc_position = oit3DTransparentizeShader.attributeLocation("vPosition");
            GLint loc_normal = oit3DTransparentizeShader.attributeLocation("vNormal");

            //
            glGenVertexArrays(1, &m_vaoSurf);
            glBindVertexArray( m_vaoSurf );

            glGenBuffers(1, &m_vboSurf);
            glBindBuffer( GL_ARRAY_BUFFER, m_vboSurf);

            glBufferData( GL_ARRAY_BUFFER, size_positions + size_normalIndices, NULL, GL_STATIC_DRAW );
            glBufferSubData( GL_ARRAY_BUFFER, 0, size_positions, &(positions[0]) );
            glBufferSubData( GL_ARRAY_BUFFER, size_positions, size_normalIndices, &(normalIndices[0]) );

            glEnableVertexAttribArray( loc_position );
            glVertexAttribPointer( loc_position, 3, GL_FLOAT, GL_FALSE, 0, (GLvoid*)(0) );

            glEnableVertexAttribArray( loc_normal );
            glVertexAttribPointer( loc_normal, 1, GL_FLOAT, GL_FALSE, 0, (GLvoid*)(size_positions) );

            glBindBuffer( GL_ARRAY_BUFFER, 0);
            glBindVertexArray(0);

            // compositing
            // vao
//            glGenVertexArrays(1, &m_vao);
//            glBindVertexArray( m_vao );
//            //oit2DComposeProgram->setAttributeArray("composeVertPos", m_screen.constData());
//            //oit2DComposeProgram->enableAttributeArray("composeVertPos");

//            glGenBuffers(1, &m_vbo);
//            glBindBuffer( GL_ARRAY_BUFFER, m_vbo);
//            glBufferData( GL_ARRAY_BUFFER, m_screen.size()*sizeof(QVector3D), m_screen.constData(), GL_STATIC_DRAW);

//            GLint posLoc = glGetAttribLocation(oit2DComposeProgram->programId(), "composeVertPos");
//            glEnableVertexAttribArray(posLoc);
//            glVertexAttribPointer( posLoc, 3, GL_FLOAT, GL_FALSE, 0, (GLvoid*)(0));

//            glBindVertexArray(0);

            //
            m_dataChanged = false;
        }

        // render
        //glBindFramebuffer(GL_FRAMEBUFFER, m_fbo); // render to an offscreen framebuffer
//        glBindFramebuffer(GL_FRAMEBUFFER, m_preFBO);
//        glClearColor(0.0f,0.0f,0.0f,1.0f);
//        glClearDepth(1.0f);
//        glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

//        glEnable(GL_DEPTH_TEST);
//        glEnable(GL_TEXTURE_2D);
//        glEnable(GL_MULTISAMPLE);

//        glDepthMask(GL_FALSE);
//        glEnable(GL_BLEND);

        // 3D oit pass
        //glBlendFuncSeparate(GL_ONE, GL_ONE, GL_ZERO, GL_ONE_MINUS_SRC_ALPHA);

        glBindVertexArray( m_vaoSurf );
        glDrawArrays( GL_TRIANGLES, 0, positions.size() );
        glBindVertexArray(0);

        //glBindFramebuffer(GL_FRAMEBUFFER, m_preFBO);

        //glDepthMask(GL_TRUE);
        //glDisable(GL_BLEND);

        // 2D compositing pass
        //
//        if (!oit2DComposeProgram->link())
//        {
//            qWarning() << oit2DComposeProgram->log() << endl;
//        }

//        if (!oit2DComposeProgram->bind())
//        {
//            qWarning() << oit2DComposeProgram->log() << endl;
//        }

//        //
//        glBlendFunc(GL_ONE_MINUS_SRC_ALPHA, GL_SRC_ALPHA);

////        glBindFramebuffer(GL_FRAMEBUFFER, 0);
////        glClearColor(0.0f,0.0f,0.0f,1.0f);
////        glClearDepth(1.0f);
////        glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

//        //
//        glActiveTexture(GL_TEXTURE0);
//        glEnable(GL_TEXTURE_2D);
//        glBindTexture(GL_TEXTURE_2D, m_accumTexture);

//        GLuint cloc_accum = glGetUniformLocation(oit2DComposeProgram->programId(), "accumTexture");
//        glUniform1i(cloc_accum, 0);

//        glActiveTexture(GL_TEXTURE1);
//        glEnable(GL_TEXTURE_2D);
//        glBindTexture(GL_TEXTURE_2D, m_revealageTexture);

//        GLuint cloc_revealage = glGetUniformLocation(oit2DComposeProgram->programId(), "revealageTexture");
//        glUniform1i(cloc_revealage, 1);

//        GLenum DrawBuffers[2] = {GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1};
//        glDrawBuffers(2, DrawBuffers); // "2" is the size of DrawBuffers

//        glBindVertexArray( m_vao );
//        glDrawArrays( GL_TRIANGLES, 0, m_screen.size());
//        glBindVertexArray(0);

        //glBindFramebuffer(GL_FRAMEBUFFER, m_preFBO);

        //glDisable(GL_BLEND);
    } else {
        // w/o vao defined
    }

    m_cubeShaderGrp.release();

    qDebug()<<"*** roi is rendered";
}

void Z3DCubeRenderer::renderPicking(Z3DEye eye)
{
}

bool Z3DCubeRenderer::isEmpty()
{
    return positions.empty();
}
