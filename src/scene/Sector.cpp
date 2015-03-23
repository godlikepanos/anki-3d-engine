// Copyright (C) 2009-2015, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include "anki/scene/Sector.h"
#include "anki/scene/SpatialComponent.h"
#include "anki/scene/SceneNode.h"
#include "anki/scene/RenderComponent.h"
#include "anki/scene/Light.h"
#include "anki/scene/Visibility.h"
#include "anki/scene/FrustumComponent.h"
#include "anki/scene/SceneGraph.h"
#include "anki/util/Logger.h"
#include "anki/renderer/Renderer.h"

namespace anki {

//==============================================================================
// Portal                                                                      =
//==============================================================================

//==============================================================================
Portal::~Portal()
{
	auto alloc = m_group->getAllocator();
	m_shapeStorage.destroy(alloc);

	if(m_shape)
	{
		alloc.deleteInstance(m_shape);
		m_shape = nullptr;
	}
}

//==============================================================================
Error Portal::create(const SArray<Vec4>& vertPositions)
{
	m_shape = m_group->createConvexHull(vertPositions, m_shapeStorage);
	return m_shape ? ErrorCode::NONE : ErrorCode::OUT_OF_MEMORY;
}

//==============================================================================
Error Portal::addSector(Sector* sector)
{
	ANKI_ASSERT(sector);
	return m_sectors.pushBack(m_group->getAllocator(), sector);
}

//==============================================================================
// Sector                                                                      =
//==============================================================================

//==============================================================================
Sector::~Sector()
{
	auto alloc = m_group->getAllocator();

	if(m_shape)
	{
		alloc.deleteInstance(m_shape);
		m_shape = nullptr;
	}

	m_portals.destroy(alloc);
	m_nodes.destroy(alloc);
}

//==============================================================================
Error Sector::create(const SArray<Vec4>& vertPositions)
{
	m_shape = m_group->createConvexHull(vertPositions, m_shapeStorage);
	return m_shape ? ErrorCode::NONE : ErrorCode::OUT_OF_MEMORY;
}

//==============================================================================
Error Sector::addPortal(Portal* portal)
{
	ANKI_ASSERT(portal);
	return m_portals.pushBack(m_group->getAllocator(), portal);
}

//==============================================================================
Error Sector::addSceneNode(SceneNode* node)
{
	ANKI_ASSERT(node);
	ANKI_ASSERT(findSceneNode(node) == m_nodes.getEnd());
	return m_nodes.pushBack(m_group->getAllocator(), node);
}

//==============================================================================
void Sector::removeSceneNode(SceneNode* node)
{
	ANKI_ASSERT(node);
	auto it = findSceneNode(node);
	ANKI_ASSERT(it != m_nodes.getEnd());
	m_nodes.erase(m_group->getAllocator(), it);
}

//==============================================================================
List<SceneNode*>::Iterator Sector::findSceneNode(SceneNode* node)
{
	ANKI_ASSERT(node);
	auto it = m_nodes.getBegin();
	for(; it != m_nodes.getEnd(); ++it)
	{
		if(*it == node)
		{
			break;
		}
	}

	return it;
}

//==============================================================================
// SectorGroup                                                                 =
//==============================================================================

//==============================================================================
ConvexHullShape* SectorGroup::createConvexHull(
	const SArray<Vec4>& vertPositions, DArray<Vec4>& shapeStorage)
{
	Error err = ErrorCode::NONE;

	auto alloc = getAllocator();
	U vertCount = vertPositions.getSize();
	ANKI_ASSERT(vertCount >= 4 && "Minimum shape should be tetrahedron");

	// Create hull
	ConvexHullShape* hull = alloc.newInstance<ConvexHullShape>();
	if(!hull)
	{
		err = ErrorCode::OUT_OF_MEMORY;
	}

	// Alloc storage
	if(!err)
	{
		err = shapeStorage.create(alloc, vertCount);
	}

	// Assign storage to hull
	if(!err)
	{
		memcpy(&shapeStorage[0], &vertPositions[0], sizeof(Vec4) * vertCount);
		hull->initStorage(&shapeStorage[0], vertCount);
	}

	// Cleanup on error
	if(err)
	{
		shapeStorage.destroy(alloc);
		
		if(hull)
		{
			alloc.deleteInstance(hull);
			hull = nullptr;
		}
	}

	return hull;
}

//==============================================================================
Sector* SectorGroup::createNewSector(const SArray<Vec4>& vertexPositions)
{
	Sector* sector = getAllocator().newInstance<Sector>(this);
	if(sector)
	{
		Error err = sector->create(vertexPositions);
		if(err)
		{
			getAllocator().deleteInstance(sector);
			sector = nullptr;
		}
	}

	return sector;
}

//==============================================================================
Portal* SectorGroup::createNewPortal(const SArray<Vec4>& vertexPositions)
{
	Portal* portal = getAllocator().newInstance<Portal>(this);
	if(portal)
	{
		Error err = portal->create(vertexPositions);
		if(err)
		{
			getAllocator().deleteInstance(portal);
			portal = nullptr;
		}
	}

	return portal;
}

//==============================================================================
Error SectorGroup::bake()
{
	// Connect portals with sectors
	auto it = m_portals.getBegin();
	auto end = m_portals.getEnd();
	for(; it != end; ++it)
	{
		Portal& portal = *(*it);

		auto sit = m_sectors.getBegin();
		auto send = m_sectors.getEnd();

		for(; sit != send; ++sit)
		{
			Sector& sector = *(*sit);

			Bool collide = testCollisionShapes(
				portal.getCollisionShape(), sector.getCollisionShape());

			if(collide)
			{
				ANKI_CHECK(portal.addSector(&sector));
				ANKI_CHECK(sector.addPortal(&portal));
			}
		}
	}

	return ErrorCode::NONE;
}

} // end namespace anki
