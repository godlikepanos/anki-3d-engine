// Copyright (C) 2009-present, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

// To include it you need to define the type of the float: "SCALAR"

// Mat3 //

vector<SCALAR, 3> mul(TMat3<SCALAR> m, vector<SCALAR, 3> v)
{
	const SCALAR a = dot(m.m_row0, v);
	const SCALAR b = dot(m.m_row1, v);
	const SCALAR c = dot(m.m_row2, v);
	return vector<SCALAR, 3>(a, b, c);
}

TMat3<SCALAR> transpose(TMat3<SCALAR> m)
{
	TMat3<SCALAR> o;
	o.setColumns(m.m_row0, m.m_row1, m.m_row2);
	return o;
}

// Mat4 //

vector<SCALAR, 4> mul(TMat4<SCALAR> m, vector<SCALAR, 4> v)
{
	const SCALAR a = dot(m.m_row0, v);
	const SCALAR b = dot(m.m_row1, v);
	const SCALAR c = dot(m.m_row2, v);
	const SCALAR d = dot(m.m_row3, v);
	return vector<SCALAR, 4>(a, b, c, d);
}

TMat4<SCALAR> mul(TMat4<SCALAR> a_, TMat4<SCALAR> b_)
{
	const vector<SCALAR, 4> a[4] = {a_.m_row0, a_.m_row1, a_.m_row2, a_.m_row3};
	const vector<SCALAR, 4> b[4] = {b_.m_row0, b_.m_row1, b_.m_row2, b_.m_row3};
	vector<SCALAR, 4> c[4];
	[unroll] for(U32 i = 0; i < 4; i++)
	{
		vector<SCALAR, 4> t1, t2;
		t1 = a[i][0];
		t2 = b[0] * t1;
		t1 = a[i][1];
		t2 += b[1] * t1;
		t1 = a[i][2];
		t2 += b[2] * t1;
		t1 = a[i][3];
		t2 += b[3] * t1;
		c[i] = t2;
	}

	TMat4<SCALAR> o;
	o.m_row0 = c[0];
	o.m_row1 = c[1];
	o.m_row2 = c[2];
	o.m_row3 = c[3];
	return o;
}

// Mat3x4 //

vector<SCALAR, 3> mul(TMat3x4<SCALAR> m, vector<SCALAR, 4> v)
{
	const SCALAR a = dot(m.m_row0, v);
	const SCALAR b = dot(m.m_row1, v);
	const SCALAR c = dot(m.m_row2, v);
	return vector<SCALAR, 3>(a, b, c);
}

TMat3x4<SCALAR> combineTransformations(TMat3x4<SCALAR> a_, TMat3x4<SCALAR> b_)
{
	const vector<SCALAR, 4> a[3] = {a_.m_row0, a_.m_row1, a_.m_row2};
	const vector<SCALAR, 4> b[3] = {b_.m_row0, b_.m_row1, b_.m_row2};
	vector<SCALAR, 4> c[3];
	[unroll] for(U32 i = 0; i < 3; i++)
	{
		vector<SCALAR, 4> t2;
		t2 = b[0] * a[i][0];
		t2 += b[1] * a[i][1];
		t2 += b[2] * a[i][2];
		const vector<SCALAR, 4> v4 = vector<SCALAR, 4>(0.0f, 0.0f, 0.0f, a[i][3]);
		t2 += v4;
		c[i] = t2;
	}

	TMat3x4<SCALAR> o;
	o.m_row0 = c[0];
	o.m_row1 = c[1];
	o.m_row2 = c[2];
	return o;
}

#undef SCALAR
