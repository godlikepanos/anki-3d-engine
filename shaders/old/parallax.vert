uniform vec3 lightPos;
uniform mat4 model;

varying vec2 UVtextureCoordinate;
varying vec3 tanEyeVec;
varying vec4 vcol;
varying vec3 lightDir;

vec3 TBN(in vec3 v)
{
	vec3 tangent 	= vec3(1.0,0.0,0.0);
	vec3 normal		= normalize(gl_Normal);
	vec3 binormal = cross(normal,tangent);

	vec3 tvec;
	tvec.x = dot(tangent,v);
	tvec.y = dot(binormal,v);
	tvec.z = dot(normal,v);
	return tvec;
}

void main(void)
{
	gl_Position = ftransform();

	gl_TexCoord[0] = gl_MultiTexCoord0;
	UVtextureCoordinate = gl_TexCoord[0].xy;

	// Get view position in view coordinates
	vec3 eyePosition_ = gl_Position.xyz - gl_ModelViewMatrixInverse[3].xyz;

	vec3 eyeVec = eyePosition_ - ( gl_ModelViewMatrixInverse * gl_Vertex ).xyz;;



	vec3 tangent 	= gl_NormalMatrix * vec3(1.0,0.0,0.0);
	vec3 normal		= normalize(gl_NormalMatrix * gl_Normal);
	vec3 binormal = cross(normal, tangent);

	mat3 tangentBinormalNormalMatrix = mat3( tangent, binormal, normal );

	tanEyeVec = tangentBinormalNormalMatrix * eyeVec;




	lightDir = lightPos - ( gl_ModelViewMatrix * gl_Vertex ).xyz;
	vec3 v;
	v.x = dot (lightDir, tangent);
	v.y = dot (lightDir, binormal);
	v.z = dot (lightDir, normal);
	lightDir = normalize (v);
}
