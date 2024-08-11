// Copyright (C) 2009-present, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <AnKi/Gr.h>
#include <AnKi/ShaderCompiler.h>
#include <AnKi/ShaderCompiler/ShaderParser.h>
#include <AnKi/ShaderCompiler/Dxc.h>
#include <Tests/Framework/Framework.h>

namespace anki {

inline ShaderPtr createShader(CString src, ShaderType type)
{
	ShaderCompilerString header;
	ShaderParser::generateAnkiShaderHeader(type, header);
	header += src;
	ShaderCompilerDynamicArray<U8> bin;
	ShaderCompilerString errorLog;

#if ANKI_GR_BACKEND_VULKAN
	Error err = compileHlslToSpirv(header, type, false, true, {}, bin, errorLog);
#else
	Error err = compileHlslToDxil(header, type, false, true, {}, bin, errorLog);
#endif
	if(err)
	{
		ANKI_TEST_LOGE("Compile error:\n%s", errorLog.cstr());
	}
	ANKI_TEST_EXPECT_NO_ERR(err);

	ShaderReflection refl;
#if ANKI_GR_BACKEND_VULKAN
	err = doReflectionSpirv(WeakArray(bin.getBegin(), bin.getSize()), type, refl, errorLog);
#else
	err = doReflectionDxil(bin, type, refl, errorLog);
#endif
	if(err)
	{
		ANKI_TEST_LOGE("Reflection error:\n%s", errorLog.cstr());
	}
	ANKI_TEST_EXPECT_NO_ERR(err);

	ShaderInitInfo initInf(type, bin);
	initInf.m_reflection = refl;

	return GrManager::getSingleton().newShader(initInf);
}

inline ShaderProgramPtr createVertFragProg(CString vert, CString frag)
{
	ShaderPtr vertS = createShader(vert, ShaderType::kVertex);
	ShaderPtr fragS = createShader(frag, ShaderType::kFragment);

	ShaderProgramInitInfo init;
	init.m_graphicsShaders[ShaderType::kVertex] = vertS.get();
	init.m_graphicsShaders[ShaderType::kFragment] = fragS.get();

	ShaderProgramPtr prog = GrManager::getSingleton().newShaderProgram(init);

	return prog;
}

const U kWidth = 1024;
const U kHeight = 768;

inline void commonInit(Bool validation = true)
{
	DefaultMemoryPool::allocateSingleton(allocAligned, nullptr);
	ShaderCompilerMemoryPool::allocateSingleton(allocAligned, nullptr);
	g_windowWidthCVar.set(kWidth);
	g_windowHeightCVar.set(kHeight);
	g_vsyncCVar.set(false);
	g_debugMarkersCVar.set(true);
	if(validation)
	{
		g_validationCVar.set(true);
		[[maybe_unused]] const Error err = CVarSet::getSingleton().setMultiple(Array<const Char*, 2>{"GpuValidation", "1"});
	}

	initWindow();
	ANKI_TEST_EXPECT_NO_ERR(Input::allocateSingleton().init());

	initGrManager();
}

inline void commonDestroy()
{
	GrManager::freeSingleton();
	Input::freeSingleton();
	NativeWindow::freeSingleton();
	Input::freeSingleton();
	ShaderCompilerMemoryPool::freeSingleton();
	DefaultMemoryPool::freeSingleton();
}

template<typename T>
inline BufferPtr createBuffer(BufferUsageBit usage, ConstWeakArray<T> data, CString name = {})
{
	BufferPtr copyBuff =
		GrManager::getSingleton().newBuffer(BufferInitInfo(data.getSizeInBytes(), BufferUsageBit::kTransferSource, BufferMapAccessBit::kWrite));

	T* inData = static_cast<T*>(copyBuff->map(0, kMaxPtrSize, BufferMapAccessBit::kWrite));
	for(U32 i = 0; i < data.getSize(); ++i)
	{
		inData[i] = data[i];
	}
	copyBuff->unmap();

	BufferPtr buff = GrManager::getSingleton().newBuffer(BufferInitInfo(
		data.getSizeInBytes(), usage | BufferUsageBit::kTransferDestination | BufferUsageBit::kTransferSource, BufferMapAccessBit::kNone, name));

	CommandBufferInitInfo cmdbInit;
	cmdbInit.m_flags |= CommandBufferFlag::kSmallBatch;
	CommandBufferPtr cmdb = GrManager::getSingleton().newCommandBuffer(cmdbInit);
	cmdb->copyBufferToBuffer(BufferView(copyBuff.get()), BufferView(buff.get()));
	cmdb->endRecording();

	FencePtr fence;
	GrManager::getSingleton().submit(cmdb.get(), {}, &fence);
	fence->clientWait(kMaxSecond);

	return buff;
};

template<typename T>
inline BufferPtr createBuffer(BufferUsageBit usage, T pattern, U32 count, CString name = {})
{
	DynamicArray<T> arr;
	arr.resize(count, pattern);
	return createBuffer(usage, ConstWeakArray<T>(arr), name);
}

template<typename T>
inline TexturePtr createTexture2d(const TextureInitInfo& texInit, T initialValue)
{
	BufferInitInfo buffInit;
	buffInit.m_mapAccess = BufferMapAccessBit::kWrite;
	buffInit.m_size = texInit.m_height * texInit.m_width * getFormatInfo(texInit.m_format).m_texelSize;
	buffInit.m_usage = BufferUsageBit::kTransferSource;

	BufferPtr staging = GrManager::getSingleton().newBuffer(buffInit);

	T* inData = static_cast<T*>(staging->map(0, kMaxPtrSize, BufferMapAccessBit::kWrite));
	const T* endData = inData + (buffInit.m_size / sizeof(T));
	for(; inData < endData; ++inData)
	{
		*inData = initialValue;
	}
	staging->unmap();

	TexturePtr tex = GrManager::getSingleton().newTexture(texInit);

	CommandBufferInitInfo cmdbInit;
	cmdbInit.m_flags |= CommandBufferFlag::kSmallBatch;

	CommandBufferPtr cmdb = GrManager::getSingleton().newCommandBuffer(cmdbInit);

	cmdb->copyBufferToTexture(BufferView(staging.get()), TextureView(tex.get(), TextureSubresourceDesc::all()));
	cmdb->endRecording();

	FencePtr fence;
	GrManager::getSingleton().submit(cmdb.get(), {}, &fence);
	fence->clientWait(kMaxSecond);

	return tex;
};

template<typename T>
inline void readBuffer(BufferPtr buff, DynamicArray<T>& out)
{
	BufferPtr tmpBuff;

	if(!!(buff->getMapAccess() & BufferMapAccessBit::kRead))
	{
		tmpBuff = buff;
	}
	else
	{
		BufferInitInfo buffInit;
		buffInit.m_mapAccess = BufferMapAccessBit::kRead;
		buffInit.m_size = buff->getSize();
		buffInit.m_usage = BufferUsageBit::kTransferDestination;
		tmpBuff = GrManager::getSingleton().newBuffer(buffInit);

		CommandBufferPtr cmdb =
			GrManager::getSingleton().newCommandBuffer(CommandBufferInitInfo(CommandBufferFlag::kGeneralWork | CommandBufferFlag::kSmallBatch));
		cmdb->copyBufferToBuffer(BufferView(buff.get()), BufferView(tmpBuff.get()));
		cmdb->endRecording();

		FencePtr fence;
		GrManager::getSingleton().submit(cmdb.get(), {}, &fence);
		fence->clientWait(kMaxSecond);
	}

	ANKI_ASSERT((buff->getSize() % sizeof(T)) == 0);
	out.resize(U32(buff->getSize() / sizeof(T)));

	const void* data = tmpBuff->map(0, kMaxPtrSize, BufferMapAccessBit::kRead);
	memcpy(out.getBegin(), data, buff->getSize());
	tmpBuff->unmap();
}

template<typename T>
inline void validateBuffer(BufferPtr buff, ConstWeakArray<T> values)
{
	DynamicArray<T> cpuBuff;
	readBuffer<T>(buff, cpuBuff);

	ANKI_ASSERT(values.getSize() == cpuBuff.getSize());

	for(U32 i = 0; i < values.getSize(); ++i)
	{
		ANKI_TEST_EXPECT_EQ(cpuBuff[i], values[i]);
	}
}

} // end namespace anki
