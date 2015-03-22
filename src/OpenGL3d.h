/**
    Copyright (c) 2014, Universitaet Stuttgart, VISUS, SFB 716, Thomas Mueller

    MPSim is licensed under a Creative Commons
    Attribution-ShareAlike 3.0 Unported License.

    http://creativecommons.org/licenses/by-sa/3.0/deed.en_US

    @brief Header file for 3D OpenGL rendering.
    @file OpenGL3dShader.h
*/

#ifndef MPSIM_OPENGL_3D_H
#define MPSIM_OPENGL_3D_H

#include <iostream>
#include <vector>

#include "GL3/gl3w.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "Camera.h"
#include "GLShader.h"
#include "SystemData.h"

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
class OpenGL3d : public QGLWidget
{
    Q_OBJECT

public:
    OpenGL3d( QGLFormat format, SystemData* sd, QWidget* parent = 0 );
    ~OpenGL3d();

    virtual QSize minimumSizeHint() const;
    virtual QSize sizeHint() const;

public:
    //void  AddObjectsToScriptEngine ( QScriptEngine* engine );   //!< Add this object to the script engine.
    void  GrabWindow( QString filename );

public slots:
    void  resetCamera();

signals:
    void  magnetMoved(int);
    void  updateTraj();

protected:   
    virtual void initializeGL();             //!< Initialize OpenGL rendering.
    virtual void paintGL();                  //!< Main rendering method.
    virtual void resizeGL( int w, int h );   //!< Callback for OpenGL resize events.

    virtual void keyPressEvent( QKeyEvent* event );        //!< Evaluate key press event.
    virtual void keyReleaseEvent( QKeyEvent* event );      //!< Evaluate key release event.
    virtual void mouseMoveEvent( QMouseEvent* event );     //!< Evaluate mouse move event.
    virtual void mousePressEvent( QMouseEvent* event );    //!< Evaluate mouse press event.
    virtual void mouseReleaseEvent( QMouseEvent* event );  //!< Evaluate mouse release event.

    void  createShaders();   //!< Create basic shaders for grid, axis, and objects rendering.
    void  createGeometry();
    void  drawCylinder( GLShader *shader );

    void  createFBOTexture( GLuint &oudIT, const GLenum internalFormat, const GLenum format,
                            const GLenum type, GLint filter, int width, int height );
    bool  initFBO( int width, int height );
    void  deleteFBO();
    void  drawToFBO();

private:
    SystemData*       mSysData;
    Camera            mCamera;

    int               mKeyPressed;
    int               mKeyModifier;
    Qt::MouseButton   mButtonPressed;    
    QPoint            mLastPos;
    glm::vec2         cameraAngle;

    GLShader  mQuadShader;
    QString   mQuadVertShaderName;
    QString   mQuadFragShaderName;
    GLuint vaQuad, vboQuad;

    GLShader  mSceneShader;
    QString   mSceneVertShaderName;
    QString   mSceneGeomShaderName;
    QString   mSceneFragShaderName;

    GLShader  mRodShader;
    QString   mRodVertShaderName;
    QString   mRodGeomShaderName;
    QString   mRodFragShaderName;

    GLuint vaBox, vboBox, iboBox;
    GLuint vaCyl, vboCyl,nboCyl;
    int    numCylPoints;

    glm::vec3 tablePos,   tableScale,   tableRot;
    glm::vec3 holder1Pos, holder1Scale, holder1Rot;
    glm::vec3 holder2Pos, holder2Scale, holder2Rot;
    float tableRotAngle, holder1RotAngle, holder2RotAngle;

    GLuint fboID;
    GLuint colAttachID[2],dboID;
    int   mPickID;
};

#endif //
