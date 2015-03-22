/**
    Copyright (c) 2014, Universitaet Stuttgart, VISUS, SFB 716, Thomas Mueller

    MPSim is licensed under a Creative Commons
    Attribution-ShareAlike 3.0 Unported License.

    http://creativecommons.org/licenses/by-sa/3.0/deed.en_US

    @file OpenGL3d.cpp
*/

#include "OpenGL3d.h"
#include "glutils.h"

#include <QCoreApplication>
#include <QDir>
#include <QKeyEvent>
#include <QMouseEvent>
#include <QMessageBox>

#ifdef _WIN32
#define M_PI    3.1415926
#define M_PI_2  1.5707963
#endif

#define BUFFER_OFFSET(i) ((char *)NULL + (i))

const float scene_box_verts[] = {
    0.5f,-0.5f, 1.0f, 1.0f,
    0.5f, 0.5f, 1.0f, 1.0f,
    -0.5f, 0.5f, 1.0f, 1.0f,
    -0.5f,-0.5f, 1.0f, 1.0f,
    0.5f,-0.5f, 0.0f, 1.0f,
    0.5f, 0.5f, 0.0f, 1.0f,
    -0.5f, 0.5f, 0.0f, 1.0f,
    -0.5f,-0.5f, 0.0f, 1.0f
};

const unsigned int scene_box_idx[] = {
    0,1,3, 1,2,3, // top
    4,7,5, 5,7,6, // bottom
    4,5,0, 0,5,1, // front
    5,6,1, 1,6,2, // right
    6,7,2, 7,3,2, // back
    7,4,3, 4,0,3  // left
};


/**
 * @brief OpenGL3d::OpenGL3d
 * @param format
 * @param sd
 * @param parent
 */
OpenGL3d :: OpenGL3d( QGLFormat format, SystemData* sd, QWidget* parent )
    : QGLWidget(format, parent),
      mSysData(sd)
{
    mButtonPressed = Qt::NoButton;
    setFocusPolicy( Qt::ClickFocus );
    mKeyPressed = Qt::Key_No;

    setMinimumSize( 300, 300 );

    setMouseTracking(true);

    QDir appDir = QDir(QCoreApplication::applicationDirPath());
    QString pathNameShaders = appDir.absolutePath() + QString("/shaders/");

    mQuadVertShaderName = pathNameShaders + "quad.vert";
    mQuadFragShaderName = pathNameShaders + "quad.frag";

    mSceneVertShaderName = pathNameShaders + "scene.vert";
    mSceneGeomShaderName = pathNameShaders + "scene.geom";
    mSceneFragShaderName = pathNameShaders + "scene.frag";

    mRodVertShaderName = pathNameShaders + "rod.vert";
    mRodGeomShaderName = pathNameShaders + "rod.geom";
    mRodFragShaderName = pathNameShaders + "rod.frag";

    resetCamera();
    mSysData->m_currAnimPos = glm::vec2(0,0);

    fboID = 0;
    colAttachID[0] = colAttachID[1] = 0;
    dboID = 0;
    mPickID = -1;

    mCamera.setFovY(60.0f);
}

/**
 * @brief OpenGL3d::~OpenGL3d
 */
OpenGL3d::~OpenGL3d() {
    mQuadShader.RemoveAllShaders();
    mSceneShader.RemoveAllShaders();
    mRodShader.RemoveAllShaders();

    //... delete arrays, buffers
}

/**
 * @brief OpenGL3d::minimumSizeHint
 * @return
 */
QSize OpenGL3d::minimumSizeHint() const {
    return QSize( 512,512 );
}

QSize OpenGL3d::sizeHint() const {
    return QSize( 512,512 );
}

void OpenGL3d::GrabWindow( QString filename ) {
    QImage img = this->grabFrameBuffer();
    img.save(filename);
}


void OpenGL3d::resetCamera() {
    float camDist = 0.3f;
    cameraAngle = glm::vec2( glm::radians(60.0f), glm::radians(-90.0f) );
    mCamera.setPOI(0.0f,0.0f,0.0f);
    mCamera.setEyePos(camDist,0,0);
    mCamera.moveOnSphere(cameraAngle.x,cameraAngle.y);
}

void OpenGL3d::initializeGL() {
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
    createGeometry();
}


/**
 * @brief OpenGL2d::paintGL
 */
void OpenGL3d::paintGL() {
    drawToFBO();

    glViewport(0,0,width(),height());
    glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
    glm::mat4 orthoMX = glm::ortho(0.0f,1.0f,0.0f,1.0f);

    mQuadShader.Bind();
    glUniformMatrix4fv( mQuadShader.GetUniformLocation("mvp"), 1, GL_FALSE, glm::value_ptr(orthoMX) );
    glActiveTexture( GL_TEXTURE0 );
    glBindTexture( GL_TEXTURE_2D, colAttachID[0] );
    glUniform1i( mQuadShader.GetUniformLocation("tex"), 0 );
    glBindVertexArray(vaQuad);
    glDrawArrays(GL_TRIANGLE_STRIP,0,4);
    glBindVertexArray(0);
    glBindTexture( GL_TEXTURE_2D, 0 );
    mQuadShader.Release();
}

void OpenGL3d::drawCylinder(GLShader *shader) {
    assert(shader!=NULL);
    // mantle
    glUniform1i( shader->GetUniformLocation("haveNormals"), 1 );
    glBindBuffer( GL_ARRAY_BUFFER, vboCyl );
    glVertexAttribPointer(0,4,GL_FLOAT,GL_FALSE,0,NULL);
    glBindBuffer( GL_ARRAY_BUFFER, nboCyl );
    glVertexAttribPointer(1,3,GL_FLOAT,GL_FALSE,0,NULL);
    glDrawArrays( GL_TRIANGLE_STRIP, 2, numCylPoints*2);

    glUniform1i( shader->GetUniformLocation("haveNormals"), 0 );
    glBindBuffer( GL_ARRAY_BUFFER, vboCyl );
    glVertexAttribPointer(0,4,GL_FLOAT,GL_FALSE,sizeof(float)*4*2,NULL);
    glBindBuffer( GL_ARRAY_BUFFER, nboCyl );
    glVertexAttribPointer(1,3,GL_FLOAT,GL_FALSE,sizeof(float)*3*2,NULL);
    glDrawArrays( GL_TRIANGLE_FAN, 0, numCylPoints+1);

    glUniform1i( shader->GetUniformLocation("haveNormals"), 0 );
    glBindBuffer( GL_ARRAY_BUFFER, vboCyl );
    glVertexAttribPointer(0,4,GL_FLOAT,GL_FALSE,sizeof(float)*4*2,BUFFER_OFFSET(sizeof(float)*4));
    glBindBuffer( GL_ARRAY_BUFFER, nboCyl );
    glVertexAttribPointer(1,3,GL_FLOAT,GL_FALSE,sizeof(float)*3*2,BUFFER_OFFSET(sizeof(float)*3));
    glDrawArrays( GL_TRIANGLE_FAN, 0, numCylPoints+1);
}

/**
 * @brief OpenGL3d::resizeGL
 * @param w
 * @param h
 */
void OpenGL3d::resizeGL( int w, int h ) {
    glViewport(0,0,w,h);
    mCamera.setSizeAndAspect(w,h);
    initFBO(w,h);
}

/**
 * @brief OpenGL3d::keyPressEvent
 * @param event
 */
void OpenGL3d::keyPressEvent( QKeyEvent* event ) {
    mKeyPressed  = event->key();
    mKeyModifier = event->modifiers();
    switch (mKeyPressed)
    {
        case Qt::Key_S: {
            makeCurrent();
            mQuadShader.RemoveAllShaders();
            mSceneShader.RemoveAllShaders();
            mRodShader.RemoveAllShaders();
            createShaders();
            updateGL();
            break;
        }
        case Qt::Key_I: {
            resetCamera();
            updateGL();
            break;
        }
    }
    event->accept();
}

/**
 * @brief OpenGL3d::keyReleaseEvent
 * @param event
 */
void OpenGL3d::keyReleaseEvent( QKeyEvent* event ) {
    mKeyPressed  = Qt::Key_No;
    mKeyModifier = Qt::Key_No;
    event->accept();
}

/**
 * @brief OpenGL3d::mousePressEvent
 * @param event
 */
void OpenGL3d::mousePressEvent( QMouseEvent * event ) {
    mButtonPressed = event->button();
    mLastPos = event->pos();

    if (glIsFramebuffer(fboID)) {
        unsigned char buf[3];
        glBindFramebuffer(GL_READ_FRAMEBUFFER,fboID);
        glReadBuffer(GL_COLOR_ATTACHMENT0 + 1);
        glReadPixels(event->pos().x(),height()-event->pos().y(),1,1,GL_RGB,GL_UNSIGNED_BYTE,(void*)buf);
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER,0);

        int mID = mSysData->colorToId(buf) - MAGNET_COLOR_ID_OFFSET;
        if (event->modifiers().testFlag(Qt::ShiftModifier)) {
            if ((mID>=0 && mID<mSysData->m_magnets.size()) || mID==BOB_COLOR_ID-MAGNET_COLOR_ID_OFFSET) {
                mPickID = mID;
            }
        }
    }

    event->accept();
    updateGL();
}

/**
 * @brief OpenGL3d::mouseReleaseEvent
 * @param event
 */
void OpenGL3d::mouseReleaseEvent( QMouseEvent * event ) {
    mPickID = -1;
    mButtonPressed = Qt::NoButton;
    event->accept();
    updateGL();
}

/**
 * @brief OpenGL3d::mouseMoveEvent
 * @param event
 */
void OpenGL3d::mouseMoveEvent( QMouseEvent * event ) {
    glm::ivec2 d = glm::ivec2( event->pos().x() - mLastPos.x(), event->pos().y() - mLastPos.y() );

    if ((mPickID>=0 && mPickID < mSysData->m_magnets.size()) || mPickID==BOB_COLOR_ID-MAGNET_COLOR_ID_OFFSET) {
        glm::vec3 eye = mCamera.getEyePos();
        glm::vec3 td = mCamera.getViewDir(event->pos().x(),event->pos().y());

        if (mPickID>=0 && mPickID < mSysData->m_magnets.size()) {
            float lambda = -eye.z/td.z;
            glm::vec3 pos = eye + lambda*td;
            mSysData->m_magnets[mPickID].pos = pos;
            glm::vec2 currBobPos = mSysData->m_currAnimPos;
            mSysData->CalcTrajectory(currBobPos.x,currBobPos.y);
            emit magnetMoved(mPickID);
        } else {
            float lambda = (mSysData->m_pendulumHeight - mSysData->m_pendulumLength -eye.z)/td.z;
            glm::vec3 pos = eye + lambda*td;
            mSysData->CalcTrajectory(pos.x,pos.y);
            mSysData->m_currAnimPos = glm::vec2(pos.x,pos.y);
        }
        mSysData->m_currAnimTime = 0.0;
        mSysData->m_currIndex = 0;
        emit updateTraj();
        updateGL();
    } else {
        switch (mButtonPressed)
        {
            default:
                break;
            case Qt::LeftButton: {
                cameraAngle += glm::vec2((float)d.y,-(float)d.x)*0.01f;
                if (cameraAngle.x < -M_PI_2+0.001) cameraAngle[0] = static_cast<float>(-M_PI_2+0.001);
                if (cameraAngle.x > M_PI_2-0.001) cameraAngle[0] = static_cast<float>(M_PI_2-0.001);
                mCamera.moveOnSphere(cameraAngle.x,cameraAngle.y);
                //glm::vec3 eye = mCamera.getEyePos();
                updateGL();
                break;
            }
            case Qt::MidButton: {
                glm::vec3 poi = mCamera.getPOI();
                mCamera.setPOI(poi.x,poi.y,poi.z + d.y*0.01f);
                glm::vec3 pos = mCamera.getEyePos();
                mCamera.setEyePos(pos.x,pos.y,pos.z + d.y*0.01f);
                updateGL();
                break;
            }
            case Qt::RightButton: {
                float dist = mCamera.getSphereDist();
                dist += d.y*0.01f;
                mCamera.setSphereDist(dist);
                updateGL();
                break;
            }
        }
    }
    mLastPos = event->pos();
    event->accept();
}

/**
 * @brief OpenGL3d::createShaders
 */
void OpenGL3d::createShaders() {
    makeCurrent();

    // fprintf(stderr,"Create quad shader with ...\n\t%s\n\t%s\n",
    //       mQuadVertShaderName.toStdString().c_str(),
    //     mQuadFragShaderName.toStdString().c_str());
    mQuadShader.CreateProgramFromFile(mQuadVertShaderName.toStdString().c_str(),
                                      mQuadFragShaderName.toStdString().c_str());

    mSceneShader.CreateProgramFromFile(mSceneVertShaderName.toStdString().c_str(),
                                       mSceneGeomShaderName.toStdString().c_str(),
                                       mSceneFragShaderName.toStdString().c_str());

    mRodShader.CreateProgramFromFile(mRodVertShaderName.toStdString().c_str(),
                                     mRodGeomShaderName.toStdString().c_str(),
                                     mRodFragShaderName.toStdString().c_str());
}

/**
 * @brief OpenGL3d::createGeometry
 */
void OpenGL3d::createGeometry() {
    makeCurrent();

    // -------------------------------------
    //
    // -------------------------------------
    glGenVertexArrays(1,&vaBox);
    glGenBuffers(1,&vboBox);
    glGenBuffers(1,&iboBox);

    glBindVertexArray(vaBox);
    glBindBuffer( GL_ARRAY_BUFFER, vboBox );
    glBufferData( GL_ARRAY_BUFFER, sizeof(float)*8*4, scene_box_verts, GL_STATIC_DRAW );
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0,4,GL_FLOAT,GL_FALSE,0,NULL);

    glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, iboBox );
    glBufferData( GL_ELEMENT_ARRAY_BUFFER, sizeof(GLuint)*12*3, scene_box_idx, GL_STATIC_DRAW );

    glBindVertexArray(0);

    tablePos   = glm::vec3(0.0f,0.0f,-0.03f);
    tableScale = glm::vec3(1.0f,1.0f,0.02f);

    holder1Pos = glm::vec3(-0.5f,0.5f,-0.03f);
    holder1Scale = glm::vec3(0.05f,0.05f,2.0f);
    holder1Rot   = glm::vec3(0.0f,0.0f,1.0f);
    holder1RotAngle = 45.0f;

    holder2Pos = glm::vec3(-0.25f,0.25f,static_cast<float>(mSysData->m_pendulumHeight)-0.03f);
    holder2Scale = glm::vec3(0.1f,1.0f,0.05f);
    holder2Rot   = glm::vec3(0.0f,0.0f,1.0f);
    holder2RotAngle = 45.0f;

    // -------------------------------------
    //
    // -------------------------------------
    numCylPoints = 50;
    float* cylVerts = new float[(numCylPoints*2+2)*4];
    float* cptr = cylVerts;

    float* cylNormals = new float[(numCylPoints*2+2)*3];
    float* nptr = cylNormals;

    *(cptr++) = 0.0f;
    *(cptr++) = 0.0f;
    *(cptr++) = 0.5f;
    *(cptr++) = 1.0f;
    *(nptr++) = 0.0f;
    *(nptr++) = 0.0f;
    *(nptr++) = 1.0f;

    *(cptr++) = 0.0f;
    *(cptr++) = 0.0f;
    *(cptr++) =-0.5f;
    *(cptr++) = 1.0f;
    *(nptr++) = 0.0f;
    *(nptr++) = 0.0f;
    *(nptr++) =-1.0f;

    float astep = static_cast<float>(2.0*M_PI/(numCylPoints-1));
    for(int i=0; i<numCylPoints; i++) {
        *(cptr++) = 0.5f*cosf(i*astep);
        *(cptr++) = 0.5f*sinf(i*astep);
        *(cptr++) = 0.5f;
        *(cptr++) = 1.0f;
        *(nptr++) = cosf(i*astep);
        *(nptr++) = sinf(i*astep);
        *(nptr++) = 0.0f;

        *(cptr++) = 0.5f*cosf(i*astep);
        *(cptr++) = 0.5f*sinf(i*astep);
        *(cptr++) = -0.5f;
        *(cptr++) = 1.0f;
        *(nptr++) = cosf(i*astep);
        *(nptr++) = sinf(i*astep);
        *(nptr++) = 0.0f;
    }

    glGenVertexArrays(1,&vaCyl);
    glGenBuffers(1,&vboCyl);
    glGenBuffers(1,&nboCyl);

    glBindVertexArray(vaCyl);
    glBindBuffer( GL_ARRAY_BUFFER, vboCyl );
    glBufferData( GL_ARRAY_BUFFER, sizeof(float)*4*(numCylPoints*2+2), cylVerts, GL_STATIC_DRAW );
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0,4,GL_FLOAT,GL_FALSE,0,NULL);
    glBindBuffer( GL_ARRAY_BUFFER, 0 );

    glBindBuffer( GL_ARRAY_BUFFER, nboCyl );
    glBufferData( GL_ARRAY_BUFFER, sizeof(float)*3*(numCylPoints*2+2), cylNormals, GL_STATIC_DRAW );
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1,3,GL_FLOAT,GL_FALSE,0,NULL);
    glBindBuffer( GL_ARRAY_BUFFER, 0 );

    glBindVertexArray(0);
    delete [] cylVerts;
    delete [] cylNormals;
}

void OpenGL3d::createFBOTexture( GLuint &outID, const GLenum internalFormat, const GLenum format,
                                 const GLenum type, GLint filter, int width, int height ) {
    glGenTextures(1,&outID);
    glBindTexture(GL_TEXTURE_2D,outID);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER, filter);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER, filter);
    glTexImage2D(GL_TEXTURE_2D,0,internalFormat,width,height,0,format,type,NULL);
}

/**
 * Create framebuffer object
 * @param width      Width of FBO.
 * @param height     Height of FBO.
 * @return
 */
bool OpenGL3d::initFBO( int width, int height ) {
    if (width<1 || height<1) {
        return false;
    }
    // delete previous fbo
    deleteFBO();


    glGenFramebuffers(1,&fboID);
    glBindFramebuffer(GL_FRAMEBUFFER, fboID);

    // diffuse colors, textures
    createFBOTexture( colAttachID[0], GL_RGBA, GL_RGB, GL_UNSIGNED_BYTE, GL_LINEAR, width, height);
    glFramebufferTexture2D( GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, colAttachID[0], 0);

    // object ids
    createFBOTexture( colAttachID[1], GL_RGB, GL_RGB, GL_UNSIGNED_BYTE, GL_LINEAR, width, height);
    glFramebufferTexture2D( GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, colAttachID[1], 0);

    createFBOTexture( dboID, GL_DEPTH_COMPONENT32, GL_DEPTH_COMPONENT, GL_FLOAT, GL_LINEAR, width, height);
    glFramebufferTexture2D( GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, dboID, 0 );

    GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    switch (status) {
        case GL_FRAMEBUFFER_UNDEFINED: {
            fprintf(stderr,"FBO: undefined.\n");
            return false;
            break;
        }
        case GL_FRAMEBUFFER_COMPLETE: {
            //fprintf(stderr,"FBO: complete.\n");
            break;
        }
        case GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT: {
            fprintf(stderr,"FBO: incomplete attachment.\n");
            return false;
            break;
        }
        case GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT: {
            fprintf(stderr,"FBO: no buffers are attached to the FBO.\n");
            return false;
            break;
        }
        case GL_FRAMEBUFFER_UNSUPPORTED: {
            fprintf(stderr,"FBO: combination of internal buffer formats is not supported.\n");
            return false;
            break;
        }
        case GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE: {
            fprintf(stderr,"FBO: number of samples or the value for ... does not match.\n");
            return false;
            break;
        }
    }
    glBindFramebuffer( GL_FRAMEBUFFER, 0 );
    return true;
}

/**
 * Delete framebuffer object.
 */
void OpenGL3d::deleteFBO() {
    if (fboID!=0) {
        glDeleteFramebuffers(1,&fboID);
        fboID = 0;
    }

    if (dboID!=0) {
        glDeleteTextures(1,&dboID);
        dboID = 0;
    }
    if (colAttachID[0]!=0) {
        glDeleteTextures(2,colAttachID);
        colAttachID[0] = colAttachID[1] = 0;
    }
}

void OpenGL3d::drawToFBO() {
    if (!glIsFramebuffer(fboID)) {
        return;
    }

    unsigned int colAtt[2] = { GL_COLOR_ATTACHMENT0 + 0, GL_COLOR_ATTACHMENT0 + 1 };
    glBindFramebuffer( GL_FRAMEBUFFER, fboID );
    glDrawBuffers( 2, colAtt );

    glViewport( 0, 0, width(), height() );

    glClearColor(0.0f,0.0f,0.0f,0.0f);
    glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

    // float rx = static_cast<float>(mSysData->m_rmaxX);
    // float ry = static_cast<float>(mSysData->m_rmaxY);
    // tableScale = glm::vec3(2.0f*rx,2.0f*ry,0.1f);

    holder1Scale = glm::vec3(0.05f,0.05f,static_cast<float>(mSysData->m_pendulumLength));
    holder2Pos = glm::vec3(-0.25f,0.25f,static_cast<float>(mSysData->m_pendulumHeight)-0.03f);


    glm::mat4 projMX = mCamera.getProjMatrix();
    glm::mat4 viewMX = mCamera.getViewMatrix();

    glm::mat4 tableMX = glm::mat4();
    tableMX = glm::translate(tableMX,tablePos);
    tableMX = glm::scale(tableMX,tableScale);

    glm::mat4 holder1MX = glm::mat4();
    holder1MX = glm::translate(holder1MX,holder1Pos);
    holder1MX = glm::scale(holder1MX,holder1Scale);
    holder1MX = glm::rotate(holder1MX,holder1RotAngle,holder1Rot);

    glm::mat4 holder2MX = glm::mat4();
    holder2MX = glm::translate(holder2MX,holder2Pos);
    holder2MX = glm::rotate(holder2MX,holder2RotAngle,holder2Rot);
    holder2MX = glm::scale(holder2MX,holder2Scale);


    glEnable( GL_DEPTH_TEST );
    // glPolygonMode(GL_FRONT_AND_BACK,GL_LINE);

    glm::mat4 invMX = glm::inverse( viewMX );

    mSceneShader.Bind();
    glUniformMatrix4fv( mSceneShader.GetUniformLocation("projMX"), 1, GL_FALSE, glm::value_ptr(projMX) );
    glUniformMatrix4fv( mSceneShader.GetUniformLocation("viewMX"), 1, GL_FALSE, glm::value_ptr(viewMX) );
    glUniformMatrix4fv( mSceneShader.GetUniformLocation("invViewMX"), 1, GL_FALSE, glm::value_ptr(invMX) );

    QColor amb  = mSysData->ambientColor;
    QColor diff = mSysData->diffuseColor;
    QColor spec = mSysData->specularColor;

    glUniform3f( mSceneShader.GetUniformLocation("ambient"),  amb.redF(), amb.greenF(), amb.blueF() );
    glUniform3f( mSceneShader.GetUniformLocation("diffuse"),  diff.redF(), diff.greenF(), diff.blueF() );
    glUniform3f( mSceneShader.GetUniformLocation("specular"), spec.redF(), spec.greenF(), spec.blueF() );
    glUniform1f( mSceneShader.GetUniformLocation("k_amb"),  mSysData->k_ambient );
    glUniform1f( mSceneShader.GetUniformLocation("k_diff"), mSysData->k_diffuse );
    glUniform1f( mSceneShader.GetUniformLocation("k_spec"), mSysData->k_specular );
    glUniform1f( mSceneShader.GetUniformLocation("k_exp"),  mSysData->k_exp );

    glBindVertexArray(vaBox);
    glUniform1i( mSceneShader.GetUniformLocation("haveNormals"), 0 );
    glUniform1i( mSceneShader.GetUniformLocation("haveColors"), 0 );
    glUniform3f( mSceneShader.GetUniformLocation("pickIdCol"), 0.0f, 0.0f, 0.0f );

    // table
    glUniform1i( mSceneShader.GetUniformLocation("haveCheckerTex"), 1 );
    glUniformMatrix4fv( mSceneShader.GetUniformLocation("modelMX"), 1, GL_FALSE, glm::value_ptr(tableMX) );
    glDrawElements( GL_TRIANGLES, 12*3, GL_UNSIGNED_INT, NULL );
    glUniform1i( mSceneShader.GetUniformLocation("haveCheckerTex"), 0 );

    // holder1
    glUniformMatrix4fv( mSceneShader.GetUniformLocation("modelMX"), 1, GL_FALSE, glm::value_ptr(holder1MX) );
    glDrawElements( GL_TRIANGLES, 12*3, GL_UNSIGNED_INT, NULL );

    // holder2
    glUniformMatrix4fv( mSceneShader.GetUniformLocation("modelMX"), 1, GL_FALSE, glm::value_ptr(holder2MX) );
    glDrawElements( GL_TRIANGLES, 12*3, GL_UNSIGNED_INT, NULL );
    glBindVertexArray(0);

    // magnets
    glm::mat4 magnetMX;
    glUniform1i( mSceneShader.GetUniformLocation("haveColors"), 1 );
    glBindVertexArray(vaCyl);
    for(int m=0; m < mSysData->m_magnets.size(); m++) {
        magnetMX = glm::mat4();
        glm::vec3 pos = mSysData->m_magnets[m].pos;\
        pos[2] = -0.005f;
        magnetMX = glm::translate(magnetMX,pos);
        magnetMX = glm::scale(magnetMX,glm::vec3(0.01,0.01,0.01));

        glm::vec4 col = mSysData->m_magnets[m].color;
        glm::vec3 ic  = mSysData->m_magnets[m].idCol;

        glUniform3f( mSceneShader.GetUniformLocation("pickIdCol"), ic.x, ic.y, ic.z );
        glUniformMatrix4fv( mSceneShader.GetUniformLocation("modelMX"), 1, GL_FALSE, glm::value_ptr(magnetMX) );
        glUniform3f( mSceneShader.GetUniformLocation("matcolor"), col.r,col.g,col.b );
        drawCylinder(&mSceneShader);
    }


    QColor bobCol = mSysData->bobColor;

    double psi = asin(glm::length(mSysData->m_currAnimPos)/mSysData->m_pendulumLength);
    glm::vec3 bobpos = glm::vec3(mSysData->m_currAnimPos.x,mSysData->m_currAnimPos.y,static_cast<float>(mSysData->m_pendulumHeight - mSysData->m_pendulumLength*cos(psi)));
    magnetMX = glm::mat4();
    magnetMX = glm::translate(magnetMX,bobpos);
    magnetMX = glm::scale(magnetMX,glm::vec3(0.01,0.01,0.01));
    glUniformMatrix4fv( mSceneShader.GetUniformLocation("modelMX"), 1, GL_FALSE, glm::value_ptr(magnetMX) );
    glUniform3f( mSceneShader.GetUniformLocation("matcolor"), bobCol.redF(), bobCol.greenF(), bobCol.blueF() );
    glm::vec3 bcol = mSysData->idToColor(BOB_COLOR_ID);
    glUniform3f( mSceneShader.GetUniformLocation("pickIdCol"), bcol.x, bcol.y, bcol.z );
    drawCylinder(&mSceneShader);
    glBindVertexArray(0);
    mSceneShader.Release();


    glLineWidth(2);
    mRodShader.Bind();
    glUniformMatrix4fv( mRodShader.GetUniformLocation("projMX"), 1, GL_FALSE, glm::value_ptr(projMX) );
    glUniformMatrix4fv( mRodShader.GetUniformLocation("viewMX"), 1, GL_FALSE, glm::value_ptr(viewMX) );
    glUniform1f( mRodShader.GetUniformLocation("pendulumHeight"), static_cast<float>(mSysData->m_pendulumHeight) );
    glUniform1f( mRodShader.GetUniformLocation("pendulumLength"), static_cast<float>(mSysData->m_pendulumLength) );
    glUniform2f( mRodShader.GetUniformLocation("pos"), mSysData->m_currAnimPos.x, mSysData->m_currAnimPos.y );
    glDrawArrays(GL_POINTS,0,1);
    mRodShader.Release();
    glLineWidth(1);

    glPolygonMode(GL_FRONT_AND_BACK,GL_FILL);
    glDisable( GL_DEPTH_TEST );

    glBindFramebuffer( GL_FRAMEBUFFER, 0 );
}

