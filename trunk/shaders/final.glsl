#pragma anki vert_shader_begins

#pragma anki include "shaders/simple_vert.glsl"

#pragma anki frag_shader_begins

#pragma anki uniform raster_image 0
uniform sampler2D raster_image;
varying vec2 tex_coords;

void main()
{
	//if( gl_FragCoord.x > 0.5 ) discard;

	gl_FragColor.rgb = texture2D( raster_image, tex_coords ).rgb;
	//gl_FragColor.rgb = gl_FragCoord.xyz;
}
