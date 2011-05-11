#pragma anki vertShaderBegins

in vec2 position;
in vec2 texCoords;

uniform mat2 tranformation; ///< Position transformation
uniform mat2 textureTranformation; ///< Texture transformation

out vec2 vTexCoords;


void main(void)
{
	vTexCoords = textureTranformation * texCoords;
	gl_Position = vec4(tranformation * position, 0.0, 1.0);
}

#pragma anki fragShaderBegins

in vec2 vTexCoords;

uniform sampler2D texture;
uniform vec4 color;

layout(location = 0) out vec4 fColor;


void main()
{
	vec4 texCol = texture2D(texture, vTexCoords).rgba * color;

	fColor = texCol;
}
