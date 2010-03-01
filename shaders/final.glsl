#pragma anki vertShaderBegins

#pragma anki include "shaders/simple_vert.glsl"

#pragma anki fragShaderBegins

#pragma anki uniform raster_image 0
uniform sampler2D raster_image;
varying vec2 texCoords;

void main()
{
	//if( gl_FragCoord.x > 0.5 ) discard;

	gl_FragColor.rgb = texture2D( raster_image, texCoords ).rgb;

	/*vec4 c = texture2D( raster_image, texCoords );
	if( c.r == 0.0 && c.g == 0.0 && c.b==0.0 && c.a != 0.0 )*/
		//gl_FragColor.rgb = vec3( texture2D( raster_image, texCoords ).a );
	//gl_FragColor.rgb = MedianFilter( raster_image, texCoords );
	//gl_FragColor.rgb = vec3( gl_FragCoord.xy/tex_size_, 0.0 );
	//gl_FragColor.rgb = vec3( gl_FragCoord.xy*vec2( 1.0/R_W, 1.0/R_H ), 0.0 );
	//gl_FragColor.rgb = texture2D( raster_image, gl_FragCoord.xy/textureSize(raster_image,0) ).rgb;
}
