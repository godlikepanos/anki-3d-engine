#pragma anki vertShaderBegins

attribute vec3 position;
attribute vec2 texCoords;

uniform mat4 viewProjectionMat;
uniform mat4 viewMat;

varying vec2 texCoordsV2f;

void main()
{
	texCoordsV2f = texCoords;
	
	mat4 modelMat = viewMat;
	modelMat[3] = modelMat[7] = modelMat[11] = 0.0; // clear the tranlation. We want only the rotation part
	
	gl_Position = viewProjectionMat * (modelMat * vec4(position, 1.0));
}

#pragma anki fragShaderBegins

uniform sampler2D colorMap;
uniform sampler2D noiseMap;
uniform float timer = 0.0;
uniform vec3 sceneAmbientCol;

varying vec2 texCoordsV2f;

#define PI 3.14159265358979323846


void main()
{
	float scaledTimer = timer*0.4;

	vec2 displacement = texCoordsV2f - vec2(scaledTimer);

	vec3 noiseVec(normalize(texture2D(noisemap, displacement).xyz));
	noiseVec = (noiseVec * 2.0 - 1.0);
	
	/*
	 * edgeFactor is a value that varies from 1.0 to 0.0 and then again to 1.0. Its zero in the edge of the skybox quad
	 * and one in the middle of the quad. We use it in order to reduse the distortion at the polygon edges because of
	 * artifacts with the neighbor polygons
	 */
	float edgeFactor = sin(texCoordsV2f.s * PI) * sin(texCoordsV2f.t * PI);

	const float strengthFactor = 0.015;

	noiseVec = noiseVec * strengthFactor * edgeFactor;

	// write to the diffuse buffer
	gl_FragData[1].rgb = texture2D(colormap, texCoordsV2f + noiseVec.xy).rgb * sceneAmbientCol; 
}

