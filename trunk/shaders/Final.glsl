// The final pass

#pragma anki vertShaderBegins

#pragma anki include "shaders/SimpleVert.glsl"

#pragma anki fragShaderBegins

uniform sampler2D rasterImage;
in vec2 vTexCoords;
layout(location = 0) out vec3 fFragColor;

void main()
{
	//if( gl_FragCoord.x > 0.5 ) discard;

	//fFragColor = texture2D(rasterImage, vTexCoords).rgb;
	
	fFragColor = vec3(texture2D(rasterImage, vTexCoords).r);
	
}
