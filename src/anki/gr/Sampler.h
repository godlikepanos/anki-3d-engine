// Copyright (C) 2009-2017, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <anki/gr/GrObject.h>
#include <anki/gr/Texture.h>

namespace anki
{

/// @addtogroup graphics
/// @{

/// GPU sampler.
class Sampler final : public GrObject
{
	ANKI_GR_OBJECT

anki_internal:
	UniquePtr<SamplerImpl> m_impl;

	static const GrObjectType CLASS_TYPE = GrObjectType::SAMPLER;

	/// Construct.
	Sampler(GrManager* manager, U64 hash, GrObjectCache* cache);

	/// Destroy.
	~Sampler();

	/// Initialize it.
	void init(const SamplerInitInfo& init);
};
/// @}

} // end namespace anki
