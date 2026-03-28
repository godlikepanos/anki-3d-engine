// Copyright (C) 2009-present, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

// Define some common matrix types. It's a combination of templates and old school def.h files. def.h is used to workaround issues in template
// type deduction

#pragma once

#define _ANKI_DEFINE_OPERATOR_SCALAR_ROWS3(matType, scalarType, op) \
	matType operator op(scalarType f) \
	{ \
		matType o; \
		o.m_row0 = m_row0 op f; \
		o.m_row1 = m_row1 op f; \
		o.m_row2 = m_row2 op f; \
		return o; \
	}

#define _ANKI_DEFINE_OPERATOR_SCALAR_ROWS4(matType, scalarType, op) \
	matType operator op(scalarType f) \
	{ \
		matType o; \
		o.m_row0 = m_row0 op f; \
		o.m_row1 = m_row1 op f; \
		o.m_row2 = m_row2 op f; \
		o.m_row3 = m_row3 op f; \
		return o; \
	}

#define _ANKI_DEFINE_OPERATOR_SELF_ROWS3(matType, op) \
	matType operator op(matType b) \
	{ \
		matType o; \
		o.m_row0 = m_row0 op b.m_row0; \
		o.m_row1 = m_row1 op b.m_row1; \
		o.m_row2 = m_row2 op b.m_row2; \
		return o; \
	}

#define _ANKI_DEFINE_OPERATOR_SELF_ROWS4(matType, op) \
	matType operator op(matType b) \
	{ \
		matType o; \
		o.m_row0 = m_row0 op b.m_row0; \
		o.m_row1 = m_row1 op b.m_row1; \
		o.m_row2 = m_row2 op b.m_row2; \
		o.m_row3 = m_row3 op b.m_row3; \
		return o; \
	}

#define _ANKI_DEFINE_ALL_OPERATORS_ROWS3(matType, scalarType) \
	_ANKI_DEFINE_OPERATOR_SCALAR_ROWS3(matType, scalarType, +) \
	_ANKI_DEFINE_OPERATOR_SCALAR_ROWS3(matType, scalarType, -) \
	_ANKI_DEFINE_OPERATOR_SCALAR_ROWS3(matType, scalarType, *) \
	_ANKI_DEFINE_OPERATOR_SCALAR_ROWS3(matType, scalarType, /) \
	_ANKI_DEFINE_OPERATOR_SELF_ROWS3(matType, +) \
	_ANKI_DEFINE_OPERATOR_SELF_ROWS3(matType, -)

#define _ANKI_DEFINE_ALL_OPERATORS_ROWS4(matType, scalarType) \
	_ANKI_DEFINE_OPERATOR_SCALAR_ROWS4(matType, scalarType, +) \
	_ANKI_DEFINE_OPERATOR_SCALAR_ROWS4(matType, scalarType, -) \
	_ANKI_DEFINE_OPERATOR_SCALAR_ROWS4(matType, scalarType, *) \
	_ANKI_DEFINE_OPERATOR_SCALAR_ROWS4(matType, scalarType, /) \
	_ANKI_DEFINE_OPERATOR_SELF_ROWS4(matType, +) \
	_ANKI_DEFINE_OPERATOR_SELF_ROWS4(matType, -)

#define _ANKI_DEFINE_EXTRACT_SCALE_SQUARED \
	vector<T, 3> extractScaleSquared() \
	{ \
		vector<T, 3> scale; \
		[unroll] for(U32 i = 0; i < 3; ++i) \
		{ \
			const vector<T, 3> axis = vector<T, 3>(m_row0[i], m_row1[i], m_row2[i]); \
			scale[i] = dot(axis, axis); \
		} \
		return scale; \
	} \
	vector<T, 3> extractScale() \
	{ \
		const vector<T, 3> scaleSq = extractScaleSquared(); \
		return sqrt(scaleSq); \
	}

// Mat3 //

template<typename T>
struct TMat3
{
	vector<T, 3> m_row0;
	vector<T, 3> m_row1;
	vector<T, 3> m_row2;

	_ANKI_DEFINE_ALL_OPERATORS_ROWS3(TMat3, T)
	_ANKI_DEFINE_EXTRACT_SCALE_SQUARED

	void setColumns(vector<T, 3> c0, vector<T, 3> c1, vector<T, 3> c2)
	{
		m_row0 = vector<T, 3>(c0.x, c1.x, c2.x);
		m_row1 = vector<T, 3>(c0.y, c1.y, c2.y);
		m_row2 = vector<T, 3>(c0.z, c1.z, c2.z);
	}
};

// Mat4 //

template<typename T>
struct TMat4
{
	vector<T, 4> m_row0;
	vector<T, 4> m_row1;
	vector<T, 4> m_row2;
	vector<T, 4> m_row3;

	_ANKI_DEFINE_ALL_OPERATORS_ROWS4(TMat4, T)
	_ANKI_DEFINE_EXTRACT_SCALE_SQUARED

	void setColumns(vector<T, 4> c0, vector<T, 4> c1, vector<T, 4> c2, vector<T, 4> c3)
	{
		m_row0 = vector<T, 4>(c0.x, c1.x, c2.x, c3.x);
		m_row1 = vector<T, 4>(c0.y, c1.y, c2.y, c3.y);
		m_row2 = vector<T, 4>(c0.z, c1.z, c2.z, c3.z);
		m_row3 = vector<T, 4>(c0.w, c1.w, c2.w, c3.w);
	}

	vector<T, 4> getTranslationPart()
	{
		return vector<T, 4>(m_row0.w, m_row1.w, m_row2.w, m_row3.w);
	}
};

// Mat3x4 //

template<typename T>
struct TMat3x4
{
	vector<T, 4> m_row0;
	vector<T, 4> m_row1;
	vector<T, 4> m_row2;

	_ANKI_DEFINE_ALL_OPERATORS_ROWS3(TMat3x4, T)
	_ANKI_DEFINE_EXTRACT_SCALE_SQUARED

	vector<T, 3> getTranslationPart()
	{
		return vector<T, 3>(m_row0.w, m_row1.w, m_row2.w);
	}

	void setColumns(vector<T, 3> c0, vector<T, 3> c1, vector<T, 3> c2, vector<T, 3> c3)
	{
		m_row0 = vector<T, 4>(c0.x, c1.x, c2.x, c3.x);
		m_row1 = vector<T, 4>(c0.y, c1.y, c2.y, c3.y);
		m_row2 = vector<T, 4>(c0.z, c1.z, c2.z, c3.z);
	}

	void setColumn(U32 i, vector<T, 3> c)
	{
		m_row0[i] = c.x;
		m_row1[i] = c.y;
		m_row2[i] = c.z;
	}

	vector<T, 3> getColumn(U32 i)
	{
		return vector<T, 3>(m_row0[i], m_row1[i], m_row2[i]);
	}
};

typedef TMat3<F32> Mat3;
typedef TMat4<F32> Mat4;
typedef TMat3x4<F32> Mat3x4;

#define SCALAR F32
#include <AnKi/Shaders/Include/Mat.def.h>

#define SCALAR F16
#include <AnKi/Shaders/Include/Mat.def.h>
