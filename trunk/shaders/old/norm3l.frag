uniform sampler2D color_map;
uniform sampler2D normal_map;
uniform sampler2D specular_map;

varying vec3 light_dirs[3];
uniform int light_ids[3];
varying vec3 vert_modelview;

varying vec4 _tangent;

void main()
{
	vec3 n = normalize( texture2D(normal_map, gl_TexCoord[0].st).xyz * 2.0 - 1.0 );
	vec3 t = normalize( _tangent.xyz );

	vec4 diffuse, ambient, specular;
	diffuse = ambient = specular = vec4(0.0, 0.0, 0.0, 0.0);

	for( int i=0; i<3; i++ )
	{
		if( light_ids[i] == -1 ) break;
		int light_id = light_ids[i];

		vec3 b = cross( n, t );
		mat3 rotate = mat3( t, b, n );
		//mat3 rotate = mat3(t.x, b.x, n.x, t.y, b.y, n.y, t.z, b.z, n.z);

		vec3 light_dir = gl_LightSource[light_id].position.xyz - vert_modelview;
		light_dir = rotate * light_dir;
		light_dir = normalize( light_dir );



		float ndotl = dot( n, light_dir );

		ambient += gl_FrontLightProduct[light_id].ambient;

		if( ndotl > 0.0 )
		{
			//diffuse += gl_FrontLightProduct[light_id].diffuse * ndotl;


			vec3 eye = -vert_modelview;
			eye = normalize(rotate * eye);

			vec3 r = reflect( -light_dir, n );
			float power = pow( max(0.0, dot(r, eye)),  gl_FrontMaterial.shininess );

//			vec4 spec_texel = texture2D(specular_map, gl_TexCoord[0].st);
//			float shininess = clamp( (spec_texel.r + spec_texel.b + spec_texel.g), 0.0, 1.0);
//			specular += gl_LightSource[i].specular * (shininess * power);
			specular += gl_FrontLightProduct[light_id].specular * (power);
		}
	}

	vec4 col_tex = texture2D(color_map, gl_TexCoord[0].st);
	gl_FragColor = (ambient + diffuse + specular) ;
}
