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
class Sampler : public GrObject
{
	ANKI_GR_OBJECT

public:
	static const GrObjectType CLASS_TYPE = GrObjectType::SAMPLER;

protected:
	/// Construct.
	Sampler(GrManager* manager)
		: GrObject(manager, CLASS_TYPE)
	{
	}

	/// Destroy.
	~Sampler()
	{
	}

private:
	/// Allocate and initialize new instance.
	static ANKI_USE_RESULT Sampler* newInstance(GrManager* manager, const SamplerInitInfo& init);
};
/// @}

} // end namespace anki
