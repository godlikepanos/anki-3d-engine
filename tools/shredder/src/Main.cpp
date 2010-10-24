#include <iostream>
#include "MeshData.h"
#include "Collision.h"


struct TempMesh
{
	Vec<Vec3> vertCoords;
	Vec<MeshData::Triangle> tris;
	Vec<Vec2> texCoords;
	//Vec<MeshData::VertexWeight> vertWeights;
};


void shredMesh(const Plane& plane, const TempMesh& in, TempMesh& fMesh, TempMesh& bMesh)
{
	for(uint i = 0; i < in.tris.size(); i++)
	{
		const MeshData::Triangle& tri = in.tris[i];
		float testA = plane.test(in.vertCoords[tri.vertIds[0]]);
		float testB = plane.test(in.vertCoords[tri.vertIds[1]]);
		float testC = plane.test(in.vertCoords[tri.vertIds[2]]);

		if(testA < 0.0 && testB < 0.0 && testC < 0.0)
		{

		}
		else if(testA >= 0.0 && testB >= 0.0 && testC >= 0.0)
		{

		}
	}
}


int main(int argc, char** argv)
{
	MeshData md("/home/godlike/src/anki/maps/sponza/walls.mesh");	

	//
	// Get min max
	//
	const Vec<Vec3>& vc = md.getVertCoords();
	Vec3 min(vc[0]), max(vc[0]);
	for(uint i = 1; i < vc.size(); i++)
	{
		for(uint j = 0; j < 3; j++)
		{
			if(vc[i][j] < min[j])
			{
				min[j] = vc[i][j];
			}

			if(vc[i][j] > max[j])
			{
				max[j] = vc[i][j];
			}
		}
	}
	std::cout << "min(" << min << "), max(" << max << ")" << std::endl;

	//
	// Calc planes
	//
	float octreeMinSize = 1.0;
	Vec<Plane> planes;
	// Planes in x
	for(float dist = floor(min.x / octreeMinSize) * octreeMinSize; dist < max.x; dist += octreeMinSize)
	{
		planes.push_back(Plane(Vec3(1, 0, 0), dist));
	}
	std::cout << "Planes num: " << planes.size() << std::endl;

	//
	//
	//
	//Vec<TempMesh> meshes;

	/*for(Vec<Plane>::iterator plane = planes.begin(); plane != planes.end(); ++plane)
	{
		for(uint i = 0; i < md.getTris().size(); i++)
		{
			const MeshData::Triangle& tri = md.getTris()[i];
			float testA = plane->test(md.getVertCoords()[tri.vertIds[0]]);
			float testB = plane->test(md.getVertCoords()[tri.vertIds[1]]);
			float testC = plane->test(md.getVertCoords()[tri.vertIds[2]]);

			TempMesh* backMesh;

			if(testA < 0.0 && testB < 0.0 && testC < 0.0)
			{

			}
			else if(testA >= 0.0 && testB >= 0.0 && testC >= 0.0)
			{

			}
		}
		float dist0 = plane.getDistance();
		float db;
		float s =
	}*/



	std::cout << "End" << std::endl;
	return 0;
}
