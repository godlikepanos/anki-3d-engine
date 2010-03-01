/** 
 * Simple vertex shader for IS and PPS stages. It is used for rendering a quad in the 
 * screen. Notice that it does not use ftransform(). We dont need it because we can
 * get the Normalized Display Coordinates ([-1,1]) simply by looking in the vertex
 * position. The vertex positions of the quad are from 0.0 to 1.0 for both axis.
 */

#pragma anki attribute position 0
attribute vec2 position;

varying vec2 texCoords;

void main()
{
	vec2 vert_pos = position; // the vert coords are {1.0,1.0}, {0.0,1.0}, {0.0,0.0}, {1.0,0.0}
	texCoords = vert_pos;
	vec2 vert_pos_ndc = vert_pos*2.0 - 1.0;
	gl_Position = vec4( vert_pos_ndc, 0.0, 1.0 );
}


