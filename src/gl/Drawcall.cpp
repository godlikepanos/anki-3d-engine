#include "anki/gl/Drawcall.h"
#include "anki/util/Assert.h"

namespace anki {

//==============================================================================
void Drawcall::enque()
{
	ANKI_ASSERT(primitiveType && instancesCount > 0 && primCount > 0
		&& offsetsArray);

	if(indicesCountArray != nullptr)
	{
		// DrawElements

		ANKI_ASSERT(indicesCountArray && indicesType);

		if(instancesCount == 1)
		{
			// No  instancing

			if(primCount == 1)
			{
				// No multidraw

				glDrawElements(
					primitiveType, 
					indicesCountArray[0], 
					indicesType, 
					offsetsArray[0]);
			}
			else
			{
				// multidraw

#if ANKI_GL == ANKI_GL_DESKTOP
				glMultiDrawElements(
					primitiveType, 
					indicesCountArray, 
					indicesType, 
					offsetsArray, 
					primCount);
#else
				for(U i = 0; i < primCount; i++)
				{
					glDrawElements(
						primitiveType, 
						indicesCountArray[i],
						indicesType,
						offsetsArray[i]);
				}
#endif
			}
		}
		else
		{
			// Instancing
			glDrawElementsInstanced(
				primitiveType, 
				indicesCountArray[0], 
				indicesType, 
				offsetsArray[0], 
				instancesCount);
		}
	}
	else
	{
		// Draw arrays
		ANKI_ASSERT(0 && "ToDo");
	}
}

} // end namespace anki
