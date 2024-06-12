// Copyright (C) 2009-present, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <AnKi/Gr/D3D/D3DCommon.h>
#include <AnKi/Gr/BackendCommon/GraphicsStateTracker.h>
#include <AnKi/Util/WeakArray.h>
#include <AnKi/Util/HashMap.h>

namespace anki {

/// @addtogroup d3d
/// @{

class GraphicsPipelineFactory
{
public:
	~GraphicsPipelineFactory();

	/// Write state to the command buffer.
	/// @note It's thread-safe.
	void flushState(GraphicsStateTracker& state, D3D12GraphicsCommandListX& cmdList);

private:
	GrHashMap<U64, ID3D12PipelineState*> m_map;
	RWMutex m_mtx;
};
/// @}

} // end namespace anki
