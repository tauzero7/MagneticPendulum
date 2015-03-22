/**
    Copyright (c) 2014, Universitaet Stuttgart, VISUS, SFB 716, Thomas Mueller

    MPSim is licensed under a Creative Commons
    Attribution-ShareAlike 3.0 Unported License.

    http://creativecommons.org/licenses/by-sa/3.0/deed.en_US

    @file Camera.cpp
*/

#include "Camera.h"

#ifndef M_PI
#define M_PI    3.141592653589793
#define M_PI_2  1.570796326794897
#endif

/*! Constructor of the simple OpenGL camera.
*/
Camera :: Camera() {
    setStandardParams();
}

/*! Constructor of the simple OpenGL camera.
*/
Camera :: Camera ( cameraParams params ) {
    setParams(params);
}

Camera :: ~Camera(){
}


/*! Set standard camera parameters.
*/
void Camera :: setStandardParams() {
    // compare OpenGL RedBook, p 127

    mPos = glm::vec3( 1.0, 0.0, 0.0 );
    mDir = glm::vec3(-1.0, 0.0, 0.0 );
    mVup = glm::vec3( 0.0, 0.0, 1.0 );

    mRight = glm::cross(mDir,mVup);
    mPOI = mPos+mDir;

    mZnear = 0.01f;
    mZfar  = 100.0f;

    mWidth  = 100;
    mHeight = 100;

    mAspect = 1.0f;
    mFovY   = 45.0f;

    mMOSex = glm::vec3(1.0,0.0,0.0);
    mMOSey = glm::vec3(0.0,1.0,0.0);
    mMOSez = glm::vec3(0.0,0.0,1.0);
}

/*! Set camera parameters.
*\param  params : camera parameters.
*/
void   
Camera :: setParams ( cameraParams &params )
{
    mPos = glm::vec3(params.pos[0],params.pos[1],params.pos[2]);
    mDir = glm::vec3(params.dir[0],params.dir[1],params.dir[2]);
    mVup = glm::vec3(params.vup[0],params.vup[1],params.vup[2]);

    mRight  = glm::cross(mDir,mVup);
    mPOI    = mPos+mDir;

    mZnear  = params.znear;
    mZfar   = params.zfar;
    mAspect = params.aspect;
    mWidth  = params.width;
    mHeight = params.height;
    mFovY   = params.fovY;
}

/*! Get camera parameters.
*\param  params : reference to camera parameters.
*/
void   
Camera :: getParams ( cameraParams &params )
{
    params.pos[0] = mPos.x;
    params.pos[1] = mPos.y;
    params.pos[2] = mPos.z;
    params.dir[0] = mDir.x;
    params.dir[1] = mDir.y;
    params.dir[2] = mDir.z;
    params.vup[0] = mVup.x;
    params.vup[1] = mVup.y;
    params.vup[2] = mVup.z;

    params.znear  = mZnear;
    params.zfar   = mZfar;
    params.aspect = mAspect;
    params.width  = mWidth;
    params.height = mHeight;
    params.fovY   = mFovY;
}

/*! Set position of the camera.
* \param eye_x : x-position.
* \param eye_y : y-position.
* \param eye_z : z-position.
*/
void   
Camera :: setEyePos ( float eye_x, float eye_y, float eye_z )
{
    mPos = glm::vec3( eye_x, eye_y, eye_z );
    mDist = glm::length(mPos-mPOI);
}

/*! Get camera position.
* \param eye_x : reference to x-position.
* \param eye_y : reference to y-position.
* \param eye_z : reference to z-position.
*/ 
void   
Camera :: getEyePos ( float &eye_x, float &eye_y, float &eye_z )
{
    eye_x = mPos.x;
    eye_y = mPos.y;
    eye_z = mPos.z;
}

/*! Get camera position.
* \return glm::vec3 : position.
*/
glm::vec3   
Camera :: getEyePos  ( )
{
    return mPos;
}

/*! Set camera's viewing direction.
* \param dir_x : x-direction.
* \param dir_y : y-direction.
* \param dir_z : z-direction.
*/
void   
Camera :: setDir ( float dir_x, float dir_y, float dir_z )
{
    mDir = glm::vec3( dir_x, dir_y, dir_z );
    mRight = glm::cross(mDir,mVup);
}

/*! Get direction of camera view.
*  \param dir_x  :  x-component of direction.
*  \param dir_y  :  y-component of direction.
*  \param dir_z  :  z-component of direction.
*/
void   
Camera :: getDir ( float &dir_x, float &dir_y, float &dir_z )
{
    dir_x = mDir.x;
    dir_y = mDir.y;
    dir_z = mDir.z;
}

/*! Get direction of camera view.
*  \return glm::vec3 : camera's viewing direction.
*/
glm::vec3   
Camera :: getDir ( )
{
    return mDir;
}

/*! Set point of interest.
* \param c_x : x-component of the point-of-interest.
* \param c_y : y-component of the point-of-interest.
* \param c_z : z-component of the point-of-interest.
*/
void   
Camera :: setPOI ( float c_x, float c_y, float c_z )
{
    glm::vec3 center = glm::vec3(c_x,c_y,c_z);
    mDir = center - mPos;

    if (mDir!=glm::vec3(0))
    {
        mPOI = center;
        mDir = glm::normalize(mDir);
        mRight = glm::cross(mDir,mVup);
    }
}

/*! Get point of interest.
* \param c_x : x-component of the point-of-interest.
* \param c_y : y-component of the point-of-interest.
* \param c_z : z-component of the point-of-interest.
*/
void  
Camera :: getPOI ( float &c_x, float &c_y, float &c_z )
{
    c_x = mPOI.x;
    c_y = mPOI.y;
    c_z = mPOI.z;
}

/*! Get point of interest.
* \return glm::vec3 : xyz-components of point of interest.
*/
glm::vec3   
Camera :: getPOI ( )
{
    return mPOI;
}

/*! Set up-vector of camera.
* \param vup_x : x-component of vertical up-vector.
* \param vup_y : y-component of vertical up-vector.
* \param vup_z : z-component of vertical up-vector.
*/
void   
Camera :: setVup ( float vup_x, float vup_y, float vup_z )
{
    mVup = glm::vec3( vup_x, vup_y, vup_z );
    mRight = glm::cross(mDir,mVup);
}

/*! Get up-vector of camera.
* \param vup_x : reference to x-component of vertical up-vector.
* \param vup_y : reference to y-component of vertical up-vector.
* \param vup_z : reference to z-component of vertical up-vector.
*/
void   
Camera :: getVup ( float &vup_x, float &vup_y, float &vup_z )
{
    vup_x = mVup.x;
    vup_y = mVup.y;
    vup_z = mVup.z;
}

/*! Get up-vector of camera.
* \return glm::vec3 : up-vector.
*/
glm::vec3   
Camera :: getVup ( )
{
    return mVup;
}

/*! Get right-vector of camera.
*/
glm::vec3   
Camera :: getRight ( )
{
    return mRight;
}

/*! Set field-of-view in y-direction.
* \param fovy : angle in degree.
*/
void Camera::setFovY( float fovy ) {
    mFovY = fovy;
}

/*! Get field-of-view in y-direction.
*/
float Camera::getFovY() {
    return mFovY;
}

/*! Set aspect ratio.
* \param aspect : aspect ratio.
*/
void   
Camera :: setAspect ( float aspect )
{
    mAspect = aspect;
}

/*! Get aspect ratio.
*/
float   
Camera :: getAspect ( )
{
    return mAspect;
}

/*! Set intrinsic parameters.
* \param fovy : field of view in y-direction in degree.
* \param aspect : aspect ratio.
*/
void   
Camera :: setIntrinsic ( float fovy, float aspect )
{
    setIntrinsic(fovy,aspect,0.1f,10000.0f);
}

/*! Set intrinsic parameters.
* \param fovy : field of view in y-direction in degree.
* \param aspect : aspect ratio.
* \param near : near clipping plane.
* \param far : far clipping plane.
*/
void   
Camera :: setIntrinsic ( float fovy, float aspect, float near, float far )
{
    mFovY = fovy;
    mAspect = aspect;
    mZnear  = near;
    mZfar   = far;
}

/*! Get intrinsic parameters.
* \param fovy : reference to field of view in y-direction in degree.
* \param aspect : reference to aspect ratio.
* \param near : reference to near clipping plane.
* \param far : reference to far clipping plane.
*/
void   
Camera :: getIntrinsic ( float &fovy, float &aspect, float &near, float &far )
{
    fovy   = mFovY;
    aspect = mAspect;
    near   = mZnear;
    far    = mZfar;
}

/*! Set camera resolution and adjust aspect ratio.
* \param width : width of camera resolution.
* \param height : height of camera resolution.
*/
void   
Camera :: setSizeAndAspect ( int width, int  height )
{
    setSize(width,height);
    setAspect(width/(float)height);
}

/*! Set camera resolution.
* \param width : width of camera resolution.
* \param height : height of camera resolution.
*/
void   
Camera :: setSize ( int width, int height )
{
    mWidth  = width;
    mHeight = height;
}

/*! Get camera resolution.
* \param width : reference to width of camera resolution.
* \param height : reference to height of camera resolution.
*/
void   
Camera :: getSize ( int &width, int &height )
{
    width  = mWidth;
    height = mHeight;
}

/*! Get camera width.
*/
int   
Camera :: width ( )
{
    return mWidth;
}

/*! Get camera height.
*/
int   
Camera :: height ( )
{
    return mHeight;
}

/*! Put the camera on a sphere with radius 'dist' centered at 'poi'.
* \param ex: x-base vector.
* \param ey: y-base vector.
* \param ez: z-base vector.
* \param poi : point of interest.
* \param dist : distance to poi.
*/
void   
Camera :: setMoveOnSphere ( glm::vec3 ex, glm::vec3 ey, glm::vec3 ez, glm::vec3 poi, float dist )
{
    assert( dist > 0.0 );
    mPOI = poi;

    mMOSex = ex;
    mMOSey = ey;
    mMOSez = ez;
    // testen, ob mMOS-System ordentlich ist !!!!

    mDist = dist;
}

/*! Move camera on the sphere.
* \param theta : colatitude angle in rad.
* \param phi : azimuth angle in rad.
*/
void Camera :: moveOnSphere( float theta, float phi ) {
    if (theta>-M_PI_2 && theta<M_PI_2)
    {
        mPos = mPOI + mDist*( cosf(theta)*cosf(phi)*mMOSex + cosf(theta)*sinf(phi)*mMOSey + sinf(theta)*mMOSez );
        mDir = mPOI - mPos;
        mDir = glm::normalize(mDir);

        glm::vec3 z(0.0,0.0,1.0);
        mRight = glm::cross(mDir,z);
        mRight = glm::normalize(mRight);
        mVup = glm::cross(mRight,mDir);
    }
}

/*! Set distance to the point of interest.
* \param dist : distance to poi.
*/
void Camera :: setSphereDist( float dist )
{
    if (dist>0.0) {
        mDist = dist;
        mPos = mPOI - mDist*mDir;
    }
}

/*! Get distance to the point of interest.
*/
float Camera :: getSphereDist() {
    return glm::length(mPos-mPOI);
}

glm::vec3  Camera::getViewDir ( int px, int py ) {
    float sx = 2.0f*px/static_cast<float>(mWidth) - 1.0f;
    float sy = 2.0f*(mHeight - py)/static_cast<float>(mHeight) - 1.0f;
    float w  = (float)(mZnear*tan(glm::radians(mFovY)*0.5)*mAspect);
    float h  = (float)(mZnear*tan(glm::radians(mFovY)*0.5));

    glm::vec3 td = mZnear*mDir + sy*h*mVup + sx*w*mRight;
    td = glm::normalize(td);
    return td;
}

glm::mat4 Camera::getProjMatrix() {
    // DO NOT USE glm::perspectiveFov BECAUSE IT IS WRONG !!
    //return glm::perspectiveFov( mFovY, static_cast<float>(mWidth), static_cast<float>(mHeight), mZnear, mZfar );
    float cf = 1.0f/tanf(0.5f*glm::radians(mFovY));
    glm::mat4 persp = glm::mat4(cf/mAspect,0,0,0,0,cf,0,0,0,0,-(mZfar+mZnear)/(mZfar-mZnear),-2.0f*mZfar*mZnear/(mZfar-mZnear),0,0,-1.0f,0.0f);
    return glm::transpose(persp);
}

glm::mat4 Camera::getViewMatrix() {
    return glm::lookAt(mPos,mPOI,mVup);
}


/*! Print camera parameters.
* \param ptr : file pointer.
*/
void Camera :: print ( FILE* ptr )
{
    fprintf(ptr, "\nCamera parameters:\n");
    fprintf(ptr, "------------------\n");
    fprintf(ptr, "eye    :  %6.3f %6.3f %6.3f\n", mPos.x, mPos.y, mPos.z );
    fprintf(ptr, "dir    :  %6.3f %6.3f %6.3f\n", mDir.x, mDir.y, mDir.z );
    fprintf(ptr, "vup    :  %6.3f %6.3f %6.3f\n", mVup.x, mVup.y, mVup.z );
    fprintf(ptr, "right  :  %6.3f %6.3f %6.3f\n", mRight.x, mRight.y, mRight.z );
    fprintf(ptr, "size   :  %4i %4i\n",mWidth,mHeight);
    fprintf(ptr, "aspect :  %6.3f\n",mAspect);
    fprintf(ptr, "fovY   :  %6.3f\n",mFovY);
    fprintf(ptr, "near   :  %8.2f\n",mZnear);
    fprintf(ptr, "far    :  %8.2f\n",mZfar);
}

