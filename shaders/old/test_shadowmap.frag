uniform sampler2DShadow s0;
varying vec4 txtr_coord;

void main ()
{
	const float kTransparency = 0.5;
	const float smap_size = _SHADOW_MAP_QUALITY_;
	vec4 color = gl_Color;

	vec3 shadowUV = txtr_coord.xyz / txtr_coord.w; // ToDo: na do mipos kai xrimopoiiso anti gia vec3 gia to shadowUV enan aplo float. Mallon exei to idio xroma gia r kai g kai b ontas gray scale

	vec4 shadowColor = shadow2D(s0, shadowUV);

#if defined( _PCF_1_PASSES_ )
	float mapScale = 1.0 / smap_size;

	shadowColor += shadow2D(s0, shadowUV.xyz + vec3( mapScale,  mapScale, 0));
	shadowColor += shadow2D(s0, shadowUV.xyz + vec3( mapScale, -mapScale, 0));
	shadowColor += shadow2D(s0, shadowUV.xyz + vec3( mapScale,  	  0, 0));
	shadowColor += shadow2D(s0, shadowUV.xyz + vec3(-mapScale,  mapScale, 0));
	shadowColor += shadow2D(s0, shadowUV.xyz + vec3(-mapScale, -mapScale, 0));
	shadowColor += shadow2D(s0, shadowUV.xyz + vec3(-mapScale,  	  0, 0));
	shadowColor += shadow2D(s0, shadowUV.xyz + vec3(        0,  mapScale, 0));
	shadowColor += shadow2D(s0, shadowUV.xyz + vec3(        0, -mapScale, 0));
	shadowColor /= 9.0;
#elif defined( _PCF_2_PASSES_ )
	float mapScale = 1.0 / smap_size;

	shadowColor += shadow2D(s0, shadowUV.xyz + vec3( mapScale,  mapScale, 0));
	shadowColor += shadow2D(s0, shadowUV.xyz + vec3( mapScale, -mapScale, 0));
	shadowColor += shadow2D(s0, shadowUV.xyz + vec3( mapScale,  	  0, 0));
	shadowColor += shadow2D(s0, shadowUV.xyz + vec3(-mapScale,  mapScale, 0));
	shadowColor += shadow2D(s0, shadowUV.xyz + vec3(-mapScale, -mapScale, 0));
	shadowColor += shadow2D(s0, shadowUV.xyz + vec3(-mapScale,  	  0, 0));
	shadowColor += shadow2D(s0, shadowUV.xyz + vec3(        0,  mapScale, 0));
	shadowColor += shadow2D(s0, shadowUV.xyz + vec3(        0, -mapScale, 0));

	mapScale = 2.0 / smap_size;
	shadowColor += shadow2D(s0, shadowUV.xyz + vec3( mapScale,  mapScale, 0));
	shadowColor += shadow2D(s0, shadowUV.xyz + vec3( mapScale, -mapScale, 0));
	shadowColor += shadow2D(s0, shadowUV.xyz + vec3( mapScale,  	  0, 0));
	shadowColor += shadow2D(s0, shadowUV.xyz + vec3(-mapScale,  mapScale, 0));
	shadowColor += shadow2D(s0, shadowUV.xyz + vec3(-mapScale, -mapScale, 0));
	shadowColor += shadow2D(s0, shadowUV.xyz + vec3(-mapScale,  	  0, 0));
	shadowColor += shadow2D(s0, shadowUV.xyz + vec3(        0,  mapScale, 0));
	shadowColor += shadow2D(s0, shadowUV.xyz + vec3(        0, -mapScale, 0));

	shadowColor /= 18.0;

#endif


	shadowColor += kTransparency;
	shadowColor = clamp(shadowColor, 0.0, 1.0);

	if( shadowUV.x >= 0.0 && shadowUV.y >= 0.0 && shadowUV.x <= 1.0 && shadowUV.y <= 1.0 && txtr_coord.w > 0.0 && txtr_coord.w < 10.0  )
	{
		gl_FragColor = color * shadowColor;
	}
	else
	{
		gl_FragColor = color * vec4(kTransparency);
	}
}
