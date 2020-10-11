// Copyright (C) 2009-2020, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <anki/shaders/include/Common.h>

ANKI_BEGIN_NAMESPACE

#define ANKI_EVSM4 0 // 2 component EVSM or 4 component EVSM

const F32 EVSM_POSITIVE_CONSTANT = 40.0f; // EVSM positive constant
const F32 EVSM_NEGATIVE_CONSTANT = 5.0f; // EVSM negative constant
const F32 EVSM_BIAS = 0.01f;
const F32 EVSM_LIGHT_BLEEDING_REDUCTION = 0.05f;

ANKI_END_NAMESPACE
