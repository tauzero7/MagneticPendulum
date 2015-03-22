/**
    Copyright (c) 2014, Universitaet Stuttgart, VISUS, SFB 716, Thomas Mueller

    MPSim is licensed under a Creative Commons
    Attribution-ShareAlike 3.0 Unported License.

    http://creativecommons.org/licenses/by-sa/3.0/deed.en_US

    @file OpenGL2d.cpp
*/

#include "OpenGL2d.h"
#include "glutils.h"

#include <QCoreApplication>
#include <QDir>
#include <QKeyEvent>
#include <QMouseEvent>
#include <QMessageBox>

/**
 * @brief DebugCallback
 * @param source
 * @param type
 * @param id
 * @param severity
 * @param length
 * @param message
 * @param userParam
 */
void DebugCallback( unsigned int source, unsigned int type, unsigned int id,
                    unsigned int severity, int , const char* message, void* ) {
    const char *sourceText, *typeText, *severityText;
    switch(source) {
        case GL_DEBUG_SOURCE_API:
            sourceText = "API";
            break;
        case GL_DEBUG_SOURCE_WINDOW_SYSTEM:
            sourceText = "Window System";
            break;
        case GL_DEBUG_SOURCE_SHADER_COMPILER:
            sourceText = "Shader Compiler";
            break;
        case GL_DEBUG_SOURCE_THIRD_PARTY:
            sourceText = "Third Party";
            break;
        case GL_DEBUG_SOURCE_APPLICATION:
            sourceText = "Application";
            break;
        case GL_DEBUG_SOURCE_OTHER:
            sourceText = "Other";
            break;
        default:
            sourceText = "Unknown";
            break;
    }
    switch(type) {
        case GL_DEBUG_TYPE_ERROR:
            typeText = "Error";
            break;
        case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR:
            typeText = "Deprecated Behavior";
            break;
        case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:
            typeText = "Undefined Behavior";
            break;
        case GL_DEBUG_TYPE_PORTABILITY:
            typeText = "Portability";
            break;
        case GL_DEBUG_TYPE_PERFORMANCE:
            typeText = "Performance";
            break;
        case GL_DEBUG_TYPE_OTHER:
            typeText = "Other";
            break;
        case GL_DEBUG_TYPE_MARKER:
            typeText = "Marker";
            break;
        default:
            typeText = "Unknown";
            break;
    }
    switch(severity) {
        case GL_DEBUG_SEVERITY_HIGH:
            severityText = "High";
            break;
        case GL_DEBUG_SEVERITY_MEDIUM:
            severityText = "Medium";
            break;
        case GL_DEBUG_SEVERITY_LOW:
            severityText = "Low";
            break;
        case GL_DEBUG_SEVERITY_NOTIFICATION:
            severityText = "Notification";
            break;
        default:
            severityText = "Unknown";
            break;
    }
    static char outputstring[4096];
#ifdef _WIN32
    sprintf_s(outputstring, 4096, "[%s %s] (%s %u) %s\n", sourceText, severityText, typeText, id, message);
    OutputDebugStringA(outputstring);
#else
    sprintf(outputstring, "[%s %s] (%s %u) %s\n", sourceText, severityText, typeText, id, message);
#endif
    printf("%s",outputstring);
}

/**
 * @brief OpenGL2d::OpenGL2d
 * @param format
 * @param sd
 * @param parent
 */
OpenGL2d :: OpenGL2d( QGLFormat format, SystemData* sd, QWidget* parent )
    : QGLWidget(format, parent),
      mSysData(sd),
      m_fbo(0),m_rbo(0),m_fboTexture(0)
{
    mButtonPressed = Qt::NoButton;
    setFocusPolicy( Qt::ClickFocus );
    mKeyPressed = Qt::Key_No;

    setMinimumSize( 300, 300 );

    setMouseTracking(true);

    QDir appDir = QDir(QCoreApplication::applicationDirPath());
    //appDir.cd(relPath.c_str());
    QString pathNameShaders = appDir.absolutePath() + QString("/shaders/");

    mQuadVertShaderName = pathNameShaders + "quad.vert";
    mQuadFragShaderName = pathNameShaders + "quad.frag";

    mLineVertShaderName = pathNameShaders + "line.vert";
    mLineFragShaderName = pathNameShaders + "line.frag";

    mMagnetVertShaderName = pathNameShaders + "magnet.vert";
    mMagnetGeomShaderName = pathNameShaders + "magnet.geom";
    mMagnetFragShaderName = pathNameShaders + "magnet.frag";

    mPendVertShaderName = pathNameShaders + "pendulum.vert";
    mPendFragShaderName = pathNameShaders + "pendulum.frag";
    mPendCompShaderName = pathNameShaders + "pendulum.comp";

    vboLine = vaLine = 0;
    posInit = posSSbo[0] = posSSbo[1] = 0;
    rkStep = timeID = posMag = colMag = 0;

    initColor = glm::vec3(0.2,0.2,0.2);
    hInit = 0.001f;
    activeMagnet = -1;
}

/**
 * @brief OpenGL2d::~OpenGL2d
 */
OpenGL2d::~OpenGL2d() {
    mQuadShader.RemoveAllShaders();
    mLineShader.RemoveAllShaders();
    mMagnetShader.RemoveAllShaders();
    mPendShader.RemoveAllShaders();
#ifdef HAVE_COMP_SHADER    
    mPendIntShader.RemoveAllShaders();
#endif // HAVE_COMP_SHADER

    if (vaLine>0) {
        glDeleteBuffers(1,&vboLine);
        glDeleteVertexArrays(1,&vaLine);
    }
    if (vaQuad>0) {
        glDeleteBuffers(1,&vboQuad);
        glDeleteVertexArrays(1,&vaQuad);
    }
    if (vaPoints>0) {
        glDeleteBuffers(1,&vboPoints);
        glDeleteVertexArrays(1,&vaPoints);
    }
}

/**
 * @brief OpenGL2d::minimumSizeHint
 * @return
 */
QSize OpenGL2d::minimumSizeHint() const {
    return QSize( 512,512 );
}

/** This property holds the recommended size for the widget.
*/
QSize OpenGL2d::sizeHint() const {
    return QSize( 512,512 );
}

/**
 * @param engine  Pointer to main script engine.
 */
void OpenGL2d::AddObjectsToScriptEngine( QScriptEngine* engine ) {
    QScriptValue ogl = engine->newQObject(this);
    engine->globalObject().setProperty("OGL",ogl);
}

void OpenGL2d::GrabWindow( QString filename ) {
    QImage img = this->grabFrameBuffer();
    img.save(filename);
}

/**
 * @brief OpenGL2d::SetMaxNumPoints
 * @param maxNumPoints
 */
void OpenGL2d::SetMaxNumPoints( int maxNumPoints ) {
    makeCurrent();
    if (maxNumPoints > 1) {
        mSysData->m_maxNumPoints = maxNumPoints;

        if (vaLine>0) {
            glDeleteBuffers(1,&vboLine);
            glDeleteVertexArrays(1,&vaLine);
        }
        glGenVertexArrays(1,&vaLine);
        glGenBuffers(1,&vboLine);

        glBindVertexArray(vaLine);
        glBindBuffer(GL_ARRAY_BUFFER,vboLine);
        glEnableVertexAttribArray(0);
        glBufferData(GL_ARRAY_BUFFER, sizeof(float)*maxNumPoints*4,NULL,GL_DYNAMIC_DRAW);
        glVertexAttribPointer(0,4,GL_FLOAT,GL_FALSE,0,NULL);
        glBindVertexArray(0);

        if (mSysData->m_trajectory != NULL) {
            delete [] mSysData->m_trajectory;
            mSysData->m_trajTime.clear();
        }
        mSysData->m_trajectory = new float[4*maxNumPoints];
        mSysData->m_currIndex = 0;
        mSysData->m_currAnimPos = glm::vec2(0);
        mSysData->m_numPoints = 0;
    }
}

void OpenGL2d::UpdateTraj() {
    makeCurrent();
    mSysData->UpdateTrajectory(&vboLine);
    updateGL();
}

void  OpenGL2d::ResetParticleSimulation() {
    resetParticleStorage();
    updateGL();
}

/**
 * Create framebuffer object
 * @param width      Width of FBO.
 * @param height     Height of FBO.
 * @return
 */
bool OpenGL2d::CreateFBO( int width, int height ) {
    if (width<1 || height<1) {
        return false;
    }
    // delete previous fbo
    DeleteFBO();

    glGenTextures(1,&m_fboTexture);
    glBindTexture(GL_TEXTURE_2D,m_fboTexture);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexImage2D(GL_TEXTURE_2D,0,GL_RGBA,width,height,0,GL_RGBA,GL_UNSIGNED_BYTE,NULL);

    glGenFramebuffers(1,&m_fbo);
    glGenRenderbuffers(1,&m_rbo);

    glBindFramebuffer(GL_FRAMEBUFFER, m_fbo);
    glFramebufferTexture2D(GL_FRAMEBUFFER,GL_COLOR_ATTACHMENT0,GL_TEXTURE_2D,m_fboTexture,0);

    glBindRenderbuffer(GL_RENDERBUFFER, m_rbo);
    glRenderbufferStorage(GL_RENDERBUFFER,GL_DEPTH_COMPONENT24,width,height);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER,GL_DEPTH_ATTACHMENT,GL_RENDERBUFFER,m_rbo);

    GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    switch (status) {
        default:
            fprintf(stderr,"ERROR: Cannot initialize FBO\n");
            break;
        case GL_FRAMEBUFFER_COMPLETE:
            fprintf(stderr,"FBO complete.\n");
            break;
        case GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT:
            fprintf(stderr,"ERROR: FBO attachment incomplete.\n");
            break;
    }

    return true;
}

/**
 * Delete framebuffer object.
 */
void OpenGL2d::DeleteFBO() {
    if (m_fbo!=0) {
        glDeleteFramebuffers(1,&m_fbo);
        m_fbo = 0;
    }
    if (m_rbo!=0) {
        glDeleteRenderbuffers(1,&m_rbo);
        m_rbo = 0;
    }
    if (m_fboTexture!=0) {
        glDeleteTextures(1,&m_fboTexture);
        m_fboTexture = 0;
    }
}

void OpenGL2d::particleStep() {    
    makeCurrent();
#ifdef HAVE_COMP_SHADER    
    glBindBufferBase( GL_SHADER_STORAGE_BUFFER, 0, posSSbo[currSbo] );
    glBindBufferBase( GL_SHADER_STORAGE_BUFFER, 1, posSSbo[nextSbo] );
    glBindBufferBase( GL_SHADER_STORAGE_BUFFER, 2, posMag );
    glBindBufferBase( GL_SHADER_STORAGE_BUFFER, 3, colMag );
    glBindBufferBase( GL_SHADER_STORAGE_BUFFER, 4, rkStep );
    glBindBufferBase( GL_SHADER_STORAGE_BUFFER, 5, timeID );

    mPendIntShader.Bind();

#ifdef USE_SPHERICAL
    glUniform1i( mPendIntShader.GetUniformLocation("useSpherical"),1);
#else
    glUniform1i( mPendIntShader.GetUniformLocation("useSpherical"),0);
#endif
    glUniform1i( mPendIntShader.GetUniformLocation("numParticles"), numParticles );
    glUniform1i( mPendIntShader.GetUniformLocation("numMagnets"), mSysData->m_magnets.size() );
    glUniform1f( mPendIntShader.GetUniformLocation("pendulumLength"), static_cast<float>(mSysData->m_pendulumLength) );
    glUniform1f( mPendIntShader.GetUniformLocation("pendulumHeight"), static_cast<float>(mSysData->m_pendulumHeight) );
    glUniform1f( mPendIntShader.GetUniformLocation("gravity"), static_cast<float>(mSysData->m_gravity) );
    glUniform1f( mPendIntShader.GetUniformLocation("kappa"), static_cast<float>(mSysData->m_kappa) );
    glUniform1f( mPendIntShader.GetUniformLocation("gamma"), static_cast<float>(mSysData->m_damping) );
    glUniform1f( mPendIntShader.GetUniformLocation("magFactor"), static_cast<float>(mSysData->m_magFactor) );
    glDispatchCompute(numParticles/128 + 1,1,1);

    glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
    mPendIntShader.Release();
    std::swap(currSbo,nextSbo);

    mSysData->m_numSteps++;
#endif // HAVE_COMP_SHADER    
    updateGL();
}


// *********************************** protected methods ******************************
void OpenGL2d::initializeGL() {
    fprintf(stderr,"Initialize OpenGL...\n");
    if (gl3wInit()) {
        fprintf(stderr,"OpenGL::Error: Failed to initialize gl3w.\n");
        exit(1);
    }
    fprintf(stderr,"Graphics board details:\n");
    fprintf(stderr,"\tVendor:         %s\n",glGetString(GL_VENDOR));
    fprintf(stderr,"\tGPU:            %s\n",glGetString(GL_RENDERER));
    fprintf(stderr,"\tOpenGL version: %s\n",glGetString(GL_VERSION));
    fprintf(stderr,"\tGLSL version:   %s\n\n",glGetString(GL_SHADING_LANGUAGE_VERSION));

    if (!gl3wIsSupported(3,3)) {
        fprintf(stderr,"Error: OpenGL 3.3 or higher is not supported.\n");
        exit(1);
    }

#ifdef HAVE_COMP_SHADER
    if (!isExtensionAvailable("GL_ARB_compute_shader")) {
        fprintf(stderr,"Error: Compute shader not available!\n");
        exit(1);
    }
#endif    

    if (!setSyncToVBlank(0)) {
        //QMessageBox::warning(this,"Performance warning","SyncToVBlank could not be disabled. This might lead to high performance lost!");
        fprintf(stderr,"SyncToVBlank could not be disabled. This might lead to high performance lost!");
    }

#ifndef NDEBUG
    /*
    if (isExtensionAvailable("GL_KHR_debug")) {
        glEnable(GL_DEBUG_OUTPUT);
        glDebugMessageCallback(DebugCallback, NULL);
        glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, NULL, GL_TRUE);
        GLuint ignorethis;
        ignorethis = 131185;
        glDebugMessageControl(GL_DEBUG_SOURCE_API, GL_DEBUG_TYPE_OTHER, GL_DONT_CARE, 1, &ignorethis, GL_FALSE);
        ignorethis = 131184;
        glDebugMessageControl(GL_DEBUG_SOURCE_API, GL_DEBUG_TYPE_OTHER, GL_DONT_CARE, 1, &ignorethis, GL_FALSE);
        ignorethis = 131204;
        glDebugMessageControl(GL_DEBUG_SOURCE_API, GL_DEBUG_TYPE_OTHER, GL_DONT_CARE, 1, &ignorethis, GL_FALSE);
    }
    */
#endif

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);


    // ------------------------------------------
    //  generate vertex array for quad drawing
    // ------------------------------------------
    glGenVertexArrays(1,&vaQuad);
    glGenBuffers(1,&vboQuad);

    glBindVertexArray(vaQuad);
    glBindBuffer(GL_ARRAY_BUFFER,vboQuad);
    glEnableVertexAttribArray(0);
    glBufferData(GL_ARRAY_BUFFER,sizeof(GLfloat)*4*2,quadVerts, GL_STATIC_DRAW);
    glVertexAttribPointer(0,2,GL_FLOAT,GL_FALSE,0,NULL);
    glBindVertexArray(0);


    // ------------------------------------------
    //  set ...
    // ------------------------------------------
    createShaders();
    //resetParticleStorage();
    SetMaxNumPoints(1500);
}

/**
 * @brief OpenGL2d::isExtensionAvailable
 * @param extName
 * @return
 */
bool OpenGL2d::isExtensionAvailable( const char* extName ) {
    int numExt;
    glGetIntegerv( GL_NUM_EXTENSIONS, &numExt);
    for(int x=0; x<numExt; x++) {
        const char* ptrToExt = strstr(reinterpret_cast<const char*>(glGetStringi(GL_EXTENSIONS,x)), extName);
        if (ptrToExt != NULL) {
            return true;
        }
    }
    return false;
}

/**
 * @brief OpenGL2d::paintGL
 */
void OpenGL2d::paintGL() {
    glViewport( 0, 0, width(), height() );

    glClearColor(0.0f,0.0f,0.0f,0.0f);
    glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

    glm::mat4 mvp = glm::ortho(0.0f,1.0f,0.0f,1.0f);

    glDisable( GL_DEPTH_TEST);

    mQuadShader.Bind();
    glUniformMatrix4fv( mQuadShader.GetUniformLocation("mvp"), 1, GL_FALSE, glm::value_ptr(mvp) );
    glBindVertexArray(vaQuad);
    // glDrawArrays( GL_TRIANGLE_STRIP, 0, 4);
    glBindVertexArray(0);
    mQuadShader.Release();

    float rx = static_cast<float>(mSysData->m_rmaxX);
    float ry = static_cast<float>(mSysData->m_rmaxY);
    mvp = glm::ortho( -rx, rx, -ry, ry );
    //fprintf(stderr,"rs: %f %f\n",rx,ry);

#ifdef HAVE_COMP_SHADER
    if (posSSbo[0]>0) {
        mPendShader.Bind();
        glUniformMatrix4fv( mPendShader.GetUniformLocation("mvp"), 1, GL_FALSE, glm::value_ptr(mvp) );
        glUniform1f( mPendShader.GetUniformLocation("tScale"), static_cast<float>(mSysData->m_tScale) );
        glUniform1f( mPendShader.GetUniformLocation("pendulumLength"), static_cast<float>(mSysData->m_pendulumLength) );
#ifdef USE_SPHERICAL
        glUniform1i( mPendShader.GetUniformLocation("useSpherical"), 1 );
#else
        glUniform1i( mPendShader.GetUniformLocation("useSpherical"), 0 );
#endif
        glPointSize(2);        
        // initial position
        // glBindBuffer(GL_ARRAY_BUFFER,posSSbo[currSbo]);
        glBindBuffer(GL_ARRAY_BUFFER,posInit);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer( 0, 4, GL_FLOAT, GL_FALSE, 0, NULL );

        // color
        glBindBuffer(GL_ARRAY_BUFFER,rkStep); // maps to color

        glEnableVertexAttribArray(1);
        glVertexAttribPointer( 1, 4, GL_FLOAT, GL_FALSE, 0, NULL );

        glBindBuffer(GL_ARRAY_BUFFER,timeID);
        glEnableVertexAttribArray(2);
        glVertexAttribPointer( 2, 1, GL_FLOAT, GL_FALSE, 0, NULL );

        glDrawArrays(GL_POINTS,0,numParticles);

        glBindBuffer(GL_ARRAY_BUFFER,0);
        mPendShader.Release();
    }
#endif // HAVE_COMP_SHADER    

    mMagnetShader.Bind();
    glUniformMatrix4fv( mMagnetShader.GetUniformLocation("mvp"),1,GL_FALSE,glm::value_ptr(mvp));
    glUniform1f( mMagnetShader.GetUniformLocation("magnetSize"), mSysData->m_magnetSize );
    for(int m=0; m<mSysData->m_magnets.size(); m++) {
        glUniform1i( mMagnetShader.GetUniformLocation("isActiveMagnet"), (m==activeMagnet?1:0) );
        glm::vec3 pos = mSysData->m_magnets[m].pos;
        glm::vec4 col = mSysData->m_magnets[m].color;
        glUniform2f( mMagnetShader.GetUniformLocation("in_pos"), pos.x, pos.y );
        glUniform3f( mMagnetShader.GetUniformLocation("in_col"), col.r, col.g, col.b );
        glDrawArrays(GL_POINTS,0,1);
    }
    mMagnetShader.Release();

    if (vaLine>0 && mSysData->m_numPoints>0) {
        QColor lc = mSysData->m_lineColor;

        glLineWidth( mSysData->m_lineWidth );
        mLineShader.Bind();
        glUniformMatrix4fv( mLineShader.GetUniformLocation("mvp"), 1, GL_FALSE, glm::value_ptr(mvp) );
        glUniform3f( mLineShader.GetUniformLocation("lineColor"), lc.redF(), lc.greenF(), lc.blueF() );
        glBindVertexArray(vaLine);
        glDrawArrays(GL_LINE_STRIP, 0, mSysData->m_numPoints );
        glBindVertexArray(0);
        mLineShader.Release();
        glLineWidth(1);
    }
}

/**
 * @brief OpenGL2d::resizeGL
 * @param w
 * @param h
 */
void OpenGL2d::resizeGL( int w, int h ) {
    glViewport(0,0,w,h);
#ifndef HAVE_COMP_SHADER  
    resetParticleStorage();
#endif
}

/**
 * @brief OpenGL2d::keyPressEvent
 * @param event
 */
void OpenGL2d::keyPressEvent( QKeyEvent* event ) {
    mKeyPressed  = event->key();
    mKeyModifier = event->modifiers();
    switch (mKeyPressed)
    {
        case Qt::Key_S: {
            makeCurrent();
            mQuadShader.RemoveAllShaders();
            mLineShader.RemoveAllShaders();
            mPendShader.RemoveAllShaders();
#ifdef HAVE_COMP_SHADER
            mPendIntShader.RemoveAllShaders();
#endif // HAVE_COMP_SHADER            
            createShaders();
            updateGL();
            break;
        }
    }
    event->accept();
}

/**
 * @brief OpenGL2d::keyReleaseEvent
 * @param event
 */
void OpenGL2d::keyReleaseEvent( QKeyEvent* event ) {
    mKeyPressed  = Qt::Key_No;
    mKeyModifier = Qt::Key_No;
    event->accept();
}

/**
 * @brief OpenGL2d::mousePressEvent
 * @param event
 */
void OpenGL2d::mousePressEvent( QMouseEvent * event ) {
    mButtonPressed = event->button();
    if (mButtonPressed == Qt::LeftButton) {
        double mx,my;
        pixelToPos(event->pos().x(),event->pos().y(),mx,my);
        mSysData->m_currAnimPos = glm::vec2(mx,my);
        mSysData->m_currAnimTime = 0.0;
        mSysData->m_currIndex = 0;
        mSysData->CalcTrajectory(mx,my);
        mSysData->UpdateTrajectory(&vboLine);
    }
    else if (mButtonPressed == Qt::MidButton) {
        mSysData->m_numPoints = 0;
    }
    event->accept();
    updateGL();
}

/**
 * @brief OpenGL2d::mouseReleaseEvent
 * @param event
 */
void OpenGL2d::mouseReleaseEvent( QMouseEvent * event ) {
    mButtonPressed = Qt::NoButton;
    event->accept();
    updateGL();
}

/**
 * @brief OpenGL2d::mouseMoveEvent
 * @param event
 */
void OpenGL2d::mouseMoveEvent( QMouseEvent * event ) {
    double mx,my;
    pixelToPos(event->pos().x(),event->pos().y(),mx,my);
    switch (mButtonPressed)
    {
        default:
            break;
        case Qt::LeftButton: {
            mSysData->CalcTrajectory(mx,my);
            mSysData->UpdateTrajectory(&vboLine);
            mSysData->m_currAnimPos = glm::vec2(mx,my);
            mSysData->m_currAnimTime = 0.0;
            mSysData->m_currIndex = 0;
            updateGL();
            break;
        }
        case Qt::MidButton: {
            break;
        }
        case Qt::RightButton: {
            if (activeMagnet>=0 && activeMagnet<mSysData->m_magnets.size()) {
                mSysData->m_magnets[activeMagnet].pos = glm::vec3( static_cast<float>(mx), static_cast<float>(my), mSysData->m_magnets[activeMagnet].pos.z );
                emit magnetMoved(activeMagnet);
            }
            break;
        }
    }

    activeMagnet = -1;
    double dist;
    for(int m=0; m<mSysData->m_magnets.size(); m++) {
        dist = glm::length( glm::dvec2(mx - mSysData->m_magnets[m].pos.x, my - mSysData->m_magnets[m].pos.y) );
        if (dist < mSysData->m_magnetSize) {
            activeMagnet = m;
            break;
        }
    }
    updateGL();
    event->accept();
}

/**
 * @brief OpenGL2d::createShaders
 */
void OpenGL2d::createShaders() {
    makeCurrent();

    fprintf(stderr,"Create quad shader with ...\n\t%s\n\t%s\n",
            mQuadVertShaderName.toStdString().c_str(),
            mQuadFragShaderName.toStdString().c_str());
    mQuadShader.CreateProgramFromFile(mQuadVertShaderName.toStdString().c_str(),
                                      mQuadFragShaderName.toStdString().c_str());

    fprintf(stderr,"Create line shader with ...\n\t%s\n\t%s\n",
            mLineVertShaderName.toStdString().c_str(),
            mLineFragShaderName.toStdString().c_str());
    mLineShader.CreateProgramFromFile(mLineVertShaderName.toStdString().c_str(),
                                      mLineFragShaderName.toStdString().c_str());

    fprintf(stderr,"Create magnet shader with ...\n\t%s\n\t%s\n\t%s\n",
            mMagnetVertShaderName.toStdString().c_str(),
            mMagnetGeomShaderName.toStdString().c_str(),
            mMagnetFragShaderName.toStdString().c_str());
    mMagnetShader.CreateProgramFromFile(mMagnetVertShaderName.toStdString().c_str(),
                                        mMagnetGeomShaderName.toStdString().c_str(),
                                        mMagnetFragShaderName.toStdString().c_str());

    fprintf(stderr,"Create pendulum shader with ...\n\t%s\n\t%s\n",
            mPendVertShaderName.toStdString().c_str(),
            mPendFragShaderName.toStdString().c_str());
    mPendShader.CreateProgramFromFile(mPendVertShaderName.toStdString().c_str(),
                                      mPendFragShaderName.toStdString().c_str());

#ifdef HAVE_COMP_SHADER
    fprintf(stderr,"Create pendulum integration shader with ...\n\t%s\n",mPendCompShaderName.toStdString().c_str());
    mPendIntShader.CreateEmptyProgram();
    mPendIntShader.AttachShaderFromFile(mPendCompShaderName.toStdString().c_str(), GL_COMPUTE_SHADER, true);
    mPendIntShader.Release();
#endif
}

/**
 * @brief OpenGL2d::resetParticleStorage
 */
void  OpenGL2d::resetParticleStorage() {
    makeCurrent();
    
    mSysData->m_rmax = mSysData->m_pendulumLength*sin(glm::radians(mSysData->m_maxTheta));

    double fw = static_cast<double>(width());
    double fh = static_cast<double>(height());
    double aspect = fw/fh;
    mSysData->m_rmaxX = mSysData->m_rmax * aspect;
    mSysData->m_rmaxY = mSysData->m_rmax;
    
#ifdef HAVE_COMP_SHADER    
    if (posSSbo[0]>0) {
        glDeleteBuffers(2,posSSbo);
        posSSbo[0] = posSSbo[1] = 0;
    }
    if (posInit>0) {
        glDeleteBuffers(1,&posInit);
        posInit = 0;
    }

    currSbo = 0;
    nextSbo = 1;

    std::vector<glm::vec2> initPos;

#ifdef USE_SPHERICAL
    //float rstep = 2.0f*rmax/static_cast<float>(width);
    float xstep = 2.0f*rmaxx/fw;
    float ystep = 2.0f*rmaxy/fh;
    for(unsigned int y=0; y<height; y++) {
        for(unsigned int x=0; x<width; x++) {
            float xx = -rmaxx + (x+0.5)*xstep;
            float yy = -rmaxy + (y+0.5)*ystep;
            float r = sqrtf(xx*xx + yy*yy);
            glm::vec2 pos = glm::vec2( asin(r/pendulumLength), atan2(yy,xx) );   // theta,phi
            initPos.push_back(pos);
        }
    }
#else
    double xstep = 2.0*mSysData->m_rmaxX/fw;
    double ystep = 2.0*mSysData->m_rmaxY/fh;
    for(int y=0; y<height(); y++) {
        for(int x=0; x<width(); x++) {
            float xx = static_cast<float>(-mSysData->m_rmaxX + x*xstep);
            float yy = static_cast<float>(-mSysData->m_rmaxY + y*ystep);

            glm::vec2 pos = glm::vec2(xx,yy) + 0.5f*glm::vec2(xstep,ystep);
            initPos.push_back(pos);
        }
    }
#endif

    numParticles = static_cast<int>(initPos.size());
    fprintf(stderr,"Reset particle storage with %d particles\n",numParticles);

    // ------------------------------------------
    //  buffer storage for ping-pong simulation
    // ------------------------------------------
    glGenBuffers(2,posSSbo);
    GLint bufMask = GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_BUFFER_BIT;

    glBindBuffer( GL_SHADER_STORAGE_BUFFER, posSSbo[0] );
    glBufferData( GL_SHADER_STORAGE_BUFFER, sizeof(float)*numParticles*4, NULL, GL_STREAM_DRAW );
    float *points = static_cast<float*>(glMapBufferRange( GL_SHADER_STORAGE_BUFFER, 0, sizeof(float)*numParticles*4, bufMask));
    for(int i=0; i<numParticles; i++) {
        points[4*i+0] = initPos[i].x;
        points[4*i+1] = initPos[i].y;
        points[4*i+2] = 0.0f;
        points[4*i+3] = 0.0f;
    }
    glUnmapBuffer( GL_SHADER_STORAGE_BUFFER );

    glBindBuffer( GL_SHADER_STORAGE_BUFFER, posSSbo[1] );
    glBufferData( GL_SHADER_STORAGE_BUFFER, sizeof(float)*numParticles*4, NULL, GL_STREAM_DRAW );
    float *zero = static_cast<float*>(glMapBufferRange( GL_SHADER_STORAGE_BUFFER, 0, sizeof(float)*numParticles*4, bufMask));
    for(int i=0; i<numParticles; i++) {
        zero[4*i+0] = 0.0f;
        zero[4*i+1] = 0.0f;
        zero[4*i+2] = 0.0f;
        zero[4*i+3] = 0.0f;
    }
    glUnmapBuffer( GL_SHADER_STORAGE_BUFFER );


    // ------------------------------------------
    //  initial position
    // ------------------------------------------

    glGenBuffers(1,&posInit);
    glBindBuffer( GL_COPY_WRITE_BUFFER, posInit );
    glBufferData( GL_COPY_WRITE_BUFFER, sizeof(float)*numParticles*4, NULL, GL_STREAM_COPY );
    glBindBuffer( GL_COPY_READ_BUFFER, posSSbo[0] );
    glCopyBufferSubData( GL_COPY_READ_BUFFER, GL_COPY_WRITE_BUFFER, 0, 0, sizeof(float)*numParticles*4 );
    glBindBuffer( GL_COPY_READ_BUFFER, 0 );
    glBindBuffer( GL_COPY_WRITE_BUFFER, 0 );

    initPos.clear();

    // ------------------------------------------
    //  buffer storage for stepsize
    // ------------------------------------------
    if (rkStep>0) {
        glDeleteBuffers(1,&rkStep);
    }
    glGenBuffers(1,&rkStep);
    glBindBuffer( GL_SHADER_STORAGE_BUFFER, rkStep );
    glBufferData( GL_SHADER_STORAGE_BUFFER, sizeof(float)*numParticles*4, NULL, GL_STREAM_DRAW );
    float *rs = static_cast<float*>(glMapBufferRange( GL_SHADER_STORAGE_BUFFER, 0, sizeof(float)*numParticles*4, bufMask));
    for(int i=0; i<numParticles; i++) {
        rs[4*i+0] = initColor.x;
        rs[4*i+1] = initColor.y;
        rs[4*i+2] = initColor.z;
        rs[4*i+3] = hInit;
    }
    glUnmapBuffer( GL_SHADER_STORAGE_BUFFER );

    // ------------------------------------------
    //  buffer storage for time id
    // ------------------------------------------
    if (timeID>0) {
        glDeleteBuffers(1,&timeID);
    }
    glGenBuffers(1,&timeID);
    glBindBuffer( GL_SHADER_STORAGE_BUFFER, timeID );
    glBufferData( GL_SHADER_STORAGE_BUFFER, sizeof(float)*numParticles*4, NULL, GL_STREAM_DRAW );
    float *tt = static_cast<float*>(glMapBufferRange( GL_SHADER_STORAGE_BUFFER, 0, sizeof(float)*numParticles, bufMask));
    for(int i=0; i<numParticles; i++) {
        tt[i] = 0;
    }
    glUnmapBuffer( GL_SHADER_STORAGE_BUFFER );

    // ------------------------------------------
    //  buffer storage for magnets
    // ------------------------------------------
    if (posMag>0) {
        glDeleteBuffers(1,&posMag);
    }
    glGenBuffers(1,&posMag);
    glBindBuffer( GL_SHADER_STORAGE_BUFFER, posMag );
    glBufferData( GL_SHADER_STORAGE_BUFFER, sizeof(float)*mSysData->m_magnets.size()*4, NULL, GL_STREAM_DRAW );
    float *mpos = static_cast<float*>(glMapBufferRange( GL_SHADER_STORAGE_BUFFER, 0, sizeof(float)*mSysData->m_magnets.size()*4, bufMask));
    for(int i=0; i<mSysData->m_magnets.size(); i++) {
        mpos[4*i+0] = mSysData->m_magnets[i].pos.x;
        mpos[4*i+1] = mSysData->m_magnets[i].pos.y;
        mpos[4*i+2] = mSysData->m_magnets[i].pos.z;
        mpos[4*i+3] = mSysData->m_magnets[i].alpha;
    }
    glUnmapBuffer( GL_SHADER_STORAGE_BUFFER );

    // ------------------------------------------
    //  buffer storage for colors
    // ------------------------------------------
    if (colMag>0) {
        glDeleteBuffers(1,&colMag);
    }
    glGenBuffers(1,&colMag);
    glBindBuffer( GL_SHADER_STORAGE_BUFFER, colMag );
    glBufferData( GL_SHADER_STORAGE_BUFFER, sizeof(float)*mSysData->m_magnets.size()*4, NULL, GL_STREAM_DRAW );
    float *mcol = static_cast<float*>(glMapBufferRange( GL_SHADER_STORAGE_BUFFER, 0, sizeof(float)*mSysData->m_magnets.size()*4, bufMask));
    for(int i=0; i<mSysData->m_magnets.size(); i++) {
        mcol[4*i+0] = mSysData->m_magnets[i].color.x;
        mcol[4*i+1] = mSysData->m_magnets[i].color.y;
        mcol[4*i+2] = mSysData->m_magnets[i].color.z;
        mcol[4*i+3] = mSysData->m_magnets[i].color.w;
    }
    glUnmapBuffer( GL_SHADER_STORAGE_BUFFER );
    glBindBuffer( GL_SHADER_STORAGE_BUFFER, 0 );
#endif // HAVE_COMP_SHADER
    mSysData->m_numSteps = 0;
}

/**
 * @brief OpenGL2d::pixelToPos
 * @param px
 * @param py
 * @param x
 * @param y
 */
void OpenGL2d::pixelToPos( int px, int py, double &x, double &y ) {
    x = (px - width()/2)/static_cast<double>(width()) * mSysData->m_rmaxX * 2.0;
    y = (height()/2 - py)/static_cast<double>(height()) * mSysData->m_rmaxY * 2.0;
}
