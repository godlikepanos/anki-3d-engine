// Copyright (C) 2009-2022, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <AnKi/Importer/GltfImporter.h>
#include <AnKi/Util/Xml.h>

namespace anki {

template<typename T>
class GltfAnimKey
{
public:
	Second m_time;
	T m_value;
};

class GltfAnimChannel
{
public:
	StringRaii m_name;
	DynamicArrayRaii<GltfAnimKey<Vec3>> m_positions;
	DynamicArrayRaii<GltfAnimKey<Quat>> m_rotations;
	DynamicArrayRaii<GltfAnimKey<F32>> m_scales;
	const cgltf_node* m_targetNode;

	GltfAnimChannel(BaseMemoryPool* pool)
		: m_name(pool)
		, m_positions(pool)
		, m_rotations(pool)
		, m_scales(pool)
	{
	}
};

/// Optimize out same animation keys.
template<typename T, typename TZeroFunc, typename TLerpFunc>
static void optimizeChannel(DynamicArrayRaii<GltfAnimKey<T>>& arr, const T& identity, TZeroFunc isZeroFunc,
							TLerpFunc lerpFunc)
{
	constexpr F32 kMinSkippedToTotalRatio = 0.1f;

	U32 iterationCount = 0;
	while(true)
	{
		if(arr.getSize() < 3)
		{
			break;
		}

		DynamicArrayRaii<GltfAnimKey<T>> newArr(&arr.getMemoryPool());
		for(U32 i = 0; i < arr.getSize() - 2; i += 2)
		{
			const GltfAnimKey<T>& left = arr[i];
			const GltfAnimKey<T>& middle = arr[i + 1];
			const GltfAnimKey<T>& right = arr[i + 2];

			newArr.emplaceBack(left);

			if(left.m_value == middle.m_value && middle.m_value == right.m_value)
			{
				// Skip it
			}
			else
			{
				const F32 factor = F32((middle.m_time - left.m_time) / (right.m_time - left.m_time));
				ANKI_ASSERT(factor > 0.0f && factor < 1.0f);
				const T lerpRez = lerpFunc(left.m_value, right.m_value, factor);
				if(isZeroFunc(middle.m_value - lerpRez))
				{
					// It's redundant, skip it
				}
				else
				{
					newArr.emplaceBack(middle);
				}
			}

			newArr.emplaceBack(right);
		}
		ANKI_ASSERT(newArr.getSize() <= arr.getSize());

		// Check if identity
		if(newArr.getSize() == 2 && isZeroFunc(newArr[0].m_value - newArr[1].m_value)
		   && isZeroFunc(newArr[0].m_value - identity))
		{
			newArr.destroy();
		}

		const F32 skippedToTotalRatio = 1.0f - F32(newArr.getSize()) / F32(arr.getSize());

		arr.destroy();
		arr = std::move(newArr);

		++iterationCount;

		if(skippedToTotalRatio <= kMinSkippedToTotalRatio)
		{
			break;
		}
	}

	ANKI_IMPORTER_LOGV("Channel optimization iteration count: %u", iterationCount);
}

Error GltfImporter::writeAnimation(const cgltf_animation& anim)
{
	StringRaii fname(m_pool);
	StringRaii animFname = computeAnimationResourceFilename(anim);
	fname.sprintf("%s%s", m_outDir.cstr(), animFname.cstr());
	fname = fixFilename(fname);
	ANKI_IMPORTER_LOGV("Importing animation %s", fname.cstr());

	// Gather the channels
	HashMapRaii<CString, Array<const cgltf_animation_channel*, 3>> channelMap(m_pool);
	U32 channelCount = 0;
	for(U i = 0; i < anim.channels_count; ++i)
	{
		const cgltf_animation_channel& channel = anim.channels[i];
		const StringRaii channelName = getNodeName(*channel.target_node);

		U idx;
		switch(channel.target_path)
		{
		case cgltf_animation_path_type_translation:
			idx = 0;
			break;
		case cgltf_animation_path_type_rotation:
			idx = 1;
			break;
		case cgltf_animation_path_type_scale:
			idx = 2;
			break;
		default:
			ANKI_ASSERT(0);
			idx = 0;
		}

		auto it = channelMap.find(channelName.toCString());
		if(it != channelMap.getEnd())
		{
			(*it)[idx] = &channel;
		}
		else
		{
			Array<const cgltf_animation_channel*, 3> arr = {};
			arr[idx] = &channel;
			channelMap.emplace(channelName.toCString(), arr);
			++channelCount;
		}
	}

	// Gather the keys
	DynamicArrayRaii<GltfAnimChannel> tempChannels(m_pool, channelCount, m_pool);
	channelCount = 0;
	for(auto it = channelMap.getBegin(); it != channelMap.getEnd(); ++it)
	{
		Array<const cgltf_animation_channel*, 3> arr = *it;
		const cgltf_animation_channel& anyChannel = (arr[0]) ? *arr[0] : ((arr[1]) ? *arr[1] : *arr[2]);
		const StringRaii channelName = getNodeName(*anyChannel.target_node);

		tempChannels[channelCount].m_name = channelName;
		tempChannels[channelCount].m_targetNode = anyChannel.target_node;

		// Positions
		if(arr[0])
		{
			const cgltf_animation_channel& channel = *arr[0];
			DynamicArrayRaii<F32> keys(m_pool);
			readAccessor(*channel.sampler->input, keys);
			DynamicArrayRaii<Vec3> positions(m_pool);
			readAccessor(*channel.sampler->output, positions);
			if(keys.getSize() != positions.getSize())
			{
				ANKI_IMPORTER_LOGE("Position count should match they keyframes");
				return Error::kUserData;
			}

			for(U32 i = 0; i < keys.getSize(); ++i)
			{
				GltfAnimKey<Vec3> key;
				key.m_time = keys[i];
				key.m_value = Vec3(positions[i].x(), positions[i].y(), positions[i].z());

				tempChannels[channelCount].m_positions.emplaceBack(key);
			}
		}

		// Rotations
		if(arr[1])
		{
			const cgltf_animation_channel& channel = *arr[1];
			DynamicArrayRaii<F32> keys(m_pool);
			readAccessor(*channel.sampler->input, keys);
			DynamicArrayRaii<Quat> rotations(m_pool);
			readAccessor(*channel.sampler->output, rotations);
			if(keys.getSize() != rotations.getSize())
			{
				ANKI_IMPORTER_LOGE("Rotation count should match they keyframes");
				return Error::kUserData;
			}

			for(U32 i = 0; i < keys.getSize(); ++i)
			{
				GltfAnimKey<Quat> key;
				key.m_time = keys[i];
				key.m_value = Quat(rotations[i].x(), rotations[i].y(), rotations[i].z(), rotations[i].w());

				tempChannels[channelCount].m_rotations.emplaceBack(key);
			}
		}

		// Scales
		if(arr[2])
		{
			const cgltf_animation_channel& channel = *arr[2];
			DynamicArrayRaii<F32> keys(m_pool);
			readAccessor(*channel.sampler->input, keys);
			DynamicArrayRaii<Vec3> scales(m_pool);
			readAccessor(*channel.sampler->output, scales);
			if(keys.getSize() != scales.getSize())
			{
				ANKI_IMPORTER_LOGE("Scale count should match they keyframes");
				return Error::kUserData;
			}

			Bool scaleErrorReported = false;
			for(U32 i = 0; i < keys.getSize(); ++i)
			{
				const F32 scaleEpsilon = 0.0001f;

				// Normalize the scale because scaleEpsilon is relative
				Vec3 scale = scales[i];
				scale.normalize();

				if(!scaleErrorReported
				   && (absolute(scale[0] - scale[1]) > scaleEpsilon || absolute(scale[0] - scale[2]) > scaleEpsilon))
				{
					ANKI_IMPORTER_LOGW("Expecting uniform scale (%f %f %f)", scales[i].x(), scales[i].y(),
									   scales[i].z());
					scaleErrorReported = true;
				}

				GltfAnimKey<F32> key;
				key.m_time = keys[i];
				key.m_value = scales[i][0];

				if(absolute(key.m_value - 1.0f) <= scaleEpsilon)
				{
					key.m_value = 1.0f;
				}

				tempChannels[channelCount].m_scales.emplaceBack(key);
			}
		}

		++channelCount;
	}

	// Optimize animation
	if(m_optimizeAnimations)
	{
		constexpr F32 kKillEpsilon = 1.0_cm;
		for(GltfAnimChannel& channel : tempChannels)
		{
			optimizeChannel(
				channel.m_positions, Vec3(0.0f),
				[&](const Vec3& a) -> Bool {
					return a.abs() < kKillEpsilon;
				},
				[&](const Vec3& a, const Vec3& b, F32 u) -> Vec3 {
					return linearInterpolate(a, b, u);
				});
			optimizeChannel(
				channel.m_rotations, Quat::getIdentity(),
				[&](const Quat& a) -> Bool {
					return a.abs() < Quat(kEpsilonf * 20.0f);
				},
				[&](const Quat& a, const Quat& b, F32 u) -> Quat {
					return a.slerp(b, u);
				});
			optimizeChannel(
				channel.m_scales, 1.0f,
				[&](const F32& a) -> Bool {
					return absolute(a) < kKillEpsilon;
				},
				[&](const F32& a, const F32& b, F32 u) -> F32 {
					return linearInterpolate(a, b, u);
				});
		}
	}

	// Write file
	File file;
	ANKI_CHECK(file.open(fname.toCString(), FileOpenFlag::kWrite));

	ANKI_CHECK(file.writeTextf("%s\n<animation>\n", XmlDocument::kXmlHeader.cstr()));
	ANKI_CHECK(file.writeText("\t<channels>\n"));

	for(const GltfAnimChannel& channel : tempChannels)
	{
		ANKI_CHECK(file.writeTextf("\t\t<channel name=\"%s\">\n", channel.m_name.cstr()));

		// Positions
		if(channel.m_positions.getSize())
		{
			ANKI_CHECK(file.writeText("\t\t\t<positionKeys>\n"));
			for(const GltfAnimKey<Vec3>& key : channel.m_positions)
			{
				ANKI_CHECK(file.writeTextf("\t\t\t\t<key time=\"%f\">%f %f %f</key>\n", key.m_time, key.m_value.x(),
										   key.m_value.y(), key.m_value.z()));
			}
			ANKI_CHECK(file.writeText("\t\t\t</positionKeys>\n"));
		}

		// Rotations
		if(channel.m_rotations.getSize())
		{
			ANKI_CHECK(file.writeText("\t\t\t<rotationKeys>\n"));
			for(const GltfAnimKey<Quat>& key : channel.m_rotations)
			{
				ANKI_CHECK(file.writeTextf("\t\t\t\t<key time=\"%f\">%f %f %f %f</key>\n", key.m_time, key.m_value.x(),
										   key.m_value.y(), key.m_value.z(), key.m_value.w()));
			}
			ANKI_CHECK(file.writeText("\t\t\t</rotationKeys>\n"));
		}

		// Scales
		if(channel.m_scales.getSize())
		{
			ANKI_CHECK(file.writeText("\t\t\t<scaleKeys>\n"));
			for(const GltfAnimKey<F32>& key : channel.m_scales)
			{
				ANKI_CHECK(file.writeTextf("\t\t\t\t<key time=\"%f\">%f</key>\n", key.m_time, key.m_value));
			}
			ANKI_CHECK(file.writeText("\t\t\t</scaleKeys>\n"));
		}

		ANKI_CHECK(file.writeText("\t\t</channel>\n"));
	}

	ANKI_CHECK(file.writeText("\t</channels>\n"));
	ANKI_CHECK(file.writeText("</animation>\n"));

	// Hook up the animation to the scene
	for(const GltfAnimChannel& channel : tempChannels)
	{
		if(channel.m_targetNode == nullptr)
		{
			continue;
		}

		// Only animate cameras for now
		const cgltf_node& node = *channel.m_targetNode;
		if((node.camera == nullptr) || node.name == nullptr)
		{
			continue;
		}

		// ANKI_CHECK(m_sceneFile.writeText("--[[\n"));

		ANKI_CHECK(m_sceneFile.writeTextf("\nnode = scene:tryFindSceneNode(\"%s\")\n", node.name));
		ANKI_CHECK(m_sceneFile.writeTextf("getEventManager():newAnimationEvent(\"%s%s\", \"%s\", node)\n",
										  m_rpath.cstr(), animFname.cstr(), node.name));

		// ANKI_CHECK(m_sceneFile.writeText("--]]\n"));
	}

	return Error::kNone;
}

} // end namespace anki
