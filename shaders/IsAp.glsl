/// @file
/// Illumination stage ambient pass shader program

#pragma anki start vertexShader

#pragma anki include "shaders/SimpleVert.glsl"

#pragma anki start fragmentShader

uniform vec3 ambientCol;
uniform usampler2D msFai0;

in vec2 vTexCoords;

out vec3 fColor;

void main()
{
	vec4 c = unpackUnorm4x8(texture(msFai0, vTexCoords).r);
	fColor = c.rgb * ambientCol;
}
