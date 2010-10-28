#ifndef SHREDDER_H
#define SHREDDER_H

#include "MeshData.h"


class Shredder
{
	public:
		Shredder(const char* filename, float octreeMinSize);

		const MeshData& getOriginalMesh() const {return originalMesh;}

	private:
		MeshData originalMesh;
};


inline Shredder::Shredder(const char* filename, float octreeMinSize):
	originalMesh(filename)
{}


#endif
