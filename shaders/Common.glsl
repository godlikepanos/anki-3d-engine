// Copyright (C) 2014, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

// This file contains common code for all shaders. It's optional but it's 
// recomended to include it

#ifndef ANKI_SHADERS_COMMON_GLSL
#define ANKI_SHADERS_COMMON_GLSL

// Default precision
#ifndef DEFAULT_FLOAT_PRECISION
#define DEFAULT_FLOAT_PRECISION highp
#endif

#ifndef DEFAULT_INT_PRECISION
#define DEFAULT_INT_PRECISION highp
#endif

precision DEFAULT_FLOAT_PRECISION float;
precision DEFAULT_FLOAT_PRECISION int;

// Types
#define F16 mediump float
#define F32 highp float
#define U32 highp uint
#define I32 highp int

#define F16Vec2 mediump vec2
#define F32Vec2 highp vec2
#define U32Vec2 highp uvec2
#define I32Vec2 highp ivec2

#define F16Vec3 mediump vec3
#define F32Vec3 highp vec3
#define U32Vec3 highp uvec3
#define I32Vec3 highp ivec3

#define F16Vec4 mediump vec4
#define F32Vec4 highp vec4
#define U32Vec4 highp uvec4
#define I32Vec4 highp ivec4

#define F32Mat4 highp mat4
#define F16Mat4 mediump mat4

#define F32Mat3 highp mat3
#define F16Mat3 mediump mat3

#define F32Sampler2D highp sampler2D
#define F16Sampler2D mediump sampler2D

// Read from a render target texture
//#define textureRt(tex_, texc_) texture(tex_, texc_)
#define textureRt(tex_, texc_) textureLod(tex_, texc_, 0.0)

// Commpn locations
#define POSITION_LOCATION 0
#define NORMAL_LOCATION 1
#define TANGENT_LOCATION 2
#define TEXTURE_COORDINATE_LOCATION 3
#define TEXTURE_COORDINATE_LOCATION_1 4
#define TEXTURE_COORDINATE_LOCATION_2 5
#define SCALE_LOCATION 6
#define ALPHA_LOCATION 7

#endif
