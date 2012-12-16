/// Generic add
#define add_DEFINED
#define add(a, b) ((a) + (b))

/// Generic mul
#define mul_DEFINED
#define mul(a, b) ((a) * (b))

#define assign_DEFINED
#define assign(a, b) ((a) = (b))

#define vec4ToVec3_DEFINED
#define vec4ToVec3(a) ((a).xyz)

#define vec3ToVec4_DEFINED
#define vec3ToVec4(a, w) (vec4((a), (w)))

#define getW_DEFINED
#define getW(a) ((a).w)

#define setW_DEFINED
#define setW(a, b) ((a).w = (b))
