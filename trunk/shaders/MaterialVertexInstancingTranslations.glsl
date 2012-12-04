/// @file 
/// Contains variables and functions for instancing of translations

layout(location = 1) uniform instancingBlock
{
	vec3 instancingTranslations[];
};

//==============================================================================
#define setVaryings_particle_DEFINED
void setVaryings_particle(in mat4 billboardModelViewProjectionMat)
{
#if defined(PASS_DEPTH) && LOD > 0
	// No tex coords for you
#else
	vTexCoords = texCoords;
#endif

	gl_Position = billboardModelViewProjectionMat 
		* vec4(position + instancingTranslations[gl_InstanceID], 1.0);
}
