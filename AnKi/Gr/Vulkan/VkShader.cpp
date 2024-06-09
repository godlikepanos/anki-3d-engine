// Copyright (C) 2009-2023, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <AnKi/Gr/Vulkan/VkShader.h>
#include <AnKi/Gr/Vulkan/VkGrManager.h>

#define ANKI_DUMP_SHADERS 0

#if ANKI_DUMP_SHADERS
#	include <AnKi/Util/File.h>
#	include <AnKi/Gr/GrManager.h>
#endif

namespace anki {

Shader* Shader::newInstance(const ShaderInitInfo& init)
{
	ShaderImpl* impl = anki::newInstance<ShaderImpl>(GrMemoryPool::getSingleton(), init.getName());
	const Error err = impl->init(init);
	if(err)
	{
		deleteInstance(GrMemoryPool::getSingleton(), impl);
		impl = nullptr;
	}
	return impl;
}

ShaderImpl::~ShaderImpl()
{
}

Error ShaderImpl::init(const ShaderInitInfo& inf)
{
	inf.validate();
	ANKI_ASSERT((inf.m_binary.getSize() % sizeof(U32)) == 0);
	m_shaderType = inf.m_shaderType;
	m_hasDiscard = inf.m_reflection.m_fragment.m_discards;
	m_shaderBinarySize = U32(inf.m_binary.getSizeInBytes());
	m_reflection = inf.m_reflection;
	m_reflection.validate();

	m_spirvBin.resize(inf.m_binary.getSize() / sizeof(U32));
	memcpy(m_spirvBin.getBegin(), inf.m_binary.getBegin(), inf.m_binary.getSizeInBytes());

#if ANKI_DUMP_SHADERS
	{
		StringRaii fnameSpirv(getAllocator());
		fnameSpirv.sprintf("%s/%s_t%u_%05u.spv", getManager().getCacheDirectory().cstr(), getName().cstr(), U(m_shaderType), getUuid());

		File fileSpirv;
		ANKI_CHECK(fileSpirv.open(fnameSpirv.toCString(), FileOpenFlag::kBinary | FileOpenFlag::kWrite | FileOpenFlag::kSpecial));
		ANKI_CHECK(fileSpirv.write(&inf.m_binary[0], inf.m_binary.getSize()));
	}
#endif

	return Error::kNone;
}

} // end namespace anki
