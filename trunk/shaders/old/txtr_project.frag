/**
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



void Phong( in int _light_id, in vec3 _light_dir, in vec3 _eye_vec, in vec3 _normal, out vec4 _color, out float _lambert_term )
{
	vec4 _ambient = gl_FrontMaterial.ambient * gl_LightSource[_light_id].ambient;
	_color = _ambient;

	_lambert_term = dot( _normal, _light_dir );

	if( _lambert_term < 0.0 ) return; // dont calculate specular or diffuse

	vec4 _diffuse = gl_FrontMaterial.diffuse * gl_LightSource[_light_id].diffuse;
	_color += _diffuse * _lambert_term;

	/// TEXTBOOK METHOD:
	/// the textbook way to calc spec color is as explained on the top:
	/// S = Sm * Sl * si;
	/// MY METHOD:
	/// The "ndotl > 0.0" check removes some unwanted specular calculations but it doesnt produces color under some conditions. This problem happens when:
	/// 1) the angle of the vert's normal and light is grater than than 90degr and
	/// 2) the material_shininess is small (close to 8.0)
	/// So we mul the si with dot(normal, light) to correct this. The result is not so correct but it looks close to the standard one and it
	/// allows us to use the the "ndotl > 0.0" check without the incorrect results. So:
	/// S = Sm * Sl * ( si * dot(normal, light) );

	float _shininess = gl_FrontMaterial.shininess;

	vec3 _h = normalize( _light_dir + _eye_vec );
	float _spec_intensity = pow(max(0.0, dot(_normal, _h)), _shininess);
	_color += gl_FrontMaterial.specular * gl_LightSource[_light_id].specular * (_spec_intensity * _lambert_term);
}

varying vec3 normal, vert_light_vec, eye_vec;
varying vec4 txtr_coord;
uniform sampler2D t0;

void main()
{
	vec3 _n = normalize( normal );
  vec3 _light_dir = normalize( vert_light_vec );
  vec3 _eye_vec = normalize( eye_vec );
	vec4 _color;
	float _lambert_term;
	Phong( 0, _light_dir, _eye_vec, _n, _color, _lambert_term );

	if( txtr_coord.q > 0.0 )
	{
		vec4 _texel = texture2DProj( t0, txtr_coord.xyz );
		vec4 _texel_with_lambert = _texel * _lambert_term;
		gl_FragColor = _color * _texel_with_lambert;
	}
	else
		gl_FragColor = vec4(0.0);
}

