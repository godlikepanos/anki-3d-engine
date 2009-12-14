#ifndef _GL_WRAPPER_H_
#define _GL_WRAPPER_H_

#include "common.h"
#include <GL/glew.h>
#include <GL/gl.h>
#include <GL/glu.h>
#include "math.h"

namespace gl {

// basic
inline void Begin( uint mode ) { glBegin(mode); }
inline void End() { glEnd(); }
inline void GetIntegerv( uint pname, int* params ) { glGetIntegerv(pname, params); }
inline void CullFace( uint mode ) { glCullFace(mode); }

// enable disable
inline void Enable( uint cap ) { glEnable(cap); }
inline void Disable( uint cap ) { glDisable(cap); }

// matrices
inline void MultMatrixf( const mat4_t& m4 ) { glMultMatrixf( &(m4.Transposed())(0,0) ); }
inline void LoadMatrixf( const mat4_t& m4 ) { glLoadMatrixf( &(m4.Transposed())(0,0) ); }
inline void LoadIdentity() { glLoadIdentity(); }
inline void MatrixMode( uint x ) { glMatrixMode(x); }
inline void Viewport( uint x, uint y, uint w, uint h ) { glViewport(x,y,w,h); }
inline void PushMatrix() { glPushMatrix(); }
inline void PopMatrix() { glPopMatrix(); }

// textures
inline void GenTextures(uint n, uint* textures) { glGenTextures(n, textures); }
inline void TexImage2D(uint target, int level, int internalFormat, uint width, uint height, int border, uint format, uint type,
	const void* data ) { glTexImage2D(	target, level, internalFormat, width, height, border, format, type, data ); }
inline void TexParameteri(uint target, uint pname, int param ) { glTexParameteri( target, pname, param ); }

// shaders
inline void UseProgram( uint program ) { glUseProgram(program); }
inline void ShaderSource( uint shader, uint count, const char** string, const int* length ) { glShaderSource(shader,count,string,length); }
inline void GetShaderiv( uint shader, uint pname, int* params) { glGetShaderiv(shader, pname, params); }
inline void GetShaderInfoLog( uint shader, uint maxLength, int* length, char* infoLog) {glGetShaderInfoLog(shader, maxLength, length, infoLog);}
inline uint CreateProgram() { return glCreateProgram(); }
inline void LinkProgram( uint program ) { glLinkProgram(program); }
inline void GetProgramiv( uint program, uint pname, int* params) { glGetProgramiv( program, pname, params); }
inline void GetProgramInfoLog( uint program, uint maxLength, int* length, char* infoLog ) { glGetProgramInfoLog( program, maxLength, length, infoLog ); }
inline int GetUniformLocation( uint program, const char* name ) { return glGetUniformLocation( program, name ); }
inline int GetAttribLocation( uint program, const char* name ) { return glGetAttribLocation( program, name ); }

// color
inline void Color3f( float r, float g, float b ) { glColor3f( r,g,b ); }
inline void Color3fv( const vec3_t& v ) { glColor3fv( &((vec3_t&)v)[0] ); }
inline void Color4f( float r, float g, float b, float a ) { glColor4f( r,g,b,a ); }
inline void Color4fv( const vec4_t& v ) { glColor4fv( &((vec4_t&)v)[0] ); }

// buffers
inline void ClearColor( float red, float green, float blue, float alpha ) { glClearColor(red,green,blue,alpha); }
inline void Clear( uint mask ) { glClear(mask); }

// vertex
inline void Vertex3f( float x, float y, float z ) { glVertex3f(x,y,z); }
inline void Vertex3fv( const vec3_t& v ) { glVertex3fv( &((vec3_t&)v)[0] ); }
inline void Normal3f( float x, float y, float z ) { glNormal3f(x,y,z); }
inline void Normal3fv( const vec3_t& v ) { glNormal3fv( &((vec3_t&)v)[0] ); }
inline void TexCoord2f( float s, float t ) { glTexCoord2f(s,t); }
inline void TexCoord2fv( const vec2_t& v ) { glTexCoord2fv( &((vec2_t&)v)[0] ); }



} // end namespace
#endif
