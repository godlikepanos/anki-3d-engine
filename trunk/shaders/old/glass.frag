// vertex to fragment shader io
varying vec3 normal_v;
varying vec3 vert_pos_modelview_v;

uniform vec3 color;

// globals
const float edgefalloff = 6.0;

// entry point
void main()
{
    float opac = dot( normalize(-normal_v), normalize(-vert_pos_modelview_v) );
    opac = abs(opac);
    opac = 1.0-pow(opac, edgefalloff);

    gl_FragColor =  vec4( opac * color, opac );
}
