// The final pass

#pragma anki vertShaderBegins

#pragma anki include "shaders/SimpleVert.glsl"

#pragma anki fragShaderBegins

uniform sampler2D rasterImage;
in vec2 vTexCoords;
layout(location = 0) out vec3 fFragColor;


void main()
{
	/*if(vTexCoords.y > 0.5)
	{
		fFragColor = vec3(1.0, 0.0, 1.0);
		return;
	}*/

	fFragColor = texture2D(rasterImage, vTexCoords).rgb;
	//fFragColor = vec3(texture2D(rasterImage, vTexCoords).b);
}
