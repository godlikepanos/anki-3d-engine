// Copyright (C) 2009-2023, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#define groupshared
#define globallycoherent
#define SV_DISPATCHTHREADID // gl_GlobalInvocationID
#define SV_GROUPINDEX // gl_LocalInvocationIndex
#define SV_GROUPID // gl_WorkGroupID
#define SV_GROUPTHREADID // gl_LocalInvocationID
#define SV_VERTEXID
#define SV_POSITION
#define SV_INSTANCEID
#define numthreads(x, y, z) [nodiscard]
#define unroll [nodiscard]
#define loop [nodiscard]

#define ANKI_BEGIN_NAMESPACE
#define ANKI_END_NAMESPACE
#define ANKI_HLSL 1

using I8 = int;
using I16 = int;
using I32 = int;
using U8 = unsigned int;
using U16 = unsigned int;
using U32 = unsigned int;
using F32 = float;
using Bool = bool;

struct UVec2
{
	U32 x;
	U32 y;
};

struct UVec3
{
	U32 x;
	U32 y;
	U32 z;
};

struct UVec4
{
	U32 x;
	U32 y;
	U32 z;
	U32 w;
};

struct IVec2
{
	I32 x;
	I32 y;
};

struct IVec3
{
	I32 x;
	I32 y;
	I32 z;
};

struct IVec4
{
	I32 x;
	I32 y;
	I32 z;
	I32 w;
};

struct Vec2
{
	F32 x;
	F32 y;
};

struct Vec3
{
	F32 x;
	F32 y;
	F32 z;
};

struct Vec4
{
	F32 x;
	F32 y;
	F32 z;
	F32 w;
};

struct Mat4
{
	F32 arr[16];
};

struct SamplerState
{
};

struct SamplerComparisonState
{
};

template<typename T>
struct Texture2D
{
	void GetDimensions(U32& width, U32& height);

	void GetDimensions(F32& width, F32& height);

	T SampleLevel(SamplerState sampler, Vec2 uvs, F32 lod, IVec2 offset = {});

	T& operator[](UVec2 coords);
};

template<typename T>
using RWTexture2D = Texture2D<T>;

template<typename T>
struct StructuredBuffer
{
	T& operator[](U32 index);

	void GetDimensions(U32& length, U32& stride);
};

template<typename T>
using RWStructuredBuffer = StructuredBuffer<T>;

template<typename T>
struct ConstantBuffer : public T
{
};

struct ByteAddressBuffer
{
	template<typename T>
	T& Load(U32 offset);
};

struct RaytracingAccelerationStructure
{
};

// Basic functions

template<typename T>
T min(T a, T b);

template<typename T>
T max(T a, T b);

template<typename T>
T saturate(T a);

template<typename T>
float dot(T a, T b);

// Atomics

template<typename T>
void InterlockedAdd(T dest, T value, T& originalValue);

template<typename T>
void InterlockedAdd(T dest, T value);

// Wave ops

template<typename T>
T WaveActiveMin(T value);

template<typename T>
T WaveActiveMax(T value);

bool WaveIsFirstLane();

unsigned WaveActiveCountBits(bool bit);

unsigned WaveGetLaneCount();
