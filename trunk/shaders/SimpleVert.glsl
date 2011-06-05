/// Simple vertex shader for IS and PPS stages. It is used for rendering a quad in the 
/// screen. Notice that it does not use ftransform(). We dont need it because we can
/// get the Normalized Display Coordinates ([-1,1]) simply by looking in the vertex
/// position. The vertex positions of the quad are from 0.0 to 1.0 for both axis.

layout(location = 0) in vec2 position; // the vert coords are {1.0,1.0}, {0.0,1.0}, {0.0,0.0}, {1.0,0.0}

out vec2 vTexCoords;

void main()
{
	vTexCoords = position;
	vec2 vertPosNdc = position * 2.0 - 1.0;
	gl_Position = vec4(vertPosNdc, 0.0, 1.0);
}


