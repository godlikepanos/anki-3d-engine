// Template function that is used to bin lights to a tile

#if !defined(TYPE) || !defined(NAME)
#	error See file
#endif

#define CONCAT(x, y) x##y

void CONCAT(bin, TYPE)(uint lightIndex)
{
	LIGHT_TYPE light = lights. CONCAT(NAME, Lights) [lightIndex];

	uvec2 from = uvec2(0, 0);
	uvec2 to = uvec2(TILES_X_COUNT, TILES_Y_COUNT);

	while(1)
	{
		uvec2 m = (to - from) / uvec2(2); // Middle dist

		// Handle final
		if(m.x == 0 || m.y == 0)
		{
			if(CONCAT(NAME, LightInsidePlane)(
				light, tilegrid.planesFar[tileY][tileX])
				&& CONCAT(NAME, LightInsidePlane)(
				light, tilegrid.planesNear[tileY][tileX]))
			{
				// It's inside tile
				uint pos = 
					atomicCounterIncrement(CONCAT(NAME, LightsCounter));

				// Bin it
				tiles.tile[tileY][tileX]. CONCAT(NAME, LightIndices) [pos] = 
					lightIndex;
			}

			break;
		}

		// do the plane checks
		bool inside[2][2];

		// Top looking plane check
		if(CONCAT(NAME, LightInsidePlane)(
			light, tilegrid.planesY[from.y + m.y - 1]))
		{
			inside[][]
		}
	}
}

// Undef everything
#undef LIGHT_TYPE
#undef PLANE_TEST_FUNC
#undef COUNTER
#undef FUNC_NAME
