#pragma anki vert_shader_begins

#pragma anki include "shaders/simple_vert.glsl"

#pragma anki frag_shader_begins

#pragma anki include "shaders/linear_depth.glsl"
#pragma anki include "shaders/pack.glsl"

#pragma anki uniform noise_map 2
uniform sampler2D noise_map;
uniform sampler2D ms_normal_fai;
#pragma anki uniform ms_depth_fai 1
uniform sampler2D ms_depth_fai;
#pragma anki uniform camerarange 0
uniform vec2 camerarange;  // = vec2( znear, zfar )

varying vec2 tex_coords;
const float totStrength = 1.0;
const float strength = 0.07;
const float offset = 18.0;
const float falloff = 0.000002;
const float rad = 0.006;
const int SAMPLES = 16; // 10 is good
const float invSamples = 1.0/float(SAMPLES);

void main(void)
{
	// these are the random vectors inside a unit sphere
	vec3 pSphere[16] = vec3[](vec3(0.53812504, 0.18565957, -0.43192),vec3(0.13790712, 0.24864247, 0.44301823),vec3(0.33715037, 0.56794053, -0.005789503),vec3(-0.6999805, -0.04511441, -0.0019965635),vec3(0.06896307, -0.15983082, -0.85477847),vec3(0.056099437, 0.006954967, -0.1843352),vec3(-0.014653638, 0.14027752, 0.0762037),vec3(0.010019933, -0.1924225, -0.034443386),vec3(-0.35775623, -0.5301969, -0.43581226),vec3(-0.3169221, 0.106360726, 0.015860917),vec3(0.010350345, -0.58698344, 0.0046293875),vec3(-0.08972908, -0.49408212, 0.3287904),vec3(0.7119986, -0.0154690035, -0.09183723),vec3(-0.053382345, 0.059675813, -0.5411899),vec3(0.035267662, -0.063188605, 0.54602677),vec3(-0.47761092, 0.2847911, -0.0271716));
	//const vec3 pSphere[8] = vec3[](vec3(0.24710192, 0.6445882, 0.033550154),vec3(0.00991752, -0.21947019, 0.7196721),vec3(0.25109035, -0.1787317, -0.011580509),vec3(-0.08781511, 0.44514698, 0.56647956),vec3(-0.011737816, -0.0643377, 0.16030222),vec3(0.035941467, 0.04990871, -0.46533614),vec3(-0.058801126, 0.7347013, -0.25399926),vec3(-0.24799341, -0.022052078, -0.13399573));
	//const vec3 pSphere[12] = vec3[](vec3(-0.13657719, 0.30651027, 0.16118456),vec3(-0.14714938, 0.33245975, -0.113095455),vec3(0.030659059, 0.27887347, -0.7332209),vec3(0.009913514, -0.89884496, 0.07381549),vec3(0.040318526, 0.40091, 0.6847858),vec3(0.22311053, -0.3039437, -0.19340435),vec3(0.36235332, 0.21894878, -0.05407306),vec3(-0.15198798, -0.38409665, -0.46785462),vec3(-0.013492276, -0.5345803, 0.11307949),vec3(-0.4972847, 0.037064247, -0.4381323),vec3(-0.024175806, -0.008928787, 0.17719103),vec3(0.694014, -0.122672155, 0.33098832));
	//const vec3 pSphere[10] = vec3[](vec3(-0.010735935, 0.01647018, 0.0062425877),vec3(-0.06533369, 0.3647007, -0.13746321),vec3(-0.6539235, -0.016726388, -0.53000957),vec3(0.40958285, 0.0052428036, -0.5591124),vec3(-0.1465366, 0.09899267, 0.15571679),vec3(-0.44122112, -0.5458797, 0.04912532),vec3(0.03755566, -0.10961345, -0.33040273),vec3(0.019100213, 0.29652783, 0.066237666),vec3(0.8765323, 0.011236004, 0.28265962),vec3(0.29264435, -0.40794238, 0.15964167));
	// grab a normal for reflecting the sample rays later on
	vec3 fres = normalize((texture2D(noise_map,tex_coords*offset).xyz*2.0) - vec3(1.0));

	vec4 currentPixelSample = texture2D(ms_normal_fai,tex_coords);

	float currentPixelDepth = ReadFromTexAndLinearizeDepth( ms_depth_fai, tex_coords, camerarange.x, camerarange.y );

	// current fragment coords in screen space
	vec3 ep = vec3( tex_coords.xy, currentPixelDepth );
	// get the normal of current fragment
	vec3 norm = UnpackNormal(currentPixelSample.xy);

	float bl = 0.0;
	// adjust for the depth ( not shure if this is good..)
	float radD = rad/currentPixelDepth;

	vec3 ray, se, occNorm;
	float occluderDepth, depthDifference, normDiff;

	for( int i=0; i<SAMPLES; ++i )
	{
		// get a vector (randomized inside of a sphere with radius 1.0) from a texture and reflect it
		ray = radD*reflect(pSphere[i],fres);

		// if the ray is outside the hemisphere then change direction
		se = ep + sign(dot(ray,norm) )*ray;

		// get the depth of the occluder fragment
		vec4 occluderFragment = texture2D(ms_normal_fai,se.xy);

		// get the normal of the occluder fragment
		occNorm = UnpackNormal(occluderFragment.xy);

		// if depthDifference is negative = occluder is behind current fragment
		depthDifference = currentPixelDepth - ReadFromTexAndLinearizeDepth( ms_depth_fai, se.xy, camerarange.x, camerarange.y );;

		// calculate the difference between the normals as a weight

		normDiff = (1.0-dot(occNorm,norm));
		// the falloff equation, starts at falloff and is kind of 1/x^2 falling
		bl += step(falloff,depthDifference)*normDiff*(1.0-smoothstep(falloff,strength,depthDifference));
	}

	// output the result
	float ao = 1.0-totStrength*bl*invSamples;
	gl_FragColor.a = ao;
}
