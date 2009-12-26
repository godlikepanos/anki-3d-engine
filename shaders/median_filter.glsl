/*
3x3 Median
Morgan McGuire and Kyle Whitson
http://graphics.cs.williams.edu
*/

#define s2(a, b)				temp = a; a = min(a, b); b = max(temp, b);
#define mn3(a, b, c)			s2(a, b); s2(a, c);
#define mx3(a, b, c)			s2(b, c); s2(a, c);

#define mnmx3(a, b, c)			mx3(a, b, c); s2(a, b);                                   // 3 exchanges
#define mnmx4(a, b, c, d)		s2(a, b); s2(c, d); s2(a, c); s2(b, d);                   // 4 exchanges
#define mnmx5(a, b, c, d, e)	s2(a, b); s2(c, d); mn3(a, c, e); mx3(b, d, e);           // 6 exchanges
#define mnmx6(a, b, c, d, e, f) s2(a, d); s2(b, e); s2(c, f); mn3(a, b, c); mx3(d, e, f); // 7 exchanges


//=====================================================================================================================================
// MedianFilterRGB                                                                                                                    =
//=====================================================================================================================================
vec3 MedianFilterRGB( in sampler2D tex, in vec2 tex_coords )
{
	vec2 tex_inv_size = 1.0/vec2(textureSize(tex, 0));
  vec3 v[9];

  // Add the pixels which make up our window to the pixel array.
	for(int dX = -1; dX <= 1; ++dX) 
	{
		for(int dY = -1; dY <= 1; ++dY) 
		{
			vec2 offset = vec2(float(dX), float(dY));

			// If a pixel in the window is located at (x+dX, y+dY), put it at index (dX + R)(2R + 1) + (dY + R) of the
			// pixel array. This will fill the pixel array, with the top left pixel of the window at pixel[0] and the
			// bottom right pixel of the window at pixel[N-1].
			v[(dX + 1) * 3 + (dY + 1)] = texture2D(tex, tex_coords + offset * tex_inv_size).rgb;
		}
	}

  vec3 temp;

  // Starting with a subset of size 6, remove the min and max each time
  mnmx6(v[0], v[1], v[2], v[3], v[4], v[5]);
  mnmx5(v[1], v[2], v[3], v[4], v[6]);
  mnmx4(v[2], v[3], v[4], v[7]);
  mnmx3(v[3], v[4], v[8]);
  return v[4];
}


//=====================================================================================================================================
// MedianFilterA                                                                                                                      =
//=====================================================================================================================================
float MedianFilterA( in sampler2D tex, in vec2 tex_coords )
{
	vec2 tex_inv_size = 1.0/vec2(textureSize(tex, 0));
  float v[9];

  // Add the pixels which make up our window to the pixel array.
	for(int dX = -1; dX <= 1; ++dX)
	{
		for(int dY = -1; dY <= 1; ++dY)
		{
			vec2 offset = vec2(float(dX), float(dY));

			// If a pixel in the window is located at (x+dX, y+dY), put it at index (dX + R)(2R + 1) + (dY + R) of the
			// pixel array. This will fill the pixel array, with the top left pixel of the window at pixel[0] and the
			// bottom right pixel of the window at pixel[N-1].
			v[(dX + 1) * 3 + (dY + 1)] = texture2D(tex, tex_coords + offset * tex_inv_size).a;
		}
	}

  float temp;

  // Starting with a subset of size 6, remove the min and max each time
  mnmx6(v[0], v[1], v[2], v[3], v[4], v[5]);
  mnmx5(v[1], v[2], v[3], v[4], v[6]);
  mnmx4(v[2], v[3], v[4], v[7]);
  mnmx3(v[3], v[4], v[8]);
  return v[4];
}


//=====================================================================================================================================
// MedianAndBlurA                                                                                                                     =
//=====================================================================================================================================
float MedianAndBlurA( in sampler2D tex, in vec2 tex_coords )
{
	vec2 tex_inv_size = 1.0/vec2(textureSize(tex, 0));
  float v[9];
  float sum = 0.0;

  // Add the pixels which make up our window to the pixel array.
	for(int dX = -1; dX <= 1; ++dX)
	{
		for(int dY = -1; dY <= 1; ++dY)
		{
			vec2 offset = vec2(float(dX), float(dY));

			// If a pixel in the window is located at (x+dX, y+dY), put it at index (dX + R)(2R + 1) + (dY + R) of the
			// pixel array. This will fill the pixel array, with the top left pixel of the window at pixel[0] and the
			// bottom right pixel of the window at pixel[N-1].
			float f = texture2D(tex, tex_coords + offset * tex_inv_size).a;
			v[(dX + 1) * 3 + (dY + 1)] = f;
			sum += f;
		}
	}

  float temp;

  // Starting with a subset of size 6, remove the min and max each time
  mnmx6(v[0], v[1], v[2], v[3], v[4], v[5]);
  mnmx5(v[1], v[2], v[3], v[4], v[6]);
  mnmx4(v[2], v[3], v[4], v[7]);
  mnmx3(v[3], v[4], v[8]);
  return v[4]*0.5 + sum/18.0;
}


//=====================================================================================================================================
// MedianFilterPCF                                                                                                                    =
//=====================================================================================================================================
float MedianFilterPCF( in sampler2DShadow tex, in vec3 tex_coords )
{
	vec2 tex_inv_size = 1.0/vec2(textureSize(tex, 0));
  float v[9];

  // Add the pixels which make up our window to the pixel array.
	for(int dX = -1; dX <= 1; ++dX)
	{
		for(int dY = -1; dY <= 1; ++dY)
		{
			vec2 offset = vec2(float(dX), float(dY));

			// If a pixel in the window is located at (x+dX, y+dY), put it at index (dX + R)(2R + 1) + (dY + R) of the
			// pixel array. This will fill the pixel array, with the top left pixel of the window at pixel[0] and the
			// bottom right pixel of the window at pixel[N-1].
			v[(dX + 1) * 3 + (dY + 1)] = shadow2D(tex, tex_coords + vec3(offset * tex_inv_size, 0.0)).r;
		}
	}

  float temp;

  // Starting with a subset of size 6, remove the min and max each time
  mnmx6(v[0], v[1], v[2], v[3], v[4], v[5]);
  mnmx5(v[1], v[2], v[3], v[4], v[6]);
  mnmx4(v[2], v[3], v[4], v[7]);
  mnmx3(v[3], v[4], v[8]);
  return v[4];
}
