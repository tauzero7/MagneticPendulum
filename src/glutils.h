/**
    Copyright (c) 2014, Universitaet Stuttgart, VISUS, SFB 716, Thomas Mueller

    MPSim is licensed under a Creative Commons
    Attribution-ShareAlike 3.0 Unported License.

    http://creativecommons.org/licenses/by-sa/3.0/deed.en_US

    @file glutils.h
*/

#ifndef MPSIM_GL_UTILS_H
#define MPSIM_GL_UTILS_H

#include <iostream>
#include <cstdlib>
#include <fstream>
#include <vector>


#include <GL3/gl3w.h>


void    getOpenGLInfo( FILE* fptr = stderr );
GLenum  checkError( std::string file, int line );

bool    setSyncToVBlank( unsigned int sync );
bool    isVSyncEnabled();
#endif
