// Copyright (C) 2014, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

// This file contains common code for all shaders. It's optional but it's 
// recomended to include it

#ifndef ANKI_SHADERS_COMMON_GLSL
#define ANKI_SHADERS_COMMON_GLSL

#ifndef DEFAULT_FLOAT_PRECISION
#define DEFAULT_FLOAT_PRECISION highp
#endif

#ifndef DEFAULT_INT_PRECISION
#define DEFAULT_INT_PRECISION highp
#endif

precision DEFAULT_FLOAT_PRECISION float;
precision DEFAULT_FLOAT_PRECISION int;

// Read from a render target texture
//#define textureRt(tex_, texc_) texture(tex_, texc_)
#define textureRt(tex_, texc_) textureLod(tex_, texc_, 0.0)

#define POSITION_LOCATION 0
#define NORMAL_LOCATION 1
#define TANGENT_LOCATION 2
#define TEXTURE_COORDINATE_LOCATION 3
#define TEXTURE_COORDINATE_LOCATION_1 4
#define TEXTURE_COORDINATE_LOCATION_2 5
#define SCALE_LOCATION 6
#define ALPHA_LOCATION 7

#endif
