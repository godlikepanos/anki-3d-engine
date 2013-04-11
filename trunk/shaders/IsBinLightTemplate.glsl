// Template function that is used to bin lights to a tile

#if !defined(LIGHT_TYPE) || !defined(PLANE_TEST_FUNC) || !defined(COUNTER)
#	error See file
#endif

#define FUNC_NAME bin ## LIGHT_TYPE ## COUNTER

void FUNC_NAME(uint lightIndex)
{
	LIGHT_TYPE light = lights.

	uvec2 from = uvec2(0, 0);
	uvec2 to = uvec2(TILES_X_COUNT, TILES_Y_COUNT);

	while(1)
	{
		uvec2 m = (to - from) / uvec2(2);

		// Handle final
		if(m.x == 0 || m.y == 0)
		{
			if(PLANE_TEST_FUNC(light, tilegrid.planesFar[tileY][tileX])
				&& PLANE_TEST_FUNC(light, tilegrid.planesNear[tileY][tileX]))
			{
				// It's inside tile
				uint pos = atomicCounterIncrement(COUNTER);
			}
		}
	}
}

// Undef everything
#undef LIGHT_TYPE
#undef PLANE_TEST_FUNC
#undef COUNTER
#undef FUNC_NAME
