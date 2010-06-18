#pragma anki vertShaderBegins

#pragma anki include "shaders/SimpleVert.glsl"

#pragma anki fragShaderBegins

uniform vec3 ambientCol;
uniform sampler2D sceneColMap;

varying vec2 texCoords;

void main()
{
	gl_FragData[0].rgb = texture2D( sceneColMap, texCoords ).rgb * ambientCol;
}
