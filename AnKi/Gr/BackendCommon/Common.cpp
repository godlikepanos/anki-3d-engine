// Copyright (C) 2009-present, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <AnKi/Gr/BackendCommon/Common.h>

namespace anki {

NumericCVar<U32> g_maxBindlessSampledTextureCountCVar(CVarSubsystem::kGr, "MaxBindlessSampledTextureCountCVar", 512, 16, kMaxU16);

} // end namespace anki
