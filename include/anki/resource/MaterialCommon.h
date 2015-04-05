// Copyright (C) 2009-2015, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#ifndef ANKI_RESOURSE_MATERIAL_COMMON_H
#define ANKI_RESOURSE_MATERIAL_COMMON_H

#include "anki/resource/Common.h"
#include "anki/resource/RenderingKey.h"
#include "anki/util/NonCopyable.h"
#include "anki/Gr.h"

namespace anki {

/// @addtogroup resource
/// @{

/// Data that will be used in material loading.
class MaterialResourceData: public NonCopyable
{
public:
	PipelineHandle m_pipelines[U(Pass::COUNT)];

	MaterialResourceData() = default;
	~MaterialResourceData() = default;

	ANKI_USE_RESULT Error create(ResourceManager& manager);
};
/// @}

} // end namespace anki

#endif

