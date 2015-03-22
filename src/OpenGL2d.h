/**
    Copyright (c) 2014, Universitaet Stuttgart, VISUS, SFB 716, Thomas Mueller

    MPSim is licensed under a Creative Commons
    Attribution-ShareAlike 3.0 Unported License.

    http://creativecommons.org/licenses/by-sa/3.0/deed.en_US

    @brief Header file for 2D OpenGL rendering.
    @file OpenGL2dShader.h
*/

#ifndef MPSIM_OPENGL_2D_H
#define MPSIM_OPENGL_2D_H

#include <iostream>
#include <vector>

#include "GL3/gl3w.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "GLShader.h"
#include <SystemData.h>

#include <QGLWidget>
#include <QGLFormat>
#include <QMap>
#include <QScriptEngine>

/**
  *  @brief OpenGL render engine.
  *
  *  The OpenGL render engine is based on the QGLWidget. Several parameters can be changed
  *  by means of the properties listed as 'Q_PROPERTY' values. For example, change the
  *  camera's field of view by "OGL.fov = 45"; or get the current value by "OGL.fov".
  */
class OpenGL2d : public QGLWidget
{
    Q_OBJECT

public:
    OpenGL2d( QGLFormat format, SystemData* sd, QWidget* parent = 0 );
    ~OpenGL2d();

    virtual QSize minimumSizeHint() const;
    virtual QSize sizeHint() const;

public:
    void  AddObjectsToScriptEngine ( QScriptEngine* engine );   //!< Add this object to the script engine.
    void  GrabWindow( QString filename );
    void  ResetParticleSimulation();

    bool CreateFBO( int width, int height );  //!< Create framebuffer object.
    void DeleteFBO();                         //!< Delete framebuffer object.


public slots:
    void  particleStep();
    void  SetMaxNumPoints( int maxNumPoints );
    void  UpdateTraj();

signals:
    void magnetMoved(int);

protected:   
    virtual void initializeGL();             //!< Initialize OpenGL rendering.
    virtual void paintGL();                  //!< Main rendering method.
    virtual void resizeGL( int w, int h );   //!< Callback for OpenGL resize events.

    virtual void keyPressEvent( QKeyEvent* event );        //!< Evaluate key press event.
    virtual void keyReleaseEvent( QKeyEvent* event );      //!< Evaluate key release event.
    virtual void mouseMoveEvent( QMouseEvent* event );     //!< Evaluate mouse move event.
    virtual void mousePressEvent( QMouseEvent* event );    //!< Evaluate mouse press event.
    virtual void mouseReleaseEvent( QMouseEvent* event );  //!< Evaluate mouse release event.

    bool  isExtensionAvailable( const char* extName );
    void  createShaders();   //!< Create basic shaders for grid, axis, and objects rendering.

    void  resetParticleStorage();
    void  pixelToPos( int px, int py, double &x, double &y );

private:
    SystemData*       mSysData;

    int               mKeyPressed;
    int               mKeyModifier;
    Qt::MouseButton   mButtonPressed;    

    GLShader  mQuadShader;
    QString   mQuadVertShaderName;
    QString   mQuadFragShaderName;

    GLShader  mPendShader;
    GLShader  mPendIntShader;
    QString   mPendVertShaderName;
    QString   mPendFragShaderName;
    QString   mPendCompShaderName;

    GLShader  mMagnetShader;
    QString   mMagnetVertShaderName;
    QString   mMagnetGeomShaderName;
    QString   mMagnetFragShaderName;

    GLShader  mLineShader;
    QString   mLineVertShaderName;
    QString   mLineFragShaderName;

    // Framebuffer for 'periodic boundary rendering'
    GLuint m_fbo,m_rbo,m_fboTexture;
    GLuint vaQuad, vboQuad;
    GLuint vaLine, vboLine;
    GLuint posSSbo[2], posInit;
    GLuint rkStep,timeID, posMag, colMag;
    int    currSbo,nextSbo,numParticles;

    GLuint vaPoints,vboPoints;

    glm::vec3 initColor;
    float     hInit;
    int  activeMagnet;
};

#endif // MPSIM_OPENGL_2D_H
