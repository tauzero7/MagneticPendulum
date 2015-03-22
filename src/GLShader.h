/**
    Copyright (c) 2014, Universitaet Stuttgart, VISUS, SFB 716, Thomas Mueller

    MPSim is licensed under a Creative Commons
    Attribution-ShareAlike 3.0 Unported License.

    http://creativecommons.org/licenses/by-sa/3.0/deed.en_US

    @brief Header file for OpenGL Shader class.
    @file GLShader.h
*/

#ifndef MY_GL_SHADER
#define MY_GL_SHADER

#include <iostream>
#include <cstdio>
#include <cstring>
#include <map>

#include <GL3/gl3w.h>


/**
 * A new shader program can be created most easily using one of the
 * constructors. The filenames of the shaders have to be given with
 * an absolute path. If there are no shader errors, the shader program
 * is automatically being linked.
 * 
 * To use the shader program, just use Bind() and Release(). Uniform 
 * variables can be accessed via 'GetUniformLocation()'.
 * 
 * When you leave your plugin, do not forget to call 'RemoveAllShaders()'.
 */
class GLShader
{
public:
    GLShader();
    ~GLShader();
    
    /** Bind shader program
     */
    void    Bind();
    
    /** Bind shader program attribute location.
     * @param attribIndex   Index of attribute.
     * @param attribName    Name of attribute.
     * @param linkProg      Link program after binding attribute.
     */
    bool    BindAttribLocation( unsigned int attribIndex, const char* attribName, bool linkProg = false );
    
    /** Create empty shader program.
     *    You have to attach shaders and link the program by yourself.
     */
    void    CreateEmptyProgram();
    
    /** Create shader program from file.
     * @param vShaderName  File name of vertex shader.
     * @param fShaderName  File name of fragment shader.
     */
    bool    CreateProgramFromFile( const char* vShaderName, 
                                   const char* fShaderName, FILE* fptr = stderr );
    
    /** Create shader program from file.
     * @param vShaderName  File name of vertex shader.
     * @param gShaderName  File name of geometry shader.
     * @param fShaderName  File name of fragment shader.
     */
    bool    CreateProgramFromFile( const char* vShaderName, 
                                   const char* gShaderName, 
                                   const char* fShaderName );
    
    /** Create shader program from file.
     * @param vShaderName  File name of vertex shader.     
     * @param tcShaderName File name of tessellation control shader.
     * @param teShaderName File name of tessellation evaluation shader.
     * @param fShaderName  File name of fragment shader.
     */
    bool    CreateProgramFromFile( const char* vShaderName,
                                   const char* tcShaderName,
                                   const char* teShaderName,
                                   const char* fShaderName );

    /** Create shader program from file.
     * @param vShaderName  File name of vertex shader.     
     * @param tcShaderName File name of tessellation control shader.
     * @param teShaderName File name of tessellation evaluation shader.
     * @param gShaderName  File name of geometry shader.
     * @param fShaderName  File name of fragment shader.
     */
    bool    CreateProgramFromFile( const char* vShaderName,
                                   const char* tcShaderName,
                                   const char* teShaderName,
                                   const char* gShaderName, 
                                   const char* fShaderName );
                                                           
    /** Create shader program from strings.
     * @param vShaderText  String text of vertex shader.
     * @param vShaderLen   Length of vertex shader text.
     * @param fShaderText  String text of fragment shader.
     * @param fShaderLen   Length of fragment shader text.
     */
    bool    CreateProgramFromString( const char* vShaderText, const size_t vShaderLen,
                                     const char* fShaderText, const size_t fShaderLen, FILE* fptr = stderr );
    
    /** Attach shader from file.
     * @param shaderName  File name of shader.
     * @param shaderType  Type of shader.
     * @param linkProg    Link program after attaching shader.
     */
    bool    AttachShaderFromFile( const char* shaderName, GLenum shaderType, bool linkProg = false );
    
    /** Attach shader from string.
     * @param shaderText  String text of shader.
     * @param shaderLen   Length of shader text.
     * @param shaderType  Type of shader.
     * @param linkProg    Link program after attaching shader.
     */
    bool    AttachShaderFromString( const char* shaderText, const size_t shaderLen, GLenum shaderType, bool linkProg = false );
    
    /** Get shader program handle.
     */
    GLuint  GetProgHandle();
    
    /** Get uniform location of shader variable.
     * @param name  Uniform variable name
     */
    GLint   GetUniformLocation(const char* name);  
    
    /** Link shader program.
     *    Linking is only necessary if you build the shader program
     *    by yourself.
     */ 
    bool    Link(FILE* fptr = stderr);
    
    void    PrintInfo(FILE* fptr = stderr);
    
    /** Remove all shaders from shader program.
     */
    void    RemoveAllShaders();
    
    /** Release shader program.
     */
    void    Release();   
    
    void    ClearSubsStrings();
    void    AddSubsStrings( const char* phText, const char* subsText );
    
    /** Prepend the header text to every shader that is read from file.
     *    E.g.:  prepend the version number of GLSL to be used:
     *            #version 330
     */
    void    PrependHeaderText( const char* header );

protected:  
    GLuint  createShaderFromFile(const char* shaderFilename, GLenum type, FILE* fptr = stderr );      
        
    /** Create shader from string.
     * @param shaderText  Shader text.
     * @param shaderLen   Length of shader text.
     * @param type        Shader type.
     * \return Shader id.
     */ 
    GLuint  createShaderFromString(const char* shaderText, const size_t shaderLen, GLenum type, FILE* fptr = stderr);   
    size_t  readShaderFromFile(const char* shaderFilename, std::string &shaderContent, FILE* fptr = stderr );
      
    /** Print shader information log.
     * @param shader  Shader id.
     */
    bool    printShaderInfoLog( GLuint shader, FILE* fptr = stderr );
    
    /** Print program information log.
     */
    bool    printProgramInfoLog( FILE* fptr = stderr );   
    

 private:
    GLuint  progHandle;
    
    std::string  headerText;
    std::map<std::string,std::string> subsStrings;
    std::map<std::string,std::string>::iterator subsStringsItr;
    
};
  
#endif
