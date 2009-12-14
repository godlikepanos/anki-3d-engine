/*
TEXTBOOK LIGHTING MODEL

A = Am * Al
A is the final ambitent color, Am is the material's ambient and Al is the light's

D = Dm * Dl * dot( normal, light_dir )
D stands for diffuse color, Dm for material's diffuse and Dl for light's

S = Sm * Sl * si     ** this shader uses a slightly different method **
S is specular color and Sm, Sl as above.
si is the specular intensity and its equal to: si = pow( dot(normal, h), material_shininess )
h is h = normalize( light_dir + eye )

C = A * D * S
C is the final color vector


OTHER METHOD FOR SPEC LIGHTING:
The "ndotl > 0.0" check removes some unwanted specular calculations but it doesnt produces color under some conditions. This problem happens when:
1) the angle of the vert's normal and light is grater than than 90degr and
2) the material_shininess is small (close to 8.0)
So we mul the si with dot(normal, light) to correct this. The result is not so correct but it looks close to the standard one and it
allows us to use the the "ndotl > 0.0" check without the incorrect results. So:
S = Sm * Sl * ( si * dot(normal, light) );

*/

uniform sampler2D color_map;
uniform int light_ids[3];
varying vec3 normal, vert_pos;

void main()
{
	vec3 n = normalize( normal );

	vec4 ctex = texture2D( color_map, gl_TexCoord[0].st );

	vec4 color = vec4(0.0);


	vec3 eye = normalize(-vert_pos.xyz);

	for( int i=0; i<1; i++ )
	{
		if( light_ids[i] == -1 ) break;
		int light_id = light_ids[i];

		vec3 light_dir = normalize( gl_LightSource[light_id].position.xyz - vert_pos );


		color += gl_FrontLightProduct[light_id].ambient;


		float ndotl = dot( n, light_dir );

		if( ndotl > 0.0 )
		{
			// diffuse
			color += gl_FrontLightProduct[light_id].diffuse * ndotl;

			// specular

			vec3 r = reflect( -light_dir, n );
			float power = pow(max(0.0, dot(r, eye)), gl_FrontMaterial.shininess);
			/*vec3 h = normalize(light_dir + eye);
			float power = pow(max(0.0, dot(n, h)), shininess);*/
			color += gl_FrontLightProduct[light_id].specular * (power * ndotl);
		}

	}

  gl_FragColor = (color) * ctex;
  //gl_FragColor = vec4( light_ids.x, light_ids.y, light_ids.z, 1 );
}
