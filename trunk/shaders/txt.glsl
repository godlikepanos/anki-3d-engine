#pragma anki vertShaderBegins

varying vec2 texCoords;

void main(void)
{
	texCoords = gl_MultiTexCoord0.xy;
	gl_FrontColor = gl_Color;
	gl_Position = ftransform();
}

#pragma anki fragShaderBegins

uniform sampler2D fontMap;
varying vec2 texCoords;

void main()
{
	vec2 texCol = texture2D( fontMap, texCoords ).ra; // get only one value and the alpha

	if( texCol.y == 0.0 ) discard;

	gl_FragColor = vec4(texCol.x) * gl_Color;
}
