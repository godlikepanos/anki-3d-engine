// Copyright (C) 2009-present, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <AnKi/Scene/Common.h>
#include <AnKi/Resource/ResourceManager.h>

namespace anki {

constexpr U32 kSceneBinaryVersion = 1;

#define ANKI_SERIALIZE(var, version) \
	{ \
		static_assert(version <= kSceneBinaryVersion); \
		ANKI_CHECK(serializer.serialize(#var, version, false, var)); \
	}

#define ANKI_SERIALIZE_ALIAS(varName, var, version) \
	{ \
		static_assert(version <= kSceneBinaryVersion); \
		ANKI_CHECK(serializer.serialize(varName, version, false, var)); \
	}

#define ANKI_SERIALIZE_DEPRECATED(var, version) \
	{ \
		static_assert(version <= kSceneBinaryVersion); \
		ANKI_CHECK(serializer.serialize(#var, version, true, var)); \
	}

// Interface class for scene serialization
class SceneSerializer
{
public:
	SceneSerializer(Bool writingMode)
		: m_writeMode(writingMode)
	{
	}

	virtual Error write(CString name, ConstWeakArray<U32> values) = 0;
	virtual Error read(CString name, WeakArray<U32> values) = 0;

	virtual Error write(CString name, ConstWeakArray<I32> values) = 0;
	virtual Error read(CString name, WeakArray<I32> values) = 0;

	virtual Error write(CString name, ConstWeakArray<F32> values) = 0;
	virtual Error read(CString name, WeakArray<F32> values) = 0;

	virtual Error write(CString name, CString value) = 0;
	virtual Error read(CString name, SceneString& value) = 0;

	virtual Error write(CString name, ConstWeakArray<U8> byteArray) = 0;
	virtual Error read(CString name, SceneDynamicArray<U8>& byteArray, U32& arraySize) = 0;

	// For resources
	template<typename T>
	Error serialize(String varName, U32 varVersion, Bool varDeprecated, ResourcePtr<T>& rsrc)
	{
		SceneString filename;

		if(m_writeMode && rsrc)
		{
			filename = rsrc->getFilename();
		}

		ANKI_CHECK(serializeInternal(varName, varVersion, varDeprecated, filename));

		if(!m_writeMode && filename.getLength())
		{
			ANKI_CHECK(ResourceManager::getSingleton().loadResource(filename, rsrc));
		}

		return Error::kNone;
	}

	// For regular arithmetic
	template<typename T>
	Error serialize(String varName, U32 varVersion, Bool varDeprecated, T& varValue) requires(std::is_arithmetic_v<T>)
	{
		WeakArray<T> arr(&varValue, 1);
		return serializeInternal(varName, varVersion, varDeprecated, arr);
	}

	// For regular arithmetic
	Error serialize(String varName, U32 varVersion, Bool varDeprecated, F64& varValue)
	{
		F32 val = F32(varValue);
		WeakArray<F32> arr(&val, 1);
		ANKI_CHECK(serializeInternal(varName, varVersion, varDeprecated, arr));
		varValue = val;
		return Error::kNone;
	}

	// Vector 3
	template<typename T>
	Error serialize(String varName, U32 varVersion, Bool varDeprecated, TVec<T, 3>& varValue)
	{
		WeakArray<T> arr(&varValue[0], 3);
		return serializeInternal(varName, varVersion, varDeprecated, arr);
	}

	// Vector 4
	template<typename T>
	Error serialize(String varName, U32 varVersion, Bool varDeprecated, TVec<T, 4>& varValue)
	{
		WeakArray<T> arr(&varValue[0], 4);
		return serializeInternal(varName, varVersion, varDeprecated, arr);
	}

	// Enums
	template<typename T>
	Error serialize(String varName, U32 varVersion, Bool varDeprecated, T& varValue) requires(std::is_enum_v<T>)
	{
		static_assert(sizeof(T) <= sizeof(U32));
		U32 val = U32(varValue);
		WeakArray<U32> arr(&val, 1);
		ANKI_CHECK(serializeInternal(varName, varVersion, varDeprecated, arr));
		varValue = T(val);
		return Error::kNone;
	}

	// Strings
	Error serialize(String varName, U32 varVersion, Bool varDeprecated, SceneString& varValue)
	{
		return serializeInternal(varName, varVersion, varDeprecated, varValue);
	}

	Bool isInWriteMode() const
	{
		return m_writeMode;
	}

	Bool isInReadMode() const
	{
		return !m_writeMode;
	}

public:
	Bool m_writeMode = false;
	U32 m_crntBinaryVersion = kMaxU32;

	template<typename T>
	Error serializeInternal(CString varName, U32 varVersion, Bool varDeprecated, T& varValue)
	{
		ANKI_ASSERT(varVersion <= kSceneBinaryVersion);

		if(m_writeMode)
		{
			// Always writing in the latest version so skip deprecated vars
			const Bool skip = varDeprecated;

			if(!skip)
			{
				return write(varName, varValue);
			}
		}
		else
		{
			// Var is newer than the binary we are reading, skip it
			Bool skip = varVersion >= m_crntBinaryVersion;

			// Var is depracated and binary is newer, skip it
			skip = skip || (varDeprecated && m_crntBinaryVersion > varVersion);

			if(!skip)
			{
				return read(varName, varValue);
			}
		}

		return Error::kNone;
	}
};

// Serialize in a custom text format
class TextSceneSerializer : public SceneSerializer
{
public:
	TextSceneSerializer(File* file, Bool writingMode)
		: SceneSerializer(writingMode)
		, m_file(*file)
	{
	}

	Error write(CString name, ConstWeakArray<U32> values) final
	{
		if(values.getSize() == 1)
		{
			ANKI_CHECK(m_file.writeTextf("%s %u\n", name.cstr(), values[0]));
		}
		else
		{
			ANKI_CHECK(m_file.writeTextf("%s %u ", name.cstr(), values[0]));
			for(U32 i = 1; i < values.getSize(); ++i)
			{
				ANKI_CHECK(m_file.writeTextf((i < values.getSize() - 1) ? "%u " : "%u\n", values[i]));
			}
		}
		return Error::kNone;
	}

	Error read([[maybe_unused]] CString name, [[maybe_unused]] WeakArray<U32> values) final
	{
		ANKI_ASSERT(!"TODO");
		return Error::kNone;
	}

	Error write(CString name, ConstWeakArray<I32> values) final
	{
		if(values.getSize() == 1)
		{
			ANKI_CHECK(m_file.writeTextf("%s %d\n", name.cstr(), values[0]));
		}
		else
		{
			ANKI_CHECK(m_file.writeTextf("%s %d ", name.cstr(), values[0]));
			for(U32 i = 1; i < values.getSize(); ++i)
			{
				ANKI_CHECK(m_file.writeTextf((i < values.getSize() - 1) ? "%d " : "%d\n", values[i]));
			}
		}
		return Error::kNone;
	}

	Error read([[maybe_unused]] CString name, [[maybe_unused]] WeakArray<I32> values) final
	{
		ANKI_ASSERT(!"TODO");
		return Error::kNone;
	}

	Error write(CString name, ConstWeakArray<F32> values) final
	{
		if(values.getSize() == 1)
		{
			ANKI_CHECK(m_file.writeTextf("%s %f\n", name.cstr(), values[0]));
		}
		else
		{
			ANKI_CHECK(m_file.writeTextf("%s %f ", name.cstr(), values[0]));
			for(U32 i = 1; i < values.getSize(); ++i)
			{
				ANKI_CHECK(m_file.writeTextf((i < values.getSize() - 1) ? "%f " : "%f\n", values[i]));
			}
		}
		return Error::kNone;
	}

	Error read([[maybe_unused]] CString name, [[maybe_unused]] WeakArray<F32> values)
	{
		ANKI_ASSERT(!"TODO");
		return Error::kNone;
	}

	Error write(CString name, CString value) final
	{
		return m_file.writeTextf("%s %s\n", name.cstr(), value.cstr());
	}

	Error read([[maybe_unused]] CString name, [[maybe_unused]] SceneString& value) final
	{
		ANKI_ASSERT(!"TODO");
		return Error::kNone;
	}

	Error write(CString name, ConstWeakArray<U8> byteArray) final
	{
		ANKI_CHECK(m_file.writeTextf("%s %u ", name.cstr(), byteArray.getSize()));

		for(U32 i = 0; i < byteArray.getSize(); ++i)
		{
			ANKI_CHECK(m_file.writeTextf((i < byteArray.getSize() - 1) ? "%u " : "%u\n", byteArray[i]));
		}

		return Error::kNone;
	}

	Error read([[maybe_unused]] CString name, [[maybe_unused]] SceneDynamicArray<U8>& byteArray, [[maybe_unused]] U32& arraySize) final
	{
		ANKI_ASSERT(!"TODO");
		return Error::kNone;
	}

private:
	File& m_file;
};

} // end namespace anki
