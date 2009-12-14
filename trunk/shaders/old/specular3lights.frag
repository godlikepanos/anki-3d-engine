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

*/

uniform sampler2D color_map, normal_map;
uniform int lights[3];
varying vec3 normal, vert_pos;

void main()
{
  float shininess = gl_FrontMaterial.shininess;
	vec3 n = normalize( normal );

	vec4 diffuse, ambient, specular;
	diffuse = ambient = specular = vec4(0.0, 0.0, 0.0, 0.0);

	vec3 light_dir;
	float ndotl;

	for( int i=0; i<3; i++ )
	{
		if( lights[i] == -1 ) continue;

		light_dir = normalize( gl_LightSource[i].position.xyz - vert_pos );

		ndotl = dot( n, light_dir );


		ambient += gl_FrontLightProduct[i].ambient;
		diffuse += gl_FrontLightProduct[i].diffuse * max(0.0, ndotl);


		// add some specular lighting IF necessary
		if( ndotl > 0.0 )
		{
			/*
			TEXTBOOK METHOD:
			the textbook way to calc spec color is as explained on the top:
			S = Sm * Sl * si;

			MY METHOD:
			The "ndotl > 0.0" check removes some unwanted specular calculations but it doesnt produces color under some conditions. This problem happens when:
			1) the angle of the vert's normal and light is grater than than 90degr and
			2) the material_shininess is small (close to 8.0)
			So we mul the si with dot(normal, light) to correct this. The result is not so correct but it looks close to the standard one and it
			allows us to use the the "ndotl > 0.0" check without the incorrect results. So:
			S = Sm * Sl * ( si * dot(normal, light) );
			*/

			vec3 eye = -vert_pos.xyz;
			eye = normalize(eye);

			vec3 h = normalize( light_dir + eye );
			float power = pow(max(0.0, dot(n, h)), shininess);
			specular += gl_FrontLightProduct[i].specular * (power * ndotl);
		}
	}

	vec4 color_texel = texture2D( color_map, gl_TexCoord[0].st );
	vec4 normal_texel = texture2D( normal_map, gl_TexCoord[0].st );
	vec4 texel = color_texel;

  gl_FragColor = (ambient + diffuse + specular) * texel;
}
