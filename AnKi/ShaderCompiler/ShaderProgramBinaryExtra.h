// Copyright (C) 2009-2023, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <AnKi/ShaderCompiler/Common.h>
#include <AnKi/Util/Serializer.h>

namespace anki {

/// Serialize ShaderReflection
template<>
class SerializeFunctor<ShaderReflection>
{
public:
	template<typename TSerializer>
	void operator()(const ShaderReflection& x, TSerializer& serializer)
	{
		const U8* arr = reinterpret_cast<const U8*>(&x);
		serializer.doArray("binblob", 0, arr, sizeof(ShaderReflection));
	}
};

/// Deserialize ShaderReflection
template<>
class DeserializeFunctor<ShaderReflection>
{
public:
	template<typename TDeserializer>
	void operator()(ShaderReflection& x, TDeserializer& deserializer)
	{
		U8* arr = reinterpret_cast<U8*>(&x);
		deserializer.doArray("binblob", 0, arr, sizeof(ShaderReflection));
	}
};

} // end namespace anki
