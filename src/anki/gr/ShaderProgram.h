// Copyright (C) 2009-2017, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <anki/gr/GrObject.h>

namespace anki
{

/// @addtogroup graphics
/// @{

/// GPU program.
class ShaderProgram final : public GrObject
{
	ANKI_GR_OBJECT

anki_internal:
	UniquePtr<ShaderProgramImpl> m_impl;

	static const GrObjectType CLASS_TYPE = GrObjectType::SHADER_PROGRAM;

	/// Construct.
	ShaderProgram(GrManager* manager, U64 hash, GrObjectCache* cache);

	/// Destroy.
	~ShaderProgram();

	/// Create vertex+fragment.
	void init(ShaderPtr vert, ShaderPtr frag);

	/// Create compute.
	void init(ShaderPtr comp);

	/// Create with all.
	void init(ShaderPtr vert, ShaderPtr tessc, ShaderPtr tesse, ShaderPtr geom, ShaderPtr frag);
};
/// @}

} // end namespace anki
