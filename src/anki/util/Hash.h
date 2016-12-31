// Copyright (C) 2009-2017, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <anki/util/StdTypes.h>

namespace anki
{

/// @addtogroup util_other
/// @{

/// Computes a hash of a buffer. This function implements the MurmurHash2 algorithm by Austin Appleby.
/// @param[in] buffer The buffer to hash.
/// @param bufferSize The size of the buffer.
/// @param seed A unique seed.
/// @return The hash.
U64 computeHash(const void* buffer, U32 bufferSize, U64 seed = 123);

/// Computes a hash of a buffer. This function implements the MurmurHash2 algorithm by Austin Appleby.
/// @param[in] buffer The buffer to hash.
/// @param bufferSize The size of the buffer.
/// @param prevHash The hash to append to.
/// @return The new hash.
U64 appendHash(const void* buffer, U32 bufferSize, U64 prevHash);
/// @}

} // end namespace anki
