varying vec2 tex_coords;
uniform mat4 matrix;

void main()
{
	tex_coords = gl_MultiTexCoord0.xy;

	vec4 pos = gl_ModelViewProjectionMatrix * gl_Vertex;
	//pos = pos / pos.w;

	float L = length( pos.xyz );
	pos = pos / L;

	pos.z = pos.z + 1;
	pos.x = pos.x / pos.z;
	pos.y = pos.y / pos.z;

	pos.z = (L - 0.1)/(200.0-0.1);
	pos.w = 1.0;

	gl_Position = pos;
}
