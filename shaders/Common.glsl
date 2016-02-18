// Copyright (C) 2009-2016, Panagiotis Christopoulos Charitos.
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

const float EPSILON = 0.000001;
const float PI = 3.14159265358979323846;

// Read from a render target texture
//#define textureRt(tex_, texc_) texture(tex_, texc_)
#define textureRt(tex_, texc_) textureLod(tex_, texc_, 0.0)

// Binding
#define UBO_BINDING(slot_, binding_) binding = slot_ * 1 + binding_
#define SS_BINDING(slot_, binding_) binding = slot_ * 8 + binding_
#define TEX_BINDING(slot_, binding_) binding = slot_ * 10 + binding_
#define ATOMIC_BINDING(slot_, binding_) binding = slot_ * 1 + binding_

// Common locations
#define POSITION_LOCATION 0
#define TEXTURE_COORDINATE_LOCATION 1
#define NORMAL_LOCATION 2
#define TANGENT_LOCATION 3
#define SCALE_LOCATION 1
#define ALPHA_LOCATION 2

#endif
