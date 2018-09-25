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

	tinygltf::TinyGLTF m_loader;
	tinygltf::Model m_model;

	/// Load the scene.
	Error load();

	Error exportAll();

private:
	Error exportMesh(const tinygltf::Mesh& mesh);

	void getAttributeInfo(
		const tinygltf::Primitive& primitive, CString attribName, const U8*& buff, U32& stride, U32& count) const;
};

} // end namespace anki