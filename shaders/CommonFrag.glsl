#ifndef ANKI_SHADERS_COMMON_FRAG_GLSL
#define ANKI_SHADERS_COMMON_FRAG_GLSL

#ifndef DEFAULT_FLOAT_PRECISION
precision highp float;
#else
precision DEFAULT_FLOAT_PRECISION float;
#endif

#ifndef DEFAULT_INT_PRECISION
precision highp int;
#else
precision DEFAULT_FLOAT_PRECISION int;
#endif

#endif
