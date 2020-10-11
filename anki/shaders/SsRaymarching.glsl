// Copyright (C) 2009-2020, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

// Screen space ray marching

#pragma once

#include <anki/shaders/Common.glsl>

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
			  U32 randFrom0To3, U32 maxIterations, texture2D hizTex, sampler hizSampler, U32 hizMipCount,
			  UVec2 hizMip0Size, out Vec3 hitPoint, out F32 attenuation)
{
	attenuation = 0.0;

	// Check for view facing reflections [sakibsaikia]
	const Vec3 viewDir = normalize(rayOrigin);
	const F32 cameraContribution = 1.0 - smoothstep(0.25, 0.5, dot(-viewDir, rayDir));
	ANKI_BRANCH if(cameraContribution <= 0.0)
	{
		return;
	}

	// Dither and set starting pos
	const F32 bayerMat[4] = F32[](1.0, 4.0, 2.0, 3.0);
	const Vec3 p0 = rayOrigin + rayDir * (tmin * bayerMat[randFrom0To3]);

	// p1
	const F32 tmax = 10.0;
	const Vec3 p1 = rayOrigin + rayDir * tmax;

	// Compute start & end in clip space (well not clip space since x,y are in [0, 1])
	Vec4 v4 = projMat * Vec4(p0, 1.0);
	Vec3 start = v4.xyz / v4.w;
	start.xy = NDC_TO_UV(start.xy);
	v4 = projMat * Vec4(p1, 1.0);
	Vec3 end = v4.xyz / v4.w;
	end.xy = NDC_TO_UV(end.xy);

	// Ray
	Vec3 origin = start;
	const Vec3 dir = normalize(end - start);

	// Start looping
	I32 mipLevel = 0;
	while(mipLevel > -1 && maxIterations > 0)
	{
		// Step to the next cell
		Vec3 newOrigin;
		stepToNextCell(origin, dir, U32(mipLevel), hizMip0Size, newOrigin);
		origin = newOrigin;

		if(all(greaterThan(origin.xy, Vec2(0.0))) && all(lessThan(origin.xy, Vec2(1.0))))
		{
			const F32 newDepth = textureLod(hizTex, hizSampler, origin.xy, F32(mipLevel)).r;

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
	const Vec2 marginAttenuation2d = smoothstep(blackMargin, whiteMargin, origin.xy)
									 * (1.0 - smoothstep(1.0 - whiteMargin, 1.0 - blackMargin, origin.xy));
	const F32 marginAttenuation = marginAttenuation2d.x * marginAttenuation2d.y;
	attenuation = marginAttenuation * cameraContribution;

	hitPoint = origin;
}

// Note: All calculations in view space
void raymarchGroundTruth(Vec3 rayOrigin, // Ray origin in view space
						 Vec3 rayDir, // Ray dir in view space
						 Vec2 uv, // UV the ray starts
						 F32 depthRef, // Depth the ray starts
						 Mat4 projMat, // Projection matrix
						 U32 maxIterations, texture2D depthTex, sampler depthSampler, F32 depthLod, UVec2 depthTexSize,
						 U32 bigStep, U32 randInitialStep, out Vec3 hitPoint, out F32 attenuation)
{
	attenuation = 0.0;

	// Check for view facing reflections [sakibsaikia]
	const Vec3 viewDir = normalize(rayOrigin);
	const F32 cameraContribution = 1.0 - smoothstep(0.25, 0.5, dot(-viewDir, rayDir));
	if(cameraContribution <= 0.0)
	{
		return;
	}

	// Start point
	const Vec3 p0 = rayOrigin;
	const Vec3 start = Vec3(uv, depthRef);

	// Project end point
	const Vec3 p1 = rayOrigin + rayDir * 0.1;
	const Vec4 end4 = projMat * Vec4(p1, 1.0);
	Vec3 end = end4.xyz / end4.w;
	end.xy = NDC_TO_UV(end.xy);

	// Compute the ray and step size
	Vec3 dir = end - start;
	const Vec2 texelSize = abs(dir.xy) * Vec2(depthTexSize);
	const F32 stepSize = length(dir.xy) / max(texelSize.x, texelSize.y);
	dir = normalize(dir);

	// Compute step
	I32 stepSkip = I32(bigStep);
	I32 step = I32(randInitialStep);

	// Iterate
	Vec3 origin;
	ANKI_LOOP while(maxIterations-- != 0)
	{
		origin = start + dir * (F32(step) * stepSize);

		// Check if it's out of the view
		if(origin.x <= 0.0 || origin.y <= 0.0 || origin.x >= 1.0 || origin.y >= 1.0)
		{
			break;
		}

		const F32 depth = textureLod(depthTex, depthSampler, origin.xy, depthLod).r;
		const Bool hit = origin.z - depth >= 0.0;
		if(!hit)
		{
			step += stepSkip;
		}
		else if(stepSkip > 1)
		{
			step = max(1, step - stepSkip + 1);
			stepSkip = stepSkip / 2;
		}
		else
		{
			// Found it

			// Compute attenuation
			const F32 blackMargin = 0.05 / 4.0;
			const F32 whiteMargin = 0.1 / 2.0;
			const Vec2 marginAttenuation2d = smoothstep(blackMargin, whiteMargin, origin.xy)
											 * (1.0 - smoothstep(1.0 - whiteMargin, 1.0 - blackMargin, origin.xy));
			const F32 marginAttenuation = marginAttenuation2d.x * marginAttenuation2d.y;
			attenuation = marginAttenuation * cameraContribution;

			// ...and hit point
			hitPoint = origin;
			break;
		}
	}
}

void rejectBackFaces(Vec3 reflection, Vec3 normalAtHitPoint, out F32 attenuation)
{
	attenuation = smoothstep(-0.17, 0.0, dot(normalAtHitPoint, -reflection));
}
