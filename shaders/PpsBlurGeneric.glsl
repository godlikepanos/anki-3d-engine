#pragma anki vertShaderBegins

layout(location = 0) in vec2 position;

out vec2 vTexCoords;
out float vOffsets[2]; ///< For side pixels

uniform float imgDimension = 0.0; ///< the img width for hspass or the img height for vpass

const float BLURRING_OFFSET[2] = float[](1.3846153846, 3.2307692308); ///< The offset of side pixels


void main()
{
	vTexCoords = position;

	vOffsets[0] = BLURRING_OFFSET[0] / imgDimension;
	vOffsets[1] = BLURRING_OFFSET[1] / imgDimension;

	gl_Position = vec4(position * 2.0 - 1.0, 0.0, 1.0);
}


#pragma anki fragShaderBegins


uniform sampler2D img; ///< Input FAI
uniform sampler2D msNormalFai;

in vec2 vTexCoords;
in float vOffsets[2];

layout(location = 0) out vec3 fFragColor;


const float FIRST_WEIGHT = 0.2255859375;
const float WEIGHTS[2] = float[](0.314208984375, 0.06982421875);


void main()
{
	float blurringDist = texture2D(msNormalFai, vTexCoords).b;

	/*if(blurringDist == 0.0)
	{
		fFragColor = texture2D(img, vTexCoords).rgb;
	}
	else*/
	{
		// the center (0,0) pixel
		vec3 col = texture2D(img, vTexCoords).rgb * FIRST_WEIGHT;

		// side pixels
		for(int i = 0; i < 2; i++)
		{
			#if defined(HPASS)
				vec2 texCoords = vec2(vTexCoords.x + vOffsets[i] * blurringDist, vTexCoords.y);
				col += texture2D(img, texCoords).rgb * WEIGHTS[i];

				texCoords.x = vTexCoords.x - blurringDist * vOffsets[i];
				col += texture2D(img, texCoords).rgb * WEIGHTS[i];
			#elif defined(VPASS)
				vec2 texCoords = vec2(vTexCoords.x, vTexCoords.y + blurringDist * vOffsets[i]);
				col += texture2D(img, texCoords).rgb * WEIGHTS[i];

				texCoords.y = vTexCoords.y - blurringDist * vOffsets[i];
				col += texture2D(img, texCoords).rgb * WEIGHTS[i];
			#endif
		}

		fFragColor = col;
	}
}
