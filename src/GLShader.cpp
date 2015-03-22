/**
    Copyright (c) 2014, Universitaet Stuttgart, VISUS, SFB 716, Thomas Mueller

    MPSim is licensed under a Creative Commons
    Attribution-ShareAlike 3.0 Unported License.

    http://creativecommons.org/licenses/by-sa/3.0/deed.en_US

    @file GLShader.cpp
*/

#include "GLShader.h"

#include <fstream>
#include <sstream>

GLShader::GLShader() {
    progHandle = 0;
    headerText = std::string();
}

GLShader::~GLShader() {
    RemoveAllShaders();
}

bool GLShader::CreateProgramFromFile(const char* vShaderName, const char* fShaderName, FILE* fptr) {
    GLuint vShader = createShaderFromFile(vShaderName,GL_VERTEX_SHADER,fptr);
    //fprintf(fptr,"vshader is %d\n",vShader);
    if (vShader==0)  return false;
    
    GLuint fShader = createShaderFromFile(fShaderName,GL_FRAGMENT_SHADER,fptr);
    //fprintf(fptr,"fshader is %d\n",fShader);
    if (fShader==0)  return false;

    progHandle = glCreateProgram();
    //fprintf(fptr,"progID: %d\n",progHandle);
    glAttachShader(progHandle, vShader );
    glAttachShader(progHandle, fShader );
    return Link(fptr);
}

bool GLShader::CreateProgramFromFile(const char* vShaderName, 
        const char* gShaderName, const char* fShaderName) {
        
    GLuint vShader = createShaderFromFile(vShaderName,GL_VERTEX_SHADER);
    if (vShader==0)  return false;
    GLuint gShader = createShaderFromFile(gShaderName,GL_GEOMETRY_SHADER);
    if (gShader==0)  return false;
    GLuint fShader = createShaderFromFile(fShaderName,GL_FRAGMENT_SHADER);
    if (fShader==0)  return false;

    progHandle = glCreateProgram();
    glAttachShader(progHandle, vShader );
    glAttachShader(progHandle, gShader );
    glAttachShader(progHandle, fShader );
    return Link();
}

bool GLShader::CreateProgramFromFile( const char* vShaderName,
        const char* tcShaderName, const char* teShaderName,
        const char* fShaderName ) {
 
    GLuint vShader = createShaderFromFile(vShaderName,GL_VERTEX_SHADER);
    if (vShader==0)  return false;
    GLuint tcShader = createShaderFromFile(tcShaderName,GL_TESS_CONTROL_SHADER);
    if (tcShader==0)  return false;
    GLuint teShader = createShaderFromFile(teShaderName,GL_TESS_EVALUATION_SHADER);
    if (teShader==0)  return false;
    GLuint fShader = createShaderFromFile(fShaderName,GL_FRAGMENT_SHADER);
    if (fShader==0)  return false;

    progHandle = glCreateProgram();
    glAttachShader(progHandle, vShader );
    glAttachShader(progHandle, tcShader );
    glAttachShader(progHandle, teShader );
    glAttachShader(progHandle, fShader );
    return Link();
}

bool GLShader::CreateProgramFromFile( const char* vShaderName,
        const char* tcShaderName, const char* teShaderName,
        const char* gShaderName, const char* fShaderName ) {
 
    GLuint vShader = createShaderFromFile(vShaderName,GL_VERTEX_SHADER);
    if (vShader==0)  return false;
    GLuint tcShader = createShaderFromFile(tcShaderName,GL_TESS_CONTROL_SHADER);
    if (tcShader==0)  return false;
    GLuint teShader = createShaderFromFile(teShaderName,GL_TESS_EVALUATION_SHADER);
    if (teShader==0)  return false;
    GLuint gShader = createShaderFromFile(gShaderName,GL_GEOMETRY_SHADER);
    if (gShader==0)  return false;
    GLuint fShader = createShaderFromFile(fShaderName,GL_FRAGMENT_SHADER);
    if (fShader==0)  return false;

    progHandle = glCreateProgram();
    glAttachShader(progHandle, vShader );
    glAttachShader(progHandle, tcShader );
    glAttachShader(progHandle, teShader );
    glAttachShader(progHandle, gShader );
    glAttachShader(progHandle, fShader );
    return Link();
}

bool GLShader::CreateProgramFromString(const char* vShaderText, const size_t vShaderLen,
                                       const char* fShaderText, const size_t fShaderLen, FILE* fptr) {
    GLuint vShader = createShaderFromString(vShaderText,vShaderLen,GL_VERTEX_SHADER,fptr);
    if (vShader==0)  return false;
    GLuint fShader = createShaderFromString(fShaderText,fShaderLen,GL_FRAGMENT_SHADER,fptr);
    if (fShader==0)  return false;

    progHandle = glCreateProgram();
    glAttachShader(progHandle, vShader);
    glAttachShader(progHandle, fShader);
    return Link(fptr);
}

bool GLShader::AttachShaderFromFile( const char* shaderName, GLenum shaderType, bool linkProg ) {
    if (progHandle==0) {
        return false;  
    }
    GLuint shaderHandle = createShaderFromFile(shaderName,shaderType);
    if (shaderHandle==0) {
        return false;
    }
    glAttachShader(progHandle,shaderHandle);
    if (linkProg) {
        return Link();
    }
    return true;
}

bool GLShader::AttachShaderFromString( const char* shaderText, const size_t shaderLen, GLenum shaderType, bool linkProg ) {
    if (progHandle==0) {
        return false;
    }
    GLuint shaderHandle = createShaderFromString(shaderText,shaderLen,shaderType);
    if (shaderHandle==0) {
        return false;
    }
    glAttachShader(progHandle,shaderHandle);
    if (linkProg) {
        return Link();
    }
    return true;
}

/*!
 */
void GLShader::Bind() {
    glUseProgram(progHandle);
}

bool GLShader::BindAttribLocation( unsigned int attribIndex, const char* attribName, bool linkProg ) {
    glBindAttribLocation( progHandle, attribIndex, attribName ); 
    if (linkProg) {
        return Link();
    }
    return true;    
}

void GLShader::CreateEmptyProgram() {
   if (progHandle!=0) {
       RemoveAllShaders();
   }
   progHandle = glCreateProgram();
}

void GLShader::Release() {
    glUseProgram(0);
}

GLuint GLShader::GetProgHandle() {
    return progHandle;
}

GLint GLShader::GetUniformLocation(const char* name) {
    return glGetUniformLocation(progHandle,name);
}

bool GLShader::Link(FILE* fptr) {
    glLinkProgram(progHandle);
    bool status = printProgramInfoLog(fptr);
    glUseProgram(0);
    return status;
}

void GLShader::PrintInfo( FILE* fptr ) {
    if (progHandle>0) {
        /*
        GLsizei maxCount = static_cast<GLsizei>(5);
        GLsizei count; 
        GLuint  *shaders = new GLuint[5];
        glGetAttachedShaders( progHandle, maxCount, &count, shaders );
        for(int i=0; i<static_cast<int>(count); i++) {
            printShaderInfoLog( shaders[i], fptr );
        }
        */
        printProgramInfoLog(fptr);
    }
}

size_t GLShader::readShaderFromFile(const char* shaderFilename, std::string &shaderContent, FILE* fptr ) {
#ifndef _WIN32
    // WinQ: Does this work on Windows??
    if (shaderFilename[0]=='\0') {
        return 0;
    }
#endif    
    std::ifstream in(shaderFilename);  
    if (in.bad()) {
       const unsigned int length = 256;
       char msg[length];
#ifdef _WIN32
       sprintf_s(msg, length, "GLShader::readShaderFromFile() ... Cannot open file \"%s\"",shaderFilename);
#else
       sprintf(msg,"GLShader::readShaderFromFile() ... Cannot open file \"%s\"",shaderFilename);
#endif
       fprintf(fptr,"Error: %s\n",msg);
       return 0;
    }

    std::stringstream shaderData;
    shaderData << in.rdbuf();
    in.close();
    shaderContent = shaderData.str();
    //fprintf(fptr,"%s\n",shaderContent.c_str());
    return shaderContent.size();
}

GLuint GLShader::createShaderFromFile(const char* shaderFilename, GLenum type, FILE* fptr ) {
    std::string shaderText;
    size_t iShaderLen = readShaderFromFile(shaderFilename,shaderText,fptr);
    if (iShaderLen==std::string::npos || iShaderLen==0) {
        return 0;
    }

    for(subsStringsItr = subsStrings.begin(); subsStringsItr!=subsStrings.end(); subsStringsItr++) {
        size_t pos = 0;
        std::string phText   = subsStringsItr->first;
        std::string subsText = subsStringsItr->second;
        while (pos!=std::string::npos) {
            pos = shaderText.find(phText,pos);
            if (pos!=std::string::npos) {
                shaderText.replace(pos, phText.length(), subsText);
                pos += phText.length();
                iShaderLen = shaderText.size();
            }
        } 
    }

    shaderText = headerText + "\n" + shaderText;
    iShaderLen = shaderText.size();

    GLuint shader = glCreateShader(type);
    const char *strShaderVar = shaderText.c_str();
    glShaderSource(shader,1,(const GLchar**)&strShaderVar, (GLint*)&iShaderLen);
    glCompileShader(shader);

    if (!printShaderInfoLog(shader,fptr)) {
        std::stringstream iss(shaderText);
        std::string sLine;
        int lineCounter = 1;
        while(std::getline(iss,sLine)) {
            fprintf(fptr,"%4d : %s\n",(lineCounter++),sLine.c_str());
        }
        return 0;
    }
    return shader;
}

GLuint GLShader::createShaderFromString(const char* shaderText, const size_t shaderLen, GLenum type, FILE* fptr) {    
    if (shaderLen==0) {
        return 0;
    }

    GLuint shader = glCreateShader(type);
    glShaderSource(shader,1,(const GLchar**)&shaderText, (GLint*)&shaderLen);
    glCompileShader(shader);
   
    if (!printShaderInfoLog(shader,fptr)) {
        std::stringstream iss(shaderText);
        std::string sLine;
        int lineCounter = 1;
        while(std::getline(iss,sLine)) {
            fprintf(fptr,"%4d : %s\n",(lineCounter++),sLine.c_str());
        }
        return 0;
    }
    return shader;
}

bool GLShader::printShaderInfoLog( GLuint shader, FILE* fptr ) {
    int infoLogLen   = 0;
    int charsWritten = 0;
    GLchar *infoLog;

    GLint compileStatus;
    glGetShaderiv(shader, GL_COMPILE_STATUS,  &compileStatus);
    glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &infoLogLen);

    if (compileStatus==GL_FALSE && infoLogLen > 1)  {
        infoLog = new GLchar[infoLogLen];
        // error check for fail to allocate memory omitted
        glGetShaderInfoLog(shader, infoLogLen, &charsWritten, infoLog);
        fprintf(fptr,"InfoLog : %s\n", infoLog);
        delete [] infoLog;
        return false;
    }
    return (compileStatus == GL_TRUE);
}

bool GLShader::printProgramInfoLog( FILE* fptr ) {
    int infoLogLen   = 0;
    int charsWritten = 0;
    GLchar *infoLog;

    GLint linkStatus;
    glGetProgramiv(progHandle, GL_INFO_LOG_LENGTH, &infoLogLen);
    glGetProgramiv(progHandle, GL_LINK_STATUS, &linkStatus);

    if (linkStatus==GL_FALSE && infoLogLen > 1)  {
        infoLog = new GLchar[infoLogLen];
        // error check for fail to allocate memory omitted
        glGetProgramInfoLog(progHandle, infoLogLen, &charsWritten, infoLog);
        fprintf(fptr,"\nProgramInfoLog :\n\n%s\n", infoLog);
        delete [] infoLog;
        return false;
    }
    return (linkStatus == GL_TRUE);
}

void GLShader::RemoveAllShaders() {
    if (progHandle==0) {
        return;
    }
      
    if (!glIsProgram(progHandle)) {
        return;
    }
   
    const GLsizei numShaders = 1024;
    GLsizei numReturned;
    GLuint shaders[numShaders];
    glUseProgram(0);
   
    glGetAttachedShaders(progHandle, numShaders, &numReturned, shaders);
    for (GLsizei i = 0; i < numReturned; i++) {
        glDetachShader(progHandle, shaders[i]);
        glDeleteShader(shaders[i]);
    }
    glDeleteProgram(progHandle);
    progHandle = 0;
}

void GLShader::ClearSubsStrings() {
    subsStrings.clear();
}

void GLShader::AddSubsStrings( const char* phText, const char* subsText ) {    
    subsStringsItr = subsStrings.find(std::string(phText));
    if (subsStringsItr!=subsStrings.end()) {
        subsStringsItr->second = std::string(subsText);
    } else {
        std::pair<std::string,std::string> subs = std::pair<std::string,std::string>(std::string(phText),std::string(subsText));
        subsStrings.insert(subs);
    }
}

void GLShader::PrependHeaderText( const char* header ) {
    headerText = std::string(header);
}
