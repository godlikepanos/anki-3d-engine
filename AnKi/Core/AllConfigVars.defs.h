// Copyright (C) 2009-2021, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#if !defined(ANKI_CONFIG_VAR_GROUP)
#	define ANKI_CONFIG_VAR_GROUP(name)
#endif

#if !defined(ANKI_CONFIG_VAR_U8)
#	define ANKI_CONFIG_VAR_U8(name, defaultValue, minValue, maxValue, description)
#endif

#if !defined(ANKI_CONFIG_VAR_U32)
#	define ANKI_CONFIG_VAR_U32(name, defaultValue, minValue, maxValue, description)
#endif

#if !defined(ANKI_CONFIG_VAR_PTR_SIZE)
#	define ANKI_CONFIG_VAR_PTR_SIZE(name, defaultValue, minValue, maxValue, description)
#endif

#if !defined(ANKI_CONFIG_VAR_F32)
#	define ANKI_CONFIG_VAR_F32(name, defaultValue, minValue, maxValue, description)
#endif

#if !defined(ANKI_CONFIG_VAR_BOOL)
#	define ANKI_CONFIG_VAR_BOOL(name, defaultValue, description)
#endif

#if !defined(ANKI_CONFIG_VAR_STRING)
#	define ANKI_CONFIG_VAR_STRING(name, defaultValue, description)
#endif

#include <AnKi/Core/ConfigVars.defs.h>
#include <AnKi/Gr/ConfigVars.defs.h>
#include <AnKi/Resource/ConfigVars.defs.h>
#include <AnKi/Scene/ConfigVars.defs.h>
#include <AnKi/Renderer/ConfigVars.defs.h>

#undef ANKI_CONFIG_VAR_GROUP
#undef ANKI_CONFIG_VAR_U8
#undef ANKI_CONFIG_VAR_U32
#undef ANKI_CONFIG_VAR_PTR_SIZE
#undef ANKI_CONFIG_VAR_F32
#undef ANKI_CONFIG_VAR_BOOL
#undef ANKI_CONFIG_VAR_STRING
