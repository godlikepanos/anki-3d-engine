// Copyright (C) 2009-present, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <AnKi/Importer/GltfImporter.h>
#include <AnKi/Util/System.h>
#include <AnKi/Util/ThreadJobManager.h>
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

	xAxis = xAxis.normalize();
	yAxis = yAxis.normalize();
	zAxis = zAxis.normalize();

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

		scale = Vec3(xAxis.length(), yAxis.length(), zAxis.length());

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

	trf.setOrigin(tsl.xyz0());
	trf.setRotation(Mat3x4(Vec3(0.0f), rot));
	trf.setScale(scale);

	return Error::kNone;
}

static Bool stringsExist(const ImporterHashMap<CString, ImporterString>& map, const std::initializer_list<CString>& list)
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

static Error getExtra(const ImporterHashMap<CString, ImporterString>& extras, CString name, F32& val, Bool& found)
{
	found = false;
	ImporterHashMap<CString, ImporterString>::ConstIterator it = extras.find(name);

	if(it != extras.getEnd())
	{
		const Error err = it->toNumber(val);
		if(err)
		{
			ANKI_IMPORTER_LOGE("Failed to parse %s with value: %s", name.cstr(), it->cstr());
			return err;
		}

		found = true;
	}

	return Error::kNone;
}

static Error getExtra(const ImporterHashMap<CString, ImporterString>& extras, CString name, ImporterString& val, Bool& found)
{
	found = false;
	ImporterHashMap<CString, ImporterString>::ConstIterator it = extras.find(name);

	if(it != extras.getEnd())
	{
		val = *it;
		found = true;
	}

	return Error::kNone;
}

static Error getExtra(const ImporterHashMap<CString, ImporterString>& extras, CString name, Bool& val, Bool& found)
{
	found = false;
	ImporterHashMap<CString, ImporterString>::ConstIterator it = extras.find(name);

	if(it != extras.getEnd())
	{
		if(*it == "true" || *it == "1")
		{
			val = true;
		}
		else if(*it == "false" || *it == "0")
		{
			val = false;
		}
		else
		{
			U32 valu;
			const Error err = it->toNumber(valu);
			if(err || valu != 0 || valu != 1)
			{
				ANKI_IMPORTER_LOGE("Failed to parse %s with value: %s", name.cstr(), it->cstr());
				return err;
			}

			val = valu != 0;
		}

		found = true;
	}

	return Error::kNone;
}

static Error getExtra(const ImporterHashMap<CString, ImporterString>& extras, CString name, Vec3& val, Bool& found)
{
	found = false;
	ImporterHashMap<CString, ImporterString>::ConstIterator it = extras.find(name);

	if(it != extras.getEnd())
	{
		ImporterStringList tokens;
		tokens.splitString(*it, ' ');
		if(tokens.getSize() != 3)
		{
			ANKI_IMPORTER_LOGE("Error parsing %s with value: %s", name.cstr(), it->cstr());
			return Error::kUserData;
		}

		U count = 0;
		for(auto& token : tokens)
		{
			F32 f;
			const Error err = token.toNumber(f);
			if(err)
			{
				ANKI_IMPORTER_LOGE("Error parsing %s with value: %s", name.cstr(), it->cstr());
				return Error::kUserData;
			}

			val[count++] = f;
		}

		found = true;
	}

	return Error::kNone;
}

GltfImporter::GltfImporter()
{
}

GltfImporter::~GltfImporter()
{
	if(m_gltf)
	{
		cgltf_free(m_gltf);
		m_gltf = nullptr;
	}

	deleteInstance(ImporterMemoryPool::getSingleton(), m_jobManager);
}

Error GltfImporter::init(const GltfImporterInitInfo& initInfo)
{
	m_inputFname = initInfo.m_inputFilename;
	m_outDir = initInfo.m_outDirectory;
	m_rpath = initInfo.m_rpath;
	m_texrpath = initInfo.m_texrpath;
	m_optimizeMeshes = initInfo.m_optimizeMeshes;
	m_optimizeAnimations = initInfo.m_optimizeAnimations;
	m_comment = initInfo.m_comment;

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
		m_jobManager = newInstance<ThreadJobManager>(ImporterMemoryPool::getSingleton(), threadCount, true);
	}

	m_importTextures = initInfo.m_importTextures;

	return Error::kNone;
}

Error GltfImporter::writeAll()
{
	populateNodePtrToIdx();

	ImporterString sceneFname;
	sceneFname.sprintf("%sScene.lua", m_outDir.cstr());
	ANKI_CHECK(m_sceneFile.open(sceneFname.toCString(), FileOpenFlag::kWrite));
	ANKI_CHECK(m_sceneFile.writeTextf("-- Generated by: %s\n", m_comment.cstr()));
	ANKI_CHECK(m_sceneFile.writeText("local scene = getSceneGraph()\nlocal events = getEventManager()\n"));

	// Nodes
	for(const cgltf_scene* scene = m_gltf->scenes; scene < m_gltf->scenes + m_gltf->scenes_count; ++scene)
	{
		for(cgltf_node* const* node = scene->nodes; node < scene->nodes + scene->nodes_count; ++node)
		{
			ANKI_CHECK(visitNode(*(*node), Transform::getIdentity(), ImporterHashMap<CString, ImporterString>()));
		}
	}

	// Fire up all requests
	for(auto& req : m_meshImportRequests)
	{
		if(m_jobManager)
		{
			m_jobManager->dispatchTask([&req]([[maybe_unused]] U32 threadId) {
				Error err = req.m_importer->m_errorInThread.load();

				if(!err)
				{
					err = req.m_importer->writeMesh(*req.m_value);
				}

				if(err)
				{
					req.m_importer->m_errorInThread.store(err._getCode());
				}
			});
		}
		else
		{
			ANKI_CHECK(writeMesh(*req.m_value));
		}
	}

	for(auto& req : m_materialImportRequests)
	{
		if(m_jobManager)
		{
			m_jobManager->dispatchTask([&req]([[maybe_unused]] U32 threadId) {
				Error err = req.m_importer->m_errorInThread.load();

				if(!err)
				{
					err = req.m_importer->writeMaterial(*req.m_value.m_cgltfMaterial, req.m_value.m_writeRt);
				}

				if(err)
				{
					req.m_importer->m_errorInThread.store(err._getCode());
				}
			});
		}
		else
		{
			ANKI_CHECK(writeMaterial(*req.m_value.m_cgltfMaterial, req.m_value.m_writeRt));
		}
	}

	for(auto& req : m_skinImportRequests)
	{
		if(m_jobManager)
		{
			m_jobManager->dispatchTask([&req]([[maybe_unused]] U32 threadId) {
				Error err = req.m_importer->m_errorInThread.load();

				if(!err)
				{
					err = req.m_importer->writeSkeleton(*req.m_value);
				}

				if(err)
				{
					req.m_importer->m_errorInThread.store(err._getCode());
				}
			});
		}
		else
		{
			ANKI_CHECK(writeSkeleton(*req.m_value));
		}
	}

	if(m_jobManager)
	{
		m_jobManager->waitForAllTasksToFinish();

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

Error GltfImporter::appendExtras(const cgltf_extras& extras, ImporterHashMap<CString, ImporterString>& out) const
{
	cgltf_size extrasSize;
	cgltf_copy_extras_json(m_gltf, &extras, nullptr, &extrasSize);
	if(extrasSize == 0)
	{
		return Error::kNone;
	}

	ImporterDynamicArrayLarge<char> json;
	json.resize(extrasSize + 1);
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

	ImporterDynamicArray<jsmntok_t> tokens;
	tokens.resize(U32(tokenCount));

	// Get tokens
	jsmn_init(&parser);
	jsmn_parse(&parser, jsonTxt.cstr(), jsonTxt.getLength(), &tokens[0], tokens.getSize());

	ImporterStringList tokenStrings;
	for(const jsmntok_t& token : tokens)
	{
		if(token.type != JSMN_STRING && token.type != JSMN_PRIMITIVE)
		{
			continue;
		}

		ImporterString tokenStr(&jsonTxt[token.start], &jsonTxt[token.end]);
		tokenStrings.pushBack(tokenStr.toCString());
	}

	if((tokenStrings.getSize() % 2) != 0)
	{
		ANKI_IMPORTER_LOGV("Ignoring: %s", jsonTxt.cstr());
		return Error::kNone;
	}

	// Write them to the map
	auto it = tokenStrings.getBegin();
	while(it != tokenStrings.getEnd())
	{
		auto it2 = it;
		++it2;

		out.emplace(it->toCString(), ImporterString(it2->toCString()));
		++it;
		++it;
	}

	return Error::kNone;
}

static void appendExtrasMap(const ImporterHashMap<CString, ImporterString>& in, ImporterHashMap<CString, ImporterString>& out)
{
	auto& outs = out.getSparseArray();
	for(auto it = in.getBegin(); it != in.getEnd(); ++it)
	{
		outs.emplace(it.getKey(), *it);
	}
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

ImporterString GltfImporter::getNodeName(const cgltf_node& node) const
{
	ImporterString out;

	if(node.name)
	{
		out = node.name;
	}
	else
	{
		auto it = m_nodePtrToIdx.find(&node);
		ANKI_ASSERT(it != m_nodePtrToIdx.getEnd());
		out.sprintf("unnamed_node_%u", *it);
	}

	return out;
}

Error GltfImporter::parseArrayOfNumbers(CString str, ImporterDynamicArray<F64>& out, const U32* expectedArraySize)
{
	ImporterStringList list;
	list.splitString(str, ' ');

	out.resize(U32(list.getSize()));

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
		ANKI_IMPORTER_LOGE("Failed to parse floats. Expecting %u floats got %u: %s", *expectedArraySize, out.getSize(), str.cstr());
	}

	return Error::kNone;
}

Error GltfImporter::visitNode(const cgltf_node& node, const Transform& parentTrf, const ImporterHashMap<CString, ImporterString>& parentExtras)
{
	// Check error from a thread
	const Error threadErr = m_errorInThread.load();
	if(threadErr)
	{
		ANKI_IMPORTER_LOGE("Error happened in a thread");
		return threadErr;
	}

	ImporterHashMap<CString, ImporterString> outExtras;
	if(node.light)
	{
		ImporterHashMap<CString, ImporterString> extras;
		ANKI_CHECK(appendExtras(node.light->extras, extras));
		ANKI_CHECK(appendExtras(node.extras, extras));
		appendExtrasMap(parentExtras, extras);

		ANKI_CHECK(writeLight(node, extras));

		Transform localTrf;
		ANKI_CHECK(getNodeTransform(node, localTrf));
		localTrf.setScale(Vec4(1.0f, 1.0f, 1.0f, 0.0f)); // Remove scale
		ANKI_CHECK(writeTransform(parentTrf.combineTransformations(localTrf)));
	}
	else if(node.camera)
	{
		ImporterHashMap<CString, ImporterString> extras;
		ANKI_CHECK(appendExtras(node.camera->extras, extras));
		ANKI_CHECK(appendExtras(node.extras, extras));
		appendExtrasMap(parentExtras, extras);

		ANKI_CHECK(writeCamera(node, extras));

		Transform localTrf;
		ANKI_CHECK(getNodeTransform(node, localTrf));
		localTrf.setScale(Vec4(1.0f, 1.0f, 1.0f, 0.0f)); // Remove scale
		ANKI_CHECK(writeTransform(parentTrf.combineTransformations(localTrf)));
	}
	else if(node.mesh)
	{
		// Handle special nodes
		ImporterHashMap<CString, ImporterString> extras;
		ANKI_CHECK(appendExtras(node.mesh->extras, extras));
		ANKI_CHECK(appendExtras(node.extras, extras));
		appendExtrasMap(parentExtras, extras);

		ImporterString extraValueStr;
		Bool extraValueBool = false;
		Vec3 extraValueVec3;
		F32 extraValuef = 0.0f;
		Bool extraFound = false;

		ANKI_CHECK(getExtra(extras, "particles", extraValueStr, extraFound));
		if(extraFound)
		{
			Bool gpuParticles = false;
			ANKI_CHECK(getExtra(extras, "gpu_particles", extraValueBool, extraFound));
			if(extraFound)
			{
				gpuParticles = extraValueBool;
			}

			if(!gpuParticles) // TODO Re-enable GPU particles
			{
				ANKI_CHECK(m_sceneFile.writeTextf("\nnode = scene:newSceneNode(\"%s\")\n", getNodeName(node).cstr()));

				ANKI_CHECK(m_sceneFile.writeTextf("comp = node:newParticleEmitterComponent()\n"));
				ANKI_CHECK(m_sceneFile.writeTextf("comp:loadParticleEmitterResource(\"%s\")\n", extraValueStr.cstr()));

				Transform localTrf;
				ANKI_CHECK(getNodeTransform(node, localTrf));
				ANKI_CHECK(writeTransform(parentTrf.combineTransformations(localTrf)));
			}
		}
		else if(stringsExist(extras, {"skybox_solid_color", "skybox_image", "fog_min_density", "fog_max_density", "fog_height_of_min_density",
									  "fog_height_of_max_density", "fog_diffuse_color", "skybox_generated"}))
		{
			// Atmosphere

			ANKI_CHECK(m_sceneFile.writeTextf("\nnode = scene:newSceneNode(\"%s\")\n", getNodeName(node).cstr()));
			ANKI_CHECK(m_sceneFile.writeText("comp = node:newSkyboxComponent()\n"));

			ANKI_CHECK(getExtra(extras, "skybox_solid_color", extraValueVec3, extraFound));
			if(extraFound)
			{
				ANKI_CHECK(
					m_sceneFile.writeTextf("comp:setSolidColor(Vec3.new(%f, %f, %f))\n", extraValueVec3.x(), extraValueVec3.y(), extraValueVec3.z()));
			}

			ANKI_CHECK(getExtra(extras, "skybox_image", extraValueStr, extraFound));
			if(extraFound)
			{
				ANKI_CHECK(m_sceneFile.writeTextf("comp:loadImageResource(\"%s\")\n", extraValueStr.cstr()));
			}

			ANKI_CHECK(getExtra(extras, "skybox_image_scale", extraValueVec3, extraFound));
			if(extraFound)
			{
				ANKI_CHECK(
					m_sceneFile.writeTextf("comp:setImageScale(Vec3.new(%f, %f, %f))\n", extraValueVec3.x(), extraValueVec3.y(), extraValueVec3.z()));
			}

			ANKI_CHECK(getExtra(extras, "fog_min_density", extraValuef, extraFound));
			if(extraFound)
			{
				ANKI_CHECK(m_sceneFile.writeTextf("comp:setMinFogDensity(%f)\n", extraValuef));
			}

			ANKI_CHECK(getExtra(extras, "fog_max_density", extraValuef, extraFound));
			if(extraFound)
			{
				ANKI_CHECK(m_sceneFile.writeTextf("comp:setMaxFogDensity(%f)\n", extraValuef));
			}

			ANKI_CHECK(getExtra(extras, "fog_height_of_min_density", extraValuef, extraFound));
			if(extraFound)
			{
				ANKI_CHECK(m_sceneFile.writeTextf("comp:setHeightOfMinFogDensity(%f)\n", extraValuef));
			}

			ANKI_CHECK(getExtra(extras, "fog_height_of_max_density", extraValuef, extraFound));
			if(extraFound)
			{
				ANKI_CHECK(m_sceneFile.writeTextf("comp:setHeightOfMaxFogDensity(%f)\n", extraValuef));
			}

			ANKI_CHECK(getExtra(extras, "fog_diffuse_color", extraValueVec3, extraFound));
			if(extraFound)
			{
				ANKI_CHECK(m_sceneFile.writeTextf("comp:setFogDiffuseColor(Vec3.new(%f, %f, %f))\n", extraValueVec3.x(), extraValueVec3.y(),
												  extraValueVec3.z()));
			}

			ANKI_CHECK(getExtra(extras, "skybox_generated", extraValueBool, extraFound));
			if(extraFound && extraValueBool)
			{
				ANKI_CHECK(m_sceneFile.writeTextf("comp:setGeneratedSky()\n"));
			}

			Transform localTrf;
			ANKI_CHECK(getNodeTransform(node, localTrf));
			ANKI_CHECK(writeTransform(parentTrf.combineTransformations(localTrf)));
		}
		else if(stringsExist(extras, {"collision"}))
		{
			ANKI_CHECK(m_sceneFile.writeTextf("\nnode = scene:newSceneNode(\"%s\")\n", getNodeName(node).cstr()));

			ANKI_CHECK(m_sceneFile.writeText("comp = scene:newBodyComponent()\n"));
			const ImporterString meshFname = computeMeshResourceFilename(*node.mesh);
			ANKI_CHECK(m_sceneFile.writeTextf("comp:loadMeshResource(\"%s%s\")\n", m_rpath.cstr(), meshFname.cstr()));

			Transform localTrf;
			ANKI_CHECK(getNodeTransform(node, localTrf));
			ANKI_CHECK(writeTransform(parentTrf.combineTransformations(localTrf)));
		}
		else if(stringsExist(extras, {"reflection_probe"}))
		{
			Vec3 tsl;
			Mat3 rot;
			Vec3 scale;
			getNodeTransform(node, tsl, rot, scale);

			ANKI_CHECK(m_sceneFile.writeTextf("\nnode = scene:newSceneNode(\"%s\")\n", getNodeName(node).cstr()));

			ANKI_CHECK(m_sceneFile.writeText("comp = node:newReflectionProbeComponent()\n"));

			const Transform localTrf = Transform(tsl, rot, scale);
			ANKI_CHECK(writeTransform(parentTrf.combineTransformations(localTrf)));
		}
		else if(stringsExist(extras, {"gi_probe"}))
		{
			Vec3 tsl;
			Mat3 rot;
			Vec3 scale;
			getNodeTransform(node, tsl, rot, scale);

			ANKI_CHECK(m_sceneFile.writeTextf("\nnode = scene:newSceneNode(\"%s\")\n", getNodeName(node).cstr()));
			ANKI_CHECK(m_sceneFile.writeText("comp = node:newGlobalIlluminationProbeComponent()\n"));

			ANKI_CHECK(getExtra(extras, "gi_probe_fade_distance", extraValuef, extraFound));
			if(extraFound && extraValuef > 0.0f)
			{
				ANKI_CHECK(m_sceneFile.writeTextf("comp:setFadeDistance(%f)\n", extraValuef));
			}

			ANKI_CHECK(getExtra(extras, "gi_probe_cell_size", extraValuef, extraFound));
			if(extraFound && extraValuef > 0.0f)
			{
				ANKI_CHECK(m_sceneFile.writeTextf("comp:setCellSize(%f)\n", extraValuef));
			}

			const Transform localTrf = Transform(tsl, rot, scale);
			ANKI_CHECK(writeTransform(parentTrf.combineTransformations(localTrf)));
		}
		else if(stringsExist(extras, {"decal"}))
		{
			ANKI_CHECK(m_sceneFile.writeTextf("\nnode = scene:newSceneNode(\"%s\")\n", getNodeName(node).cstr()));
			ANKI_CHECK(m_sceneFile.writeText("comp = node:newDecalComponent()\n"));

			ANKI_CHECK(getExtra(extras, "decal_diffuse", extraValueStr, extraFound));
			if(extraFound)
			{
				ANKI_CHECK(getExtra(extras, "decal_diffuse_factor", extraValuef, extraFound));

				ANKI_CHECK(
					m_sceneFile.writeTextf("comp:loadDiffuseImageResource(\"%s\", %f)\n", extraValueStr.cstr(), (extraFound) ? extraValuef : 1.0f));
			}

			ANKI_CHECK(getExtra(extras, "decal_metal_roughness", extraValueStr, extraFound));
			if(extraFound)
			{
				ANKI_CHECK(getExtra(extras, "decal_metal_roughness_factor", extraValuef, extraFound));

				ANKI_CHECK(m_sceneFile.writeTextf("comp:loadMetalRoughnessImageResource(\"%s\", %f)\n", extraValueStr.cstr(),
												  (extraFound) ? extraValuef : 1.0f));
			}

			Vec3 tsl;
			Mat3 rot;
			Vec3 scale;
			getNodeTransform(node, tsl, rot, scale);
			const Transform localTrf = Transform(tsl, rot, scale);
			ANKI_CHECK(writeTransform(parentTrf.combineTransformations(localTrf)));
		}
		else
		{
			// Mesh+Material node

			ANKI_CHECK(getExtra(extras, "no_rt", extraValueBool, extraFound));
			const Bool skipRt = (extraFound) ? extraValueBool : false;

			Transform localTrf;
			const Error err = getNodeTransform(node, localTrf);

			if(err)
			{
				ANKI_IMPORTER_LOGE("Will skip node: %s", getNodeName(node).cstr());
			}
			else
			{
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

				ImporterHashMap<CString, ImporterString>::Iterator it2;
				const Bool selfCollision = (it2 = extras.find("collision_mesh")) != extras.getEnd() && *it2 == "self";

				ANKI_CHECK(writeMeshMaterialNode(node, parentExtras));

				ANKI_CHECK(writeTransform(parentTrf.combineTransformations(localTrf)));

				if(selfCollision)
				{
					ANKI_CHECK(m_sceneFile.writeText("comp = node:newBodyComponent()\n"));

					const ImporterString meshFname = computeMeshResourceFilename(*node.mesh);

					ANKI_CHECK(m_sceneFile.writeText("comp:setCollisionShapeType(BodyComponentCollisionShapeType.kFromMeshComponent)\n"));
					ANKI_CHECK(m_sceneFile.writeText("comp:teleportTo(trf:getOrigin(), trf:getRotation())\n"));
				}
			}
		}
	}
	else
	{
		ANKI_IMPORTER_LOGV("Ignoring node %s. Assuming transform node", getNodeName(node).cstr());
		ANKI_CHECK(appendExtras(node.extras, outExtras));
	}

	// Visit children
	Transform nodeTrf;
	{
		Vec3 tsl;
		Mat3 rot;
		Vec3 scale;
		getNodeTransform(node, tsl, rot, scale);
		nodeTrf = Transform(tsl.xyz0(), Mat3x4(Vec3(0.0f), rot), scale.xyz0());
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
	ANKI_CHECK(m_sceneFile.writeTextf("trf:setOrigin(Vec3.new(%f, %f, %f))\n", trf.getOrigin().x(), trf.getOrigin().y(), trf.getOrigin().z()));

	ANKI_CHECK(m_sceneFile.writeText("rot = Mat3.new()\n"));
	ANKI_CHECK(m_sceneFile.writeText("rot:setAll("));
	for(U i = 0; i < 9; i++)
	{
		ANKI_CHECK(m_sceneFile.writeTextf((i != 8) ? "%f, " : "%f)\n", trf.getRotation().getRotationPart()[i]));
	}
	ANKI_CHECK(m_sceneFile.writeText("trf:setRotation(rot)\n"));

	ANKI_CHECK(m_sceneFile.writeTextf("trf:setScale(Vec3.new(%f, %f, %f))\n", trf.getScale().x(), trf.getScale().y(), trf.getScale().z()));

	ANKI_CHECK(m_sceneFile.writeText("node:setLocalTransform(trf)\n"));

	return Error::kNone;
}

Error GltfImporter::writeSkeleton(const cgltf_skin& skin) const
{
	ImporterString fname;
	fname.sprintf("%s%s", m_outDir.cstr(), computeSkeletonResourceFilename(skin).cstr());
	ANKI_IMPORTER_LOGV("Importing skeleton %s", fname.cstr());

	// Get matrices
	ImporterDynamicArray<Mat4> boneMats;
	readAccessor(*skin.inverse_bind_matrices, boneMats);
	if(boneMats.getSize() != skin.joints_count)
	{
		ANKI_IMPORTER_LOGE("Bone matrices should match the joint count");
		return Error::kUserData;
	}

	// Write file
	File file;
	ANKI_CHECK(file.open(fname.toCString(), FileOpenFlag::kWrite));

	ANKI_CHECK(file.writeTextf("%s\n<skeleton>\n", XmlDocument<MemoryPoolPtrWrapper<BaseMemoryPool>>::kXmlHeader.cstr()));
	ANKI_CHECK(file.writeTextf("\t<bones>\n"));

	for(U32 i = 0; i < skin.joints_count; ++i)
	{
		const cgltf_node& boneNode = *skin.joints[i];

		ImporterString parent;

		// Name & parent
		ANKI_CHECK(file.writeTextf("\t\t<bone name=\"%s\" ", getNodeName(boneNode).cstr()));
		if(boneNode.parent && getNodeName(*boneNode.parent) != skin.name)
		{
			ANKI_CHECK(file.writeTextf("parent=\"%s\" ", getNodeName(*boneNode.parent).cstr()));
		}

		// Bone transform
		ANKI_CHECK(file.writeText("boneTransform=\""));
		Mat4 btrf(&boneMats[i][0]);
		btrf = btrf.transpose();
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

Error GltfImporter::writeLight(const cgltf_node& node, const ImporterHashMap<CString, ImporterString>& parentExtras)
{
	const cgltf_light& light = *node.light;
	ImporterString nodeName = getNodeName(node);
	ANKI_IMPORTER_LOGV("Importing light %s", nodeName.cstr());

	ImporterHashMap<CString, ImporterString> extras(parentExtras);
	ANKI_CHECK(appendExtras(light.extras, extras));
	ANKI_CHECK(appendExtras(node.extras, extras));

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
	ANKI_CHECK(m_sceneFile.writeTextf("lcomp:setDiffuseColor(Vec4.new(%f, %f, %f, 1))\n", color.x(), color.y(), color.z()));

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
		ANKI_CHECK(m_sceneFile.writeTextf("lcomp:setRadius(%f)\n", (light.range > 0.0f) ? light.range : computeLightRadius(color)));
	}
	else if(light.type == cgltf_light_type_spot)
	{
		ANKI_CHECK(m_sceneFile.writeTextf("lcomp:setDistance(%f)\n", (light.range > 0.0f) ? light.range : computeLightRadius(color)));

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
			ImporterDynamicArray<F64> numbers;
			const U32 count = 2;
			ANKI_CHECK(parseArrayOfNumbers(lsSpriteSize->toCString(), numbers, &count));

			ANKI_CHECK(m_sceneFile.writeTextf("lfcomp:setFirstFlareSize(Vec2.new(%f, %f))\n", numbers[0], numbers[1]));
		}

		if(lsColor != extras.getEnd())
		{
			ImporterDynamicArray<F64> numbers;
			const U32 count = 4;
			ANKI_CHECK(parseArrayOfNumbers(lsColor->toCString(), numbers, &count));

			ANKI_CHECK(
				m_sceneFile.writeTextf("lfcomp:setColorMultiplier(Vec4.new(%f, %f, %f, %f))\n", numbers[0], numbers[1], numbers[2], numbers[3]));
		}
	}

	auto lightEventIntensity = extras.find("light_event_intensity");
	auto lightEventFrequency = extras.find("light_event_frequency");
	if(lightEventIntensity != extras.getEnd() || lightEventFrequency != extras.getEnd())
	{
		ANKI_CHECK(m_sceneFile.writeText("event = events:newLightEvent(0.0, -1.0, node)\n"));

		if(lightEventIntensity != extras.getEnd())
		{
			ImporterDynamicArray<F64> numbers;
			const U32 count = 4;
			ANKI_CHECK(parseArrayOfNumbers(lightEventIntensity->toCString(), numbers, &count));
			ANKI_CHECK(
				m_sceneFile.writeTextf("event:setIntensityMultiplier(Vec4.new(%f, %f, %f, %f))\n", numbers[0], numbers[1], numbers[2], numbers[3]));
		}

		if(lightEventFrequency != extras.getEnd())
		{
			ImporterDynamicArray<F64> numbers;
			const U32 count = 2;
			ANKI_CHECK(parseArrayOfNumbers(lightEventFrequency->toCString(), numbers, &count));
			ANKI_CHECK(m_sceneFile.writeTextf("event:setFrequency(%f, %f)\n", numbers[0], numbers[1]));
		}
	}

	return Error::kNone;
}

Error GltfImporter::writeCamera(const cgltf_node& node, [[maybe_unused]] const ImporterHashMap<CString, ImporterString>& parentExtras)
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

	ANKI_CHECK(
		m_sceneFile.writeTextf("comp:setPerspective(%f, %f, getRenderer():getAspectRatio() * %f, %f)\n", cam.znear, cam.zfar, cam.yfov, cam.yfov));

	return Error::kNone;
}

Error GltfImporter::writeMeshMaterialNode(const cgltf_node& node, const ImporterHashMap<CString, ImporterString>& parentExtras)
{
	ANKI_IMPORTER_LOGV("Importing mesh&material node %s", getNodeName(node).cstr());

	ImporterHashMap<CString, ImporterString> extras(parentExtras);
	ANKI_CHECK(appendExtras(node.extras, extras));

	ANKI_CHECK(m_sceneFile.writeTextf("\nnode = scene:newSceneNode(\"%s\")\n", getNodeName(node).cstr()));

	const cgltf_mesh& mesh = *node.mesh;

	ANKI_CHECK(
		m_sceneFile.writeTextf("node:newMeshComponent():setMeshFilename(\"%s%s\")\n", m_rpath.cstr(), computeMeshResourceFilename(mesh).cstr()));

	for(U32 primIdx = 0; primIdx < mesh.primitives_count; ++primIdx)
	{
		ANKI_CHECK(m_sceneFile.writeText("mtlc = node:newMaterialComponent()\n"));

		ANKI_CHECK(m_sceneFile.writeTextf("mtlc:setMaterialFilename(\"%s%s\")\n", m_rpath.cstr(),
										  computeMaterialResourceFilename(*mesh.primitives[primIdx].material).cstr()));

		ANKI_CHECK(m_sceneFile.writeTextf("mtlc:setSubmeshIndex(%d)\n", primIdx));
	}

	if(node.skin)
	{
		ANKI_CHECK(m_sceneFile.writeTextf("node:newSkinComponent():loadSkeletonResource(\"%s%s\")\n", m_rpath.cstr(),
										  computeSkeletonResourceFilename(*node.skin).cstr()));
	}

	return Error::kNone;
}

ImporterString GltfImporter::computeMeshResourceFilename(const cgltf_mesh& mesh) const
{
	const U64 hash = computeHash(mesh.name, strlen(mesh.name));
	ImporterString out;
	out.sprintf("%.64s_%" PRIx64 ".ankimesh", mesh.name, hash); // Limit the filename size
	return out;
}

ImporterString GltfImporter::computeMaterialResourceFilename(const cgltf_material& mtl) const
{
	const U64 hash = computeHash(mtl.name, strlen(mtl.name));

	ImporterString out;

	out.sprintf("%.64s_%" PRIx64 ".ankimtl", mtl.name, hash); // Limit the filename size

	return out;
}

ImporterString GltfImporter::computeAnimationResourceFilename(const cgltf_animation& anim) const
{
	const U64 hash = computeHash(anim.name, strlen(anim.name));

	ImporterString out;

	out.sprintf("%.64s_%" PRIx64 ".ankianim", anim.name, hash); // Limit the filename size

	return out;
}

ImporterString GltfImporter::computeSkeletonResourceFilename(const cgltf_skin& skin) const
{
	const U64 hash = computeHash(skin.name, strlen(skin.name));

	ImporterString out;

	out.sprintf("%.64s_%" PRIx64 ".ankiskel", skin.name, hash); // Limit the filename size

	return out;
}

} // end namespace anki
