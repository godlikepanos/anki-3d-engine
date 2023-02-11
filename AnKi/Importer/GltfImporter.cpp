// Copyright (C) 2009-2022, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <AnKi/Importer/GltfImporter.h>
#include <AnKi/Util/System.h>
#include <AnKi/Util/ThreadHive.h>
#include <AnKi/Util/StringList.h>
#include <AnKi/Util/Xml.h>

#if ANKI_COMPILER_GCC_COMPATIBLE
#	pragma GCC diagnostic push
#	pragma GCC diagnostic ignored "-Wconversion"
#endif
#define CGLTF_IMPLEMENTATION
#include <Cgltf/cgltf.h>
#if ANKI_COMPILER_GCC_COMPATIBLE
#	pragma GCC diagnostic pop
#endif

namespace anki {

static F32 computeLightRadius(const Vec3 color)
{
	// Based on the attenuation equation: att = 1 - fragLightDist^2 / lightRadius^2
	const F32 minAtt = 0.01f;
	const F32 maxIntensity = max(max(color.x(), color.y()), color.z());
	return sqrt(maxIntensity / minAtt);
}

#if 0
static Error getUniformScale(const Mat4& m, F32& out)
{
	const F32 SCALE_THRESHOLD = 0.01f; // 1 cm

	Vec3 xAxis = m.getColumn(0).xyz();
	Vec3 yAxis = m.getColumn(1).xyz();
	Vec3 zAxis = m.getColumn(2).xyz();

	const F32 scale = xAxis.getLength();
	if(absolute(scale - yAxis.getLength()) > SCALE_THRESHOLD || absolute(scale - zAxis.getLength()) > SCALE_THRESHOLD)
	{
		ANKI_IMPORTER_LOGE("No uniform scale in the matrix");
		return Error::kUserData;
	}

	out = scale;
	return Error::kNone;
}
#endif

static void removeScale(Mat4& m)
{
	Vec3 xAxis = m.getColumn(0).xyz();
	Vec3 yAxis = m.getColumn(1).xyz();
	Vec3 zAxis = m.getColumn(2).xyz();

	xAxis.normalize();
	yAxis.normalize();
	zAxis.normalize();

	Mat3 rot;
	rot.setColumns(xAxis, yAxis, zAxis);
	m.setRotationPart(rot);
}

static void getNodeTransform(const cgltf_node& node, Vec3& tsl, Mat3& rot, Vec3& scale)
{
	if(node.has_matrix)
	{
		Mat4 trf = Mat4(node.matrix);

		Vec3 xAxis = trf.getColumn(0).xyz();
		Vec3 yAxis = trf.getColumn(1).xyz();
		Vec3 zAxis = trf.getColumn(2).xyz();

		scale = Vec3(xAxis.getLength(), yAxis.getLength(), zAxis.getLength());

		removeScale(trf);
		rot = trf.getRotationPart();
		tsl = trf.getTranslationPart().xyz();
	}
	else
	{
		if(node.has_translation)
		{
			tsl = Vec3(node.translation[0], node.translation[1], node.translation[2]);
		}
		else
		{
			tsl = Vec3(0.0f);
		}

		if(node.has_rotation)
		{
			rot = Mat3(Quat(node.rotation[0], node.rotation[1], node.rotation[2], node.rotation[3]));
		}
		else
		{
			rot = Mat3::getIdentity();
		}

		if(node.has_scale)
		{
			ANKI_ASSERT(node.scale[0] > 0.0f);
			ANKI_ASSERT(node.scale[1] > 0.0f);
			ANKI_ASSERT(node.scale[2] > 0.0f);
			scale = Vec3(node.scale[0], node.scale[1], node.scale[2]);
		}
		else
		{
			scale = Vec3(1.0f);
		}
	}
}

static Error getNodeTransform(const cgltf_node& node, Transform& trf)
{
	Vec3 tsl;
	Mat3 rot;
	Vec3 scale;
	getNodeTransform(node, tsl, rot, scale);

	const F32 scaleEpsilon = 0.01f;
	if(absolute(scale[0] - scale[1]) > scaleEpsilon || absolute(scale[0] - scale[2]) > scaleEpsilon)
	{
		ANKI_IMPORTER_LOGE("Expecting uniform scale");
		return Error::kUserData;
	}

	trf.setOrigin(tsl.xyz0());
	trf.setRotation(Mat3x4(Vec3(0.0f), rot));
	trf.setScale(scale[0]);

	return Error::kNone;
}

static Bool stringsExist(const HashMapRaii<CString, StringRaii>& map, const std::initializer_list<CString>& list)
{
	for(CString item : list)
	{
		if(map.find(item) != map.getEnd())
		{
			return true;
		}
	}

	return false;
}

GltfImporter::GltfImporter(BaseMemoryPool* pool)
	: m_pool(pool)
{
}

GltfImporter::~GltfImporter()
{
	if(m_gltf)
	{
		cgltf_free(m_gltf);
		m_gltf = nullptr;
	}

	deleteInstance(*m_pool, m_hive);
}

Error GltfImporter::init(const GltfImporterInitInfo& initInfo)
{
	m_inputFname.create(initInfo.m_inputFilename);
	m_outDir.create(initInfo.m_outDirectory);
	m_rpath.create(initInfo.m_rpath);
	m_texrpath.create(initInfo.m_texrpath);
	m_optimizeMeshes = initInfo.m_optimizeMeshes;
	m_optimizeAnimations = initInfo.m_optimizeAnimations;
	m_comment.create(initInfo.m_comment);

	m_lightIntensityScale = max(initInfo.m_lightIntensityScale, kEpsilonf);

	m_lodCount = clamp(initInfo.m_lodCount, 1u, 3u);
	m_lodFactor = clamp(initInfo.m_lodFactor, 0.0f, 1.0f);
	if(m_lodFactor * F32(m_lodCount - 1) > 0.7f)
	{
		ANKI_IMPORTER_LOGE("LOD factor is too high %f", m_lodFactor);
		return Error::kUserData;
	}

	if(m_lodFactor < kEpsilonf || m_lodCount == 1)
	{
		m_lodCount = 1;
		m_lodFactor = 0.0f;
	}

	ANKI_IMPORTER_LOGV("Having %u LODs with LOD factor %f", m_lodCount, m_lodFactor);

	cgltf_options options = {};
	cgltf_result res = cgltf_parse_file(&options, m_inputFname.cstr(), &m_gltf);
	if(res != cgltf_result_success)
	{
		ANKI_IMPORTER_LOGE("Failed to open the GLTF file. Code: %d", res);
		return Error::kFunctionFailed;
	}

	res = cgltf_load_buffers(&options, m_gltf, m_inputFname.cstr());
	if(res != cgltf_result_success)
	{
		ANKI_IMPORTER_LOGE("Failed to load GLTF data. Code: %d", res);
		return Error::kFunctionFailed;
	}

	if(initInfo.m_threadCount > 0)
	{
		const U32 threadCount = min(getCpuCoresCount(), initInfo.m_threadCount);
		m_hive = newInstance<ThreadHive>(*m_pool, threadCount, m_pool, true);
	}

	m_importTextures = initInfo.m_importTextures;

	return Error::kNone;
}

Error GltfImporter::writeAll()
{
	populateNodePtrToIdx();

	StringRaii sceneFname(m_pool);
	sceneFname.sprintf("%sScene.lua", m_outDir.cstr());
	ANKI_CHECK(m_sceneFile.open(sceneFname.toCString(), FileOpenFlag::kWrite));
	ANKI_CHECK(m_sceneFile.writeTextf("-- Generated by: %s\n", m_comment.cstr()));
	ANKI_CHECK(m_sceneFile.writeText("local scene = getSceneGraph()\nlocal events = getEventManager()\n"));

	// Nodes
	for(const cgltf_scene* scene = m_gltf->scenes; scene < m_gltf->scenes + m_gltf->scenes_count; ++scene)
	{
		for(cgltf_node* const* node = scene->nodes; node < scene->nodes + scene->nodes_count; ++node)
		{
			ANKI_CHECK(visitNode(*(*node), Transform::getIdentity(), HashMapRaii<CString, StringRaii>(m_pool)));
		}
	}

	// Fire up all requests
	for(auto& req : m_meshImportRequests)
	{
		if(m_hive)
		{
			m_hive->submitTask(
				[](void* userData, [[maybe_unused]] U32 threadId, [[maybe_unused]] ThreadHive& hive,
				   [[maybe_unused]] ThreadHiveSemaphore* signalSemaphore) {
					ImportRequest<const cgltf_mesh*>* req = static_cast<ImportRequest<const cgltf_mesh*>*>(userData);
					Error err = req->m_importer->m_errorInThread.load();

					if(!err)
					{
						err = req->m_importer->writeMesh(*req->m_value);
					}

					if(err)
					{
						req->m_importer->m_errorInThread.store(err._getCode());
					}
				},
				&req);
		}
		else
		{
			ANKI_CHECK(writeMesh(*req.m_value));
		}
	}

	for(auto& req : m_materialImportRequests)
	{
		if(m_hive)
		{
			m_hive->submitTask(
				[](void* userData, [[maybe_unused]] U32 threadId, [[maybe_unused]] ThreadHive& hive,
				   [[maybe_unused]] ThreadHiveSemaphore* signalSemaphore) {
					ImportRequest<MaterialImportRequest>* req =
						static_cast<ImportRequest<MaterialImportRequest>*>(userData);
					Error err = req->m_importer->m_errorInThread.load();

					if(!err)
					{
						err = req->m_importer->writeMaterial(*req->m_value.m_cgltfMaterial, req->m_value.m_writeRt);
					}

					if(err)
					{
						req->m_importer->m_errorInThread.store(err._getCode());
					}
				},
				&req);
		}
		else
		{
			ANKI_CHECK(writeMaterial(*req.m_value.m_cgltfMaterial, req.m_value.m_writeRt));
		}
	}

	for(auto& req : m_skinImportRequests)
	{
		if(m_hive)
		{
			m_hive->submitTask(
				[](void* userData, [[maybe_unused]] U32 threadId, [[maybe_unused]] ThreadHive& hive,
				   [[maybe_unused]] ThreadHiveSemaphore* signalSemaphore) {
					ImportRequest<const cgltf_skin*>* req = static_cast<ImportRequest<const cgltf_skin*>*>(userData);
					Error err = req->m_importer->m_errorInThread.load();

					if(!err)
					{
						err = req->m_importer->writeSkeleton(*req->m_value);
					}

					if(err)
					{
						req->m_importer->m_errorInThread.store(err._getCode());
					}
				},
				&req);
		}
		else
		{
			ANKI_CHECK(writeSkeleton(*req.m_value));
		}
	}

	for(auto& req : m_modelImportRequests)
	{
		if(m_hive)
		{
			m_hive->submitTask(
				[](void* userData, [[maybe_unused]] U32 threadId, [[maybe_unused]] ThreadHive& hive,
				   [[maybe_unused]] ThreadHiveSemaphore* signalSemaphore) {
					ImportRequest<const cgltf_mesh*>* req = static_cast<ImportRequest<const cgltf_mesh*>*>(userData);
					Error err = req->m_importer->m_errorInThread.load();

					if(!err)
					{
						err = req->m_importer->writeModel(*req->m_value);
					}

					if(err)
					{
						req->m_importer->m_errorInThread.store(err._getCode());
					}
				},
				&req);
		}
		else
		{
			ANKI_CHECK(writeModel(*req.m_value));
		}
	}

	if(m_hive)
	{
		m_hive->waitAllTasks();

		const Error threadErr = m_errorInThread.load();
		if(threadErr)
		{
			ANKI_IMPORTER_LOGE("Error happened in a thread");
			return threadErr;
		}
	}

	// Animations
	for(const cgltf_animation* anim = m_gltf->animations; anim < m_gltf->animations + m_gltf->animations_count; ++anim)
	{
		ANKI_CHECK(writeAnimation(*anim));
	}

	ANKI_IMPORTER_LOGV("Importing GLTF has completed");
	return Error::kNone;
}

Error GltfImporter::getExtras(const cgltf_extras& extras, HashMapRaii<CString, StringRaii>& out) const
{
	cgltf_size extrasSize;
	cgltf_copy_extras_json(m_gltf, &extras, nullptr, &extrasSize);
	if(extrasSize == 0)
	{
		return Error::kNone;
	}

	DynamicArrayRaii<char, PtrSize> json(m_pool);
	json.create(extrasSize + 1);
	cgltf_result res = cgltf_copy_extras_json(m_gltf, &extras, &json[0], &extrasSize);
	if(res != cgltf_result_success)
	{
		ANKI_IMPORTER_LOGE("cgltf_copy_extras_json failed: %d", res);
		return Error::kFunctionFailed;
	}

	json[json.getSize() - 1] = '\0';

	// Get token count
	CString jsonTxt(&json[0]);
	jsmn_parser parser;
	jsmn_init(&parser);
	const I tokenCount = jsmn_parse(&parser, jsonTxt.cstr(), jsonTxt.getLength(), nullptr, 0);
	if(tokenCount < 1)
	{
		return Error::kNone;
	}

	DynamicArrayRaii<jsmntok_t> tokens(m_pool);
	tokens.create(U32(tokenCount));

	// Get tokens
	jsmn_init(&parser);
	jsmn_parse(&parser, jsonTxt.cstr(), jsonTxt.getLength(), &tokens[0], tokens.getSize());

	StringListRaii tokenStrings(m_pool);
	for(const jsmntok_t& token : tokens)
	{
		if(token.type != JSMN_STRING && token.type != JSMN_PRIMITIVE)
		{
			continue;
		}

		StringRaii tokenStr(m_pool);
		tokenStr.create(&jsonTxt[token.start], &jsonTxt[token.end]);
		tokenStrings.pushBack(tokenStr.toCString());
	}

	if((tokenStrings.getSize() % 2) != 0)
	{
		ANKI_IMPORTER_LOGE("Unable to parse: %s", jsonTxt.cstr());
		return Error::kFunctionFailed;
	}

	// Write them to the map
	auto it = tokenStrings.getBegin();
	while(it != tokenStrings.getEnd())
	{
		auto it2 = it;
		++it2;

		out.emplace(it->toCString(), StringRaii(m_pool, it2->toCString()));
		++it;
		++it;
	}

	return Error::kNone;
}

void GltfImporter::populateNodePtrToIdxInternal(const cgltf_node& node, U32& idx)
{
	m_nodePtrToIdx.emplace(&node, idx++);

	for(cgltf_node* const* c = node.children; c < node.children + node.children_count; ++c)
	{
		populateNodePtrToIdxInternal(**c, idx);
	}
}

void GltfImporter::populateNodePtrToIdx()
{
	U32 idx = 0;

	for(const cgltf_scene* scene = m_gltf->scenes; scene < m_gltf->scenes + m_gltf->scenes_count; ++scene)
	{
		for(cgltf_node* const* node = scene->nodes; node < scene->nodes + scene->nodes_count; ++node)
		{
			populateNodePtrToIdxInternal(**node, idx);
		}
	}
}

StringRaii GltfImporter::getNodeName(const cgltf_node& node) const
{
	StringRaii out(m_pool);

	if(node.name)
	{
		out.create(node.name);
	}
	else
	{
		auto it = m_nodePtrToIdx.find(&node);
		ANKI_ASSERT(it != m_nodePtrToIdx.getEnd());
		out.sprintf("unnamed_node_%u", *it);
	}

	return out;
}

Error GltfImporter::parseArrayOfNumbers(CString str, DynamicArrayRaii<F64>& out, const U32* expectedArraySize)
{
	StringListRaii list(m_pool);
	list.splitString(str, ' ');

	out.create(U32(list.getSize()));

	Error err = Error::kNone;
	auto it = list.getBegin();
	auto end = list.getEnd();
	U32 i = 0;
	while(it != end && !err)
	{
		err = it->toNumber(out[i++]);
		++it;
	}

	if(err)
	{
		ANKI_IMPORTER_LOGE("Failed to parse floats: %s", str.cstr());
	}

	if(expectedArraySize && *expectedArraySize != out.getSize())
	{
		ANKI_IMPORTER_LOGE("Failed to parse floats. Expecting %u floats got %u: %s", *expectedArraySize, out.getSize(),
						   str.cstr());
	}

	return Error::kNone;
}

Error GltfImporter::visitNode(const cgltf_node& node, const Transform& parentTrf,
							  const HashMapRaii<CString, StringRaii>& parentExtras)
{
	// Check error from a thread
	const Error threadErr = m_errorInThread.load();
	if(threadErr)
	{
		ANKI_IMPORTER_LOGE("Error happened in a thread");
		return threadErr;
	}

	HashMapRaii<CString, StringRaii> outExtras(m_pool);
	if(node.light)
	{
		ANKI_CHECK(writeLight(node, parentExtras));

		Transform localTrf;
		ANKI_CHECK(getNodeTransform(node, localTrf));
		localTrf.setScale(1.0f); // Remove scale
		ANKI_CHECK(writeTransform(parentTrf.combineTransformations(localTrf)));
	}
	else if(node.camera)
	{
		ANKI_CHECK(writeCamera(node, parentExtras));

		Transform localTrf;
		ANKI_CHECK(getNodeTransform(node, localTrf));
		localTrf.setScale(1.0f); // Remove scale
		ANKI_CHECK(writeTransform(parentTrf.combineTransformations(localTrf)));
	}
	else if(node.mesh)
	{
		// Handle special nodes
		HashMapRaii<CString, StringRaii> extras(parentExtras);
		ANKI_CHECK(getExtras(node.mesh->extras, extras));
		ANKI_CHECK(getExtras(node.extras, extras));

		HashMapRaii<CString, StringRaii>::Iterator it;

		if((it = extras.find("particles")) != extras.getEnd())
		{
			const StringRaii& fname = *it;

			Bool gpuParticles = false;
			if((it = extras.find("gpu_particles")) != extras.getEnd() && *it == "true")
			{
				gpuParticles = true;
			}

			if(!gpuParticles) // TODO Re-enable GPU particles
			{
				ANKI_CHECK(m_sceneFile.writeTextf("\nnode = scene:newSceneNode(\"%s\")\n", getNodeName(node).cstr()));

				ANKI_CHECK(m_sceneFile.writeTextf("comp = node:newParticleEmitterComponent()\n"));
				ANKI_CHECK(m_sceneFile.writeTextf("comp:loadParticleEmitterResource(\"%s\")\n", fname.cstr()));

				Transform localTrf;
				ANKI_CHECK(getNodeTransform(node, localTrf));
				ANKI_CHECK(writeTransform(parentTrf.combineTransformations(localTrf)));
			}
		}
		else if(stringsExist(extras, {"skybox_solid_color", "skybox_image", "fog_min_density", "fog_max_density",
									  "fog_height_of_min_density", "fog_height_of_max_density"}))
		{
			// Atmosphere

			ANKI_CHECK(m_sceneFile.writeTextf("\nnode = scene:newSceneNode(\"%s\")\n", getNodeName(node).cstr()));
			ANKI_CHECK(m_sceneFile.writeText("comp = node:newSkyboxComponent()\n"));

			if((it = extras.find("skybox_solid_color")) != extras.getEnd())
			{
				StringListRaii tokens(m_pool);
				tokens.splitString(*it, ' ');
				if(tokens.getSize() != 3)
				{
					ANKI_IMPORTER_LOGE("Error parsing \"skybox_solid_color\" of node %s", getNodeName(node).cstr());
					return Error::kUserData;
				}

				U count = 0;
				Vec3 solidColor(0.0f);
				for(auto& it : tokens)
				{
					F32 f;
					const Error err = it.toNumber(f);
					if(err)
					{
						ANKI_IMPORTER_LOGE("Error parsing \"skybox_solid_color\" of node %s", getNodeName(node).cstr());
						return Error::kUserData;
					}

					solidColor[count++] = f;
				}

				ANKI_CHECK(m_sceneFile.writeTextf("comp:setSolidColor(Vec3.new(%f, %f, %f))\n", solidColor.x(),
												  solidColor.y(), solidColor.z()));
			}
			else if((it = extras.find("skybox_image")) != extras.getEnd())
			{
				ANKI_CHECK(m_sceneFile.writeTextf("comp:loadImageResource(\"%s\")\n", it->cstr()));
			}

			if((it = extras.find("fog_min_density")) != extras.getEnd())
			{
				F32 val;
				ANKI_CHECK(it->toNumber(val));
				ANKI_CHECK(m_sceneFile.writeTextf("comp:setMinFogDensity(\"%f\")\n", val));
			}

			if((it = extras.find("fog_max_density")) != extras.getEnd())
			{
				F32 val;
				ANKI_CHECK(it->toNumber(val));
				ANKI_CHECK(m_sceneFile.writeTextf("comp:setMaxFogDensity(\"%f\")\n", val));
			}

			if((it = extras.find("fog_height_of_min_density")) != extras.getEnd())
			{
				F32 val;
				ANKI_CHECK(it->toNumber(val));
				ANKI_CHECK(m_sceneFile.writeTextf("comp:setHeightOfMinFogDensity(\"%f\")\n", val));
			}

			if((it = extras.find("fog_height_of_max_density")) != extras.getEnd())
			{
				F32 val;
				ANKI_CHECK(it->toNumber(val));
				ANKI_CHECK(m_sceneFile.writeTextf("comp:setHeightOfMaxFogDensity(\"%f\")\n", val));
			}

			Transform localTrf;
			ANKI_CHECK(getNodeTransform(node, localTrf));
			ANKI_CHECK(writeTransform(parentTrf.combineTransformations(localTrf)));
		}
		else if((it = extras.find("collision")) != extras.getEnd() && (*it == "true" || *it == "1"))
		{
			ANKI_CHECK(m_sceneFile.writeTextf("\nnode = scene:newSceneNode(\"%s\")\n", getNodeName(node).cstr()));

			ANKI_CHECK(m_sceneFile.writeText("comp = scene:newBodyComponent()\n"));
			const StringRaii meshFname = computeMeshResourceFilename(*node.mesh);
			ANKI_CHECK(m_sceneFile.writeTextf("comp:loadMeshResource(\"%s%s\")\n", m_rpath.cstr(), meshFname.cstr()));

			Transform localTrf;
			ANKI_CHECK(getNodeTransform(node, localTrf));
			ANKI_CHECK(writeTransform(parentTrf.combineTransformations(localTrf)));
		}
		else if((it = extras.find("reflection_probe")) != extras.getEnd() && (*it == "true" || *it == "1"))
		{
			Vec3 tsl;
			Mat3 rot;
			Vec3 scale;
			getNodeTransform(node, tsl, rot, scale);

			const Vec3 boxSize = scale * 2.0f;

			ANKI_CHECK(m_sceneFile.writeTextf("\nnode = scene:newSceneNode(\"%s\")\n", getNodeName(node).cstr()));

			ANKI_CHECK(m_sceneFile.writeText("comp = node:newReflectionProbeComponent()\n"));
			ANKI_CHECK(m_sceneFile.writeTextf("comp:setBoxVolumeSize(Vec3.new(%f, %f, %f))\n", boxSize.x(), boxSize.y(),
											  boxSize.z()));

			const Transform localTrf = Transform(tsl.xyz0(), Mat3x4(Vec3(0.0f), rot), 1.0f);
			ANKI_CHECK(writeTransform(parentTrf.combineTransformations(localTrf)));
		}
		else if((it = extras.find("gi_probe")) != extras.getEnd() && (*it == "true" || *it == "1"))
		{
			Vec3 tsl;
			Mat3 rot;
			Vec3 scale;
			getNodeTransform(node, tsl, rot, scale);

			const Vec3 boxSize = scale * 2.0f;

			F32 fadeDistance = -1.0f;
			if((it = extras.find("gi_probe_fade_distance")) != extras.getEnd())
			{
				ANKI_CHECK(it->toNumber(fadeDistance));
			}

			F32 cellSize = -1.0f;
			if((it = extras.find("gi_probe_cell_size")) != extras.getEnd())
			{
				ANKI_CHECK(it->toNumber(cellSize));
			}

			ANKI_CHECK(m_sceneFile.writeTextf("\nnode = scene:newSceneNode(\"%s\")\n", getNodeName(node).cstr()));
			ANKI_CHECK(m_sceneFile.writeText("comp = node:newGlobalIlluminationProbeComponent()\n"));
			ANKI_CHECK(m_sceneFile.writeTextf("comp:setBoxVolumeSize(Vec3.new(%f, %f, %f))\n", boxSize.x(), boxSize.y(),
											  boxSize.z()));

			if(fadeDistance > 0.0f)
			{
				ANKI_CHECK(m_sceneFile.writeTextf("comp:setFadeDistance(%f)\n", fadeDistance));
			}

			if(cellSize > 0.0f)
			{
				ANKI_CHECK(m_sceneFile.writeTextf("comp:setCellSize(%f)\n", cellSize));
			}

			const Transform localTrf = Transform(tsl.xyz0(), Mat3x4(Vec3(0.0f), rot), 1.0f);
			ANKI_CHECK(writeTransform(parentTrf.combineTransformations(localTrf)));
		}
		else if((it = extras.find("decal")) != extras.getEnd() && (*it == "true" || *it == "1"))
		{
			StringRaii diffuseAtlas(m_pool);
			if((it = extras.find("decal_diffuse_atlas")) != extras.getEnd())
			{
				diffuseAtlas.create(it->toCString());
			}

			StringRaii diffuseSubtexture(m_pool);
			if((it = extras.find("decal_diffuse_sub_texture")) != extras.getEnd())
			{
				diffuseSubtexture.create(it->toCString());
			}

			F32 diffuseFactor = -1.0f;
			if((it = extras.find("decal_diffuse_factor")) != extras.getEnd())
			{
				ANKI_CHECK(it->toNumber(diffuseFactor));
			}

			StringRaii specularRougnessMetallicAtlas(m_pool);
			if((it = extras.find("decal_specular_roughness_metallic_atlas")) != extras.getEnd())
			{
				specularRougnessMetallicAtlas.create(it->toCString());
			}

			StringRaii specularRougnessMetallicSubtexture(m_pool);
			if((it = extras.find("decal_specular_roughness_metallic_sub_texture")) != extras.getEnd())
			{
				specularRougnessMetallicSubtexture.create(it->toCString());
			}

			F32 specularRougnessMetallicFactor = -1.0f;
			if((it = extras.find("decal_specular_roughness_metallic_factor")) != extras.getEnd())
			{
				ANKI_CHECK(it->toNumber(specularRougnessMetallicFactor));
			}

			ANKI_CHECK(m_sceneFile.writeTextf("\nnode = scene:newSceneNode(\"%s\")\n", getNodeName(node).cstr()));
			ANKI_CHECK(m_sceneFile.writeText("comp = node:newDecalComponent()\n"));
			if(diffuseAtlas)
			{
				ANKI_CHECK(m_sceneFile.writeTextf("comp:loadDiffuseImageResource(\"%s\", %f)\n", diffuseAtlas.cstr(),
												  diffuseFactor));
			}

			if(specularRougnessMetallicAtlas)
			{
				ANKI_CHECK(m_sceneFile.writeTextf("comp:loadRoughnessMetallnessTexture(\"%s\", %f)\n",
												  specularRougnessMetallicAtlas.cstr(),
												  specularRougnessMetallicFactor));
			}

			Vec3 tsl;
			Mat3 rot;
			Vec3 scale;
			getNodeTransform(node, tsl, rot, scale);
			const Transform localTrf = Transform(tsl.xyz0(), Mat3x4(Vec3(0.0f), rot), 1.0f);
			ANKI_CHECK(writeTransform(parentTrf.combineTransformations(localTrf)));
		}
		else
		{
			// Model node

			const Bool skipRt = (it = extras.find("no_rt")) != extras.getEnd() && (*it == "true" || *it == "1");

			addRequest<const cgltf_mesh*>(node.mesh, m_meshImportRequests);
			for(U32 i = 0; i < node.mesh->primitives_count; ++i)
			{
				const MaterialImportRequest req = {node.mesh->primitives[i].material, !skipRt};
				addRequest<MaterialImportRequest>(req, m_materialImportRequests);
			}

			if(node.skin)
			{
				addRequest<const cgltf_skin*>(node.skin, m_skinImportRequests);
			}

			HashMapRaii<CString, StringRaii>::Iterator it2;
			const Bool selfCollision = (it2 = extras.find("collision_mesh")) != extras.getEnd() && *it2 == "self";

			ANKI_CHECK(writeModelNode(node, parentExtras));

			Transform localTrf;
			ANKI_CHECK(getNodeTransform(node, localTrf));
			ANKI_CHECK(writeTransform(parentTrf.combineTransformations(localTrf)));

			if(selfCollision)
			{
				ANKI_CHECK(m_sceneFile.writeText("comp = node:newBodyComponent()\n"));

				const StringRaii meshFname = computeMeshResourceFilename(*node.mesh);

				ANKI_CHECK(m_sceneFile.writeText("comp:setMeshFromModelComponent()\n"));
				ANKI_CHECK(m_sceneFile.writeText("comp:teleportTo(trf)\n"));
			}
		}
	}
	else
	{
		ANKI_IMPORTER_LOGV("Ignoring node %s. Assuming transform node", getNodeName(node).cstr());
		ANKI_CHECK(getExtras(node.extras, outExtras));
	}

	// Visit children
	Transform nodeTrf;
	{
		Vec3 tsl;
		Mat3 rot;
		Vec3 scale;
		getNodeTransform(node, tsl, rot, scale);
		nodeTrf = Transform(tsl.xyz0(), Mat3x4(Vec3(0.0f), rot), scale.x());
	}
	for(cgltf_node* const* c = node.children; c < node.children + node.children_count; ++c)
	{
		ANKI_CHECK(visitNode(*(*c), nodeTrf, outExtras));
	}

	return Error::kNone;
}

Error GltfImporter::writeTransform(const Transform& trf)
{
	ANKI_CHECK(m_sceneFile.writeText("trf = Transform.new()\n"));
	ANKI_CHECK(m_sceneFile.writeTextf("trf:setOrigin(Vec4.new(%f, %f, %f, 0))\n", trf.getOrigin().x(),
									  trf.getOrigin().y(), trf.getOrigin().z()));

	ANKI_CHECK(m_sceneFile.writeText("rot = Mat3x4.new()\n"));
	ANKI_CHECK(m_sceneFile.writeText("rot:setAll("));
	for(U i = 0; i < 12; i++)
	{
		ANKI_CHECK(m_sceneFile.writeTextf((i != 11) ? "%f, " : "%f)\n", trf.getRotation()[i]));
	}
	ANKI_CHECK(m_sceneFile.writeText("trf:setRotation(rot)\n"));

	ANKI_CHECK(m_sceneFile.writeTextf("trf:setScale(%f)\n", trf.getScale()));

	ANKI_CHECK(m_sceneFile.writeText("node:setLocalTransform(trf)\n"));

	return Error::kNone;
}

Error GltfImporter::writeModel(const cgltf_mesh& mesh) const
{
	const StringRaii modelFname = computeModelResourceFilename(mesh);
	ANKI_IMPORTER_LOGV("Importing model %s", modelFname.cstr());

	HashMapRaii<CString, StringRaii> extras(m_pool);
	ANKI_CHECK(getExtras(mesh.extras, extras));

	File file;
	StringRaii modelFullFname(m_pool);
	modelFullFname.sprintf("%s/%s", m_outDir.cstr(), modelFname.cstr());
	ANKI_CHECK(file.open(modelFullFname, FileOpenFlag::kWrite));

	ANKI_CHECK(file.writeText("<model>\n"));
	ANKI_CHECK(file.writeText("\t<modelPatches>\n"));

	for(U32 primIdx = 0; primIdx < mesh.primitives_count; ++primIdx)
	{
		ANKI_CHECK(file.writeText("\t\t<modelPatch>\n"));

		const StringRaii meshFname = computeMeshResourceFilename(mesh);
		if(mesh.primitives_count == 1)
		{
			ANKI_CHECK(file.writeTextf("\t\t\t<mesh>%s%s</mesh>\n", m_rpath.cstr(), meshFname.cstr()));
		}
		else
		{
			ANKI_CHECK(file.writeTextf("\t\t\t<mesh subMeshIndex=\"%u\">%s%s</mesh>\n", primIdx, m_rpath.cstr(),
									   meshFname.cstr()));
		}

		HashMapRaii<CString, StringRaii> materialExtras(m_pool);
		ANKI_CHECK(getExtras(mesh.primitives[primIdx].material->extras, materialExtras));
		auto mtlOverride = materialExtras.find("material_override");
		if(mtlOverride != materialExtras.getEnd())
		{
			ANKI_CHECK(file.writeTextf("\t\t\t<material>%s</material>\n", mtlOverride->cstr()));
		}
		else
		{
			const StringRaii mtlFname = computeMaterialResourceFilename(*mesh.primitives[primIdx].material);
			ANKI_CHECK(file.writeTextf("\t\t\t<material>%s%s</material>\n", m_rpath.cstr(), mtlFname.cstr()));
		}

		ANKI_CHECK(file.writeText("\t\t</modelPatch>\n"));
	}

	ANKI_CHECK(file.writeText("\t</modelPatches>\n"));

	ANKI_CHECK(file.writeText("</model>\n"));

	return Error::kNone;
}

Error GltfImporter::writeSkeleton(const cgltf_skin& skin) const
{
	StringRaii fname(m_pool);
	fname.sprintf("%s%s", m_outDir.cstr(), computeSkeletonResourceFilename(skin).cstr());
	ANKI_IMPORTER_LOGV("Importing skeleton %s", fname.cstr());

	// Get matrices
	DynamicArrayRaii<Mat4> boneMats(m_pool);
	readAccessor(*skin.inverse_bind_matrices, boneMats);
	if(boneMats.getSize() != skin.joints_count)
	{
		ANKI_IMPORTER_LOGE("Bone matrices should match the joint count");
		return Error::kUserData;
	}

	// Write file
	File file;
	ANKI_CHECK(file.open(fname.toCString(), FileOpenFlag::kWrite));

	ANKI_CHECK(file.writeTextf("%s\n<skeleton>\n", XmlDocument::kXmlHeader.cstr()));
	ANKI_CHECK(file.writeTextf("\t<bones>\n"));

	for(U32 i = 0; i < skin.joints_count; ++i)
	{
		const cgltf_node& boneNode = *skin.joints[i];

		StringRaii parent(m_pool);

		// Name & parent
		ANKI_CHECK(file.writeTextf("\t\t<bone name=\"%s\" ", getNodeName(boneNode).cstr()));
		if(boneNode.parent && getNodeName(*boneNode.parent) != skin.name)
		{
			ANKI_CHECK(file.writeTextf("parent=\"%s\" ", getNodeName(*boneNode.parent).cstr()));
		}

		// Bone transform
		ANKI_CHECK(file.writeText("boneTransform=\""));
		Mat4 btrf(&boneMats[i][0]);
		btrf.transpose();
		const Mat3x4 btrf3x4(btrf);
		for(U32 j = 0; j < 12; j++)
		{
			ANKI_CHECK(file.writeTextf("%f ", btrf3x4[j]));
		}
		ANKI_CHECK(file.writeText("\" "));

		// Transform
		Transform trf;
		ANKI_CHECK(getNodeTransform(boneNode, trf));
		Mat3x4 mat(trf);
		ANKI_CHECK(file.writeText("transform=\""));
		for(U j = 0; j < 12; j++)
		{
			ANKI_CHECK(file.writeTextf("%f ", mat[j]));
		}
		ANKI_CHECK(file.writeText("\" "));

		ANKI_CHECK(file.writeText("/>\n"));
	}

	ANKI_CHECK(file.writeText("\t</bones>\n"));
	ANKI_CHECK(file.writeText("</skeleton>\n"));

	return Error::kNone;
}

Error GltfImporter::writeLight(const cgltf_node& node, const HashMapRaii<CString, StringRaii>& parentExtras)
{
	const cgltf_light& light = *node.light;
	StringRaii nodeName = getNodeName(node);
	ANKI_IMPORTER_LOGV("Importing light %s", nodeName.cstr());

	HashMapRaii<CString, StringRaii> extras(parentExtras);
	ANKI_CHECK(getExtras(light.extras, extras));
	ANKI_CHECK(getExtras(node.extras, extras));

	CString lightTypeStr;
	switch(light.type)
	{
	case cgltf_light_type_point:
		lightTypeStr = "Point";
		break;
	case cgltf_light_type_spot:
		lightTypeStr = "Spot";
		break;
	case cgltf_light_type_directional:
		lightTypeStr = "Directional";
		break;
	default:
		ANKI_IMPORTER_LOGE("Unsupporter light type %d", light.type);
		return Error::kUserData;
	}

	ANKI_CHECK(m_sceneFile.writeTextf("\nnode = scene:newSceneNode(\"%s\")\n", nodeName.cstr()));
	ANKI_CHECK(m_sceneFile.writeText("lcomp = node:newLightComponent()\n"));
	ANKI_CHECK(m_sceneFile.writeTextf("lcomp:setLightComponentType(LightComponentType.k%s)\n", lightTypeStr.cstr()));

	Vec3 color(light.color[0], light.color[1], light.color[2]);
	color *= light.intensity;
	color *= m_lightIntensityScale;
	ANKI_CHECK(
		m_sceneFile.writeTextf("lcomp:setDiffuseColor(Vec4.new(%f, %f, %f, 1))\n", color.x(), color.y(), color.z()));

	auto shadow = extras.find("shadow");
	if(shadow != extras.getEnd())
	{
		if(*shadow == "true" || *shadow == "1")
		{
			ANKI_CHECK(m_sceneFile.writeText("lcomp:setShadowEnabled(1)\n"));
		}
		else
		{
			ANKI_CHECK(m_sceneFile.writeText("lcomp:setShadowEnabled(0)\n"));
		}
	}

	if(light.type == cgltf_light_type_point)
	{
		ANKI_CHECK(m_sceneFile.writeTextf("lcomp:setRadius(%f)\n",
										  (light.range > 0.0f) ? light.range : computeLightRadius(color)));
	}
	else if(light.type == cgltf_light_type_spot)
	{
		ANKI_CHECK(m_sceneFile.writeTextf("lcomp:setDistance(%f)\n",
										  (light.range > 0.0f) ? light.range : computeLightRadius(color)));

		const F32 outer = light.spot_outer_cone_angle * 2.0f;
		ANKI_CHECK(m_sceneFile.writeTextf("lcomp:setOuterAngle(%f)\n", outer));

		auto angStr = extras.find("inner_cone_angle_factor");
		F32 inner;
		if(angStr != extras.getEnd())
		{
			F32 factor;
			ANKI_CHECK(angStr->toNumber(factor));
			inner = light.spot_inner_cone_angle * 2.0f * min(1.0f, factor);
		}
		else
		{
			inner = light.spot_inner_cone_angle * 2.0f;
		}

		if(inner >= 0.95f * outer)
		{
			inner = 0.75f * outer;
		}

		ANKI_CHECK(m_sceneFile.writeTextf("lcomp:setInnerAngle(%f)\n", inner));
	}

	auto lensFlaresFname = extras.find("lens_flare");
	if(lensFlaresFname != extras.getEnd())
	{
		ANKI_CHECK(m_sceneFile.writeTextf("lfcomp = node:newLensFlareComponent()\n"));
		ANKI_CHECK(m_sceneFile.writeTextf("lfcomp:loadImageResource(\"%s\")\n", lensFlaresFname->cstr()));

		auto lsSpriteSize = extras.find("lens_flare_first_sprite_size");
		auto lsColor = extras.find("lens_flare_color");

		if(lsSpriteSize != extras.getEnd())
		{
			DynamicArrayRaii<F64> numbers(m_pool);
			const U32 count = 2;
			ANKI_CHECK(parseArrayOfNumbers(lsSpriteSize->toCString(), numbers, &count));

			ANKI_CHECK(m_sceneFile.writeTextf("lfcomp:setFirstFlareSize(Vec2.new(%f, %f))\n", numbers[0], numbers[1]));
		}

		if(lsColor != extras.getEnd())
		{
			DynamicArrayRaii<F64> numbers(m_pool);
			const U32 count = 4;
			ANKI_CHECK(parseArrayOfNumbers(lsColor->toCString(), numbers, &count));

			ANKI_CHECK(m_sceneFile.writeTextf("lfcomp:setColorMultiplier(Vec4.new(%f, %f, %f, %f))\n", numbers[0],
											  numbers[1], numbers[2], numbers[3]));
		}
	}

	auto lightEventIntensity = extras.find("light_event_intensity");
	auto lightEventFrequency = extras.find("light_event_frequency");
	if(lightEventIntensity != extras.getEnd() || lightEventFrequency != extras.getEnd())
	{
		ANKI_CHECK(m_sceneFile.writeText("event = events:newLightEvent(0.0, -1.0, node)\n"));

		if(lightEventIntensity != extras.getEnd())
		{
			DynamicArrayRaii<F64> numbers(m_pool);
			const U32 count = 4;
			ANKI_CHECK(parseArrayOfNumbers(lightEventIntensity->toCString(), numbers, &count));
			ANKI_CHECK(m_sceneFile.writeTextf("event:setIntensityMultiplier(Vec4.new(%f, %f, %f, %f))\n", numbers[0],
											  numbers[1], numbers[2], numbers[3]));
		}

		if(lightEventFrequency != extras.getEnd())
		{
			DynamicArrayRaii<F64> numbers(m_pool);
			const U32 count = 2;
			ANKI_CHECK(parseArrayOfNumbers(lightEventFrequency->toCString(), numbers, &count));
			ANKI_CHECK(m_sceneFile.writeTextf("event:setFrequency(%f, %f)\n", numbers[0], numbers[1]));
		}
	}

	return Error::kNone;
}

Error GltfImporter::writeCamera(const cgltf_node& node,
								[[maybe_unused]] const HashMapRaii<CString, StringRaii>& parentExtras)
{
	if(node.camera->type != cgltf_camera_type_perspective)
	{
		ANKI_IMPORTER_LOGV("Unsupported camera type: %s", getNodeName(node).cstr());
		return Error::kNone;
	}

	const cgltf_camera_perspective& cam = node.camera->data.perspective;
	ANKI_IMPORTER_LOGV("Importing camera %s", getNodeName(node).cstr());

	ANKI_CHECK(m_sceneFile.writeTextf("\nnode = scene:newSceneNode(\"%s\")\n", getNodeName(node).cstr()));
	ANKI_CHECK(m_sceneFile.writeText("scene:setActiveCameraNode(node)\n"));
	ANKI_CHECK(m_sceneFile.writeText("comp = node:newCameraComponent()\n"));

	ANKI_CHECK(m_sceneFile.writeTextf("comp:setPerspective(%f, %f, getMainRenderer():getAspectRatio() * %f, %f)\n",
									  cam.znear, cam.zfar, cam.yfov, cam.yfov));

	return Error::kNone;
}

Error GltfImporter::writeModelNode(const cgltf_node& node, const HashMapRaii<CString, StringRaii>& parentExtras)
{
	ANKI_IMPORTER_LOGV("Importing model node %s", getNodeName(node).cstr());

	HashMapRaii<CString, StringRaii> extras(parentExtras);
	ANKI_CHECK(getExtras(node.extras, extras));

	const StringRaii modelFname = computeModelResourceFilename(*node.mesh);

	ANKI_CHECK(m_sceneFile.writeTextf("\nnode = scene:newSceneNode(\"%s\")\n", getNodeName(node).cstr()));
	ANKI_CHECK(m_sceneFile.writeTextf("node:newModelComponent():loadModelResource(\"%s%s\")\n", m_rpath.cstr(),
									  modelFname.cstr()));

	if(node.skin)
	{
		ANKI_CHECK(m_sceneFile.writeTextf("node:newSkinComponent():loadSkeletonResource(\"%s%s\")\n", m_rpath.cstr(),
										  computeSkeletonResourceFilename(*node.skin).cstr()));
	}

	return Error::kNone;
}

StringRaii GltfImporter::computeModelResourceFilename(const cgltf_mesh& mesh) const
{
	StringListRaii list(m_pool);

	list.pushBack(mesh.name);

	for(U i = 0; i < mesh.primitives_count; ++i)
	{
		const Char* mtlName = (mesh.primitives[i].material) ? mesh.primitives[i].material->name : "UnamedMtl";
		list.pushBackSprintf("_%s", mtlName);
	}

	StringRaii joined(m_pool);
	list.join("", joined);

	const U64 hash = computeHash(joined.getBegin(), joined.getLength());

	StringRaii out(m_pool);
	out.sprintf("%.64s_%" PRIx64 ".ankimdl", joined.cstr(), hash); // Limit the filename size

	return out;
}

StringRaii GltfImporter::computeMeshResourceFilename(const cgltf_mesh& mesh) const
{
	const U64 hash = computeHash(mesh.name, strlen(mesh.name));
	StringRaii out(m_pool);
	out.sprintf("%.64s_%" PRIx64 ".ankimesh", mesh.name, hash); // Limit the filename size
	return out;
}

StringRaii GltfImporter::computeMaterialResourceFilename(const cgltf_material& mtl) const
{
	const U64 hash = computeHash(mtl.name, strlen(mtl.name));

	StringRaii out(m_pool);

	out.sprintf("%.64s_%" PRIx64 ".ankimtl", mtl.name, hash); // Limit the filename size

	return out;
}

StringRaii GltfImporter::computeAnimationResourceFilename(const cgltf_animation& anim) const
{
	const U64 hash = computeHash(anim.name, strlen(anim.name));

	StringRaii out(m_pool);

	out.sprintf("%.64s_%" PRIx64 ".ankianim", anim.name, hash); // Limit the filename size

	return out;
}

StringRaii GltfImporter::computeSkeletonResourceFilename(const cgltf_skin& skin) const
{
	const U64 hash = computeHash(skin.name, strlen(skin.name));

	StringRaii out(m_pool);

	out.sprintf("%.64s_%" PRIx64 ".ankiskel", skin.name, hash); // Limit the filename size

	return out;
}

} // end namespace anki
