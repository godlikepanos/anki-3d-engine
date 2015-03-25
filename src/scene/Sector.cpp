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
	m_spatials.destroy(alloc);
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
Error Sector::tryAddSpatialComponent(SpatialComponent* sp)
{
	ANKI_ASSERT(sp);

	// Find sector in spatial
	auto itsp = sp->getSectorInfo().getBegin();
	auto endsp = sp->getSectorInfo().getEnd();
	for(; itsp != endsp; ++itsp)
	{
		if(*itsp == this)
		{
			break;
		}
	}
	
	Error err = ErrorCode::NONE;
	if(itsp == endsp)
	{
		// Not found, add it

		// Lock since this might be done from a thread
		LockGuard<SpinLock> g(m_lock);

		ANKI_ASSERT(findSpatialComponent(sp) == m_spatials.getEnd());

		m_dirty = true;
		Error err = m_spatials.pushBack(m_group->getAllocator(), sp);

		if(!err)
		{
			err = sp->getSectorInfo().pushBack(m_group->getAllocator(), this);
		}
	}

	return err;
}

//==============================================================================
void Sector::tryRemoveSpatialComponent(SpatialComponent* sp)
{
	ANKI_ASSERT(sp);
	
	// Find sector in spatial
	auto itsp = sp->getSectorInfo().getBegin();
	auto endsp = sp->getSectorInfo().getEnd();
	for(; itsp != endsp; ++itsp)
	{
		if(*itsp == this)
		{
			break;
		}
	}

	if(itsp != endsp)
	{
		// Found, remove

		// Lock since this might be dont from a thread
		LockGuard<SpinLock> g(m_lock);

		m_dirty = true;

		sp->getSectorInfo().erase(m_group->getAllocator(), itsp);

		auto it = findSpatialComponent(sp);
		ANKI_ASSERT(it != m_spatials.getEnd());
		m_spatials.erase(m_group->getAllocator(), it);
	}
}

//==============================================================================
List<SpatialComponent*>::Iterator Sector::findSpatialComponent(
	SpatialComponent* sp)
{
	ANKI_ASSERT(sp);
	auto it = m_spatials.getBegin();
	auto end = m_spatials.getEnd();
	for(; it != end; ++it)
	{
		if(*it == sp)
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

//==============================================================================
Error SectorGroup::spatialUpdated(SpatialComponent* sp)
{
	Error err = ErrorCode::NONE;

	// Iterate all sectors and bin the spatial
	auto it = m_sectors.getBegin();
	auto end = m_sectors.getEnd();
	for(; it != end && !err; ++it)
	{
		Sector& sector = *(*it);

		Bool collide = testCollisionShapes(
			sector.getCollisionShape(), sp->getSpatialCollisionShape());

		if(collide)	
		{
			err = sector.tryAddSpatialComponent(sp);
		}
		else
		{
			sector.tryRemoveSpatialComponent(sp);
		}
	}

	return err;
}

//==============================================================================
void SectorGroup::spatialDeleted(SpatialComponent* sp)
{
	auto it = m_sectors.getBegin();
	auto end = m_sectors.getEnd();
	for(; it != end; ++it)
	{
		Sector& sector = *(*it);
		sector.tryRemoveSpatialComponent(sp);
	}
}

//==============================================================================
Error SectorGroup::doVisibilityTests(const FrustumComponent& frc)
{
	auto alloc = m_scene->getFrameAllocator();

	// Find the sector the frc is in
	Sphere eye(frc.getFrustumOrigin(), frc.getFrustum().getNear());

	auto it = m_sectors.getBegin();
	auto end = m_sectors.getEnd();
	for(; it != end; ++it)
	{
		//if(frc.insideFrustum())
	}

	// Initial storage
	visibleNodes = reinterpret_cast<SceneNode*>(
		alloc.allocate(sizeof(void*) * 100));
	visibleNodesStorage = 100;

}

} // end namespace anki
