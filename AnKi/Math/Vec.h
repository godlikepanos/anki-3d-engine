// Copyright (C) 2009-present, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <AnKi/Math/Common.h>
#include <AnKi/Util/F16.h>

namespace anki {

template<typename T, U32 kComponentCount>
class TVec;

#define ANKI_VEC_METHOD0(returnType, method) \
	[[nodiscard]] returnType method() const \
	{ \
		return OutVec(*this).method(); \
	}

#define ANKI_VEC_METHOD1(returnType, method, argType) \
	[[nodiscard]] returnType method(argType arg0) const \
	{ \
		return OutVec(*this).method(arg0); \
	}

#define ANKI_VEC_METHOD2(returnType, method, argType0, argType1) \
	[[nodiscard]] returnType method(argType0 arg0, argType1 arg1) const \
	{ \
		return OutVec(*this).method(arg0, arg1); \
	}

// The TVecXXXData classes need to re-define those and forward them to the actual TVec class. This is needed in cases we do something like:
// a = v4.xyz.normalize();
#define ANKI_VEC_METHODS \
	ANKI_VEC_METHOD1(OutVec, operator*, OutVec) \
	ANKI_VEC_METHOD1(OutVec, operator*, T) \
	ANKI_VEC_METHOD1(OutVec, operator*=, OutVec) \
	ANKI_VEC_METHOD1(OutVec, operator*=, T) \
	ANKI_VEC_METHOD1(OutVec, operator/, OutVec) \
	ANKI_VEC_METHOD1(OutVec, operator/, T) \
	ANKI_VEC_METHOD1(OutVec, operator/=, OutVec) \
	ANKI_VEC_METHOD1(OutVec, operator/=, T) \
	ANKI_VEC_METHOD1(OutVec, operator+, OutVec) \
	ANKI_VEC_METHOD1(OutVec, operator+, T) \
	ANKI_VEC_METHOD1(OutVec, operator+=, OutVec) \
	ANKI_VEC_METHOD1(OutVec, operator+=, T) \
	ANKI_VEC_METHOD1(OutVec, operator-, OutVec) \
	ANKI_VEC_METHOD1(OutVec, operator-, T) \
	ANKI_VEC_METHOD1(OutVec, operator-=, OutVec) \
	ANKI_VEC_METHOD1(OutVec, operator-=, T) \
	ANKI_VEC_METHOD0(OutVec, operator-) \
	ANKI_VEC_METHOD1(OutVec, operator<<, OutVec) \
	ANKI_VEC_METHOD1(OutVec, operator<<, T) \
	ANKI_VEC_METHOD1(OutVec, operator<<=, OutVec) \
	ANKI_VEC_METHOD1(OutVec, operator<<=, T) \
	ANKI_VEC_METHOD1(OutVec, operator>>, OutVec) \
	ANKI_VEC_METHOD1(OutVec, operator>>, T) \
	ANKI_VEC_METHOD1(OutVec, operator>>=, OutVec) \
	ANKI_VEC_METHOD1(OutVec, operator>>=, T) \
	ANKI_VEC_METHOD1(OutVec, operator|, OutVec) \
	ANKI_VEC_METHOD1(OutVec, operator|, T) \
	ANKI_VEC_METHOD1(OutVec, operator|=, OutVec) \
	ANKI_VEC_METHOD1(OutVec, operator|=, T) \
	ANKI_VEC_METHOD1(OutVec, operator^, OutVec) \
	ANKI_VEC_METHOD1(OutVec, operator^, T) \
	ANKI_VEC_METHOD1(OutVec, operator^=, OutVec) \
	ANKI_VEC_METHOD1(OutVec, operator^=, T) \
	ANKI_VEC_METHOD1(OutVec, operator&, OutVec) \
	ANKI_VEC_METHOD1(OutVec, operator&, T) \
	ANKI_VEC_METHOD1(OutVec, operator&=, OutVec) \
	ANKI_VEC_METHOD1(OutVec, operator&=, T) \
	ANKI_VEC_METHOD1(OutVec, operator%, OutVec) \
	ANKI_VEC_METHOD1(OutVec, operator%, T) \
	ANKI_VEC_METHOD1(OutVec, operator%=, OutVec) \
	ANKI_VEC_METHOD1(OutVec, operator%=, T) \
	ANKI_VEC_METHOD1(Bool, operator==, OutVec) \
	ANKI_VEC_METHOD1(Bool, operator==, T) \
	ANKI_VEC_METHOD1(Bool, operator<, OutVec) \
	ANKI_VEC_METHOD1(Bool, operator<, T) \
	ANKI_VEC_METHOD1(Bool, operator<=, OutVec) \
	ANKI_VEC_METHOD1(Bool, operator<=, T) \
	ANKI_VEC_METHOD1(Bool, operator>, OutVec) \
	ANKI_VEC_METHOD1(Bool, operator>, T) \
	ANKI_VEC_METHOD1(Bool, operator>=, OutVec) \
	ANKI_VEC_METHOD1(Bool, operator>=, T) \
	ANKI_VEC_METHOD1(T, dot, OutVec) \
	ANKI_VEC_METHOD1(OutVec, cross, OutVec) \
	ANKI_VEC_METHOD0(T, lengthSquared) \
	ANKI_VEC_METHOD0(T, length) \
	ANKI_VEC_METHOD0(OutVec, normalize) \
	ANKI_VEC_METHOD2(OutVec, clamp, OutVec, OutVec) \
	ANKI_VEC_METHOD2(OutVec, clamp, T, T) \
	ANKI_VEC_METHOD0(U32, packSnorm4x8) \
	ANKI_VEC_METHOD0(String, toString)

template<typename T, U32 kComponentCount, U32... kIndices>
class TVecSwizzledData
{
public:
	static constexpr U32 kIndexCount = sizeof...(kIndices);

	using OutVec = TVec<T, kIndexCount>;
	using MainVec = TVec<T, kComponentCount>;

	// For doing something like this: v4.xw = Vec2(1.0f)
	MainVec& operator=(OutVec in) requires(kComponentCount >= kIndexCount)
	{
		const U32 indices[] = {kIndices...};
		for(U32 i = 0; i < kIndexCount; ++i)
		{
			m_arr[indices[i]] = in.m_arr[i];
		}
		return *reinterpret_cast<MainVec*>(this);
	}

	// For doing something like this: v2 = v4.xw
	operator OutVec() const
	{
		OutVec vec;
		const U32 indices[] = {kIndices...};
		for(U32 i = 0; i < kIndexCount; ++i)
		{
			vec.m_arr[i] = m_arr[indices[i]];
		}
		return vec;
	}

	template<U32 kYComponentCount, U32... kYIndices>
	Bool operator==(const TVecSwizzledData<T, kYComponentCount, kYIndices...>& b) const requires(sizeof...(kIndices) == sizeof...(kYIndices))
	{
		return OutVec(*this) == OutVec(b);
	}

	ANKI_VEC_METHODS

private:
	T m_arr[kComponentCount];
};

// Note: kSpecialConst is not of type T because some compilers don't like floats as template constants
template<typename T, U32 kComponentCount, U32 kSpecialConst>
class TVec4SpecialData
{
public:
	using OutVec = TVec<T, 4>;

	operator OutVec() const
	{
		OutVec vec;
		for(U32 i = 0; i < kComponentCount; ++i)
		{
			vec.m_arr[i] = m_arr[i];
		}
		vec.m_arr[3] = T(kSpecialConst);
		return vec;
	}

	ANKI_VEC_METHODS

private:
	T m_arr[kComponentCount];
};

template<typename T, U32 kComponentCount>
class TVecSimdData
{
public:
	T m_simd[kComponentCount];
};

template<typename T>
class TVecSimdData<T, 4>
{
public:
#if ANKI_SIMD_SSE
	__m128 m_simd;
#elif ANKI_SIMD_NEON
	float32x4_t m_simd;
#else
	T m_simd[4];
#endif
};

// Skip some warnings cause we really nead anonymous structs inside unions
#if ANKI_COMPILER_MSVC
#	pragma warning(push)
#	pragma warning(disable : 4201)
#elif ANKI_COMPILER_CLANG
#	pragma GCC diagnostic push
#	pragma GCC diagnostic ignored "-Wgnu-anonymous-struct"
#	pragma GCC diagnostic ignored "-Wnested-anon-types"
#elif ANKI_COMPILER_GCC
#	pragma GCC diagnostic push
#	pragma GCC diagnostic ignored "-Wpedantic"
#endif

template<typename T, U32 kComponentCount>
class TVecData;

// Data specializations

template<typename T>
class TVecData<T, 2>
{
public:
	using Simd = Array<T, 2>;

	union
	{
		struct
		{
			T x;
			T y;
		};

		T m_carr[2];
		Array<T, 2> m_arr;
		Simd m_simd;

		TVecSwizzledData<T, 2, 0, 0> xx;
		TVecSwizzledData<T, 2, 0, 1> xy;
		TVecSwizzledData<T, 2, 1, 0> yx;
		TVecSwizzledData<T, 2, 1, 1> yy;
		TVecSwizzledData<T, 2, 0, 0, 0> xxx;
		TVecSwizzledData<T, 2, 0, 0, 1> xxy;
		TVecSwizzledData<T, 2, 0, 1, 0> xyx;
		TVecSwizzledData<T, 2, 0, 1, 1> xyy;
		TVecSwizzledData<T, 2, 1, 0, 0> yxx;
		TVecSwizzledData<T, 2, 1, 0, 1> yxy;
		TVecSwizzledData<T, 2, 1, 1, 0> yyx;
		TVecSwizzledData<T, 2, 1, 1, 1> yyy;
		TVecSwizzledData<T, 2, 0, 0, 0, 0> xxxx;
		TVecSwizzledData<T, 2, 0, 0, 0, 1> xxxy;
		TVecSwizzledData<T, 2, 0, 0, 1, 0> xxyx;
		TVecSwizzledData<T, 2, 0, 0, 1, 1> xxyy;
		TVecSwizzledData<T, 2, 0, 1, 0, 0> xyxx;
		TVecSwizzledData<T, 2, 0, 1, 0, 1> xyxy;
		TVecSwizzledData<T, 2, 0, 1, 1, 0> xyyx;
		TVecSwizzledData<T, 2, 0, 1, 1, 1> xyyy;
		TVecSwizzledData<T, 2, 1, 0, 0, 0> yxxx;
		TVecSwizzledData<T, 2, 1, 0, 0, 1> yxxy;
		TVecSwizzledData<T, 2, 1, 0, 1, 0> yxyx;
		TVecSwizzledData<T, 2, 1, 0, 1, 1> yxyy;
		TVecSwizzledData<T, 2, 1, 1, 0, 0> yyxx;
		TVecSwizzledData<T, 2, 1, 1, 0, 1> yyxy;
		TVecSwizzledData<T, 2, 1, 1, 1, 0> yyyx;
		TVecSwizzledData<T, 2, 1, 1, 1, 1> yyyy;
	};

	constexpr TVecData(T x_, T y_)
		: x(x_)
		, y(y_)
	{
	}

	constexpr TVecData()
		: TVecData(T(0.0f), T(0.0f))
	{
	}
};

template<typename T>
class TVecData<T, 3>
{
public:
	using Simd = Array<T, 3>;

	union
	{
		struct
		{
			T x;
			T y;
			T z;
		};

		T m_carr[3];
		Array<T, 3> m_arr;
		Simd m_simd;

		TVec4SpecialData<T, 3, 0> xyz0;
		TVec4SpecialData<T, 3, 1> xyz1;

		TVecSwizzledData<T, 3, 0, 0> xx;
		TVecSwizzledData<T, 3, 0, 1> xy;
		TVecSwizzledData<T, 3, 0, 2> xz;
		TVecSwizzledData<T, 3, 1, 0> yx;
		TVecSwizzledData<T, 3, 1, 1> yy;
		TVecSwizzledData<T, 3, 1, 2> yz;
		TVecSwizzledData<T, 3, 2, 0> zx;
		TVecSwizzledData<T, 3, 2, 1> zy;
		TVecSwizzledData<T, 3, 2, 2> zz;
		TVecSwizzledData<T, 3, 0, 0, 0> xxx;
		TVecSwizzledData<T, 3, 0, 0, 1> xxy;
		TVecSwizzledData<T, 3, 0, 0, 2> xxz;
		TVecSwizzledData<T, 3, 0, 1, 0> xyx;
		TVecSwizzledData<T, 3, 0, 1, 1> xyy;
		TVecSwizzledData<T, 3, 0, 1, 2> xyz;
		TVecSwizzledData<T, 3, 0, 2, 0> xzx;
		TVecSwizzledData<T, 3, 0, 2, 1> xzy;
		TVecSwizzledData<T, 3, 0, 2, 2> xzz;
		TVecSwizzledData<T, 3, 1, 0, 0> yxx;
		TVecSwizzledData<T, 3, 1, 0, 1> yxy;
		TVecSwizzledData<T, 3, 1, 0, 2> yxz;
		TVecSwizzledData<T, 3, 1, 1, 0> yyx;
		TVecSwizzledData<T, 3, 1, 1, 1> yyy;
		TVecSwizzledData<T, 3, 1, 1, 2> yyz;
		TVecSwizzledData<T, 3, 1, 2, 0> yzx;
		TVecSwizzledData<T, 3, 1, 2, 1> yzy;
		TVecSwizzledData<T, 3, 1, 2, 2> yzz;
		TVecSwizzledData<T, 3, 2, 0, 0> zxx;
		TVecSwizzledData<T, 3, 2, 0, 1> zxy;
		TVecSwizzledData<T, 3, 2, 0, 2> zxz;
		TVecSwizzledData<T, 3, 2, 1, 0> zyx;
		TVecSwizzledData<T, 3, 2, 1, 1> zyy;
		TVecSwizzledData<T, 3, 2, 1, 2> zyz;
		TVecSwizzledData<T, 3, 2, 2, 0> zzx;
		TVecSwizzledData<T, 3, 2, 2, 1> zzy;
		TVecSwizzledData<T, 3, 2, 2, 2> zzz;
		TVecSwizzledData<T, 3, 0, 0, 0, 0> xxxx;
		TVecSwizzledData<T, 3, 0, 0, 0, 1> xxxy;
		TVecSwizzledData<T, 3, 0, 0, 0, 2> xxxz;
		TVecSwizzledData<T, 3, 0, 0, 1, 0> xxyx;
		TVecSwizzledData<T, 3, 0, 0, 1, 1> xxyy;
		TVecSwizzledData<T, 3, 0, 0, 1, 2> xxyz;
		TVecSwizzledData<T, 3, 0, 0, 2, 0> xxzx;
		TVecSwizzledData<T, 3, 0, 0, 2, 1> xxzy;
		TVecSwizzledData<T, 3, 0, 0, 2, 2> xxzz;
		TVecSwizzledData<T, 3, 0, 1, 0, 0> xyxx;
		TVecSwizzledData<T, 3, 0, 1, 0, 1> xyxy;
		TVecSwizzledData<T, 3, 0, 1, 0, 2> xyxz;
		TVecSwizzledData<T, 3, 0, 1, 1, 0> xyyx;
		TVecSwizzledData<T, 3, 0, 1, 1, 1> xyyy;
		TVecSwizzledData<T, 3, 0, 1, 1, 2> xyyz;
		TVecSwizzledData<T, 3, 0, 1, 2, 0> xyzx;
		TVecSwizzledData<T, 3, 0, 1, 2, 1> xyzy;
		TVecSwizzledData<T, 3, 0, 1, 2, 2> xyzz;
		TVecSwizzledData<T, 3, 0, 2, 0, 0> xzxx;
		TVecSwizzledData<T, 3, 0, 2, 0, 1> xzxy;
		TVecSwizzledData<T, 3, 0, 2, 0, 2> xzxz;
		TVecSwizzledData<T, 3, 0, 2, 1, 0> xzyx;
		TVecSwizzledData<T, 3, 0, 2, 1, 1> xzyy;
		TVecSwizzledData<T, 3, 0, 2, 1, 2> xzyz;
		TVecSwizzledData<T, 3, 0, 2, 2, 0> xzzx;
		TVecSwizzledData<T, 3, 0, 2, 2, 1> xzzy;
		TVecSwizzledData<T, 3, 0, 2, 2, 2> xzzz;
		TVecSwizzledData<T, 3, 1, 0, 0, 0> yxxx;
		TVecSwizzledData<T, 3, 1, 0, 0, 1> yxxy;
		TVecSwizzledData<T, 3, 1, 0, 0, 2> yxxz;
		TVecSwizzledData<T, 3, 1, 0, 1, 0> yxyx;
		TVecSwizzledData<T, 3, 1, 0, 1, 1> yxyy;
		TVecSwizzledData<T, 3, 1, 0, 1, 2> yxyz;
		TVecSwizzledData<T, 3, 1, 0, 2, 0> yxzx;
		TVecSwizzledData<T, 3, 1, 0, 2, 1> yxzy;
		TVecSwizzledData<T, 3, 1, 0, 2, 2> yxzz;
		TVecSwizzledData<T, 3, 1, 1, 0, 0> yyxx;
		TVecSwizzledData<T, 3, 1, 1, 0, 1> yyxy;
		TVecSwizzledData<T, 3, 1, 1, 0, 2> yyxz;
		TVecSwizzledData<T, 3, 1, 1, 1, 0> yyyx;
		TVecSwizzledData<T, 3, 1, 1, 1, 1> yyyy;
		TVecSwizzledData<T, 3, 1, 1, 1, 2> yyyz;
		TVecSwizzledData<T, 3, 1, 1, 2, 0> yyzx;
		TVecSwizzledData<T, 3, 1, 1, 2, 1> yyzy;
		TVecSwizzledData<T, 3, 1, 1, 2, 2> yyzz;
		TVecSwizzledData<T, 3, 1, 2, 0, 0> yzxx;
		TVecSwizzledData<T, 3, 1, 2, 0, 1> yzxy;
		TVecSwizzledData<T, 3, 1, 2, 0, 2> yzxz;
		TVecSwizzledData<T, 3, 1, 2, 1, 0> yzyx;
		TVecSwizzledData<T, 3, 1, 2, 1, 1> yzyy;
		TVecSwizzledData<T, 3, 1, 2, 1, 2> yzyz;
		TVecSwizzledData<T, 3, 1, 2, 2, 0> yzzx;
		TVecSwizzledData<T, 3, 1, 2, 2, 1> yzzy;
		TVecSwizzledData<T, 3, 1, 2, 2, 2> yzzz;
		TVecSwizzledData<T, 3, 2, 0, 0, 0> zxxx;
		TVecSwizzledData<T, 3, 2, 0, 0, 1> zxxy;
		TVecSwizzledData<T, 3, 2, 0, 0, 2> zxxz;
		TVecSwizzledData<T, 3, 2, 0, 1, 0> zxyx;
		TVecSwizzledData<T, 3, 2, 0, 1, 1> zxyy;
		TVecSwizzledData<T, 3, 2, 0, 1, 2> zxyz;
		TVecSwizzledData<T, 3, 2, 0, 2, 0> zxzx;
		TVecSwizzledData<T, 3, 2, 0, 2, 1> zxzy;
		TVecSwizzledData<T, 3, 2, 0, 2, 2> zxzz;
		TVecSwizzledData<T, 3, 2, 1, 0, 0> zyxx;
		TVecSwizzledData<T, 3, 2, 1, 0, 1> zyxy;
		TVecSwizzledData<T, 3, 2, 1, 0, 2> zyxz;
		TVecSwizzledData<T, 3, 2, 1, 1, 0> zyyx;
		TVecSwizzledData<T, 3, 2, 1, 1, 1> zyyy;
		TVecSwizzledData<T, 3, 2, 1, 1, 2> zyyz;
		TVecSwizzledData<T, 3, 2, 1, 2, 0> zyzx;
		TVecSwizzledData<T, 3, 2, 1, 2, 1> zyzy;
		TVecSwizzledData<T, 3, 2, 1, 2, 2> zyzz;
		TVecSwizzledData<T, 3, 2, 2, 0, 0> zzxx;
		TVecSwizzledData<T, 3, 2, 2, 0, 1> zzxy;
		TVecSwizzledData<T, 3, 2, 2, 0, 2> zzxz;
		TVecSwizzledData<T, 3, 2, 2, 1, 0> zzyx;
		TVecSwizzledData<T, 3, 2, 2, 1, 1> zzyy;
		TVecSwizzledData<T, 3, 2, 2, 1, 2> zzyz;
		TVecSwizzledData<T, 3, 2, 2, 2, 0> zzzx;
		TVecSwizzledData<T, 3, 2, 2, 2, 1> zzzy;
		TVecSwizzledData<T, 3, 2, 2, 2, 2> zzzz;
	};

	constexpr TVecData(T x_, T y_, T z_)
		: x(x_)
		, y(y_)
		, z(z_)
	{
	}

	constexpr TVecData()
		: TVecData(T(0.0f), T(0.0f), T(0.0f))
	{
	}
};

template<typename T>
class TVecData<T, 4>
{
public:
	using Simd = MathSimd<T, 4>::Type;

	union
	{
		struct
		{
			T x;
			T y;
			T z;
			T w;
		};

		T m_carr[4];
		Array<T, 4> m_arr;
		Simd m_simd;

		TVec4SpecialData<T, 4, 0> xyz0;
		TVec4SpecialData<T, 4, 1> xyz1;

		TVecSwizzledData<T, 4, 0, 0> xx;
		TVecSwizzledData<T, 4, 0, 1> xy;
		TVecSwizzledData<T, 4, 0, 2> xz;
		TVecSwizzledData<T, 4, 0, 3> xw;
		TVecSwizzledData<T, 4, 1, 0> yx;
		TVecSwizzledData<T, 4, 1, 1> yy;
		TVecSwizzledData<T, 4, 1, 2> yz;
		TVecSwizzledData<T, 4, 1, 3> yw;
		TVecSwizzledData<T, 4, 2, 0> zx;
		TVecSwizzledData<T, 4, 2, 1> zy;
		TVecSwizzledData<T, 4, 2, 2> zz;
		TVecSwizzledData<T, 4, 2, 3> zw;
		TVecSwizzledData<T, 4, 3, 0> wx;
		TVecSwizzledData<T, 4, 3, 1> wy;
		TVecSwizzledData<T, 4, 3, 2> wz;
		TVecSwizzledData<T, 4, 3, 3> ww;
		TVecSwizzledData<T, 4, 0, 0, 0> xxx;
		TVecSwizzledData<T, 4, 0, 0, 1> xxy;
		TVecSwizzledData<T, 4, 0, 0, 2> xxz;
		TVecSwizzledData<T, 4, 0, 0, 3> xxw;
		TVecSwizzledData<T, 4, 0, 1, 0> xyx;
		TVecSwizzledData<T, 4, 0, 1, 1> xyy;
		TVecSwizzledData<T, 4, 0, 1, 2> xyz;
		TVecSwizzledData<T, 4, 0, 1, 3> xyw;
		TVecSwizzledData<T, 4, 0, 2, 0> xzx;
		TVecSwizzledData<T, 4, 0, 2, 1> xzy;
		TVecSwizzledData<T, 4, 0, 2, 2> xzz;
		TVecSwizzledData<T, 4, 0, 2, 3> xzw;
		TVecSwizzledData<T, 4, 0, 3, 0> xwx;
		TVecSwizzledData<T, 4, 0, 3, 1> xwy;
		TVecSwizzledData<T, 4, 0, 3, 2> xwz;
		TVecSwizzledData<T, 4, 0, 3, 3> xww;
		TVecSwizzledData<T, 4, 1, 0, 0> yxx;
		TVecSwizzledData<T, 4, 1, 0, 1> yxy;
		TVecSwizzledData<T, 4, 1, 0, 2> yxz;
		TVecSwizzledData<T, 4, 1, 0, 3> yxw;
		TVecSwizzledData<T, 4, 1, 1, 0> yyx;
		TVecSwizzledData<T, 4, 1, 1, 1> yyy;
		TVecSwizzledData<T, 4, 1, 1, 2> yyz;
		TVecSwizzledData<T, 4, 1, 1, 3> yyw;
		TVecSwizzledData<T, 4, 1, 2, 0> yzx;
		TVecSwizzledData<T, 4, 1, 2, 1> yzy;
		TVecSwizzledData<T, 4, 1, 2, 2> yzz;
		TVecSwizzledData<T, 4, 1, 2, 3> yzw;
		TVecSwizzledData<T, 4, 1, 3, 0> ywx;
		TVecSwizzledData<T, 4, 1, 3, 1> ywy;
		TVecSwizzledData<T, 4, 1, 3, 2> ywz;
		TVecSwizzledData<T, 4, 1, 3, 3> yww;
		TVecSwizzledData<T, 4, 2, 0, 0> zxx;
		TVecSwizzledData<T, 4, 2, 0, 1> zxy;
		TVecSwizzledData<T, 4, 2, 0, 2> zxz;
		TVecSwizzledData<T, 4, 2, 0, 3> zxw;
		TVecSwizzledData<T, 4, 2, 1, 0> zyx;
		TVecSwizzledData<T, 4, 2, 1, 1> zyy;
		TVecSwizzledData<T, 4, 2, 1, 2> zyz;
		TVecSwizzledData<T, 4, 2, 1, 3> zyw;
		TVecSwizzledData<T, 4, 2, 2, 0> zzx;
		TVecSwizzledData<T, 4, 2, 2, 1> zzy;
		TVecSwizzledData<T, 4, 2, 2, 2> zzz;
		TVecSwizzledData<T, 4, 2, 2, 3> zzw;
		TVecSwizzledData<T, 4, 2, 3, 0> zwx;
		TVecSwizzledData<T, 4, 2, 3, 1> zwy;
		TVecSwizzledData<T, 4, 2, 3, 2> zwz;
		TVecSwizzledData<T, 4, 2, 3, 3> zww;
		TVecSwizzledData<T, 4, 3, 0, 0> wxx;
		TVecSwizzledData<T, 4, 3, 0, 1> wxy;
		TVecSwizzledData<T, 4, 3, 0, 2> wxz;
		TVecSwizzledData<T, 4, 3, 0, 3> wxw;
		TVecSwizzledData<T, 4, 3, 1, 0> wyx;
		TVecSwizzledData<T, 4, 3, 1, 1> wyy;
		TVecSwizzledData<T, 4, 3, 1, 2> wyz;
		TVecSwizzledData<T, 4, 3, 1, 3> wyw;
		TVecSwizzledData<T, 4, 3, 2, 0> wzx;
		TVecSwizzledData<T, 4, 3, 2, 1> wzy;
		TVecSwizzledData<T, 4, 3, 2, 2> wzz;
		TVecSwizzledData<T, 4, 3, 2, 3> wzw;
		TVecSwizzledData<T, 4, 3, 3, 0> wwx;
		TVecSwizzledData<T, 4, 3, 3, 1> wwy;
		TVecSwizzledData<T, 4, 3, 3, 2> wwz;
		TVecSwizzledData<T, 4, 3, 3, 3> www;
		TVecSwizzledData<T, 4, 0, 0, 0, 0> xxxx;
		TVecSwizzledData<T, 4, 0, 0, 0, 1> xxxy;
		TVecSwizzledData<T, 4, 0, 0, 0, 2> xxxz;
		TVecSwizzledData<T, 4, 0, 0, 0, 3> xxxw;
		TVecSwizzledData<T, 4, 0, 0, 1, 0> xxyx;
		TVecSwizzledData<T, 4, 0, 0, 1, 1> xxyy;
		TVecSwizzledData<T, 4, 0, 0, 1, 2> xxyz;
		TVecSwizzledData<T, 4, 0, 0, 1, 3> xxyw;
		TVecSwizzledData<T, 4, 0, 0, 2, 0> xxzx;
		TVecSwizzledData<T, 4, 0, 0, 2, 1> xxzy;
		TVecSwizzledData<T, 4, 0, 0, 2, 2> xxzz;
		TVecSwizzledData<T, 4, 0, 0, 2, 3> xxzw;
		TVecSwizzledData<T, 4, 0, 0, 3, 0> xxwx;
		TVecSwizzledData<T, 4, 0, 0, 3, 1> xxwy;
		TVecSwizzledData<T, 4, 0, 0, 3, 2> xxwz;
		TVecSwizzledData<T, 4, 0, 0, 3, 3> xxww;
		TVecSwizzledData<T, 4, 0, 1, 0, 0> xyxx;
		TVecSwizzledData<T, 4, 0, 1, 0, 1> xyxy;
		TVecSwizzledData<T, 4, 0, 1, 0, 2> xyxz;
		TVecSwizzledData<T, 4, 0, 1, 0, 3> xyxw;
		TVecSwizzledData<T, 4, 0, 1, 1, 0> xyyx;
		TVecSwizzledData<T, 4, 0, 1, 1, 1> xyyy;
		TVecSwizzledData<T, 4, 0, 1, 1, 2> xyyz;
		TVecSwizzledData<T, 4, 0, 1, 1, 3> xyyw;
		TVecSwizzledData<T, 4, 0, 1, 2, 0> xyzx;
		TVecSwizzledData<T, 4, 0, 1, 2, 1> xyzy;
		TVecSwizzledData<T, 4, 0, 1, 2, 2> xyzz;
		TVecSwizzledData<T, 4, 0, 1, 2, 3> xyzw;
		TVecSwizzledData<T, 4, 0, 1, 3, 0> xywx;
		TVecSwizzledData<T, 4, 0, 1, 3, 1> xywy;
		TVecSwizzledData<T, 4, 0, 1, 3, 2> xywz;
		TVecSwizzledData<T, 4, 0, 1, 3, 3> xyww;
		TVecSwizzledData<T, 4, 0, 2, 0, 0> xzxx;
		TVecSwizzledData<T, 4, 0, 2, 0, 1> xzxy;
		TVecSwizzledData<T, 4, 0, 2, 0, 2> xzxz;
		TVecSwizzledData<T, 4, 0, 2, 0, 3> xzxw;
		TVecSwizzledData<T, 4, 0, 2, 1, 0> xzyx;
		TVecSwizzledData<T, 4, 0, 2, 1, 1> xzyy;
		TVecSwizzledData<T, 4, 0, 2, 1, 2> xzyz;
		TVecSwizzledData<T, 4, 0, 2, 1, 3> xzyw;
		TVecSwizzledData<T, 4, 0, 2, 2, 0> xzzx;
		TVecSwizzledData<T, 4, 0, 2, 2, 1> xzzy;
		TVecSwizzledData<T, 4, 0, 2, 2, 2> xzzz;
		TVecSwizzledData<T, 4, 0, 2, 2, 3> xzzw;
		TVecSwizzledData<T, 4, 0, 2, 3, 0> xzwx;
		TVecSwizzledData<T, 4, 0, 2, 3, 1> xzwy;
		TVecSwizzledData<T, 4, 0, 2, 3, 2> xzwz;
		TVecSwizzledData<T, 4, 0, 2, 3, 3> xzww;
		TVecSwizzledData<T, 4, 0, 3, 0, 0> xwxx;
		TVecSwizzledData<T, 4, 0, 3, 0, 1> xwxy;
		TVecSwizzledData<T, 4, 0, 3, 0, 2> xwxz;
		TVecSwizzledData<T, 4, 0, 3, 0, 3> xwxw;
		TVecSwizzledData<T, 4, 0, 3, 1, 0> xwyx;
		TVecSwizzledData<T, 4, 0, 3, 1, 1> xwyy;
		TVecSwizzledData<T, 4, 0, 3, 1, 2> xwyz;
		TVecSwizzledData<T, 4, 0, 3, 1, 3> xwyw;
		TVecSwizzledData<T, 4, 0, 3, 2, 0> xwzx;
		TVecSwizzledData<T, 4, 0, 3, 2, 1> xwzy;
		TVecSwizzledData<T, 4, 0, 3, 2, 2> xwzz;
		TVecSwizzledData<T, 4, 0, 3, 2, 3> xwzw;
		TVecSwizzledData<T, 4, 0, 3, 3, 0> xwwx;
		TVecSwizzledData<T, 4, 0, 3, 3, 1> xwwy;
		TVecSwizzledData<T, 4, 0, 3, 3, 2> xwwz;
		TVecSwizzledData<T, 4, 0, 3, 3, 3> xwww;
		TVecSwizzledData<T, 4, 1, 0, 0, 0> yxxx;
		TVecSwizzledData<T, 4, 1, 0, 0, 1> yxxy;
		TVecSwizzledData<T, 4, 1, 0, 0, 2> yxxz;
		TVecSwizzledData<T, 4, 1, 0, 0, 3> yxxw;
		TVecSwizzledData<T, 4, 1, 0, 1, 0> yxyx;
		TVecSwizzledData<T, 4, 1, 0, 1, 1> yxyy;
		TVecSwizzledData<T, 4, 1, 0, 1, 2> yxyz;
		TVecSwizzledData<T, 4, 1, 0, 1, 3> yxyw;
		TVecSwizzledData<T, 4, 1, 0, 2, 0> yxzx;
		TVecSwizzledData<T, 4, 1, 0, 2, 1> yxzy;
		TVecSwizzledData<T, 4, 1, 0, 2, 2> yxzz;
		TVecSwizzledData<T, 4, 1, 0, 2, 3> yxzw;
		TVecSwizzledData<T, 4, 1, 0, 3, 0> yxwx;
		TVecSwizzledData<T, 4, 1, 0, 3, 1> yxwy;
		TVecSwizzledData<T, 4, 1, 0, 3, 2> yxwz;
		TVecSwizzledData<T, 4, 1, 0, 3, 3> yxww;
		TVecSwizzledData<T, 4, 1, 1, 0, 0> yyxx;
		TVecSwizzledData<T, 4, 1, 1, 0, 1> yyxy;
		TVecSwizzledData<T, 4, 1, 1, 0, 2> yyxz;
		TVecSwizzledData<T, 4, 1, 1, 0, 3> yyxw;
		TVecSwizzledData<T, 4, 1, 1, 1, 0> yyyx;
		TVecSwizzledData<T, 4, 1, 1, 1, 1> yyyy;
		TVecSwizzledData<T, 4, 1, 1, 1, 2> yyyz;
		TVecSwizzledData<T, 4, 1, 1, 1, 3> yyyw;
		TVecSwizzledData<T, 4, 1, 1, 2, 0> yyzx;
		TVecSwizzledData<T, 4, 1, 1, 2, 1> yyzy;
		TVecSwizzledData<T, 4, 1, 1, 2, 2> yyzz;
		TVecSwizzledData<T, 4, 1, 1, 2, 3> yyzw;
		TVecSwizzledData<T, 4, 1, 1, 3, 0> yywx;
		TVecSwizzledData<T, 4, 1, 1, 3, 1> yywy;
		TVecSwizzledData<T, 4, 1, 1, 3, 2> yywz;
		TVecSwizzledData<T, 4, 1, 1, 3, 3> yyww;
		TVecSwizzledData<T, 4, 1, 2, 0, 0> yzxx;
		TVecSwizzledData<T, 4, 1, 2, 0, 1> yzxy;
		TVecSwizzledData<T, 4, 1, 2, 0, 2> yzxz;
		TVecSwizzledData<T, 4, 1, 2, 0, 3> yzxw;
		TVecSwizzledData<T, 4, 1, 2, 1, 0> yzyx;
		TVecSwizzledData<T, 4, 1, 2, 1, 1> yzyy;
		TVecSwizzledData<T, 4, 1, 2, 1, 2> yzyz;
		TVecSwizzledData<T, 4, 1, 2, 1, 3> yzyw;
		TVecSwizzledData<T, 4, 1, 2, 2, 0> yzzx;
		TVecSwizzledData<T, 4, 1, 2, 2, 1> yzzy;
		TVecSwizzledData<T, 4, 1, 2, 2, 2> yzzz;
		TVecSwizzledData<T, 4, 1, 2, 2, 3> yzzw;
		TVecSwizzledData<T, 4, 1, 2, 3, 0> yzwx;
		TVecSwizzledData<T, 4, 1, 2, 3, 1> yzwy;
		TVecSwizzledData<T, 4, 1, 2, 3, 2> yzwz;
		TVecSwizzledData<T, 4, 1, 2, 3, 3> yzww;
		TVecSwizzledData<T, 4, 1, 3, 0, 0> ywxx;
		TVecSwizzledData<T, 4, 1, 3, 0, 1> ywxy;
		TVecSwizzledData<T, 4, 1, 3, 0, 2> ywxz;
		TVecSwizzledData<T, 4, 1, 3, 0, 3> ywxw;
		TVecSwizzledData<T, 4, 1, 3, 1, 0> ywyx;
		TVecSwizzledData<T, 4, 1, 3, 1, 1> ywyy;
		TVecSwizzledData<T, 4, 1, 3, 1, 2> ywyz;
		TVecSwizzledData<T, 4, 1, 3, 1, 3> ywyw;
		TVecSwizzledData<T, 4, 1, 3, 2, 0> ywzx;
		TVecSwizzledData<T, 4, 1, 3, 2, 1> ywzy;
		TVecSwizzledData<T, 4, 1, 3, 2, 2> ywzz;
		TVecSwizzledData<T, 4, 1, 3, 2, 3> ywzw;
		TVecSwizzledData<T, 4, 1, 3, 3, 0> ywwx;
		TVecSwizzledData<T, 4, 1, 3, 3, 1> ywwy;
		TVecSwizzledData<T, 4, 1, 3, 3, 2> ywwz;
		TVecSwizzledData<T, 4, 1, 3, 3, 3> ywww;
		TVecSwizzledData<T, 4, 2, 0, 0, 0> zxxx;
		TVecSwizzledData<T, 4, 2, 0, 0, 1> zxxy;
		TVecSwizzledData<T, 4, 2, 0, 0, 2> zxxz;
		TVecSwizzledData<T, 4, 2, 0, 0, 3> zxxw;
		TVecSwizzledData<T, 4, 2, 0, 1, 0> zxyx;
		TVecSwizzledData<T, 4, 2, 0, 1, 1> zxyy;
		TVecSwizzledData<T, 4, 2, 0, 1, 2> zxyz;
		TVecSwizzledData<T, 4, 2, 0, 1, 3> zxyw;
		TVecSwizzledData<T, 4, 2, 0, 2, 0> zxzx;
		TVecSwizzledData<T, 4, 2, 0, 2, 1> zxzy;
		TVecSwizzledData<T, 4, 2, 0, 2, 2> zxzz;
		TVecSwizzledData<T, 4, 2, 0, 2, 3> zxzw;
		TVecSwizzledData<T, 4, 2, 0, 3, 0> zxwx;
		TVecSwizzledData<T, 4, 2, 0, 3, 1> zxwy;
		TVecSwizzledData<T, 4, 2, 0, 3, 2> zxwz;
		TVecSwizzledData<T, 4, 2, 0, 3, 3> zxww;
		TVecSwizzledData<T, 4, 2, 1, 0, 0> zyxx;
		TVecSwizzledData<T, 4, 2, 1, 0, 1> zyxy;
		TVecSwizzledData<T, 4, 2, 1, 0, 2> zyxz;
		TVecSwizzledData<T, 4, 2, 1, 0, 3> zyxw;
		TVecSwizzledData<T, 4, 2, 1, 1, 0> zyyx;
		TVecSwizzledData<T, 4, 2, 1, 1, 1> zyyy;
		TVecSwizzledData<T, 4, 2, 1, 1, 2> zyyz;
		TVecSwizzledData<T, 4, 2, 1, 1, 3> zyyw;
		TVecSwizzledData<T, 4, 2, 1, 2, 0> zyzx;
		TVecSwizzledData<T, 4, 2, 1, 2, 1> zyzy;
		TVecSwizzledData<T, 4, 2, 1, 2, 2> zyzz;
		TVecSwizzledData<T, 4, 2, 1, 2, 3> zyzw;
		TVecSwizzledData<T, 4, 2, 1, 3, 0> zywx;
		TVecSwizzledData<T, 4, 2, 1, 3, 1> zywy;
		TVecSwizzledData<T, 4, 2, 1, 3, 2> zywz;
		TVecSwizzledData<T, 4, 2, 1, 3, 3> zyww;
		TVecSwizzledData<T, 4, 2, 2, 0, 0> zzxx;
		TVecSwizzledData<T, 4, 2, 2, 0, 1> zzxy;
		TVecSwizzledData<T, 4, 2, 2, 0, 2> zzxz;
		TVecSwizzledData<T, 4, 2, 2, 0, 3> zzxw;
		TVecSwizzledData<T, 4, 2, 2, 1, 0> zzyx;
		TVecSwizzledData<T, 4, 2, 2, 1, 1> zzyy;
		TVecSwizzledData<T, 4, 2, 2, 1, 2> zzyz;
		TVecSwizzledData<T, 4, 2, 2, 1, 3> zzyw;
		TVecSwizzledData<T, 4, 2, 2, 2, 0> zzzx;
		TVecSwizzledData<T, 4, 2, 2, 2, 1> zzzy;
		TVecSwizzledData<T, 4, 2, 2, 2, 2> zzzz;
		TVecSwizzledData<T, 4, 2, 2, 2, 3> zzzw;
		TVecSwizzledData<T, 4, 2, 2, 3, 0> zzwx;
		TVecSwizzledData<T, 4, 2, 2, 3, 1> zzwy;
		TVecSwizzledData<T, 4, 2, 2, 3, 2> zzwz;
		TVecSwizzledData<T, 4, 2, 2, 3, 3> zzww;
		TVecSwizzledData<T, 4, 2, 3, 0, 0> zwxx;
		TVecSwizzledData<T, 4, 2, 3, 0, 1> zwxy;
		TVecSwizzledData<T, 4, 2, 3, 0, 2> zwxz;
		TVecSwizzledData<T, 4, 2, 3, 0, 3> zwxw;
		TVecSwizzledData<T, 4, 2, 3, 1, 0> zwyx;
		TVecSwizzledData<T, 4, 2, 3, 1, 1> zwyy;
		TVecSwizzledData<T, 4, 2, 3, 1, 2> zwyz;
		TVecSwizzledData<T, 4, 2, 3, 1, 3> zwyw;
		TVecSwizzledData<T, 4, 2, 3, 2, 0> zwzx;
		TVecSwizzledData<T, 4, 2, 3, 2, 1> zwzy;
		TVecSwizzledData<T, 4, 2, 3, 2, 2> zwzz;
		TVecSwizzledData<T, 4, 2, 3, 2, 3> zwzw;
		TVecSwizzledData<T, 4, 2, 3, 3, 0> zwwx;
		TVecSwizzledData<T, 4, 2, 3, 3, 1> zwwy;
		TVecSwizzledData<T, 4, 2, 3, 3, 2> zwwz;
		TVecSwizzledData<T, 4, 2, 3, 3, 3> zwww;
		TVecSwizzledData<T, 4, 3, 0, 0, 0> wxxx;
		TVecSwizzledData<T, 4, 3, 0, 0, 1> wxxy;
		TVecSwizzledData<T, 4, 3, 0, 0, 2> wxxz;
		TVecSwizzledData<T, 4, 3, 0, 0, 3> wxxw;
		TVecSwizzledData<T, 4, 3, 0, 1, 0> wxyx;
		TVecSwizzledData<T, 4, 3, 0, 1, 1> wxyy;
		TVecSwizzledData<T, 4, 3, 0, 1, 2> wxyz;
		TVecSwizzledData<T, 4, 3, 0, 1, 3> wxyw;
		TVecSwizzledData<T, 4, 3, 0, 2, 0> wxzx;
		TVecSwizzledData<T, 4, 3, 0, 2, 1> wxzy;
		TVecSwizzledData<T, 4, 3, 0, 2, 2> wxzz;
		TVecSwizzledData<T, 4, 3, 0, 2, 3> wxzw;
		TVecSwizzledData<T, 4, 3, 0, 3, 0> wxwx;
		TVecSwizzledData<T, 4, 3, 0, 3, 1> wxwy;
		TVecSwizzledData<T, 4, 3, 0, 3, 2> wxwz;
		TVecSwizzledData<T, 4, 3, 0, 3, 3> wxww;
		TVecSwizzledData<T, 4, 3, 1, 0, 0> wyxx;
		TVecSwizzledData<T, 4, 3, 1, 0, 1> wyxy;
		TVecSwizzledData<T, 4, 3, 1, 0, 2> wyxz;
		TVecSwizzledData<T, 4, 3, 1, 0, 3> wyxw;
		TVecSwizzledData<T, 4, 3, 1, 1, 0> wyyx;
		TVecSwizzledData<T, 4, 3, 1, 1, 1> wyyy;
		TVecSwizzledData<T, 4, 3, 1, 1, 2> wyyz;
		TVecSwizzledData<T, 4, 3, 1, 1, 3> wyyw;
		TVecSwizzledData<T, 4, 3, 1, 2, 0> wyzx;
		TVecSwizzledData<T, 4, 3, 1, 2, 1> wyzy;
		TVecSwizzledData<T, 4, 3, 1, 2, 2> wyzz;
		TVecSwizzledData<T, 4, 3, 1, 2, 3> wyzw;
		TVecSwizzledData<T, 4, 3, 1, 3, 0> wywx;
		TVecSwizzledData<T, 4, 3, 1, 3, 1> wywy;
		TVecSwizzledData<T, 4, 3, 1, 3, 2> wywz;
		TVecSwizzledData<T, 4, 3, 1, 3, 3> wyww;
		TVecSwizzledData<T, 4, 3, 2, 0, 0> wzxx;
		TVecSwizzledData<T, 4, 3, 2, 0, 1> wzxy;
		TVecSwizzledData<T, 4, 3, 2, 0, 2> wzxz;
		TVecSwizzledData<T, 4, 3, 2, 0, 3> wzxw;
		TVecSwizzledData<T, 4, 3, 2, 1, 0> wzyx;
		TVecSwizzledData<T, 4, 3, 2, 1, 1> wzyy;
		TVecSwizzledData<T, 4, 3, 2, 1, 2> wzyz;
		TVecSwizzledData<T, 4, 3, 2, 1, 3> wzyw;
		TVecSwizzledData<T, 4, 3, 2, 2, 0> wzzx;
		TVecSwizzledData<T, 4, 3, 2, 2, 1> wzzy;
		TVecSwizzledData<T, 4, 3, 2, 2, 2> wzzz;
		TVecSwizzledData<T, 4, 3, 2, 2, 3> wzzw;
		TVecSwizzledData<T, 4, 3, 2, 3, 0> wzwx;
		TVecSwizzledData<T, 4, 3, 2, 3, 1> wzwy;
		TVecSwizzledData<T, 4, 3, 2, 3, 2> wzwz;
		TVecSwizzledData<T, 4, 3, 2, 3, 3> wzww;
		TVecSwizzledData<T, 4, 3, 3, 0, 0> wwxx;
		TVecSwizzledData<T, 4, 3, 3, 0, 1> wwxy;
		TVecSwizzledData<T, 4, 3, 3, 0, 2> wwxz;
		TVecSwizzledData<T, 4, 3, 3, 0, 3> wwxw;
		TVecSwizzledData<T, 4, 3, 3, 1, 0> wwyx;
		TVecSwizzledData<T, 4, 3, 3, 1, 1> wwyy;
		TVecSwizzledData<T, 4, 3, 3, 1, 2> wwyz;
		TVecSwizzledData<T, 4, 3, 3, 1, 3> wwyw;
		TVecSwizzledData<T, 4, 3, 3, 2, 0> wwzx;
		TVecSwizzledData<T, 4, 3, 3, 2, 1> wwzy;
		TVecSwizzledData<T, 4, 3, 3, 2, 2> wwzz;
		TVecSwizzledData<T, 4, 3, 3, 2, 3> wwzw;
		TVecSwizzledData<T, 4, 3, 3, 3, 0> wwwx;
		TVecSwizzledData<T, 4, 3, 3, 3, 1> wwwy;
		TVecSwizzledData<T, 4, 3, 3, 3, 2> wwwz;
		TVecSwizzledData<T, 4, 3, 3, 3, 3> wwww;
	};

	constexpr TVecData(T x_, T y_, T z_, T w_)
		: x(x_)
		, y(y_)
		, z(z_)
		, w(w_)
	{
	}

	constexpr TVecData()
		: TVecData(T(0.0f), T(0.0f), T(0.0f), T(0.0f))
	{
	}
};

#if ANKI_COMPILER_MSVC
#	pragma warning(pop)
#elif ANKI_COMPILER_GCC_COMPATIBLE
#	pragma GCC diagnostic pop
#endif

template<typename T, U32 kTComponentCount>
class TVec : public TVecData<T, kTComponentCount>
{
public:
	using Scalar = T;

	static constexpr Bool kVec4Simd = kTComponentCount == 4 && std::is_same<T, F32>::value && ANKI_ENABLE_SIMD;
	static constexpr Bool kIsInteger = std::is_integral<T>::value;
	static constexpr U32 kComponentCount = kTComponentCount;

	using Base = TVecData<T, kTComponentCount>;
	using Base::m_arr;
	using Base::m_simd;

	// Constructors

	constexpr TVec()
		: Base()
	{
	}

	explicit TVec(T f)
	{
		if constexpr(kVec4Simd)
		{
#if ANKI_SIMD_SSE
			m_simd = _mm_set1_ps(f);
#else
			m_simd = vdupq_n_f32(f);
#endif
		}
		else
		{
			for(U32 i = 0; i < kTComponentCount; ++i)
			{
				m_arr[i] = f;
			}
		}
	}

	// Copy
	TVec(const TVec& b)
		: Base()
	{
		if constexpr(kVec4Simd)
		{
			m_simd = b.m_simd;
		}
		else
		{
			for(U32 i = 0; i < kTComponentCount; i++)
			{
				m_arr[i] = b.m_arr[i];
			}
		}
	}

	// Convert from another type. From int to float vectors and the opposite.
	template<typename Y>
	explicit TVec(const TVec<Y, kTComponentCount>& b) requires(!std::is_same<Y, T>::value)
	{
		for(U32 i = 0; i < kTComponentCount; i++)
		{
			m_arr[i] = T(b[i]);
		}
	}

	// Convert from the swizzle of another type. From int to float vectors and the opposite.
	template<typename Y, U32 kOtherComponentCount, U32... kIndices>
	explicit TVec(const TVecSwizzledData<Y, kOtherComponentCount, kIndices...>& b) requires(!std::is_same<Y, T>::value)
	{
		const TVec<Y, kTComponentCount> bb = b;
		for(U32 i = 0; i < kTComponentCount; i++)
		{
			m_arr[i] = T(bb[i]);
		}
	}

	// Convert from the swizzle of another type. From int to float vectors and the opposite.
	template<typename Y, U32 kSpecialConst>
	explicit TVec(const TVec4SpecialData<Y, 3, kSpecialConst>& b) requires(!std::is_same<Y, T>::value && kTComponentCount == 4)
	{
		const TVec<Y, kTComponentCount> bb = b;
		for(U32 i = 0; i < kTComponentCount; i++)
		{
			m_arr[i] = T(bb[i]);
		}
	}

	explicit TVec(const T arr[])
	{
		if constexpr(kVec4Simd)
		{
#if ANKI_SIMD_SSE
			m_simd = _mm_loadu_ps(arr); // _mm_loadu_ps doesn't require arr to be 16 byte aligned unlike _mm_load_ps
#else
			m_simd = vld1q_f32(arr);
#endif
		}
		else
		{
			for(U32 i = 0; i < kTComponentCount; ++i)
			{
				m_arr[i] = arr[i];
			}
		}
	}

	explicit TVec(const Array<T, kTComponentCount>& arr)
		: TVec(arr.getBegin())
	{
	}

	// Vec2 specific

	constexpr TVec(T x, T y) requires(kTComponentCount == 2)
		: Base(x, y)
	{
	}

	// Vec2 specific

	constexpr TVec(T x, T y, T z) requires(kTComponentCount == 3)
		: Base(x, y, z)
	{
	}

	constexpr TVec(TVec<T, 2> a, T z) requires(kTComponentCount == 3)
		: Base(a.m_arr[0], a.m_arr[1], z)
	{
	}

	constexpr TVec(T x, TVec<T, 2> a) requires(kTComponentCount == 3)
		: Base(x, a.m_arr[0], a.m_arr[1])
	{
	}

	// Vec4 specific

	explicit TVec(T x, T y, T z, T w) requires(kTComponentCount == 4)
	{
		if constexpr(kVec4Simd)
		{
#if ANKI_SIMD_SSE
			m_simd = _mm_set_ps(w, z, y, x);
#else
			alignas(16) T data[4] = {x, y, z, w};
			m_simd = vld1q_f32(data);
#endif
		}
		else
		{
			this->m_arr = {x, y, z, w};
		}
	}

	constexpr TVec(TVec<T, 3> a, T w) requires(kTComponentCount == 4)
		: Base(a.m_arr[0], a.m_arr[1], a.m_arr[2], w)
	{
	}

	constexpr TVec(T x, TVec<T, 3> a) requires(kTComponentCount == 4)
		: Base(x, a.m_arr[0], a.m_arr[1], a.m_arr[2])
	{
	}

	constexpr TVec(TVec<T, 2> a, T z, T w) requires(kTComponentCount == 4)
		: Base(a.m_arr[0], a.m_arr[1], z, w)
	{
	}

	constexpr TVec(T x, TVec<T, 2> a, T w) requires(kTComponentCount == 4)
		: Base(x, a.m_arr[0], a.m_arr[1], w)
	{
	}

	constexpr TVec(T x, T y, TVec<T, 2> a) requires(kTComponentCount == 4)
		: Base(x, y, a.m_arr[0], a.m_arr[1])
	{
	}

	constexpr TVec(TVec<T, 2> a, TVec<T, 2> b) requires(kTComponentCount == 4)
		: Base(a.m_arr[0], a.m_arr[1], b.m_arr[0], b.m_arr[1])
	{
	}

	explicit TVec(Base::Simd simd) requires(kTComponentCount == 4)
	{
		m_simd = simd;
	}

	// Accessors

	T& operator[](U32 i)
	{
		return m_arr[i];
	}

	T operator[](U32 i) const
	{
		return m_arr[i];
	}

	// Operators with the same type

	// Copy
	TVec& operator=(const TVec& b)
	{
		if constexpr(kVec4Simd)
		{
			m_simd = b.m_simd;
		}
		else
		{
			for(U32 i = 0; i < kTComponentCount; i++)
			{
				m_arr[i] = b.m_carr[i];
			}
		}
		return *this;
	}

	[[nodiscard]] TVec operator+(TVec b) const
	{
		if constexpr(kVec4Simd)
		{
#if ANKI_SIMD_SSE
			return TVec(_mm_add_ps(m_simd, b.m_simd));
#else
			return TVec(vaddq_f32(m_simd, b.m_simd));
#endif
		}
		else
		{
			TVec out;
			for(U32 i = 0; i < kTComponentCount; i++)
			{
				out.m_arr[i] = m_arr[i] + b.m_arr[i];
			}

			return out;
		}
	}

	TVec& operator+=(TVec b)
	{
		if constexpr(kVec4Simd)
		{
#if ANKI_SIMD_SSE
			m_simd = _mm_add_ps(m_simd, b.m_simd);
#else
			m_simd = vaddq_f32(m_simd, b.m_simd);
#endif
		}
		else
		{
			for(U32 i = 0; i < kTComponentCount; i++)
			{
				m_arr[i] += b.m_arr[i];
			}
		}
		return *this;
	}

	[[nodiscard]] TVec operator-(TVec b) const
	{
		if constexpr(kVec4Simd)
		{
#if ANKI_SIMD_SSE
			return TVec(_mm_sub_ps(m_simd, b.m_simd));
#else
			return TVec(vsubq_f32(m_simd, b.m_simd));
#endif
		}
		else
		{
			TVec out;
			for(U32 i = 0; i < kTComponentCount; i++)
			{
				out.m_arr[i] = m_arr[i] - b.m_arr[i];
			}
			return out;
		}
	}

	TVec& operator-=(TVec b)
	{
		if constexpr(kVec4Simd)
		{
#if ANKI_SIMD_SSE
			m_simd = _mm_sub_ps(m_simd, b.m_simd);
#else
			m_simd = vsubq_f32(m_simd, b.m_simd);
#endif
		}
		else
		{
			for(U32 i = 0; i < kTComponentCount; i++)
			{
				m_arr[i] -= b.m_arr[i];
			}
		}
		return *this;
	}

	[[nodiscard]] TVec operator*(TVec b) const
	{
		if constexpr(kVec4Simd)
		{
#if ANKI_SIMD_SSE
			return TVec(_mm_mul_ps(m_simd, b.m_simd));
#else
			return TVec(vmulq_f32(m_simd, b.m_simd));
#endif
		}
		else
		{
			TVec out;
			for(U32 i = 0; i < kTComponentCount; i++)
			{
				out.m_arr[i] = m_arr[i] * b.m_arr[i];
			}
			return out;
		}
	}

	TVec& operator*=(TVec b)
	{
		if constexpr(kVec4Simd)
		{
#if ANKI_SIMD_SSE
			m_simd = _mm_mul_ps(m_simd, b.m_simd);
#else
			m_simd = vmulq_f32(m_simd, b.m_simd);
#endif
		}
		else
		{
			for(U32 i = 0; i < kTComponentCount; i++)
			{
				m_arr[i] *= b.m_arr[i];
			}
		}
		return *this;
	}

	[[nodiscard]] TVec operator/(TVec b) const
	{
		if constexpr(kVec4Simd)
		{
#if ANKI_SIMD_SSE
			return TVec(_mm_div_ps(m_simd, b.m_simd));
#else
			return TVec(vdivq_f32(m_simd, b.m_simd));
#endif
		}
		else
		{
			TVec out;
			for(U32 i = 0; i < kTComponentCount; i++)
			{
				ANKI_ASSERT(b.m_arr[i] != 0.0);
				out.m_arr[i] = m_arr[i] / b.m_arr[i];
			}
			return out;
		}
	}

	TVec& operator/=(TVec b)
	{
		if constexpr(kVec4Simd)
		{
#if ANKI_SIMD_SSE
			m_simd = _mm_div_ps(m_simd, b.m_simd);
#else
			m_simd = vdivq_f32(m_simd, b.m_simd);
#endif
		}
		else
		{
			for(U32 i = 0; i < kTComponentCount; i++)
			{
				ANKI_ASSERT(b.m_arr[i] != 0.0);
				m_arr[i] /= b.m_arr[i];
			}
		}
		return *this;
	}

	[[nodiscard]] TVec operator-() const
	{
		if constexpr(kVec4Simd)
		{
#if ANKI_SIMD_SSE
			return TVec(_mm_xor_ps(m_simd, _mm_set1_ps(-0.0)));
#else
			return TVec(veorq_s32(m_simd, vdupq_n_f32(-0.0)));
#endif
		}
		else
		{
			TVec out;
			for(U32 i = 0; i < kTComponentCount; i++)
			{
				out.m_arr[i] = -m_arr[i];
			}
			return out;
		}
	}

	[[nodiscard]] TVec operator<<(TVec b) const requires(kIsInteger)
	{
		TVec out;
		for(U32 i = 0; i < kTComponentCount; i++)
		{
			out.m_arr[i] = m_arr[i] << b.m_arr[i];
		}
		return out;
	}

	TVec& operator<<=(TVec b) requires(kIsInteger)
	{
		for(U32 i = 0; i < kTComponentCount; i++)
		{
			m_arr[i] <<= b.m_arr[i];
		}
		return *this;
	}

	[[nodiscard]] TVec operator>>(TVec b) const requires(kIsInteger)
	{
		TVec out;
		for(U32 i = 0; i < kTComponentCount; i++)
		{
			out.m_arr[i] = m_arr[i] >> b.m_arr[i];
		}
		return out;
	}

	TVec& operator>>=(TVec b) requires(kIsInteger)
	{
		for(U32 i = 0; i < kTComponentCount; i++)
		{
			m_arr[i] >>= b.m_arr[i];
		}
		return *this;
	}

	[[nodiscard]] TVec operator&(TVec b) const requires(kIsInteger)
	{
		TVec out;
		for(U32 i = 0; i < kTComponentCount; i++)
		{
			out.m_arr[i] = m_arr[i] & b.m_arr[i];
		}
		return out;
	}

	TVec& operator&=(TVec b) requires(kIsInteger)
	{
		for(U32 i = 0; i < kTComponentCount; i++)
		{
			m_arr[i] &= b.m_arr[i];
		}
		return *this;
	}

	[[nodiscard]] TVec operator|(TVec b) const requires(kIsInteger)
	{
		TVec out;
		for(U32 i = 0; i < kTComponentCount; i++)
		{
			out.m_arr[i] = m_arr[i] | b.m_arr[i];
		}
		return out;
	}

	TVec& operator|=(TVec b) requires(kIsInteger)
	{
		for(U32 i = 0; i < kTComponentCount; i++)
		{
			m_arr[i] |= b.m_arr[i];
		}
		return *this;
	}

	[[nodiscard]] TVec operator^(TVec b) const requires(kIsInteger)
	{
		TVec out;
		for(U32 i = 0; i < kTComponentCount; i++)
		{
			out.m_arr[i] = m_arr[i] ^ b.m_arr[i];
		}
		return out;
	}

	TVec& operator^=(TVec b) requires(kIsInteger)
	{
		for(U32 i = 0; i < kTComponentCount; i++)
		{
			m_arr[i] ^= b.m_arr[i];
		}
		return *this;
	}

	[[nodiscard]] TVec operator%(TVec b) const requires(kIsInteger)
	{
		TVec out;
		for(U32 i = 0; i < kTComponentCount; i++)
		{
			out.m_arr[i] = m_arr[i] % b.m_arr[i];
		}
		return out;
	}

	TVec& operator%=(TVec b) requires(kIsInteger)
	{
		for(U32 i = 0; i < kTComponentCount; i++)
		{
			m_arr[i] %= b.m_arr[i];
		}
		return *this;
	}

	[[nodiscard]] Bool operator==(TVec b) const
	{
		for(U32 i = 0; i < kTComponentCount; i++)
		{
			if(!isZero<T>(m_arr[i] - b.m_arr[i]))
			{
				return false;
			}
		}
		return true;
	}

	[[nodiscard]] Bool operator!=(TVec b) const
	{
		return !operator==(b);
	}

	[[nodiscard]] Bool operator<(TVec b) const
	{
		for(U32 i = 0; i < kTComponentCount; i++)
		{
			if(m_arr[i] >= b.m_arr[i])
			{
				return false;
			}
		}
		return true;
	}

	[[nodiscard]] Bool operator<=(TVec b) const
	{
		for(U32 i = 0; i < kTComponentCount; i++)
		{
			if(m_arr[i] > b.m_arr[i])
			{
				return false;
			}
		}
		return true;
	}

	[[nodiscard]] Bool operator>(TVec b) const
	{
		for(U32 i = 0; i < kTComponentCount; i++)
		{
			if(m_arr[i] <= b.m_arr[i])
			{
				return false;
			}
		}
		return true;
	}

	[[nodiscard]] Bool operator>=(TVec b) const
	{
		for(U32 i = 0; i < kTComponentCount; i++)
		{
			if(m_arr[i] < b.m_arr[i])
			{
				return false;
			}
		}
		return true;
	}

	// Operators with T

	[[nodiscard]] TVec operator+(T f) const
	{
		return (*this) + TVec(f);
	}

	TVec& operator+=(T f)
	{
		(*this) += TVec(f);
		return *this;
	}

	[[nodiscard]] TVec operator-(T f) const
	{
		return (*this) - TVec(f);
	}

	TVec& operator-=(T f)
	{
		(*this) -= TVec(f);
		return *this;
	}

	[[nodiscard]] TVec operator*(T f) const
	{
		return (*this) * TVec(f);
	}

	TVec& operator*=(T f)
	{
		(*this) *= TVec(f);
		return *this;
	}

	[[nodiscard]] TVec operator/(T f) const
	{
		return (*this) / TVec(f);
	}

	TVec& operator/=(T f)
	{
		(*this) /= TVec(f);
		return *this;
	}

	[[nodiscard]] TVec operator<<(T f) const requires(kIsInteger)
	{
		return (*this) << TVec(f);
	}

	TVec& operator<<=(T f) requires(kIsInteger)
	{
		(*this) <<= TVec(f);
		return *this;
	}

	[[nodiscard]] TVec operator>>(T f) const requires(kIsInteger)
	{
		return (*this) >> TVec(f);
	}

	TVec& operator>>=(T f) requires(kIsInteger)
	{
		(*this) >>= TVec(f);
		return *this;
	}

	[[nodiscard]] TVec operator&(T f) const requires(kIsInteger)
	{
		return (*this) & TVec(f);
	}

	TVec& operator&=(T f) requires(kIsInteger)
	{
		(*this) &= TVec(f);
		return *this;
	}

	[[nodiscard]] TVec operator|(T f) const requires(kIsInteger)
	{
		return (*this) | TVec(f);
	}

	TVec& operator|=(T f) requires(kIsInteger)
	{
		(*this) |= TVec(f);
		return *this;
	}

	[[nodiscard]] TVec operator^(T f) const requires(kIsInteger)
	{
		return (*this) ^ TVec(f);
	}

	TVec& operator^=(T f) requires(kIsInteger)
	{
		(*this) ^= TVec(f);
		return *this;
	}

	[[nodiscard]] TVec operator%(T f) const requires(kIsInteger)
	{
		return (*this) % TVec(f);
	}

	TVec& operator%=(T f) requires(kIsInteger)
	{
		(*this) %= TVec(f);
		return *this;
	}

	[[nodiscard]] Bool operator==(T f) const
	{
		return *this == TVec(f);
	}

	[[nodiscard]] Bool operator!=(T f) const
	{
		return *this != TVec(f);
	}

	[[nodiscard]] Bool operator<(T f) const
	{
		return *this < TVec(f);
	}

	[[nodiscard]] Bool operator<=(T f) const
	{
		return *this <= TVec(f);
	}

	[[nodiscard]] Bool operator>(T f) const
	{
		return *this > TVec(f);
	}

	[[nodiscard]] Bool operator>=(T f) const
	{
		return *this >= TVec(f);
	}

	// Operators with other

	[[nodiscard]] TVec operator*(const TMat<T, 4, 4>& m4) const requires(kTComponentCount == 4)
	{
		TVec out;
		out.x = this->x * m4(0, 0) + this->y * m4(1, 0) + this->z * m4(2, 0) + this->w * m4(3, 0);
		out.y = this->x * m4(0, 1) + this->y * m4(1, 1) + this->z * m4(2, 1) + this->w * m4(3, 1);
		out.z = this->x * m4(0, 2) + this->y * m4(1, 2) + this->z * m4(2, 2) + this->w * m4(3, 2);
		out.w = this->x * m4(0, 3) + this->y * m4(1, 3) + this->z * m4(2, 3) + this->w * m4(3, 3);
		return out;
	}

	// Other

	[[nodiscard]] T dot(TVec b) const
	{
		T out = T(0);
		if constexpr(kVec4Simd)
		{
#if ANKI_SIMD_SSE
			_mm_store_ss(&out, _mm_dp_ps(m_simd, b.m_simd, 0xF1));
#else
			out = vaddvq_f32(vmulq_f32(m_simd, b.m_simd));
#endif
		}
		else
		{
			for(U32 i = 0; i < kTComponentCount; i++)
			{
				out += m_arr[i] * b.m_arr[i];
			}
		}
		return out;
	}

	// 6 muls, 3 adds
	[[nodiscard]] TVec cross(TVec b) const requires(kTComponentCount == 3)
	{
		return TVec(this->y * b.z - this->z * b.y, this->z * b.x - this->x * b.z, this->x * b.y - this->y * b.x);
	}

	// It's like calculating the cross of a 3 component TVec.
	[[nodiscard]] TVec cross(TVec b_) const requires(kTComponentCount == 4)
	{
		ANKI_ASSERT(this->w == T(0));
		ANKI_ASSERT(b_.w == T(0));

#if ANKI_SIMD_SSE
		const auto& a = m_simd;
		const auto& b = b_.m_simd;

		__m128 t1 = _mm_shuffle_ps(b, b, _MM_SHUFFLE(0, 0, 2, 1));
		t1 = _mm_mul_ps(t1, a);
		__m128 t2 = _mm_shuffle_ps(a, a, _MM_SHUFFLE(0, 0, 2, 1));
		t2 = _mm_mul_ps(t2, b);
		__m128 t3 = _mm_sub_ps(t1, t2);

		return TVec(_mm_shuffle_ps(t3, t3, _MM_SHUFFLE(0, 0, 2, 1))).xyz0;
#elif ANKI_SIMD_NEON
		const auto& a = m_simd;
		const auto& b = b_.m_simd;

		float32x4_t t1 = ANKI_NEON_SHUFFLE_F32x4(b, b, 0, 0, 2, 1);
		t1 = vmulq_f32(t1, a);
		float32x4_t t2 = ANKI_NEON_SHUFFLE_F32x4(a, a, 0, 0, 2, 1);
		t2 = vmulq_f32(t2, b);
		float32x4_t t3 = vsubq_f32(t1, t2);

		return TVec(ANKI_NEON_SHUFFLE_F32x4(t3, t3, 0, 0, 2, 1)).xyz0;
#else
		return TVec(xyz.cross(b_.xyz), T(0));
#endif
	}

	[[nodiscard]] TVec projectTo(TVec toThis) const
	{
		if constexpr(kTComponentCount < 4)
		{
			return toThis * ((*this).dot(toThis) / (toThis.dot(toThis)));
		}
		else
		{
			ANKI_ASSERT(this->w == T(0));
			return (toThis * ((*this).dot(toThis) / (toThis.dot(toThis)))).xyz0;
		}
	}

	[[nodiscard]] TVec projectTo(TVec rayOrigin, TVec rayDir) const
	{
		if constexpr(kTComponentCount < 4)
		{
			const auto& a = *this;
			return rayOrigin + rayDir * ((a - rayOrigin).dot(rayDir));
		}
		else
		{
			ANKI_ASSERT(this->w == T(0));
			ANKI_ASSERT(rayOrigin.w == T(0));
			ANKI_ASSERT(rayDir.w == T(0));
			const auto& a = *this;
			return rayOrigin + rayDir * ((a - rayOrigin).dot(rayDir));
		}
	}

	// Perspective divide. Divide the xyzw of this to the w of this. This method will handle some edge cases.
	[[nodiscard]] TVec perspectiveDivide() const requires(kTComponentCount == 4 && !kIsInteger)
	{
		auto invw = T(1) / this->w; // This may become (+-)inf
		invw = (invw > T(1e+11)) ? T(1e+11) : invw; // Clamp
		invw = (invw < T(-1e+11)) ? T(-1e+11) : invw; // Clamp
		return (*this) * invw;
	}

	[[nodiscard]] T lengthSquared() const
	{
		return dot(*this);
	}

	[[nodiscard]] T length() const
	{
		return sqrt<T>(lengthSquared());
	}

	[[nodiscard]] T distanceSquared(TVec b) const
	{
		return ((*this) - b).lengthSquared();
	}

	[[nodiscard]] T distance(TVec b) const
	{
		return sqrt<T>(distance(b));
	}

	[[nodiscard]] TVec normalize() const
	{
		if constexpr(kVec4Simd)
		{
#if ANKI_SIMD_SSE
			__m128 v = _mm_dp_ps(m_simd, m_simd, 0xFF);
			v = _mm_sqrt_ps(v);
			v = _mm_div_ps(m_simd, v);
			return TVec(v);
#else
			float32x4_t v = vmulq_f32(m_simd, m_simd);
			v = vdupq_n_f32(vaddvq_f32(v));
			v = vsqrtq_f32(v);
			v = vdivq_f32(m_simd, v);
			return TVec(v);
#endif
		}
		else
		{
			return (*this) / length();
		}
	}

	// Return lerp(this, v1, t)
	[[nodiscard]] TVec lerp(TVec v1, TVec t) const
	{
		TVec out;
		for(U32 i = 0; i < kTComponentCount; ++i)
		{
			out[i] = m_arr[i] * (T(1) - t.m_arr[i]) + v1.m_arr[i] * t.m_arr[i];
		}
		return out;
	}

	// Return lerp(this, v1, t)
	[[nodiscard]] TVec lerp(TVec v1, T t) const
	{
		return ((*this) * (T(1) - t)) + (v1 * t);
	}

	[[nodiscard]] TVec abs() const
	{
		if constexpr(kVec4Simd)
		{
#if ANKI_SIMD_SSE
			const __m128 signMask = _mm_set1_ps(-0.0f);
			return TVec(_mm_andnot_ps(signMask, m_simd));
#else
			return TVec(vabsq_f32(m_simd));
#endif
		}
		else
		{
			TVec out;
			for(U32 i = 0; i < kTComponentCount; ++i)
			{
				out[i] = absolute<T>(m_arr[i]);
			}
			return out;
		}
	}

	// Get clamped between two values.
	[[nodiscard]] TVec clamp(T minv, T maxv) const
	{
		return max(TVec(minv)).min(TVec(maxv));
	}

	// Get clamped between two vectors.
	[[nodiscard]] TVec clamp(TVec minv, TVec maxv) const
	{
		return max(minv).min(maxv);
	}

	// Get the min of all components.
	[[nodiscard]] TVec min(TVec b) const
	{
		if constexpr(kVec4Simd)
		{
#if ANKI_SIMD_SSE
			return TVec(_mm_min_ps(m_simd, b.m_simd));
#else
			return TVec(vminq_f32(m_simd, b.m_simd));
#endif
		}
		else
		{
			TVec out;
			for(U32 i = 0; i < kTComponentCount; ++i)
			{
				out[i] = anki::min<T>(m_arr[i], b[i]);
			}
			return out;
		}
	}

	// Get the min of all components.
	[[nodiscard]] TVec min(T b) const
	{
		return min(TVec(b));
	}

	// Get the max of all components.
	[[nodiscard]] TVec max(TVec b) const
	{
		if constexpr(kVec4Simd)
		{
#if ANKI_SIMD_SSE
			return TVec(_mm_max_ps(m_simd, b.m_simd));
#else
			return TVec(vmaxq_f32(m_simd, b.m_simd));
#endif
		}
		else
		{
			TVec out;
			for(U32 i = 0; i < kTComponentCount; ++i)
			{
				out[i] = anki::max<T>(m_arr[i], b[i]);
			}
			return out;
		}
	}

	// Get the max of all components.
	[[nodiscard]] TVec max(T b) const
	{
		return max(TVec(b));
	}

	[[nodiscard]] TVec round() const requires(!kIsInteger)
	{
		TVec out;
		for(U32 i = 0; i < kTComponentCount; ++i)
		{
			out[i] = T(::round(m_arr[i]));
		}
		return out;
	}

	// Get a safe 1 / (*this)
	[[nodiscard]] TVec reciprocal() const
	{
		TVec out;
		for(U32 i = 0; i < kTComponentCount; ++i)
		{
			out[i] = T(1) / m_arr[i];
		}
		return out;
	}

	// Power
	[[nodiscard]] TVec pow(TVec b) const
	{
		TVec out;
		for(U32 i = 0; i < kTComponentCount; ++i)
		{
			out[i] = anki::pow(m_arr[i], b.m_arr[i]);
		}
		return out;
	}

	// Power
	[[nodiscard]] TVec pow(T b) const
	{
		return pow(TVec(b));
	}

	[[nodiscard]] static TVec xAxis() requires(kTComponentCount == 2)
	{
		return TVec(T(1), T(0));
	}

	[[nodiscard]] static TVec xAxis() requires(kTComponentCount == 3)
	{
		return TVec(T(1), T(0), T(0));
	}

	[[nodiscard]] static TVec xAxis() requires(kTComponentCount == 4)
	{
		return TVec(T(1), T(0), T(0), T(0));
	}

	[[nodiscard]] static TVec yAxis() requires(kTComponentCount == 2)
	{
		return TVec(T(0), T(1));
	}

	[[nodiscard]] static TVec yAxis() requires(kTComponentCount == 3)
	{
		return TVec(T(0), T(1), T(0));
	}

	[[nodiscard]] static TVec yAxis() requires(kTComponentCount == 4)
	{
		return TVec(T(0), T(1), T(0), T(0));
	}

	[[nodiscard]] static TVec zAxis() requires(kTComponentCount == 3)
	{
		return TVec(T(0), T(0), T(1));
	}

	[[nodiscard]] static TVec zAxis() requires(kTComponentCount == 4)
	{
		return TVec(T(0), T(0), T(1), T(0));
	}

	// Serialize the structure.
	void serialize(void* data, PtrSize& size) const
	{
		size = sizeof(*this);
		if(data)
		{
			memcpy(data, this, sizeof(*this));
		}
	}

	// De-serialize the structure.
	void deserialize(const void* data)
	{
		ANKI_ASSERT(data);
		memcpy(this, data, sizeof(*this));
	}

	[[nodiscard]] static constexpr U8 getSize()
	{
		return U8(kTComponentCount);
	}

	U32 packSnorm4x8() const requires(std::is_floating_point_v<T>&& kTComponentCount == 4)
	{
		union
		{
			I8 in[4];
			U32 out;
		} u;

		const TVec result = (clamp(-1.0f, 1.0f) * 127.0f).round();

		u.in[0] = I8(result[0]);
		u.in[1] = I8(result[1]);
		u.in[2] = I8(result[2]);
		u.in[3] = I8(result[3]);

		return u.out;
	}

	U32 packUnorm4x8() const requires(std::is_floating_point_v<T>&& kTComponentCount == 4)
	{
		ANKI_ASSERT((*this <= TVec(T(1)) && *this >= TVec(T(0))));
		const TVec packed(*this * T(255));
		return packed.x | (U32(packed.y) << 8u) | (U32(packed.z) << 16u) | (U32(packed.w) << 24u);
	}

	// Opposite of packUnorm4x8
	void unpackUnorm4x8(const U32 value) requires(std::is_floating_point_v<T>&& kTComponentCount == 4)
	{
		const TVec packed(value & 0xFF, (value >> 8u) & 0xFF, (value >> 16u) & 0xff, value >> 24u);
		*this = packed / T(255);
	}

	void setFromSphericalToCartesian(T polar, T azimuth) requires(std::is_floating_point_v<T>&& kTComponentCount == 3)
	{
		TVec& out = *this;
		out.x = cos(polar) * sin(azimuth);
		out.y = cos(polar) * cos(azimuth);
		out.z = sin(polar);
	}

	[[nodiscard]] String toString() const requires(std::is_floating_point<T>::value)
	{
		String str;
		for(U32 i = 0; i < kTComponentCount; ++i)
		{
			str += String().sprintf((i < i - kTComponentCount) ? "%f " : "%f", m_arr[i]);
		}
		return str;
	}

	static constexpr Bool kClangWorkaround = std::is_integral<T>::value && std::is_unsigned<T>::value;
	[[nodiscard]] String toString() const requires(kClangWorkaround)
	{
		String str;
		for(U32 i = 0; i < kTComponentCount; ++i)
		{
			str += String().sprintf((i < i - kTComponentCount) ? "%u " : "%u", m_arr[i]);
		}
		return str;
	}

	static constexpr Bool kClangWorkaround2 = std::is_integral<T>::value && std::is_signed<T>::value;
	[[nodiscard]] String toString() const requires(kClangWorkaround2)
	{
		String str;
		for(U32 i = 0; i < kTComponentCount; ++i)
		{
			str += String().sprintf((i < i - kTComponentCount) ? "%d " : "%d", m_arr[i]);
		}
		return str;
	}
};

template<typename T, U32 kTComponentCount>
TVec<T, kTComponentCount> operator+(T f, TVec<T, kTComponentCount> v)
{
	return v + f;
}

template<typename T, U32 kTComponentCount>
TVec<T, kTComponentCount> operator-(T f, TVec<T, kTComponentCount> v)
{
	return TVec<T, kTComponentCount>(f) - v;
}

template<typename T, U32 kTComponentCount>
TVec<T, kTComponentCount> operator*(T f, TVec<T, kTComponentCount> v)
{
	return v * f;
}

template<typename T, U32 kTComponentCount>
TVec<T, kTComponentCount> operator/(T f, TVec<T, kTComponentCount> v)
{
	return TVec<T, kTComponentCount>(f) / v;
}

// All vectors
using Vec2 = TVec<F32, 2>;
using HVec2 = TVec<F16, 2>;
using IVec2 = TVec<I32, 2>;
using I16Vec2 = TVec<I16, 2>;
using I8Vec2 = TVec<I8, 2>;
using UVec2 = TVec<U32, 2>;
using U16Vec2 = TVec<U16, 2>;
using U8Vec2 = TVec<U8, 2>;
using Vec3 = TVec<F32, 3>;
using HVec3 = TVec<F16, 3>;
using IVec3 = TVec<I32, 3>;
using I16Vec3 = TVec<I16, 3>;
using I8Vec3 = TVec<I8, 3>;
using UVec3 = TVec<U32, 3>;
using U16Vec3 = TVec<U16, 3>;
using U8Vec3 = TVec<U8, 3>;
using Vec4 = TVec<F32, 4>;
using HVec4 = TVec<F16, 4>;
using IVec4 = TVec<I32, 4>;
using I16Vec4 = TVec<I16, 4>;
using I8Vec4 = TVec<I8, 4>;
using UVec4 = TVec<U32, 4>;
using U16Vec4 = TVec<U16, 4>;
using U8Vec4 = TVec<U8, 4>;

} // end namespace anki
