#ifndef ANKI_GL_GL_CLIENT_BUFFER_HANDLE_H
#define ANKI_GL_GL_CLIENT_BUFFER_HANDLE_H

#include "anki/gl/GlHandle.h"

namespace anki {

// Forward
class GlClientBuffer;
class GlJobChainHandle;

/// @addtogroup opengl_other
/// @{

/// Client buffer handle
class GlClientBufferHandle: public GlHandle<GlClientBuffer>
{
public:
	typedef GlHandle<GlClientBuffer> Base;

	/// @name Constructors/Destructor
	/// @{
	GlClientBufferHandle();

	/// Create the buffer using preallocated memory or memory from the chain
	/// @param chain The job chain this client memory belongs to
	/// @param size The size of the buffer
	/// @param preallocatedMem Preallocated memory. Can be nullptr
	/// @return The address of the new memory
	GlClientBufferHandle(
		GlJobChainHandle& chain, PtrSize size, void* preallocatedMem);

	~GlClientBufferHandle();
	/// @}

	/// Get the base address
	void* getBaseAddress();

	/// Get size
	PtrSize getSize() const;
};

/// @}

} // end namespace anki

#endif
