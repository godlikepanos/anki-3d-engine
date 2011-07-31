#pragma anki start vertexShader

layout(location = 0) in vec2 position;

uniform mat3 transformation; ///< Position transformation
uniform mat3 textureTranformation; ///< Texture transformation

out vec2 vTexCoords;


void main(void)
{
	vec3 pos3d = vec3(position, 1.0);
	vTexCoords = (textureTranformation * pos3d).xy;
	gl_Position = vec4(transformation * pos3d, 1.0);
}

#pragma anki start fragmentShader

in vec2 vTexCoords;

uniform sampler2D texture;
uniform vec4 color;

layout(location = 0) out vec4 fColor;


void main()
{
	float r = texture2D(texture, vTexCoords).r;

	fColor = vec4(r) * color;
	//fColor = texCol - texCol + vec4(1.0, 0.0, 1.0, 0.1);
	//fColor = texCol - texCol + vec4(texture2D(texture, vTexCoords).r);
}
