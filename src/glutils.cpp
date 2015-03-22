/**
    Copyright (c) 2014, Universitaet Stuttgart, VISUS, SFB 716, Thomas Mueller

    MPSim is licensed under a Creative Commons
    Attribution-ShareAlike 3.0 Unported License.

    http://creativecommons.org/licenses/by-sa/3.0/deed.en_US

    @file glutils.cpp
*/

#include <cassert>
#include "glutils.h"

#ifdef _WIN32
#include "wglext.h"
//typedef bool (APIENTRY *PFNWGLSWAPINTERVALEXTPROC)        (int interval);
//typedef bool (APIENTRY *PFNWGLGETSWAPINTERVALEXTPROC)     (void);
#else
#include <GL/glx.h>
#endif


void   getOpenGLInfo( FILE* fptr ) {
    const GLubyte *ven = glGetString(GL_VENDOR);
    ven = glGetString(GL_VERSION);

    GLint maxVertexAttribs;
    glGetIntegerv(GL_MAX_VERTEX_ATTRIBS,&maxVertexAttribs);

    fprintf(fptr,"OpenGLInfo:\n-----------\n");
    fprintf(fptr,"GL_MAX_VERTEX_ATTRIBS : %d\n",maxVertexAttribs);
    fprintf(fptr,"Vendor                : %s\n",ven);
}


GLenum  checkError( std::string file, int line ) {
    GLenum __theError;
    __theError = glGetError();
    if (__theError != GL_NO_ERROR) {
        switch(__theError) {
            case GL_INVALID_ENUM:
                printf("GL_INVALID_ENUM at %s:%u\n", file.c_str(), line);
                break;
            case GL_INVALID_VALUE:
                printf("GL_INVALID_VALUE at %s:%u\n", file.c_str(), line);
                break;
            case GL_INVALID_OPERATION:
                printf("GL_INVALID_OPERATION at %s:%u\n", file.c_str(), line);
                break;
            case GL_OUT_OF_MEMORY:
                printf("GL_OUT_OF_MEMORY at %s:%u\n", file.c_str(), line);
                break;
        }
    }
    return __theError;
}


bool setSyncToVBlank ( unsigned int sync ) {
#ifdef _WIN32
    static PFNWGLSWAPINTERVALEXTPROC wglSwapInterval = (PFNWGLSWAPINTERVALEXTPROC)gl3wGetProcAddress("wglSwapIntervalEXT");
    if (wglSwapInterval!=NULL) {
        fprintf(stderr,"+++ Windows: Sync to VBlank = %d +++\n",sync);
        wglSwapInterval(sync);
    }
    return isVSyncEnabled();
#else
    fprintf(stderr,"+++ Linux: Sync to VBlank = %d +++\n",sync);
    //glXSwapIntervalEXT(glXGetCurrentDisplay(),glXGetCurrentDrawable(),0);  
    void (*glXSwapInterval_)(int) = (void (*)(int)) glXGetProcAddress((const GLubyte*) "glXSwapIntervalSGI");
    if (glXSwapInterval_ == 0){
        glXSwapInterval_  = (void (*)(int)) glXGetProcAddress((const GLubyte*) "glXSwapIntervalMESA");
    }
    assert(glXSwapInterval_ != 0);
    glXSwapInterval_(sync);
    return true;
#endif
    return false;
}

bool isVSyncEnabled() {
#ifdef _WIN32
    static PFNWGLGETSWAPINTERVALEXTPROC wglGetSwapInterval = (PFNWGLGETSWAPINTERVALEXTPROC)gl3wGetProcAddress("wglGetSwapIntervalEXT");
    bool isEnabled = false;
    if (wglGetSwapInterval!=NULL) {
        isEnabled = (wglGetSwapInterval()==1 ? true : false);
    }
    return isEnabled;
#else
    return false;
#endif
}


