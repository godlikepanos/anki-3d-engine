#include "anki/gl/Drawcall.h"
#include "anki/util/Assert.h"
#include <cstring>

namespace anki {

//==============================================================================
Drawcall::Drawcall()
{
	memset(this, 0, sizeof(Drawcall));
	instancesCount = 1;
	drawCount = 1;
}

//==============================================================================
void Drawcall::enque()
{
	ANKI_ASSERT(instancesCount > 0);
	ANKI_ASSERT(drawCount > 0 && drawCount <= ANKI_MAX_MULTIDRAW_PRIMITIVES);

	if(indicesType != 0)
	{
		// DrawElements

		if(instancesCount == 1)
		{
			// No  instancing

			if(drawCount == 1)
			{
				// No multidraw

				glDrawElements(
					primitiveType, 
					countArray[0], 
					indicesType, 
					(const void*)offsetArray[0]);
			}
			else
			{
				// multidraw

#if ANKI_GL == ANKI_GL_DESKTOP
				glMultiDrawElements(
					primitiveType, 
					(int*)&countArray[0],
					indicesType, 
					(const void**)&offsetArray[0], 
					drawCount);
#else
				for(U i = 0; i < drawCount; i++)
				{
					glDrawElements(
						primitiveType, 
						countArray[i],
						indicesType,
						offsetArray[i]);
				}
#endif
			}
		}
		else
		{
			// Instancing
			glDrawElementsInstanced(
				primitiveType, 
				countArray[0], 
				indicesType, 
				(const void*)offsetArray[0], 
				instancesCount);
		}
	}
	else
	{
		// DrawArrays

		if(instancesCount == 1)
		{
			// No instancing

			if(drawCount == 1)
			{
				// No multidraw

				glDrawArrays(primitiveType, offsetArray[0], countArray[0]);
			}
			else
			{
				// Multidraw

				ANKI_ASSERT(0 && "TODO");
			}
		}
		else
		{
			// Instancing
			
			glDrawArraysInstanced(primitiveType, offsetArray[0], 
				countArray[0], instancesCount);
		}
	}
}

} // end namespace anki
