// Copyright (C) 2009-2018, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include "Exporter.h"

#if defined(__GNUC__)
#	pragma GCC diagnostic push
#	pragma GCC diagnostic ignored "-Wmisleading-indentation"
#	pragma GCC diagnostic ignored "-Wshift-negative-value"
#endif
#define TINYGLTF_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <tinygltf/tiny_gltf.h>
#if defined(__GNUC__)
#	pragma GCC diagnostic pop
#endif

namespace anki
{

Error Exporter::load()
{
	std::string err, warn;
	Bool ret = m_loader.LoadASCIIFromFile(&m_model, &err, &warn, m_inputFilename.cstr());

	if(!warn.empty())
	{
		ANKI_LOGW(warn.c_str());
	}

	if(!err.empty())
	{
		ANKI_LOGE(err.c_str());
	}

	if(!ret)
	{
		ANKI_LOGE("Failed to parse glTF");
		return Error::USER_DATA;
	}

	return Error::NONE;
}

Error Exporter::exportMeshes()
{
	for(const tinygltf::Mesh& mesh : m_model.meshes)
	{
		if(mesh.primitives.size() != 1)
		{
			ANKI_LOGE("Can't handle this");
			return Error::USER_DATA;
		}
		const tinygltf::Primitive& primitive = mesh.primitives[0];

		// Get positions
		if(primitive.attributes.find("POSITION") != primitive.attributes.end())
		{
			ANKI_LOGE("Positions are missing for mesh %s", mesh.name.c_str());
			return Error::USER_DATA;
		}
		const tinygltf::Accessor& posAccessor = m_model.accessors[primitive.attributes.find("POSITION")->second];
		const U32 vertCount = posAccessor.count;
		const tinygltf::BufferView& posView = m_model.bufferViews[posAccessor.bufferView];
		const U8* posBuff = reinterpret_cast<const U8*>(
			&(m_model.buffers[posView.buffer].data[posAccessor.byteOffset + posView.byteOffset]));
		const PtrSize posStride = posView.byteStride;
		if(posStride < sizeof(Vec3))
		{
			ANKI_LOGE("Position stride is wrong for mesh %s", mesh.name.c_str());
			return Error::USER_DATA;
		}

		Vec3 posMin(posAccessor.minValues[0], posAccessor.minValues[1], posAccessor.minValues[2]);
		Vec3 posMax(posAccessor.maxValues[0], posAccessor.maxValues[1], posAccessor.maxValues[2]);
	}

	return Error::NONE;
}

Error Exporter::exportAll()
{
	// m_model.
}

} // end namespace anki