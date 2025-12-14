// Copyright (C) 2009-present, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <AnKi/Util/StdTypes.h>

#if ANKI_SIMD_SSE
#	include <smmintrin.h>
#elif ANKI_SIMD_NEON
#	include <arm_neon.h>
#elif !ANKI_SIMD_NONE
#	error "See file"
#endif

namespace anki {

/// Template class that holds SIMD info for the math classes.
template<typename T, U32 N>
class MathSimd
{
public:
	using Type = T[N];
	static constexpr U32 kAlignment = alignof(T);
};

#if ANKI_SIMD_SSE
// Specialize for F32
template<>
class MathSimd<F32, 4>
{
public:
	using Type = __m128;
	static constexpr U32 kAlignment = 16;
};
#elif ANKI_SIMD_NEON
// Specialize for F32
template<>
class MathSimd<F32, 4>
{
public:
	using Type = float32x4_t;
	static constexpr U32 kAlignment = 16;
};
#endif

// Suffle NEON vector. Code stolen by Jolt
#if ANKI_SIMD_NEON

// Constructing NEON values
#	if ANKI_COMPILER_MSVC
#		define ANKI_NEON_INT32x4(v1, v2, v3, v4) \
			{ \
				I64(v1) + (I64(v2) << 32), I64(v3) + (I64(v4) << 32) \
			}
#		define ANKI_NEON_UINT32x4(v1, v2, v3, v4) \
			{ \
				U64(v1) + (U64(v2) << 32), U64(v3) + (U64(v4) << 32) \
			}
#		define ANKI_NEON_INT8x16(v1, v2, v3, v4, v5, v6, v7, v8, v9, v10, v11, v12, v13, v14, v15, v16) \
			{ \
				I64(v1) + (I64(v2) << 8) + (I64(v3) << 16) + (I64(v4) << 24) + (I64(v5) << 32) + (I64(v6) << 40) + (I64(v7) << 48) \
					+ (I64(v8) << 56), \
					I64(v9) + (I64(v10) << 8) + (I64(v11) << 16) + (I64(v12) << 24) + (I64(v13) << 32) + (I64(v14) << 40) + (I64(v15) << 48) \
						+ (I64(v16) << 56) \
			}
#		define ANKI_NEON_UINT8x16(v1, v2, v3, v4, v5, v6, v7, v8, v9, v10, v11, v12, v13, v14, v15, v16) \
			{ \
				U64(v1) + (U64(v2) << 8) + (U64(v3) << 16) + (U64(v4) << 24) + (U64(v5) << 32) + (U64(v6) << 40) + (U64(v7) << 48) \
					+ (U64(v8) << 56), \
					U64(v9) + (U64(v10) << 8) + (U64(v11) << 16) + (U64(v12) << 24) + (U64(v13) << 32) + (U64(v14) << 40) + (U64(v15) << 48) \
						+ (U64(v16) << 56) \
			}
#	else
#		define ANKI_NEON_INT32x4(v1, v2, v3, v4) \
			{ \
				v1, v2, v3, v4 \
			}
#		define ANKI_NEON_UINT32x4(v1, v2, v3, v4) \
			{ \
				v1, v2, v3, v4 \
			}
#		define ANKI_NEON_INT8x16(v1, v2, v3, v4, v5, v6, v7, v8, v9, v10, v11, v12, v13, v14, v15, v16) \
			{ \
				v1, v2, v3, v4, v5, v6, v7, v8, v9, v10, v11, v12, v13, v14, v15, v16 \
			}
#		define ANKI_NEON_UINT8x16(v1, v2, v3, v4, v5, v6, v7, v8, v9, v10, v11, v12, v13, v14, v15, v16) \
			{ \
				v1, v2, v3, v4, v5, v6, v7, v8, v9, v10, v11, v12, v13, v14, v15, v16 \
			}
#	endif

// MSVC and GCC prior to version 12 don't define __builtin_shufflevector
#	if ANKI_COMPILER_MSVC || (ANKI_COMPILER_GCC && __GNUC__ < 12)

// Generic shuffle vector template
template<unsigned I1, unsigned I2, unsigned I3, unsigned I4>
inline float32x4_t neonSuffleFloat32x4(float32x4_t inV1, float32x4_t inV2)
{
	float32x4_t ret;
	ret = vmovq_n_f32(vgetq_lane_f32(I1 >= 4 ? inV2 : inV1, I1 & 0b11));
	ret = vsetq_lane_f32(vgetq_lane_f32(I2 >= 4 ? inV2 : inV1, I2 & 0b11), ret, 1);
	ret = vsetq_lane_f32(vgetq_lane_f32(I3 >= 4 ? inV2 : inV1, I3 & 0b11), ret, 2);
	ret = vsetq_lane_f32(vgetq_lane_f32(I4 >= 4 ? inV2 : inV1, I4 & 0b11), ret, 3);
	return ret;
}

// Specializations
template<>
inline float32x4_t neonSuffleFloat32x4<0, 1, 2, 2>(float32x4_t inV1, [[maybe_unused]] float32x4_t inV2)
{
	return vcombine_f32(vget_low_f32(inV1), vdup_lane_f32(vget_high_f32(inV1), 0));
}

template<>
inline float32x4_t neonSuffleFloat32x4<0, 1, 3, 3>(float32x4_t inV1, [[maybe_unused]] float32x4_t inV2)
{
	return vcombine_f32(vget_low_f32(inV1), vdup_lane_f32(vget_high_f32(inV1), 1));
}

template<>
inline float32x4_t neonSuffleFloat32x4<0, 1, 2, 3>(float32x4_t inV1, [[maybe_unused]] float32x4_t inV2)
{
	return inV1;
}

template<>
inline float32x4_t neonSuffleFloat32x4<1, 0, 3, 2>(float32x4_t inV1, [[maybe_unused]] float32x4_t inV2)
{
	return vcombine_f32(vrev64_f32(vget_low_f32(inV1)), vrev64_f32(vget_high_f32(inV1)));
}

template<>
inline float32x4_t neonSuffleFloat32x4<2, 2, 1, 0>(float32x4_t inV1, [[maybe_unused]] float32x4_t inV2)
{
	return vcombine_f32(vdup_lane_f32(vget_high_f32(inV1), 0), vrev64_f32(vget_low_f32(inV1)));
}

template<>
inline float32x4_t neonSuffleFloat32x4<2, 3, 0, 1>(float32x4_t inV1, [[maybe_unused]] float32x4_t inV2)
{
	return vcombine_f32(vget_high_f32(inV1), vget_low_f32(inV1));
}

// Used extensively by cross product
template<>
inline float32x4_t neonSuffleFloat32x4<1, 2, 0, 0>(float32x4_t inV1, [[maybe_unused]] float32x4_t inV2)
{
	static uint8x16_t table = ANKI_NEON_UINT8x16(0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a, 0x0b, 0x00, 0x01, 0x02, 0x03, 0x00, 0x01, 0x02, 0x03);
	return vreinterpretq_f32_u8(vqtbl1q_u8(vreinterpretq_u8_f32(inV1), table));
}

#		define ANKI_NEON_SHUFFLE_F32x4(vec1, vec2, index1, index2, index3, index4) neonSuffleFloat32x4<index1, index2, index3, index4>(vec1, vec2)
#		define ANKI_NEON_SHUFFLE_U32x4(vec1, vec2, index1, index2, index3, index4) \
			vreinterpretq_u32_f32((neonSuffleFloat32x4<index1, index2, index3, index4>(vreinterpretq_f32_u32(vec1), vreinterpretq_f32_u32(vec2))))

#	else
#		define ANKI_NEON_SHUFFLE_F32x4(vec1, vec2, index1, index2, index3, index4) \
			__builtin_shufflevector(vec1, vec2, index1, index2, index3, index4)
#		define ANKI_NEON_SHUFFLE_U32x4(vec1, vec2, index1, index2, index3, index4) \
			__builtin_shufflevector(vec1, vec2, index1, index2, index3, index4)
#	endif

#endif // ANKI_SIMD_NEON

} // end namespace anki
