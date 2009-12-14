uniform sampler2D diffuse_map;
uniform sampler2D normal_map;

//varying vec3 lightDir;

varying vec3 normal, tangent_v, frag_pos_vspace;
varying float w;

void main()
{
	vec3 _l = normalize( gl_LightSource[0].position.xyz - frag_pos_vspace );

	vec3 _n = normalize( normal );
	vec3 _t = normalize( tangent_v );
	vec3 _b = cross( _n, _t ) * w;

	vec3 _N = normalize(texture2D(normal_map, gl_TexCoord[0].st).xyz * 2.0 - 1.0);

	vec3 v;
	v.x = dot(_N, _t);
	v.y = dot(_N, _b);
	v.z = dot(_N, _n);

	//_n = v;

	mat3 tbnMatrix = mat3(_t, _b, _n);
	_n = tbnMatrix * _N;

	gl_FragColor = texture2D(diffuse_map, gl_TexCoord[0].st) * max(0.0, dot(_n, _l));
	//gl_FragColor = vec4(w);


/// The proper way
//	vec3 n = normalize(texture2D(normal_map, gl_TexCoord[0].st).xyz * 2.0 - 1.0);
//	vec3 l = normalize(lightDir);
//
//	float nDotL = max(0.0, dot(n, l));
//
//	vec4 diffuse = texture2D(diffuse_map, gl_TexCoord[0].st) * nDotL;
//	vec4 color = diffuse;
//
//	gl_FragColor = color;
//	//gl_FragColor = texture2D(diffuse_map, gl_TexCoord[0].st);
//	//gl_FragColor = vec4(tangent_v * 0.5 + 0.5, 1.0);
}
