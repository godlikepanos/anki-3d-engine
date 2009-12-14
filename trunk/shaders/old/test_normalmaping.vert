varying vec3 lightDir;
attribute vec4 tangent;

varying vec3 normal, tangent_v, frag_pos_vspace;
varying float w;

void main()
{
	gl_Position = ftransform();
	gl_TexCoord[0] = gl_MultiTexCoord0;

	frag_pos_vspace = vec3( gl_ModelViewMatrix * gl_Vertex );

	normal = normalize(gl_NormalMatrix * gl_Normal);
	tangent_v = normalize( gl_NormalMatrix * vec3(tangent) );
	w =  tangent.w;

//	vec3 n = normalize(gl_NormalMatrix * gl_Normal);
//	vec3 t = normalize(gl_NormalMatrix * vec3(tangent));
//	vec3 b = cross(n, t) * tangent.w;
//
//	vec3 vert_modelview = vec3(gl_ModelViewMatrix * gl_Vertex);
//	lightDir = gl_LightSource[0].position.xyz - vert_modelview;
//
////	mat3 tbnMatrix = mat3(t.x, b.x, n.x,
////												t.y, b.y, n.y,
////												t.z, b.z, n.z);
//
//
////	mat3 tbnMatrix = mat3(t.x, t.y, t.z,
////												b.x, b.y, b.z,
////												n.x, n.y, n.z);
//
//	vec3 v;
//	v.x = dot (lightDir, t);
//	v.y = dot (lightDir, b);
//	v.z = dot (lightDir, n);
//	lightDir = normalize (v);
//
//	//lightDir = tbnMatrix * lightDir;
}
