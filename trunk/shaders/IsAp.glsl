/// @file
/// Illumination stage ambient pass shader program

#pragma anki start vertexShader

#pragma anki include "shaders/SimpleVert.glsl"

#pragma anki start fragmentShader

uniform vec3 ambientCol;
uniform sampler2D sceneColMap;

in vec2 vTexCoords;

out vec3 fColor;

void main()
{
	fColor = texture2D(sceneColMap, vTexCoords).rgb * ambientCol;
}
