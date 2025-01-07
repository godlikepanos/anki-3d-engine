// Copyright (C) 2009-present, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

// Screen space ray marching

#pragma once

#include <AnKi/Shaders/Functions.hlsl>

// If the angle between the reflection and the normal grows more than 90 deg than the return value turns to zero
F32 rejectBackFaces(Vec3 reflection, Vec3 normalAtHitPoint)
{
	return smoothstep(-0.17, 0.0, dot(normalAtHitPoint, -reflection));
}

// Find the intersection of a ray and a AABB when the ray is inside the AABB
void rayAabbIntersectionInside2d(Vec2 rayOrigin, Vec2 rayDir, Vec2 aabbMin, Vec2 aabbMax, out F32 t)
{
	// Find the boundary of the AABB that the rayDir points at
	Vec2 boundary;
	boundary.x = (rayDir.x > 0.0) ? aabbMax.x : aabbMin.x;
	boundary.y = (rayDir.y > 0.0) ? aabbMax.y : aabbMin.y;

	// Find the intersection of the ray with the line y=boundary.y
	// The intersection is: rayOrigin + T * rayDir
	// For y it's: rayOrigin.y + T * rayDir.y
	// And it's equal to the boundary.y: rayOrigin.y + T * rayDir.y = boundary.y
	const F32 ty = (boundary.y - rayOrigin.y) / rayDir.y;

	// Same for x=boundary.x
	const F32 tx = (boundary.x - rayOrigin.x) / rayDir.x;

	// Chose the shortest t
	t = min(ty, tx);
}

// Find the cell the rayOrigin is in and push it outside that cell towards the direction of the rayDir
void stepToNextCell(Vec3 rayOrigin, Vec3 rayDir, U32 mipLevel, UVec2 hizSize, out Vec3 newRayOrigin)
{
	const UVec2 mipSize = hizSize >> mipLevel;
	const Vec2 mipSizef = Vec2(mipSize);

	// Position in texture space
	const Vec2 texPos = rayOrigin.xy * mipSizef;

	// Compute the boundaries of the cell in UV space
	const Vec2 cellMin = floor(texPos) / mipSizef;
	const Vec2 cellMax = ceil(texPos) / mipSizef;

	// Find the intersection
	F32 t;
	rayAabbIntersectionInside2d(rayOrigin.xy, rayDir.xy, cellMin, cellMax, t);

	// Bump t a bit to stop touching the cell
	const F32 texelSizeX = 1.0 / mipSizef.x;
	t += texelSizeX / 10.0;

	// Compute the new origin
	newRayOrigin = rayOrigin + rayDir * t;
}

// Note: All calculations in view space
void raymarch(Vec3 rayOrigin, // Ray origin in view space
			  Vec3 rayDir, // Ray dir in view space
			  F32 tmin, // Shoot rays from
			  Vec2 uv, // UV the ray starts
			  F32 depthRef, // Depth the ray starts
			  Mat4 projMat, // Projection matrix
			  U32 randFrom0To3, U32 maxIterations, Texture2D hizTex, SamplerState hizSampler, U32 hizMipCount, UVec2 hizMip0Size, out Vec3 hitPoint,
			  out F32 attenuation)
{
	ANKI_MAYBE_UNUSED(uv);
	ANKI_MAYBE_UNUSED(depthRef);
	attenuation = 0.0;

	// Check for view facing reflections [sakibsaikia]
	const Vec3 viewDir = normalize(rayOrigin);
	const F32 cameraContribution = 1.0 - smoothstep(0.25, 0.5, dot(-viewDir, rayDir));
	[branch] if(cameraContribution <= 0.0)
	{
		return;
	}

	// Dither and set starting pos
	const F32 bayerMat[4] = {1.0, 4.0, 2.0, 3.0};
	const Vec3 p0 = rayOrigin + rayDir * (tmin * bayerMat[randFrom0To3]);

	// p1
	const F32 tmax = 10.0;
	const Vec3 p1 = rayOrigin + rayDir * tmax;

	// Compute start & end in clip space (well not clip space since x,y are in [0, 1])
	Vec4 v4 = mul(projMat, Vec4(p0, 1.0));
	Vec3 start = v4.xyz / v4.w;
	start.xy = ndcToUv(start.xy);
	v4 = mul(projMat, Vec4(p1, 1.0));
	Vec3 end = v4.xyz / v4.w;
	end.xy = ndcToUv(end.xy);

	// Ray
	Vec3 origin = start;
	const Vec3 dir = normalize(end - start);

	// Start looping
	I32 mipLevel = 0;
	while(mipLevel > -1 && maxIterations > 0u)
	{
		// Step to the next cell
		Vec3 newOrigin;
		stepToNextCell(origin, dir, U32(mipLevel), hizMip0Size, newOrigin);
		origin = newOrigin;

		if(all(origin.xy > Vec2(0.0, 0.0)) && all(origin.xy < Vec2(1.0, 1.0)))
		{
			const F32 newDepth = hizTex.SampleLevel(hizSampler, origin.xy, F32(mipLevel)).r;

			if(origin.z < newDepth)
			{
				// In front of depth
				mipLevel = min(mipLevel + 1, I32(hizMipCount - 1u));
			}
			else
			{
				// Behind depth
				const F32 t = (origin.z - newDepth) / dir.z;
				origin -= dir * t;
				--mipLevel;
			}

			--maxIterations;
		}
		else
		{
			// Out of the screen
			break;
		}
	}

	// Write the values
	const F32 blackMargin = 0.05 / 4.0;
	const F32 whiteMargin = 0.1 / 2.0;
	const Vec2 marginAttenuation2d =
		smoothstep(blackMargin, whiteMargin, origin.xy) * (1.0 - smoothstep(1.0 - whiteMargin, 1.0 - blackMargin, origin.xy));
	const F32 marginAttenuation = marginAttenuation2d.x * marginAttenuation2d.y;
	attenuation = marginAttenuation * cameraContribution;

	hitPoint = origin;
}

struct RayMarchingConfig
{
	U32 m_maxIterations; // The max iterations of the base algorithm
	F32 m_depthTextureLod; // LOD to pass to the SampleLevel of the depth texture
	U32 m_stepIncrement; // The step increment of each iteration
	U32 m_initialStepIncrement; // The initial step

	Bool m_backfaceRejection; // If it's zero then no rejection will happen
	F32 m_minRayToBackgroundDistance; // Distance between the hit point on the ray and the hit point that is touching the depth buffer.
									  // Make it kMaxF32 to disable the test
};

struct DummySampleNormalFunc
{
	Vec3 sample(Vec2 uv)
	{
		return 0.0;
	}
};

// Note: All calculations in view space
template<typename TSampleNormalFunc>
Bool raymarchGroundTruth(Vec3 rayOrigin, // Ray origin in view space
						 Vec3 rayDir, // Ray dir in view space
						 Vec2 uv, // UV the ray starts
						 F32 depthRef, // Depth the ray starts
						 Vec4 projMat00_11_22_23, // Projection matrix
						 Vec4 unprojectionParams,
						 Texture2D<Vec4> depthTex, // Depth tex
						 SamplerState depthSampler, // Sampler for depthTex
						 RayMarchingConfig config,
						 TSampleNormalFunc sampleNormalFunc, // Only used if m_backfaceRejection is true
						 out Vec3 hitPoint, // Hit point in view space
						 out Vec2 hitUv, out F32 hitDepth)
{
	hitPoint = rayOrigin;
	hitUv = uv;
	hitDepth = 0.0;

	// Check for view facing reflections [sakibsaikia]
	const Vec3 viewDir = normalize(rayOrigin);
	const F32 cameraContribution = 1.0 - smoothstep(0.25, 0.5, dot(-viewDir, rayDir));
	if(cameraContribution <= 0.0)
	{
		return false;
	}

	// Find the depth's mipmap size
	UVec2 depthTexSize;
	U32 depthTexMipCount;
	depthTex.GetDimensions(0u, depthTexSize.x, depthTexSize.y, depthTexMipCount);
	config.m_depthTextureLod = min(config.m_depthTextureLod, F32(depthTexMipCount) - 1.0f);
	const UVec2 depthTexMipSize = depthTexSize >> U32(config.m_depthTextureLod);

	// Start point
	const Vec3 start = Vec3(uv, depthRef);

	// Project end point
	const Vec3 p1 = rayOrigin + rayDir * 0.1;
	const Vec4 end4 = cheapPerspectiveProjection(projMat00_11_22_23, Vec4(p1, 1.0));
	Vec3 end = end4.xyz / end4.w;
	end.xy = ndcToUv(end.xy);

	// Compute the ray and step size
	Vec3 dir = end - start;
	const Vec2 dir2d = normalize(dir.xy);
	dir = dir * (dir2d.x / dir.x); // Normalize only on xy

	const F32 stepSize = 1.0 / ((dir.x > dir.y) ? depthTexMipSize.x : depthTexMipSize.y);

	// Compute step
	I32 stepIncrement = I32(config.m_stepIncrement);
	I32 crntStep = I32(config.m_initialStepIncrement);

	// Search
	[loop] while(config.m_maxIterations-- != 0u)
	{
		const Vec3 newHit = start + dir * (F32(crntStep) * stepSize);

		// Check if it's out of the view
		if(any(newHit <= 0.0) || any(newHit >= 1.0))
		{
			return false;
		}

		const F32 depth = depthTex.SampleLevel(depthSampler, newHit.xy, config.m_depthTextureLod).r;
		const Bool hit = newHit.z >= depth;

		// Move the hit point to where the depth is
		Vec3 newHitPoint = cheapPerspectiveUnprojection(unprojectionParams, uvToNdc(newHit.xy), newHit.z);

		if(!hit)
		{
			crntStep += stepIncrement;
			hitPoint = newHitPoint;
			hitUv = newHit.xy;
			hitDepth = depth;
		}
		else if(stepIncrement > 1)
		{
			// There is a hit but the step increment is a bit high, need a more fine-grained search

			crntStep = max(1, crntStep - stepIncrement + 1);
			stepIncrement = stepIncrement / 2;
		}
		else
		{
			// Maybe found it

			if(config.m_backfaceRejection)
			{
				const Vec3 hitNormal = sampleNormalFunc.sample(newHit.xy);
				const F32 backFaceAttenuation = rejectBackFaces(rayDir, hitNormal);
				if(backFaceAttenuation <= 0.5)
				{
					return false;
				}
			}

			if(config.m_minRayToBackgroundDistance < kMaxF32)
			{
				const Vec3 pointInDepth = cheapPerspectiveUnprojection(unprojectionParams, uvToNdc(newHit.xy), depth);
				const F32 dist = length(newHitPoint - pointInDepth);
				if(dist > config.m_minRayToBackgroundDistance)
				{
					return false;
				}

				newHitPoint = pointInDepth;
			}

			// Found it
			hitPoint = newHitPoint;
			hitUv = newHit.xy;
			hitDepth = depth;
			return true;
		}
	}

	return false;
}
