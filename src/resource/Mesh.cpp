#include "anki/resource/Mesh.h"
#include "anki/resource/Material.h"
#include "anki/resource/MeshLoader.h"
#include "anki/gl/Vbo.h"
#include "anki/util/Functions.h"

namespace anki {

//==============================================================================
void Mesh::load(const char* filename)
{
	MeshLoader loader(filename);

	// Set the non-VBO members
	vertsCount = loader.getPositions().size();
	ANKI_ASSERT(vertsCount > 0);

	indicesCount = loader.getIndices().size();
	ANKI_ASSERT(indicesCount > 0);
	ANKI_ASSERT(indicesCount % 3 == 0 && "Expecting triangles");

	weights = loader.getWeights().size() > 1;
	texChannelsCount = loader.getTextureChannelsCount();

	try
	{
		createVbos(loader);

		visibilityShape.set(loader.getPositions());
	}
	catch(std::exception& e)
	{
		throw ANKI_EXCEPTION("Mesh loading failed: " + filename) << e;
	}
}

//==============================================================================
U32 Mesh::calcVertexSize() const
{
	U32 a = sizeof(Vec3) + sizeof(Vec3) + sizeof(Vec4) 
		+ texChannelsCount * sizeof(Vec2);
	if(weights)
	{
		a += sizeof(MeshLoader::VertexWeight);
	}
	return a;
}

//==============================================================================
void Mesh::createVbos(const MeshLoader& loader)
{
	// Calculate VBO size
	U32 vertexsize = calcVertexSize();
	U32 vbosize = vertexsize * vertsCount;

	// Create a temp buffer and populate it
	Vector<U8> buff(vbosize, 0);

	U8* ptr = &buff[0];
	for(U i = 0; i < vertsCount; i++)
	{
		ANKI_ASSERT(ptr + vertexsize <= &buff[0] + vbosize);

		memcpy(ptr, &loader.getPositions()[i], sizeof(Vec3));
		ptr += sizeof(Vec3);

		memcpy(ptr, &loader.getNormals()[i], sizeof(Vec3));
		ptr += sizeof(Vec3);

		memcpy(ptr, &loader.getTangents()[i], sizeof(Vec4));
		ptr += sizeof(Vec4);

		for(U j = 0; j < texChannelsCount; j++)
		{
			memcpy(ptr, &loader.getTexureCoordinates(j)[i], sizeof(Vec2));
			ptr += sizeof(Vec2);
		}

		if(weights)
		{
			memcpy(ptr, &loader.getWeights()[i], 
				sizeof(MeshLoader::VertexWeight));
			ptr += sizeof(MeshLoader::VertexWeight);
		}
	}

	// Create VBO
	vbo.create(
		GL_ARRAY_BUFFER,
		vbosize,
		&buff[0],
		GL_STATIC_DRAW);

	/// Create the indices VBO
	indicesVbo.create(
		GL_ELEMENT_ARRAY_BUFFER,
		getVectorSizeInBytes(loader.getIndices()),
		&loader.getIndices()[0],
		GL_STATIC_DRAW);
}

//==============================================================================
void Mesh::getVboInfo(const VertexAttribute attrib, const Vbo*& v, U32& size,
	GLenum& type, U32& stride, U32& offset) const
{
	stride = calcVertexSize();

	// Set all to zero
	v = nullptr;
	size = 0;
	type = GL_NONE;
	offset = 0;

	switch(attrib)
	{
	case VA_POSITION:
		v = &vbo;
		size = 3;
		type = GL_FLOAT;
		offset = 0;
		break;
	case VA_NORMAL:
		v = &vbo;
		size = 3;
		type = GL_FLOAT;
		offset = sizeof(Vec3);
		break;
	case VA_TANGENT:
		v = &vbo;
		size = 4;
		type = GL_FLOAT;
		offset = sizeof(Vec3) * 2;
		break;
	case VA_TEXTURE_COORDS:
		if(texChannelsCount > 0)
		{
			v = &vbo;
			size = 2;
			type = GL_FLOAT;
			offset = sizeof(Vec3) * 2 + sizeof(Vec4);
		}
		break;
	case VA_TEXTURE_COORDS_1:
		if(texChannelsCount > 1)
		{
			v = &vbo;
			size = 2;
			type = GL_FLOAT;
			offset = sizeof(Vec3) * 2 + sizeof(Vec4) + sizeof(Vec2);
		}
		break;
	case VA_BONE_COUNT:
		if(weights)
		{
			v = &vbo;
			size = 1;
			type = GL_UNSIGNED_INT;
			offset = sizeof(Vec3) * 2 + sizeof(Vec4) 
				+ texChannelsCount * sizeof(Vec2);
		}
		break;
	case VA_BONE_IDS:
		if(weights)
		{
			v = &vbo;
			size = 4;
			type = GL_UNSIGNED_INT;
			offset = sizeof(Vec3) * 2 + sizeof(Vec4) 
				+ texChannelsCount * sizeof(Vec2) + sizeof(U32);
		}
		break;
	case VA_BONE_WEIGHTS:
		if(weights)
		{
			v = &vbo;
			size = 4;
			type = GL_FLOAT;
			offset = sizeof(Vec3) * 2 + sizeof(Vec4) 
				+ texChannelsCount * sizeof(Vec2) + sizeof(U32) 
				+ sizeof(U32) * 4;
		}
	case VA_INDICES:
		v = &indicesVbo;
		break;
	default:
		ANKI_ASSERT(0);
		break;
	}
}

} // end namespace anki
