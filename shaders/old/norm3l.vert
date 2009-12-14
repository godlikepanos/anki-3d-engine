attribute vec4 tangent;
uniform int light_ids[3];
varying vec3 light_dirs[3];
varying vec3 vert_modelview;
varying vec3 eye;
varying vec4 _tangent;


void main()
{
	gl_Position = ftransform();
	gl_TexCoord[0] = gl_MultiTexCoord0;
	vert_modelview = vec3(gl_ModelViewMatrix * gl_Vertex);


	_tangent = tangent;


	/*vec3 n = normalize( gl_NormalMatrix * gl_Normal );
	vec3 t = normalize( gl_NormalMatrix * tangent.xyz );
	//vec3 b = cross(n, t) * -tangent.w;             put this if you want defuse on the un-lighen area of a model
	vec3 b = cross(n, t);

	mat3 rotate = mat3(t.x, b.x, n.x,
	                   t.y, b.y, n.y,
	                   t.z, b.z, n.z);
	//mat3 rotate = mat3( t, b, n );

	for( int i=0; i<3; i++ )
	{
		int light_id = light_ids[i];
		if( light_id == -1 ) break;
		vec3 temp = gl_LightSource[light_id].position.xyz - vert_modelview;
		light_dirs[i] = normalize( rotate * temp );
		eye = rotate * -vert_modelview;
	}*/
}
