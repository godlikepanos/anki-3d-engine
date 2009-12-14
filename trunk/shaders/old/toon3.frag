// simple toon fragment shader
// www.lighthouse3d.com

uniform float specIntensity;
uniform vec4 specColor;
uniform float t[2];
uniform vec4 colors[3];

varying vec3 normal, lightDir;

void main()
{
	float intensity;
	vec3 n;
	vec4 color;

	n = normalize(normal);
	intensity = max(dot(lightDir,n),0.0); 

	if (intensity > specIntensity)
		color = specColor;
	else if (intensity > t[0])
		color = colors[0];	
	else if (intensity > t[1])
		color = colors[1];
	else
		color = colors[2];		

	gl_FragColor = color;
}
