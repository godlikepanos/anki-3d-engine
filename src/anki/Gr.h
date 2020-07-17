// Copyright (C) 2009-2020, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <anki/gr/Buffer.h>
#include <anki/gr/Texture.h>
#include <anki/gr/TextureView.h>
#include <anki/gr/Sampler.h>
#include <anki/gr/Shader.h>
#include <anki/gr/ShaderProgram.h>
#include <anki/gr/Framebuffer.h>
#include <anki/gr/CommandBuffer.h>
#include <anki/gr/OcclusionQuery.h>
#include <anki/gr/TimestampQuery.h>
#include <anki/gr/Fence.h>
#include <anki/gr/AccelerationStructure.h>
#include <anki/gr/GrManager.h>
#include <anki/gr/RenderGraph.h>

#include <anki/gr/utils/ClassGpuAllocator.h>
#include <anki/gr/utils/FrameGpuAllocator.h>
#include <anki/gr/utils/Functions.h>
#include <anki/gr/utils/StackGpuAllocator.h>

/// @defgroup graphics Graphics API abstraction

/// @defgroup opengl OpenGL backend
/// @ingroup graphics

/// @defgroup vulkan Vulkan backend
/// @ingroup graphics
