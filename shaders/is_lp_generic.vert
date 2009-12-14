attribute vec3 view_vector;
varying vec2 tex_coords;
varying vec3 vpos;

void main()
{
	vpos = view_vector;
	vec2 vert_pos = gl_Vertex.xy; // the vert coords are {1.0,1.0}, {0.0,1.0}, {0.0,0.0}, {1.0,0.0}
	tex_coords = vert_pos;
	vec2 vert_pos_ndc = vert_pos*2.0 - 1.0;
	gl_Position = vec4( vert_pos_ndc, 0.0, 1.0 );
}
