varying vec3 lightDir;
varying vec3 halfVector;
attribute vec3 tangent;

void main()
{
    gl_Position = ftransform();
    gl_TexCoord[0] = gl_MultiTexCoord0;

    vec3 n = normalize(gl_NormalMatrix * gl_Normal);
    vec3 t = normalize(gl_NormalMatrix * tangent);
    vec3 b = cross(n, t) /* gl_MultiTexCoord1.w*/;

    mat3 tbnMatrix = mat3(t.x, b.x, n.x,
                          t.y, b.y, n.y,
                          t.z, b.z, n.z);

		vec3 vert_modelview = vec3(gl_ModelViewMatrix * gl_Vertex);

    lightDir = gl_LightSource[0].position.xyz - vert_modelview;
    lightDir = tbnMatrix * lightDir;

    halfVector = gl_LightSource[0].halfVector.xyz;
    halfVector = tbnMatrix * halfVector;

    gl_FrontColor = vec4( tangent, 1.0 );
}
