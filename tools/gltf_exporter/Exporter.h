// Copyright (C) 2009-2018, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <src/anki/AnKi.h>
#include <tinygltf/tiny_gltf.h>

namespace anki
{

class Exporter
{
public:
	HeapAllocator<U8> m_alloc{allocAligned, nullptr};

	StringAuto m_inputFilename{m_alloc};
	StringAuto m_outputDirectory{m_alloc};
	StringAuto m_rpath{m_alloc};
	StringAuto m_texrpath{m_alloc};

	F32 m_normalsMergeCosAngle = cos(toRad(30.0));

	tinygltf::TinyGLTF m_loader;
	tinygltf::Model m_model;

	/// Load the scene.
	Error load();

	Error exportAll();

private:
	Error exportMesh(const tinygltf::Mesh& mesh);

	void getAttributeInfo(const tinygltf::Primitive& primitive,
		CString attribName,
		const U8*& buff,
		U32& stride,
		U32& count,
		Format& fmt) const;

	Error exportMaterial(const tinygltf::Material& mtl);

	Bool getTexture(const tinygltf::Material& mtl, CString texName, StringAuto& fname) const;
	Bool getMaterialAttrib(const tinygltf::Material& mtl, CString attribName, Vec4& value) const;
};

#define EXPORT_ASSERT(expr) \
	do \
	{ \
		if(!(expr)) \
		{ \
			ANKI_LOGE(#expr); \
			return Error::USER_DATA; \
		} \
	} while(false)

} // end namespace anki