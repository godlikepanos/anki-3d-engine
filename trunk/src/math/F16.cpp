#include "anki/math/F16.h"

namespace anki {

//==============================================================================
union Val32
{
	I32 i;
	F32 f;
	U32 u;
};

//==============================================================================
F16 F16::toF16(F32 f)
{
	F16 out;
	Val32 v32;
	v32.f = f;
	I32 i = v32.i;
	I32 s = (i >> 16) & 0x00008000;
	I32 e = ((i >> 23) & 0x000000ff) - (127 - 15);
	I32 m = i & 0x007fffff;

	if(e <= 0)
	{
		if(e < -10)
		{
			out.data = 0;
		}
		else
		{
			m = (m | 0x00800000) >> (1 - e);

			if(m & 0x00001000) 
			{
				m += 0x00002000;
			}

			out.data = s | (m >> 13);
		}
	}
	else if(e == 0xff - (127 - 15))
	{
		if(m == 0)
		{
			out.data = s | 0x7c00;
		}
		else
		{
			m >>= 13;
			out.data = s | 0x7c00 | m | (m == 0);
		}
	}
	else
	{
		if(m &  0x00001000)
		{
			m += 0x00002000;

			if(m & 0x00800000)
			{
				m =  0;
				e += 1;
			}
		}

		if (e > 30)
		{
			ANKI_ASSERT(0 && "Overflow");
			out.data = s | 0x7c00;
		}
		else
		{
			out.data = s | (e << 10) | (m >> 13);
		}
	}

	return out;
}

//==============================================================================
F32 F16::toF32(F16 h)
{
	int s = (h.data >> 15) & 0x00000001;
	int e = (h.data >> 10) & 0x0000001f;
	int m = h.data & 0x000003ff;

	U32 i;

	if(e == 0)
	{
		if(m == 0)
		{
			Val32 v32;
			v32.i = s << 31;
			return v32.f;
		}
		else
		{
			while(!(m & 0x00000400))
			{
				m <<= 1;
				e -=  1;
			}

			e += 1;
			m &= ~0x00000400;
		}
	}
	else if(e == 31)
	{
		if(m == 0)
		{
			Val32 v32;
			v32.i = (s << 31) | 0x7f800000;
			return v32.f;
		}
		else
		{
			Val32 v32;
			v32.i = (s << 31) | 0x7f800000 | (m << 13);
			return v32.f;
		}
	}

	e = e + (127 - 15);
	m = m << 13;

	Val32 v32;
	v32.i = (s << 31) | (e << 23) | m;
	return v32.f;
}

} // end namespace anki
