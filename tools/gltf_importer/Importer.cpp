// Copyright (C) 2009-2019, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include "Importer.h"

namespace anki
{

static Error getUniformScale(const Mat4& m, F32& out)
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

Importer::Importer()
{
	m_hive = m_alloc.newInstance<ThreadHive>(getCpuCoresCount(), m_alloc, true);
}

Importer::~Importer()
{
	if(m_gltf)
	{
		cgltf_free(m_gltf);
		m_gltf = nullptr;
	}

	m_alloc.deleteInstance(m_hive);
}

Error Importer::load(CString inputFname, CString outDir, CString rpath, CString texrpath)
{
	m_inputFname.create(inputFname);
	m_outDir.create(outDir);
	m_rpath.create(rpath);
	m_texrpath.create(texrpath);

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

	return Error::NONE;
}

Error Importer::writeAll()
{
	// Export meshes
	for(U i = 0; i < m_gltf->meshes_count; ++i)
	{
		ANKI_CHECK(writeMesh(m_gltf->meshes[i]));
	}

	// Export materials
	for(U i = 0; i < m_gltf->materials_count; ++i)
	{
		ANKI_CHECK(writeMaterial(m_gltf->materials[i]));
	}

	StringAuto sceneFname(m_alloc);
	sceneFname.sprintf("%sscene.lua", m_outDir.cstr());
	ANKI_CHECK(m_sceneFile.open(sceneFname.toCString(), FileOpenFlag::WRITE));

	// Nodes
	for(const cgltf_scene* scene = m_gltf->scenes; scene < m_gltf->scenes + m_gltf->scenes_count; ++scene)
	{
		for(cgltf_node* const* node = scene->nodes; node < scene->nodes + scene->nodes_count; ++node)
		{
			ANKI_CHECK(visitNode(*(*node)));
		}
	}

	return Error::NONE;
}

Error Importer::getExtras(const cgltf_extras& extras, HashMapAuto<StringAuto, StringAuto>& out)
{
	// TODO
	return Error::NONE;
}

Error Importer::parseArrayOfNumbers(CString str, DynamicArrayAuto<F64>& out, const U* expectedArraySize)
{
	StringListAuto list(m_alloc);
	list.splitString(str, ' ');

	out.create(list.getSize());

	Error err = Error::NONE;
	auto it = list.getBegin();
	auto end = list.getEnd();
	U i = 0;
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

Error Importer::visitNode(const cgltf_node& node)
{
	if(node.light)
	{
		ANKI_CHECK(writeLight(node));
	}
	else if(node.camera)
	{
		ANKI_CHECK(writeCamera(node));
	}
	else if(node.mesh)
	{
		ANKI_CHECK(writeModel(*node.mesh));
		ANKI_CHECK(writeModelNode(node));
	}
	else
	{
		ANKI_GLTF_LOGW("Ignoring node %s", node.name);
	}

	return Error::NONE;
}

Error Importer::writeTransform(const cgltf_node& node)
{
	Transform trf = Transform::getIdentity();
	if(node.has_matrix)
	{
		Mat4 mat = Mat4(node.matrix).getTransposed();
		F32 scale;
		ANKI_CHECK(getUniformScale(mat, scale));
		removeScale(mat);
		trf = Transform(mat);
	}
	else
	{
		if(node.has_translation)
		{
			trf.setOrigin(Vec4(node.translation[0], node.translation[1], node.translation[2], 0.0f));
		}

		if(node.has_rotation)
		{
			trf.setRotation(Mat3x4(Quat(node.rotation[0], node.rotation[1], node.rotation[2], node.rotation[3])));
		}

		if(node.has_scale)
		{
			if(node.scale[0] != node.scale[1] || node.scale[0] != node.scale[2])
			{
				ANKI_GLTF_LOGE("Expecting uniform scale");
				return Error::USER_DATA;
			}

			trf.setScale(node.scale[0]);
		}
	}

	ANKI_CHECK(m_sceneFile.writeText("trf = Transform.new()\n"));
	ANKI_CHECK(m_sceneFile.writeText(
		"trf:setOrigin(%f, %f, %f, 0)\n", trf.getOrigin().x(), trf.getOrigin().y(), trf.getOrigin().z()));

	ANKI_CHECK(m_sceneFile.writeText("rot = Mat3x4.new()\n"));
	ANKI_CHECK(m_sceneFile.writeText("rot:setAll(\n"));
	for(U i = 0; i < 12; i++)
	{
		ANKI_CHECK(m_sceneFile.writeText((i != 11) ? "%f, " : "%f)\n", trf.getRotation()[i]));
	}
	ANKI_CHECK(m_sceneFile.writeText("trf:setRotation(rot)\n"));

	ANKI_CHECK(m_sceneFile.writeText("trf:setScale(%f)\n", trf.getScale()));

	ANKI_CHECK(m_sceneFile.writeText("node:getSceneNodeBase():getMoveComponent():setLocalTransform(trf)\n"));

	return Error::NONE;
}

Error Importer::writeModel(const cgltf_mesh& mesh)
{
	StringAuto modelFname(m_alloc);
	modelFname.sprintf("%s%s_%s.ankimdl", m_outDir.cstr(), mesh.name, mesh.primitives[0].material->name);
	ANKI_GLTF_LOGI("Exporting model %s", modelFname.cstr());

	if(mesh.primitives_count != 1)
	{
		ANKI_GLTF_LOGE("For now only one primitive is supported");
		return Error::USER_DATA;
	}

	HashMapAuto<StringAuto, StringAuto> extras(m_alloc);
	ANKI_CHECK(getExtras(mesh.extras, extras));

	File file;
	ANKI_CHECK(file.open(modelFname.toCString(), FileOpenFlag::WRITE));

	ANKI_CHECK(file.writeText("<model>\n"));
	ANKI_CHECK(file.writeText("\t<modelPatches>\n"));

	ANKI_CHECK(file.writeText("\t\t<modelPatch>\n"));

	ANKI_CHECK(file.writeText("\t\t\t<mesh>%s%s.ankimesh</mesh>\n", m_rpath.cstr(), mesh.name));

	auto lod1 = extras.find(StringAuto(m_alloc, "lod1"));
	if(lod1 != extras.getEnd())
	{
		ANKI_CHECK(file.writeText("\t\t\t<mesh1>%s%s</mesh1>\n", m_rpath.cstr(), lod1->cstr()));
	}

	auto mtlOverride = extras.find(StringAuto(m_alloc, "material_override"));
	if(mtlOverride != extras.getEnd())
	{
		ANKI_CHECK(file.writeText("\t\t\t<material>%s%s</material>\n", m_rpath.cstr(), mtlOverride->cstr()));
	}
	else
	{
		ANKI_CHECK(
			file.writeText("\t\t\t<material>%s%s</material>\n", m_rpath.cstr(), mesh.primitives[0].material->name));
	}

	ANKI_CHECK(file.writeText("\t\t</modelPatch>\n"));

	// TODO: Skeleton

	ANKI_CHECK(file.writeText("\t</modelPatches>\n"));
	ANKI_CHECK(file.writeText("</model>\n"));

	return Error::NONE;
}

Error Importer::writeLight(const cgltf_node& node)
{
	const cgltf_light& light = *node.light;
	ANKI_GLTF_LOGI("Exporting light %s", light.name);

	HashMapAuto<StringAuto, StringAuto> extras(m_alloc);
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

	ANKI_CHECK(m_sceneFile.writeText("\nnode = scene:new%sLightNode(\"%s\")\n", lightTypeStr.cstr(), light.name));
	ANKI_CHECK(m_sceneFile.writeText("lcomp = node:getSceneNodeBase():getLightComponent()\n"));

	Vec3 color(light.color[0], light.color[1], light.color[2]);
	ANKI_CHECK(
		m_sceneFile.writeText("lcomp:setDiffuseColor(Vec4.new(%f, %f, %f, 1))\n", color.x(), color.y(), color.z()));

	auto shadow = extras.find(StringAuto(m_alloc, "shadow"));
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
		ANKI_CHECK(m_sceneFile.writeText("lcomp:setRadius(%f)\n", computeLightRadius(color)));
	}
	else if(light.type == cgltf_light_type_spot)
	{
		ANKI_CHECK(m_sceneFile.writeText("lcomp:setDistance(%f)\n", computeLightRadius(color)));
		ANKI_CHECK(m_sceneFile.writeText("lcomp:setOuterAngle(%f)\n", light.spot_outer_cone_angle));
		ANKI_CHECK(m_sceneFile.writeText("lcomp:setInnerAngle(%f)\n", light.spot_inner_cone_angle));
	}

	auto lensFlaresFname = extras.find(StringAuto(m_alloc, "lens_flare"));
	if(lensFlaresFname != extras.getEnd())
	{
		ANKI_CHECK(m_sceneFile.writeText("node:loadLensFlare(\"%s\")\n", lensFlaresFname->cstr()));

		auto lsSpriteSize = extras.find(StringAuto(m_alloc, "lens_flare_first_sprite_size"));
		auto lsColor = extras.find(StringAuto(m_alloc, "lens_flare_color"));

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

	auto lightEventIntensity = extras.find(StringAuto(m_alloc, "lens_flare"));
	auto lightEventFrequency = extras.find(StringAuto(m_alloc, "light_event_frequency"));
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

	ANKI_CHECK(writeTransform(node));

	return Error::NONE;
}

Error Importer::writeCamera(const cgltf_node& node)
{
	if(node.camera->type != cgltf_camera_type_orthographic)
	{
		ANKI_GLTF_LOGW("Unsupported camera type: %s", node.name);
		return Error::NONE;
	}

	const cgltf_camera_perspective& cam = node.camera->perspective;
	ANKI_GLTF_LOGI("Exporting camera %s", node.name);

	ANKI_CHECK(m_sceneFile.writeText("\nnode = scene:newPerspectiveCameraNode(\"%s\")", node.name));
	ANKI_CHECK(m_sceneFile.writeText("scene:setActiveCameraNode(node:getSceneNodeBase())\n"));
	ANKI_CHECK(m_sceneFile.writeText("frustumc = node:getSceneNodeBase():getFrustumComponent()\n"));

	ANKI_CHECK(
		m_sceneFile.writeText("frustumc:setPerspective(%f, %f, %f, 1.0 / getMainRenderer():getAspectRatio() * %f)\n",
			cam.znear,
			cam.zfar,
			cam.yfov,
			cam.yfov));

	ANKI_CHECK(writeTransform(node));

	return Error::NONE;
}

Error Importer::writeModelNode(const cgltf_node& node)
{
	ANKI_GLTF_LOGI("Exporting model node %s", node.name);

	StringAuto modelFname(m_alloc);
	modelFname.sprintf("%s%s_%s.ankimdl", m_rpath.cstr(), node.mesh->name, node.mesh->primitives[0].material->name);

	ANKI_CHECK(m_sceneFile.writeText("\nnode = scene:newModelNode(\"%s\", \"%s\")\n", node.name, modelFname.cstr()));

	ANKI_CHECK(writeTransform(node));

	// TODO: collision mesh

	return Error::NONE;
}

} // end namespace anki