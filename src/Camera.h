/**
    Copyright (c) 2014, Universitaet Stuttgart, VISUS, SFB 716, Thomas Mueller

    MPSim is licensed under a Creative Commons
    Attribution-ShareAlike 3.0 Unported License.

    http://creativecommons.org/licenses/by-sa/3.0/deed.en_US

    @brief Header file for camera object.
    @file Camera.h
*/

#ifndef MY_CAMERA_H
#define MY_CAMERA_H

#include <iostream>
#include <cstdio>

#include "glm.hpp"
#include "gtc/matrix_transform.hpp"
#include "gtc/quaternion.hpp"
#include "gtc/type_ptr.hpp"


typedef struct camera_params_t
{
    float pos[3];
    float dir[3];
    float vup[3];
    
    float znear;
    float zfar;
    float aspect;
    unsigned int    width;
    unsigned int    height;
    float fovY;
} cameraParams;


class Camera
{
public:
    Camera ( );
    Camera ( cameraParams params );
    virtual ~Camera ( );

public:
    void   setStandardParams  ( );
    void   setParams          ( cameraParams &params );
    void   getParams          ( cameraParams &params );

    void      setEyePos( float eye_x, float eye_y, float eye_z );
    void      getEyePos( float &eye_x, float &eye_y, float &eye_z );
    glm::vec3 getEyePos();

    void      setDir( float dir_x, float dir_y, float dir_z );
    void      getDir( float &dir_x, float &dir_y, float &dir_z );
    glm::vec3 getDir();

    void      setPOI( float c_x, float c_y, float c_z );
    void      getPOI( float &c_x, float &c_y, float &c_z );
    glm::vec3 getPOI();

    void      setVup( float vup_x, float vup_y, float vup_z );
    void      getVup( float &vup_x, float &vup_y, float &vup_z );
    glm::vec3 getVup();

    glm::vec3 getRight();

    void   setFovY( float fovy );  // in degree
    float  getFovY();

    void   setAspect( float aspect );
    float  getAspect();
    
    void   setIntrinsic( float fovy, float aspect );
    void   setIntrinsic( float fovy, float aspect, float near, float far );
    void   getIntrinsic( float &fovy, float &aspect, float &near, float &far );

    void   setSizeAndAspect  ( int width, int  height );
    void   setSize( int width, int height );
    void   getSize( int &width, int &height );
    int    width();
    int    height();

    void   setMoveOnSphere( glm::vec3 ex, glm::vec3 ey, glm::vec3 ez, glm::vec3 poi, float dist );
    void   moveOnSphere( float theta, float phi );
    void   setSphereDist( float dist );
    float  getSphereDist( );

    glm::vec3  getViewDir( int px, int py );

    glm::mat4  getProjMatrix();
    glm::mat4  getViewMatrix();

    void   print ( FILE* ptr = stderr );


protected:
    glm::vec3     mPos;
    glm::vec3     mDir;
    glm::vec3     mVup;

    float  mZnear;
    float  mZfar;
    float  mAspect;
    int    mWidth;
    int    mHeight;
    float  mFovY;

    glm::vec3  mRight;
    glm::vec3  mPOI;

    glm::vec3  mMOSex;
    glm::vec3  mMOSey;
    glm::vec3  mMOSez;
    float  mDist;
};

#endif
