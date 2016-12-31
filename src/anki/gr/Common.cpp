// Copyright (C) 2009-2017, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <anki/gr/Common.h>

namespace anki
{

Array<CString, U(GpuVendor::COUNT)> GPU_VENDOR_STR = {{"UNKNOWN", "ARM", "NVIDIA", "AMD", "INTEL"}};

} // end namespace anki