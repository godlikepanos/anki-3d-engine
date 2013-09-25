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
#define textureFai(tex_, texc_) texture(tex_, texc_)

#endif
