varying vec2 UVtextureCoordinate;
varying vec3 tanEyeVec;

varying vec3 lightDir;

uniform sampler2D texture;
uniform sampler2D heightMap;
uniform sampler2D normalMap;


void main (void)
{
	float scale = 0.04;
	float bias = scale * 0.4;

   vec3 normalizedEyeVector = normalize( tanEyeVec );

   float h = texture2D( heightMap, UVtextureCoordinate ).r;
   float height = -scale * h + bias;
   vec2 nextTextureCoordinate = height * normalizedEyeVector.xy + UVtextureCoordinate;




	vec3 n = normalize(texture2D(normalMap, nextTextureCoordinate).xyz * 2.0 - 1.0);
	vec3 l = normalize(lightDir);

	float nDotL = max(0.0, dot(n, l));

	vec4 diffuse = texture2D(texture, nextTextureCoordinate);

	gl_FragColor = diffuse * nDotL;
	//gl_FragColor = texture2D(diffuse_map, gl_TexCoord[0].st);
	//gl_FragColor = vec4(tangent_v * 0.5 + 0.5, 1.0);




   //gl_FragColor = texture2D( texture, nextTextureCoordinate );


}
