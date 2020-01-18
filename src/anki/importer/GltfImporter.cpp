// Copyright (C) 2009-2020, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <anki/importer/GltfImporter.h>
#include <anki/util/System.h>
#include <anki/util/ThreadHive.h>
#include <anki/util/StringList.h>

#if ANKI_COMPILER_GCC_COMPATIBLE
#	pragma GCC diagnostic push
#	pragma GCC diagnostic ignored "-Wconversion"
#endif
#define CGLTF_IMPLEMENTATION
#include <cgltf/cgltf.h>
#if ANKI_COMPILER_GCC_COMPATIBLE
#	pragma GCC diagnostic pop
#endif

namespace anki
{

static F32 computeLightRadius(const Vec3 color)
{
	// Based on the attenuation equation: att = 1 - fragLightDist^2 / lightRadius^2
	const F32 minAtt = 0.01f;
	const F32 maxIntensity = max(max(color.x(), color.y()), color.z());
	return sqrt(maxIntensity / minAtt);
}

#if 0
static ANKI_USE_RESULT Error getUniformScale(const Mat4& m, F32& out)
{
	const F32 SCALE_THRESHOLD = 0.01f; // 1 cm

	Vec3 xAxis = m.getColumn(0).xyz();
	Vec3 yAxis = m.getColumn(1).xyz();
	Vec3 zAxis = m.getColumn(2).xyz();

	const F32 scale = xAxis.getLength();
	if(absolute(scale - yAxis.getLength()) > SCALE_THRESHOLD || absolute(scale - zAxis.getLength()) > SCALE_THRESHOLD)
	{
		ANKI_GLTF_LOGE("No uniform scale in the matrix");
		return Error::USER_DATA;
	}

	out = scale;
	return Error::NONE;
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

static ANKI_USE_RESULT Error getNodeTransform(const cgltf_node& node, Transform& trf)
{
	Vec3 tsl;
	Mat3 rot;
	Vec3 scale;
	getNodeTransform(node, tsl, rot, scale);

	const F32 scaleEpsilon = 0.01f;
	if(absolute(scale[0] - scale[1]) > scaleEpsilon || absolute(scale[0] - scale[2]) > scaleEpsilon)
	{
		ANKI_GLTF_LOGE("Expecting uniform scale");
		return Error::USER_DATA;
	}

	trf.setOrigin(tsl.xyz0());
	trf.setRotation(Mat3x4(rot));
	trf.setScale(scale[0]);

	return Error::NONE;
}

const char* GltfImporter::XML_HEADER = R"(<?xml version="1.0" encoding="UTF-8" ?>)";

GltfImporter::GltfImporter(GenericMemoryPoolAllocator<U8> alloc)
	: m_alloc(alloc)
{
}

GltfImporter::~GltfImporter()
{
	if(m_gltf)
	{
		cgltf_free(m_gltf);
		m_gltf = nullptr;
	}

	m_alloc.deleteInstance(m_hive);
}

Error GltfImporter::init(
	CString inputFname, CString outDir, CString rpath, CString texrpath, Bool optimizeMeshes, U32 threadCount)
{
	m_inputFname.create(inputFname);
	m_outDir.create(outDir);
	m_rpath.create(rpath);
	m_texrpath.create(texrpath);
	m_optimizeMeshes = optimizeMeshes;

	cgltf_options options = {};
	cgltf_result res = cgltf_parse_file(&options, inputFname.cstr(), &m_gltf);
	if(res != cgltf_result_success)
	{
		ANKI_LOGE("Failed to open the GLTF file. Code: %d", res);
		return Error::FUNCTION_FAILED;
	}

	res = cgltf_load_buffers(&options, m_gltf, inputFname.cstr());
	if(res != cgltf_result_success)
	{
		ANKI_LOGE("Failed to load GLTF data. Code: %d", res);
		return Error::FUNCTION_FAILED;
	}

	if(threadCount > 0)
	{
		threadCount = min(getCpuCoresCount(), threadCount);
		m_hive = m_alloc.newInstance<ThreadHive>(threadCount, m_alloc, true);
	}

	return Error::NONE;
}

Error GltfImporter::writeAll()
{
	populateNodePtrToIdx();

	for(const cgltf_animation* anim = m_gltf->animations; anim < m_gltf->animations + m_gltf->animations_count; ++anim)
	{
		ANKI_CHECK(writeAnimation(*anim));
	}

	StringAuto sceneFname(m_alloc);
	sceneFname.sprintf("%sscene.lua", m_outDir.cstr());
	ANKI_CHECK(m_sceneFile.open(sceneFname.toCString(), FileOpenFlag::WRITE));
	ANKI_CHECK(m_sceneFile.writeText("local scene = getSceneGraph()\nlocal events = getEventManager()\n"));

	// Nodes
	Error err = Error::NONE;
	for(const cgltf_scene* scene = m_gltf->scenes; scene < m_gltf->scenes + m_gltf->scenes_count && !err; ++scene)
	{
		for(cgltf_node* const* node = scene->nodes; node < scene->nodes + scene->nodes_count && !err; ++node)
		{
			err = visitNode(*(*node), Transform::getIdentity(), HashMapAuto<CString, StringAuto>(m_alloc));
		}
	}

	if(m_hive)
	{
		m_hive->waitAllTasks();
	}

	// Check error
	const Error threadErr = m_errorInThread.load();
	if(threadErr)
	{
		ANKI_GLTF_LOGE("Error happened in a thread");
		return threadErr;
	}

	return err;
}

Error GltfImporter::getExtras(const cgltf_extras& extras, HashMapAuto<CString, StringAuto>& out)
{
	cgltf_size extrasSize;
	cgltf_copy_extras_json(m_gltf, &extras, nullptr, &extrasSize);
	if(extrasSize == 0)
	{
		return Error::NONE;
	}

	DynamicArrayAuto<char, PtrSize> json(m_alloc);
	json.create(extrasSize + 1);
	cgltf_result res = cgltf_copy_extras_json(m_gltf, &extras, &json[0], &extrasSize);
	if(res != cgltf_result_success)
	{
		ANKI_GLTF_LOGE("cgltf_copy_extras_json failed: %d", res);
		return Error::FUNCTION_FAILED;
	}

	json[json.getSize() - 1] = '\0';

	// Get token count
	CString jsonTxt(&json[0]);
	jsmn_parser parser;
	jsmn_init(&parser);
	const I tokenCount = jsmn_parse(&parser, jsonTxt.cstr(), jsonTxt.getLength(), nullptr, 0);
	if(tokenCount < 1)
	{
		return Error::NONE;
	}

	DynamicArrayAuto<jsmntok_t> tokens(m_alloc);
	tokens.create(U32(tokenCount));

	// Get tokens
	jsmn_init(&parser);
	jsmn_parse(&parser, jsonTxt.cstr(), jsonTxt.getLength(), &tokens[0], tokens.getSize());

	StringListAuto tokenStrings(m_alloc);
	for(const jsmntok_t& token : tokens)
	{
		if(token.type != JSMN_STRING && token.type != JSMN_PRIMITIVE)
		{
			continue;
		}

		StringAuto tokenStr(m_alloc);
		tokenStr.create(&jsonTxt[token.start], &jsonTxt[token.end]);
		tokenStrings.pushBack(tokenStr.toCString());
	}

	if((tokenStrings.getSize() % 2) != 0)
	{
		ANKI_GLTF_LOGE("Unable to parse: %s", jsonTxt.cstr());
		return Error::FUNCTION_FAILED;
	}

	// Write them to the map
	auto it = tokenStrings.getBegin();
	while(it != tokenStrings.getEnd())
	{
		auto it2 = it;
		++it2;

		out.emplace(it->toCString(), StringAuto(m_alloc, it2->toCString()));
		++it;
		++it;
	}

	return Error::NONE;
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

StringAuto GltfImporter::getNodeName(const cgltf_node& node)
{
	StringAuto out{m_alloc};

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

Error GltfImporter::parseArrayOfNumbers(CString str, DynamicArrayAuto<F64>& out, const U* expectedArraySize)
{
	StringListAuto list(m_alloc);
	list.splitString(str, ' ');

	out.create(U32(list.getSize()));

	Error err = Error::NONE;
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
		ANKI_GLTF_LOGE("Failed to parse floats: %s", str.cstr());
	}

	if(expectedArraySize && *expectedArraySize != out.getSize())
	{
		ANKI_GLTF_LOGE(
			"Failed to parse floats. Expecting %u floats got %u: %s", *expectedArraySize, out.getSize(), str.cstr());
	}

	return Error::NONE;
}

Error GltfImporter::visitNode(
	const cgltf_node& node, const Transform& parentTrf, const HashMapAuto<CString, StringAuto>& parentExtras)
{
	// Check error from a thread
	const Error threadErr = m_errorInThread.load();
	if(threadErr)
	{
		ANKI_GLTF_LOGE("Error happened in a thread");
		return threadErr;
	}

	HashMapAuto<CString, StringAuto> outExtras(m_alloc);
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
		HashMapAuto<CString, StringAuto> extras(parentExtras);
		ANKI_CHECK(getExtras(node.mesh->extras, extras));

		HashMapAuto<CString, StringAuto>::Iterator it;
		if((it = extras.find("particles")) != extras.getEnd())
		{
			const StringAuto& fname = *it;

			Bool gpuParticles = false;
			if((it = extras.find("gpu_particles")) != extras.getEnd() && *it == "true")
			{
				gpuParticles = true;
			}

			ANKI_CHECK(m_sceneFile.writeText("\nnode = scene:new%sParticleEmitterNode(\"%s\", \"%s\")\n",
				(gpuParticles) ? "Gpu" : "",
				getNodeName(node).cstr(),
				fname.cstr()));

			Transform localTrf;
			ANKI_CHECK(getNodeTransform(node, localTrf));
			ANKI_CHECK(writeTransform(parentTrf.combineTransformations(localTrf)));
		}
		else if((it = extras.find("collision")) != extras.getEnd() && *it == "true")
		{
			// Write colission mesh
			{
				StringAuto fname(m_alloc);
				fname.sprintf("%s%s.ankicl", m_outDir.cstr(), node.mesh->name);
				File file;
				ANKI_CHECK(file.open(fname.toCString(), FileOpenFlag::WRITE));

				ANKI_CHECK(file.writeText("<collisionShape>\n\t<type>staticMesh</type>\n\t<value>%s%s.ankimesh"
										  "</value>\n</collisionShape>\n",
					m_rpath.cstr(),
					node.mesh->name));
			}

			ANKI_CHECK(m_sceneFile.writeText("\nnode = scene:newStaticCollisionNode(\"%s\", \"%s%s.ankicl\")\n",
				getNodeName(node).cstr(),
				m_rpath.cstr(),
				node.mesh->name));

			Transform localTrf;
			ANKI_CHECK(getNodeTransform(node, localTrf));
			ANKI_CHECK(writeTransform(parentTrf.combineTransformations(localTrf)));
		}
		else if((it = extras.find("reflection_probe")) != extras.getEnd() && *it == "true")
		{
			Vec3 tsl;
			Mat3 rot;
			Vec3 scale;
			getNodeTransform(node, tsl, rot, scale);

			const Vec3 half = scale;
			const Vec3 aabbMin = tsl - half - tsl;
			const Vec3 aabbMax = tsl + half - tsl;

			ANKI_CHECK(m_sceneFile.writeText(
				"\nnode = scene:newReflectionProbeNode(\"%s\", Vec4.new(%f, %f, %f, 0), Vec4.new(%f, %f, %f, 0))\n",
				getNodeName(node).cstr(),
				aabbMin.x(),
				aabbMin.y(),
				aabbMin.z(),
				aabbMax.x(),
				aabbMax.y(),
				aabbMax.z()));

			Transform localTrf = Transform(tsl.xyz0(), Mat3x4(rot), 1.0f);
			ANKI_CHECK(writeTransform(parentTrf.combineTransformations(localTrf)));
		}
		else if((it = extras.find("gi_probe")) != extras.getEnd() && *it == "true")
		{
			Vec3 tsl;
			Mat3 rot;
			Vec3 scale;
			getNodeTransform(node, tsl, rot, scale);

			const Vec3 half = scale;
			const Vec3 aabbMin = tsl - half - tsl;
			const Vec3 aabbMax = tsl + half - tsl;

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

			ANKI_CHECK(m_sceneFile.writeText(
				"\nnode = scene:newGlobalIlluminationProbeNode(\"%s\")\n", getNodeName(node).cstr()));
			ANKI_CHECK(m_sceneFile.writeText("comp = node:getSceneNodeBase():getGlobalIlluminationProbeComponent()\n"));

			ANKI_CHECK(m_sceneFile.writeText("comp:setBoundingBox(Vec4.new(%f, %f, %f, 0), Vec4.new(%f, %f, %f, 0))\n",
				aabbMin.x(),
				aabbMin.y(),
				aabbMin.z(),
				aabbMax.x(),
				aabbMax.y(),
				aabbMax.z()));

			if(fadeDistance > 0.0f)
			{
				ANKI_CHECK(m_sceneFile.writeText("comp:setFadeDistance(%f)\n", fadeDistance));
			}

			if(cellSize > 0.0f)
			{
				ANKI_CHECK(m_sceneFile.writeText("comp:setCellSize(%f)\n", cellSize));
			}

			Transform localTrf = Transform(tsl.xyz0(), Mat3x4(rot), 1.0f);
			ANKI_CHECK(writeTransform(parentTrf.combineTransformations(localTrf)));
		}
		else if((it = extras.find("decal")) != extras.getEnd() && *it == "true")
		{
			StringAuto diffuseAtlas(m_alloc);
			if((it = extras.find("decal_diffuse_atlas")) != extras.getEnd())
			{
				diffuseAtlas.create(it->toCString());
			}

			StringAuto diffuseSubtexture(m_alloc);
			if((it = extras.find("decal_diffuse_sub_texture")) != extras.getEnd())
			{
				diffuseSubtexture.create(it->toCString());
			}

			F32 diffuseFactor = -1.0f;
			if((it = extras.find("decal_diffuse_factor")) != extras.getEnd())
			{
				ANKI_CHECK(it->toNumber(diffuseFactor));
			}

			StringAuto specularRougnessMetallicAtlas(m_alloc);
			if((it = extras.find("decal_specular_roughness_metallic_atlas")) != extras.getEnd())
			{
				specularRougnessMetallicAtlas.create(it->toCString());
			}

			StringAuto specularRougnessMetallicSubtexture(m_alloc);
			if((it = extras.find("decal_specular_roughness_metallic_sub_texture")) != extras.getEnd())
			{
				specularRougnessMetallicSubtexture.create(it->toCString());
			}

			F32 specularRougnessMetallicFactor = -1.0f;
			if((it = extras.find("decal_specular_roughness_metallic_factor")) != extras.getEnd())
			{
				ANKI_CHECK(it->toNumber(specularRougnessMetallicFactor));
			}

			ANKI_CHECK(m_sceneFile.writeText("\nnode = scene:newDecalNode(\"%s\")\n", getNodeName(node).cstr()));
			ANKI_CHECK(m_sceneFile.writeText("comp = node:getSceneNodeBase():getDecalComponent()\n"));
			if(diffuseAtlas)
			{
				ANKI_CHECK(m_sceneFile.writeText("comp:setDiffuseDecal(\"%s\", \"%s\", %f)\n",
					diffuseAtlas.cstr(),
					diffuseSubtexture.cstr(),
					diffuseFactor));
			}

			if(specularRougnessMetallicAtlas)
			{
				ANKI_CHECK(m_sceneFile.writeText("comp:setSpecularRoughnessDecal(\"%s\", \"%s\", %f)\n",
					specularRougnessMetallicAtlas.cstr(),
					specularRougnessMetallicSubtexture.cstr(),
					specularRougnessMetallicFactor));
			}

			Vec3 tsl;
			Mat3 rot;
			Vec3 scale;
			getNodeTransform(node, tsl, rot, scale);
			Transform localTrf = Transform(tsl.xyz0(), Mat3x4(rot), 1.0f);
			ANKI_CHECK(writeTransform(parentTrf.combineTransformations(localTrf)));
		}
		else
		{
			// Model node

			// Async because it's slow
			struct Ctx
			{
				GltfImporter* m_importer;
				cgltf_mesh* m_mesh;
				cgltf_material* m_mtl;
				cgltf_skin* m_skin;
				Bool m_selfCollision;
			};
			Ctx* ctx = m_alloc.newInstance<Ctx>();
			ctx->m_importer = this;
			ctx->m_mesh = node.mesh;
			ctx->m_mtl = node.mesh->primitives[0].material;
			ctx->m_skin = node.skin;

			HashMapAuto<CString, StringAuto>::Iterator it2;
			const Bool selfCollision = (it2 = extras.find("collision_mesh")) != extras.getEnd() && *it2 == "self";
			ctx->m_selfCollision = selfCollision;

			// Thread task
			auto callback = [](void* userData, U32 threadId, ThreadHive& hive, ThreadHiveSemaphore* signalSemaphore) {
				Ctx& self = *static_cast<Ctx*>(userData);

				Error err = self.m_importer->writeMesh(*self.m_mesh, CString(), 1.0f);

				// LOD 1
				if(!err)
				{
					StringAuto name(self.m_importer->m_alloc);
					name.sprintf("%s_lod1", self.m_mesh->name);
					err = self.m_importer->writeMesh(*self.m_mesh, name, 0.66f);
				}

				// LOD 2
				if(!err)
				{
					StringAuto name(self.m_importer->m_alloc);
					name.sprintf("%s_lod2", self.m_mesh->name);
					err = self.m_importer->writeMesh(*self.m_mesh, name, 0.33f);
				}

				if(!err)
				{
					err = self.m_importer->writeMaterial(*self.m_mtl);
				}

				if(!err)
				{
					err = self.m_importer->writeModel(*self.m_mesh, (self.m_skin) ? self.m_skin->name : CString());
				}

				if(!err && self.m_skin)
				{
					err = self.m_importer->writeSkeleton(*self.m_skin);
				}

				if(!err && self.m_selfCollision)
				{
					err = self.m_importer->writeCollisionMesh(*self.m_mesh);
				}

				if(err)
				{
					self.m_importer->m_errorInThread.store(err._getCode());
				}

				self.m_importer->m_alloc.deleteInstance(&self);
			};

			if(m_hive)
			{
				m_hive->submitTask(callback, ctx);
			}
			else
			{
				callback(ctx, 0, *m_hive, nullptr);
			}

			ANKI_CHECK(writeModelNode(node, parentExtras));

			Transform localTrf;
			ANKI_CHECK(getNodeTransform(node, localTrf));
			ANKI_CHECK(writeTransform(parentTrf.combineTransformations(localTrf)));

			if(selfCollision)
			{
				ANKI_CHECK(
					m_sceneFile.writeText("node2 = scene:newStaticCollisionNode(\"%s_cl\", \"%s%s.ankicl\", trf)\n",
						getNodeName(node).cstr(),
						m_rpath.cstr(),
						node.mesh->name));
			}
		}
	}
	else
	{
		ANKI_GLTF_LOGW("Ignoring node %s. Assuming transform node", getNodeName(node).cstr());
		ANKI_CHECK(getExtras(node.extras, outExtras));
	}

	// Visit children
	Transform nodeTrf;
	{
		Vec3 tsl;
		Mat3 rot;
		Vec3 scale;
		getNodeTransform(node, tsl, rot, scale);
		nodeTrf = Transform(tsl.xyz0(), Mat3x4(rot), scale.x());
	}
	for(cgltf_node* const* c = node.children; c < node.children + node.children_count; ++c)
	{
		ANKI_CHECK(visitNode(*(*c), nodeTrf, outExtras));
	}

	return Error::NONE;
}

Error GltfImporter::writeTransform(const Transform& trf)
{
	ANKI_CHECK(m_sceneFile.writeText("trf = Transform.new()\n"));
	ANKI_CHECK(m_sceneFile.writeText(
		"trf:setOrigin(Vec4.new(%f, %f, %f, 0))\n", trf.getOrigin().x(), trf.getOrigin().y(), trf.getOrigin().z()));

	ANKI_CHECK(m_sceneFile.writeText("rot = Mat3x4.new()\n"));
	ANKI_CHECK(m_sceneFile.writeText("rot:setAll("));
	for(U i = 0; i < 12; i++)
	{
		ANKI_CHECK(m_sceneFile.writeText((i != 11) ? "%f, " : "%f)\n", trf.getRotation()[i]));
	}
	ANKI_CHECK(m_sceneFile.writeText("trf:setRotation(rot)\n"));

	ANKI_CHECK(m_sceneFile.writeText("trf:setScale(%f)\n", trf.getScale()));

	ANKI_CHECK(m_sceneFile.writeText("node:getSceneNodeBase():getMoveComponent():setLocalTransform(trf)\n"));

	return Error::NONE;
}

Error GltfImporter::writeModel(const cgltf_mesh& mesh, CString skinName)
{
	StringAuto modelFname(m_alloc);
	modelFname.sprintf("%s%s_%s.ankimdl", m_outDir.cstr(), mesh.name, mesh.primitives[0].material->name);
	ANKI_GLTF_LOGI("Importing model %s", modelFname.cstr());

	if(mesh.primitives_count != 1)
	{
		ANKI_GLTF_LOGE("For now only one primitive is supported");
		return Error::USER_DATA;
	}

	HashMapAuto<CString, StringAuto> extras(m_alloc);
	ANKI_CHECK(getExtras(mesh.extras, extras));

	File file;
	ANKI_CHECK(file.open(modelFname.toCString(), FileOpenFlag::WRITE));

	ANKI_CHECK(file.writeText("<model>\n"));
	ANKI_CHECK(file.writeText("\t<modelPatches>\n"));

	ANKI_CHECK(file.writeText("\t\t<modelPatch>\n"));

	ANKI_CHECK(file.writeText("\t\t\t<mesh>%s%s.ankimesh</mesh>\n", m_rpath.cstr(), mesh.name));

	{
		StringAuto name(m_alloc);
		name.sprintf("%s_lod1", mesh.name);
		ANKI_CHECK(file.writeText("\t\t\t<mesh1>%s%s.ankimesh</mesh1>\n", m_rpath.cstr(), name.cstr()));
	}

	{
		StringAuto name(m_alloc);
		name.sprintf("%s_lod2", mesh.name);
		ANKI_CHECK(file.writeText("\t\t\t<mesh2>%s%s.ankimesh</mesh2>\n", m_rpath.cstr(), name.cstr()));
	}

	auto mtlOverride = extras.find("material_override");
	if(mtlOverride != extras.getEnd())
	{
		ANKI_CHECK(file.writeText("\t\t\t<material>%s%s</material>\n", m_rpath.cstr(), mtlOverride->cstr()));
	}
	else
	{
		ANKI_CHECK(file.writeText(
			"\t\t\t<material>%s%s.ankimtl</material>\n", m_rpath.cstr(), mesh.primitives[0].material->name));
	}

	ANKI_CHECK(file.writeText("\t\t</modelPatch>\n"));

	ANKI_CHECK(file.writeText("\t</modelPatches>\n"));

	if(skinName)
	{
		ANKI_CHECK(file.writeText("\t<skeleton>%s%s.ankiskel</skeleton>\n", m_rpath.cstr(), skinName.cstr()));
	}

	ANKI_CHECK(file.writeText("</model>\n"));

	return Error::NONE;
}

Error GltfImporter::writeAnimation(const cgltf_animation& anim)
{
	StringAuto fname(m_alloc);
	fname.sprintf("%s%s.ankianim", m_outDir.cstr(), anim.name);
	ANKI_GLTF_LOGI("Importing animation %s", fname.cstr());

	// Gather the channels
	HashMapAuto<CString, Array<const cgltf_animation_channel*, 3>> channelMap(m_alloc);

	for(U i = 0; i < anim.channels_count; ++i)
	{
		const cgltf_animation_channel& channel = anim.channels[i];
		StringAuto channelName = getNodeName(*channel.target_node);

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
		}
	}

	// Write file
	File file;
	ANKI_CHECK(file.open(fname.toCString(), FileOpenFlag::WRITE));

	ANKI_CHECK(file.writeText("%s\n<animation>\n", XML_HEADER));
	ANKI_CHECK(file.writeText("\t<channels>\n"));

	for(auto it = channelMap.getBegin(); it != channelMap.getEnd(); ++it)
	{
		Array<const cgltf_animation_channel*, 3> arr = *it;
		const cgltf_animation_channel& channel = (arr[0]) ? *arr[0] : ((arr[1]) ? *arr[1] : *arr[2]);
		StringAuto channelName = getNodeName(*channel.target_node);

		ANKI_CHECK(file.writeText("\t\t<channel name=\"%s\">\n", channelName.cstr()));

		// Positions
		ANKI_CHECK(file.writeText("\t\t\t<positionKeys>\n"));
		if(arr[0])
		{
			const cgltf_animation_channel& channel = *arr[0];
			DynamicArrayAuto<F32> keys(m_alloc);
			readAccessor(*channel.sampler->input, keys);
			DynamicArrayAuto<Vec3> positions(m_alloc);
			readAccessor(*channel.sampler->output, positions);
			if(keys.getSize() != positions.getSize())
			{
				ANKI_GLTF_LOGE("Position count should match they keyframes");
				return Error::USER_DATA;
			}

			for(U32 i = 0; i < keys.getSize(); ++i)
			{
				ANKI_CHECK(file.writeText("\t\t\t\t<key time=\"%f\">%f %f %f</key>\n",
					keys[i],
					positions[i].x(),
					positions[i].y(),
					positions[i].z()));
			}
		}
		ANKI_CHECK(file.writeText("\t\t\t</positionKeys>\n"));

		// Rotations
		ANKI_CHECK(file.writeText("\t\t\t<rotationKeys>\n"));
		if(arr[1])
		{
			const cgltf_animation_channel& channel = *arr[1];
			DynamicArrayAuto<F32> keys(m_alloc);
			readAccessor(*channel.sampler->input, keys);
			DynamicArrayAuto<Quat> rotations(m_alloc);
			readAccessor(*channel.sampler->output, rotations);
			if(keys.getSize() != rotations.getSize())
			{
				ANKI_GLTF_LOGE("Rotation count should match they keyframes");
				return Error::USER_DATA;
			}

			for(U32 i = 0; i < keys.getSize(); ++i)
			{
				ANKI_CHECK(file.writeText("\t\t\t\t<key time=\"%f\">%f %f %f %f</key>\n",
					keys[i],
					rotations[i].x(),
					rotations[i].y(),
					rotations[i].z(),
					rotations[i].w()));
			}
		}
		ANKI_CHECK(file.writeText("\t\t\t</rotationKeys>\n"));

		// Scales
		ANKI_CHECK(file.writeText("\t\t\t<scaleKeys>\n"));
		if(arr[2])
		{
			const cgltf_animation_channel& channel = *arr[2];
			DynamicArrayAuto<F32> keys(m_alloc);
			readAccessor(*channel.sampler->input, keys);
			DynamicArrayAuto<Vec3> scales(m_alloc);
			readAccessor(*channel.sampler->output, scales);
			if(keys.getSize() != scales.getSize())
			{
				ANKI_GLTF_LOGE("Scale count should match they keyframes");
				return Error::USER_DATA;
			}

			for(U32 i = 0; i < keys.getSize(); ++i)
			{
				const F32 scaleEpsilon = 0.0001f;
				if(absolute(scales[i][0] - scales[i][1]) > scaleEpsilon
					|| absolute(scales[i][0] - scales[i][2]) > scaleEpsilon)
				{
					ANKI_GLTF_LOGE("Expecting uniform scale");
					return Error::USER_DATA;
				}

				ANKI_CHECK(file.writeText("\t\t\t\t<key time=\"%f\">%f</key>\n", keys[i], scales[i].x()));
			}
		}
		ANKI_CHECK(file.writeText("\t\t\t</scaleKeys>\n"));

		ANKI_CHECK(file.writeText("\t\t</channel>\n"));
	}

	ANKI_CHECK(file.writeText("\t</channels>\n"));
	ANKI_CHECK(file.writeText("</animation>\n"));

	return Error::NONE;
}

Error GltfImporter::writeSkeleton(const cgltf_skin& skin)
{
	StringAuto fname(m_alloc);
	fname.sprintf("%s%s.ankiskel", m_outDir.cstr(), skin.name);
	ANKI_GLTF_LOGI("Importing skeleton %s", fname.cstr());

	// Get matrices
	DynamicArrayAuto<Mat4> boneMats(m_alloc);
	readAccessor(*skin.inverse_bind_matrices, boneMats);
	if(boneMats.getSize() != skin.joints_count)
	{
		ANKI_GLTF_LOGE("Bone matrices should match the joint count");
		return Error::USER_DATA;
	}

	// Write file
	File file;
	ANKI_CHECK(file.open(fname.toCString(), FileOpenFlag::WRITE));

	ANKI_CHECK(file.writeText("%s\n<skeleton>\n", XML_HEADER));

	for(U32 i = 0; i < skin.joints_count; ++i)
	{
		const cgltf_node& boneNode = *skin.joints[i];

		StringAuto parent(m_alloc);

		// Name & parent
		ANKI_CHECK(file.writeText("\t<bone name=\"%s\" ", getNodeName(boneNode).cstr()));
		if(boneNode.parent)
		{
			ANKI_CHECK(file.writeText("parent=\"%s\" ", getNodeName(*boneNode.parent).cstr()));
		}

		// Bone transform
		ANKI_CHECK(file.writeText("boneTransform=\""));
		for(U32 j = 0; j < 16; j++)
		{
			ANKI_CHECK(file.writeText("%f ", boneMats[i][j]));
		}
		ANKI_CHECK(file.writeText("\" "));

		// Transform
		Transform trf;
		ANKI_CHECK(getNodeTransform(boneNode, trf));
		Mat4 mat{trf};
		ANKI_CHECK(file.writeText("tansform=\""));
		for(U j = 0; j < 16; j++)
		{
			ANKI_CHECK(file.writeText("%f ", mat[j]));
		}
		ANKI_CHECK(file.writeText("\" "));

		ANKI_CHECK(file.writeText("/>\n"));
	}

	ANKI_CHECK(file.writeText("</skeleton>\n"));

	return Error::NONE;
}

Error GltfImporter::writeCollisionMesh(const cgltf_mesh& mesh)
{
	StringAuto fname(m_alloc);
	fname.sprintf("%s%s.ankicl", m_outDir.cstr(), mesh.name);
	ANKI_GLTF_LOGI("Importing collision mesh %s", fname.cstr());

	// Write file
	File file;
	ANKI_CHECK(file.open(fname.toCString(), FileOpenFlag::WRITE));

	ANKI_CHECK(file.writeText("%s\n", XML_HEADER));

	ANKI_CHECK(file.writeText("<collisionShape>\n\t<type>staticMesh</type>\n\t<value>"
							  "%s%s_lod2.ankimesh</value>\n</collisionShape>\n",
		m_rpath.cstr(),
		mesh.name));

	return Error::NONE;
}

Error GltfImporter::writeLight(const cgltf_node& node, const HashMapAuto<CString, StringAuto>& parentExtras)
{
	const cgltf_light& light = *node.light;
	StringAuto nodeName = getNodeName(node);
	ANKI_GLTF_LOGI("Importing light %s", nodeName.cstr());

	HashMapAuto<CString, StringAuto> extras(parentExtras);
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
		ANKI_GLTF_LOGE("Unsupporter light type %d", light.type);
		return Error::USER_DATA;
	}

	ANKI_CHECK(m_sceneFile.writeText("\nnode = scene:new%sLightNode(\"%s\")\n", lightTypeStr.cstr(), nodeName.cstr()));
	ANKI_CHECK(m_sceneFile.writeText("lcomp = node:getSceneNodeBase():getLightComponent()\n"));

	Vec3 color(light.color[0], light.color[1], light.color[2]);
	color *= light.intensity;
	color /= 100.0f; // Blender changed something
	ANKI_CHECK(
		m_sceneFile.writeText("lcomp:setDiffuseColor(Vec4.new(%f, %f, %f, 1))\n", color.x(), color.y(), color.z()));

	auto shadow = extras.find("shadow");
	if(shadow != extras.getEnd())
	{
		if(*shadow == "true")
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
		ANKI_CHECK(m_sceneFile.writeText(
			"lcomp:setRadius(%f)\n", (light.range > 0.0f) ? light.range : computeLightRadius(color)));
	}
	else if(light.type == cgltf_light_type_spot)
	{
		ANKI_CHECK(m_sceneFile.writeText(
			"lcomp:setDistance(%f)\n", (light.range > 0.0f) ? light.range : computeLightRadius(color)));

		const F32 outer = light.spot_outer_cone_angle * 2.0f;
		ANKI_CHECK(m_sceneFile.writeText("lcomp:setOuterAngle(%f)\n", outer));

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

		ANKI_CHECK(m_sceneFile.writeText("lcomp:setInnerAngle(%f)\n", inner));
	}

	auto lensFlaresFname = extras.find("lens_flare");
	if(lensFlaresFname != extras.getEnd())
	{
		ANKI_CHECK(m_sceneFile.writeText("node:loadLensFlare(\"%s\")\n", lensFlaresFname->cstr()));

		auto lsSpriteSize = extras.find("lens_flare_first_sprite_size");
		auto lsColor = extras.find("lens_flare_color");

		if(lsSpriteSize != extras.getEnd() || lsColor != extras.getEnd())
		{
			ANKI_CHECK(m_sceneFile.writeText("lfcomp = node:getSceneNodeBase():getLensFlareComponent()\n"));
		}

		if(lsSpriteSize != extras.getEnd())
		{
			DynamicArrayAuto<F64> numbers(m_alloc);
			const U count = 2;
			ANKI_CHECK(parseArrayOfNumbers(lsSpriteSize->toCString(), numbers, &count));

			ANKI_CHECK(m_sceneFile.writeText("lfcomp:setFirstFlareSize(Vec2.new(%f, %f))\n", numbers[0], numbers[1]));
		}

		if(lsColor != extras.getEnd())
		{
			DynamicArrayAuto<F64> numbers(m_alloc);
			const U count = 4;
			ANKI_CHECK(parseArrayOfNumbers(lsColor->toCString(), numbers, &count));

			ANKI_CHECK(m_sceneFile.writeText("lfcomp:setColorMultiplier(Vec4.new(%f, %f, %f, %f))\n",
				numbers[0],
				numbers[1],
				numbers[2],
				numbers[3]));
		}
	}

	auto lightEventIntensity = extras.find("lens_flare");
	auto lightEventFrequency = extras.find("light_event_frequency");
	if(lightEventIntensity != extras.getEnd() || lightEventFrequency != extras.getEnd())
	{
		ANKI_CHECK(m_sceneFile.writeText("event = events:newLightEvent(0.0, -1.0, node:getSceneNodeBase())\n"));

		if(lightEventIntensity != extras.getEnd())
		{
			DynamicArrayAuto<F64> numbers(m_alloc);
			const U count = 4;
			ANKI_CHECK(parseArrayOfNumbers(lightEventIntensity->toCString(), numbers, &count));
			ANKI_CHECK(m_sceneFile.writeText("event:setIntensityMultiplier(Vec4.new(%f, %f, %f, %f))\n",
				numbers[0],
				numbers[1],
				numbers[2],
				numbers[3]));
		}

		if(lightEventFrequency != extras.getEnd())
		{
			DynamicArrayAuto<F64> numbers(m_alloc);
			const U count = 2;
			ANKI_CHECK(parseArrayOfNumbers(lightEventFrequency->toCString(), numbers, &count));
			ANKI_CHECK(m_sceneFile.writeText("event:setFrequency(%f, %f)\n", numbers[0], numbers[1]));
		}
	}

	return Error::NONE;
}

Error GltfImporter::writeCamera(const cgltf_node& node, const HashMapAuto<CString, StringAuto>& parentExtras)
{
	if(node.camera->type != cgltf_camera_type_perspective)
	{
		ANKI_GLTF_LOGW("Unsupported camera type: %s", getNodeName(node).cstr());
		return Error::NONE;
	}

	const cgltf_camera_perspective& cam = node.camera->perspective;
	ANKI_GLTF_LOGI("Importing camera %s", getNodeName(node).cstr());

	ANKI_CHECK(m_sceneFile.writeText("\nnode = scene:newPerspectiveCameraNode(\"%s\")\n", getNodeName(node).cstr()));
	ANKI_CHECK(m_sceneFile.writeText("scene:setActiveCameraNode(node:getSceneNodeBase())\n"));
	ANKI_CHECK(m_sceneFile.writeText("frustumc = node:getSceneNodeBase():getFrustumComponent()\n"));

	ANKI_CHECK(m_sceneFile.writeText("frustumc:setPerspective(%f, %f, getMainRenderer():getAspectRatio() * %f, %f)\n",
		cam.znear,
		cam.zfar,
		cam.yfov,
		cam.yfov));

	return Error::NONE;
}

Error GltfImporter::writeModelNode(const cgltf_node& node, const HashMapAuto<CString, StringAuto>& parentExtras)
{
	ANKI_GLTF_LOGI("Importing model node %s", getNodeName(node).cstr());

	HashMapAuto<CString, StringAuto> extras(parentExtras);
	ANKI_CHECK(getExtras(node.extras, extras));

	StringAuto modelFname(m_alloc);
	modelFname.sprintf("%s%s_%s.ankimdl", m_rpath.cstr(), node.mesh->name, node.mesh->primitives[0].material->name);

	ANKI_CHECK(m_sceneFile.writeText(
		"\nnode = scene:newModelNode(\"%s\", \"%s\")\n", getNodeName(node).cstr(), modelFname.cstr()));

	return Error::NONE;
}

} // end namespace anki
