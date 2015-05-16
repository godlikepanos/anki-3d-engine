// Copyright (C) 2009-2015, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include "anki/scene/Sector.h"
#include "anki/scene/SpatialComponent.h"
#include "anki/scene/FrustumComponent.h"
#include "anki/scene/MoveComponent.h"
#include "anki/scene/SceneGraph.h"
#include "anki/util/Logger.h"
#include "anki/resource/ResourceManager.h"
#include "anki/resource/MeshLoader.h"

namespace anki {

//==============================================================================
// PortalSectorBase                                                            =
//==============================================================================

//==============================================================================
PortalSectorBase::~PortalSectorBase()
{
	auto alloc = getSceneAllocator();

	if(m_shape)
	{
		alloc.deleteInstance(m_shape);
		m_shape = nullptr;
	}

	m_shapeStorageLSpace.destroy(alloc);
	m_shapeStorageWSpace.destroy(alloc);
	m_vertIndices.destroy(alloc);
}

//==============================================================================
Error PortalSectorBase::create(const CString& name, const CString& modelFname)
{
	ANKI_CHECK(SceneNode::create(name));

	// Create move component
	SceneComponent* comp = getSceneAllocator().newInstance<MoveComponent>(this);
	addComponent(comp, true);

	// Create portal sector component
	comp = getSceneAllocator().newInstance<PortalSectorComponent>(this);
	addComponent(comp, true);

	// Load mesh
	StringAuto newFname(getFrameAllocator());
	getSceneGraph()._getResourceManager().fixResourceFilename(
		modelFname, newFname);

	MeshLoader loader;
	ANKI_CHECK(loader.load(getFrameAllocator(), newFname.toCString()));

	// Convert Vec3 positions to Vec4
	const MeshLoader::Header& header = loader.getHeader();
	U vertsCount = header.m_totalVerticesCount;
	PtrSize vertSize = loader.getVertexSize();

	auto alloc = getSceneAllocator();
	m_shapeStorageLSpace.create(alloc, vertsCount);
	m_shapeStorageWSpace.create(alloc, vertsCount);

	for(U i = 0; i < vertsCount; ++i)
	{
		const Vec3& pos = *reinterpret_cast<const Vec3*>(
			loader.getVertexData() + vertSize * i);

		m_shapeStorageLSpace[i] = Vec4(pos, 0.0);
	}

	// Create shape
	ConvexHullShape* hull = alloc.newInstance<ConvexHullShape>();
	m_shape = hull;
	hull->initStorage(&m_shapeStorageWSpace[0], vertsCount);

	// Store indices
	ANKI_ASSERT(
		header.m_totalIndicesCount * sizeof(U16) == loader.getIndexDataSize());
	m_vertIndices.create(alloc, header.m_totalIndicesCount);
	const U16* indicesIn = reinterpret_cast<const U16*>(loader.getIndexData());
	for(U i = 0; i < header.m_totalIndicesCount; ++i)
	{
		m_vertIndices[i] = indicesIn[i];
	}

	return ErrorCode::NONE;
}

//==============================================================================
SectorGroup& PortalSectorBase::getSectorGroup()
{
	return getSceneGraph().getSectorGroup();
}

//==============================================================================
void PortalSectorBase::updateTransform(const Transform& trf)
{
	const U count = m_shapeStorageWSpace.getSize();
	for(U i = 0; i < count; ++i)
	{
		m_shapeStorageWSpace[i] = trf.transform(m_shapeStorageLSpace[i]);
	}
}

//==============================================================================
// Portal                                                                      =
//==============================================================================

//==============================================================================
Portal::~Portal()
{
	auto alloc = getSceneAllocator();

	// Remove from sectors
	for(Sector* s : m_sectors)
	{
		s->tryRemovePortal(this);
	}

	m_sectors.destroy(getSceneAllocator());

	// Remove from group
	auto& portals = getSectorGroup().m_portals;
	auto it = portals.getBegin();
	auto end = portals.getBegin();
	for(; it != end; ++it)
	{
		if(*it == this)
		{
			portals.erase(getSceneAllocator(), it);
			break;
		}
	}
}

//==============================================================================
Error Portal::create(const CString& name, const CString& modelFname)
{
	ANKI_CHECK(Base::create(name, modelFname));
	getSectorGroup().m_portals.pushBack(getSceneAllocator(), this);
	return ErrorCode::NONE;
}

//==============================================================================
Error Portal::frameUpdate(F32 prevUpdateTime, F32 crntTime)
{
	MoveComponent& move = getComponent<MoveComponent>();
	if(move.getTimestamp() == getGlobalTimestamp())
	{
		// Move comp updated. Inform the group
		updateTransform(move.getWorldTransform());

		SectorGroup& group = getSectorGroup();

		// Gather the sectors it collides
		auto it = group.m_sectors.getBegin();
		auto end = group.m_sectors.getEnd();

		for(; it != end; ++it)
		{
			Sector* sector = *it;

			Bool collide = testCollisionShapes(
				*m_shape, sector->getBoundingShape());

			if(collide)
			{
				tryAddSector(sector);
				sector->tryAddPortal(this);
			}
			else
			{
				tryRemoveSector(sector);
				sector->tryRemovePortal(this);
			}
		}
	}

	return ErrorCode::NONE;
}

//==============================================================================
void Portal::tryAddSector(Sector* sector)
{
	LockGuard<SpinLock> lock(m_mtx);

	auto it = m_sectors.getBegin();
	auto end = m_sectors.getEnd();
	for(; it != end; ++it)
	{
		if(*it == sector)
		{
			// Already there, return
			return;
		}
	}

	m_sectors.pushBack(getSceneAllocator(), sector);
}

//==============================================================================
void Portal::tryRemoveSector(Sector* sector)
{
	LockGuard<SpinLock> lock(m_mtx);

	auto it = m_sectors.getBegin();
	auto end = m_sectors.getEnd();
	for(; it != end; ++it)
	{
		if(*it == sector)
		{
			m_sectors.erase(getSceneAllocator(), it);
			break;
		}
	}
}

//==============================================================================
// Sector                                                                      =
//==============================================================================

//==============================================================================
Sector::~Sector()
{
	auto alloc = getSceneAllocator();

	// Remove portals
	for(Portal* p : m_portals)
	{
		p->tryRemoveSector(this);
	}

	m_portals.destroy(alloc);

	// Remove spatials
	U spatialsCount = m_spatials.getSize();
	while(spatialsCount-- != 0) // Use counter
	{
		tryRemoveSpatialComponent(m_spatials.getFront());
	}

	// Remove from group
	auto& sectors = getSectorGroup().m_sectors;
	auto it = sectors.getBegin();
	auto end = sectors.getBegin();
	for(; it != end; ++it)
	{
		if(*it == this)
		{
			sectors.erase(getSceneAllocator(), it);
			break;
		}
	}
}

//==============================================================================
Error Sector::create(const CString& name, const CString& modelFname)
{
	ANKI_CHECK(PortalSectorBase::create(name, modelFname));
	getSectorGroup().m_sectors.pushBack(getSceneAllocator(), this);
	return ErrorCode::NONE;
}

//==============================================================================
void Sector::tryAddPortal(Portal* portal)
{
	ANKI_ASSERT(portal);
	LockGuard<SpinLock> lock(m_mtx);

	auto it = m_portals.getBegin();
	auto end = m_portals.getEnd();
	for(; it != end; ++it)
	{
		if(*it == portal)
		{
			// Already there, return
			return;
		}
	}

	m_portals.pushBack(getSceneAllocator(), portal);
}

//==============================================================================
void Sector::tryRemovePortal(Portal* portal)
{
	ANKI_ASSERT(portal);
	LockGuard<SpinLock> lock(m_mtx);

	auto it = m_portals.getBegin();
	auto end = m_portals.getEnd();
	for(; it != end; ++it)
	{
		if(*it == portal)
		{
			m_portals.erase(getSceneAllocator(), it);
			break;
		}
	}
}

//==============================================================================
void Sector::tryAddSpatialComponent(SpatialComponent* sp)
{
	ANKI_ASSERT(sp);
	LockGuard<SpinLock> lock(m_mtx);

	// Find sector in spatial
	auto itsp = sp->getSectorInfo().getBegin();
	auto endsp = sp->getSectorInfo().getEnd();
	for(; itsp != endsp; ++itsp)
	{
		if(*itsp == this)
		{
			// Found, return
#if ANKI_ASSERTIONS
			ANKI_ASSERT(findSpatialComponent(sp) != m_spatials.getEnd()
				&& "Spatial has reference to sector but sector not");
#endif
			return;
		}
	}

	ANKI_ASSERT(findSpatialComponent(sp) == m_spatials.getEnd());

	m_spatials.pushBack(getSceneAllocator(), sp);
	sp->getSectorInfo().pushBack(getSceneAllocator(), this);
}

//==============================================================================
void Sector::tryRemoveSpatialComponent(SpatialComponent* sp)
{
	ANKI_ASSERT(sp);
	LockGuard<SpinLock> g(m_mtx);

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

		sp->getSectorInfo().erase(getSceneAllocator(), itsp);

		auto it = findSpatialComponent(sp);
		ANKI_ASSERT(it != m_spatials.getEnd());
		m_spatials.erase(getSceneAllocator(), it);
	}
	else
	{
#if ANKI_ASSERTIONS
		ANKI_ASSERT(findSpatialComponent(sp) == m_spatials.getEnd());
#endif
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
Error Sector::frameUpdate(F32 prevUpdateTime, F32 crntTime)
{
	MoveComponent& move = getComponent<MoveComponent>();
	if(move.getTimestamp() == getGlobalTimestamp())
	{
		// Move comp updated.

		updateTransform(move.getWorldTransform());

		// Spatials should get updated
		for(SpatialComponent* sp : m_spatials)
		{
			sp->markForUpdate();
		}

		// Inform the group
		SectorGroup& group = getSectorGroup();

		auto it = group.m_portals.getBegin();
		auto end = group.m_portals.getEnd();
		for(; it != end; ++it)
		{
			Portal* portal = *it;

			Bool collide = testCollisionShapes(
				*m_shape, portal->getBoundingShape());

			if(collide)
			{
				portal->tryAddSector(this);
				tryAddPortal(portal);
			}
			else
			{
				portal->tryRemoveSector(this);
				tryRemovePortal(portal);
			}
		}
	}

	return ErrorCode::NONE;
}

//==============================================================================
// SectorGroup                                                                 =
//==============================================================================

//==============================================================================
SectorGroup::~SectorGroup()
{
	auto alloc = m_scene->getAllocator();

	m_sectors.destroy(alloc);
	m_portals.destroy(alloc);
}

//==============================================================================
void SectorGroup::spatialUpdated(SpatialComponent* sp)
{
	ANKI_ASSERT(sp);
	LockGuard<SpinLock> lock(m_mtx);
	m_spatialsDeferredBinning.pushBack(m_scene->getFrameAllocator(), sp);
}

//==============================================================================
void SectorGroup::binSpatial(SpatialComponent* sp)
{
	// Iterate all sectors and bin the spatial
	auto it = m_sectors.getBegin();
	auto end = m_sectors.getEnd();
	for(; it != end; ++it)
	{
		Sector& sector = *(*it);

		Bool collide = testCollisionShapes(
			sector.getBoundingShape(), sp->getSpatialCollisionShape());

		if(collide)
		{
			sector.tryAddSpatialComponent(sp);
		}
		else
		{
			sector.tryRemoveSpatialComponent(sp);
		}
	}
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
void SectorGroup::findVisibleSectors(
	const FrustumComponent& frc,
	List<Sector*>& visibleSectors,
	U& spatialsCount)
{
	// Find the sector the eye is in
	Sphere eye(frc.getFrustumOrigin(), frc.getFrustum().getNear());

	auto it = m_sectors.getBegin();
	auto end = m_sectors.getEnd();
	for(; it != end; ++it)
	{
		if(testCollisionShapes(eye, (*it)->getBoundingShape()))
		{
			break;
		}
	}

	if(it == end)
	{
		// eye outside all sectors, find those it collides

		it = m_sectors.getBegin();
		for(; it != end; ++it)
		{
			Sector& s = *(*it);
			if(frc.insideFrustum(s.getBoundingShape()))
			{
				findVisibleSectorsInternal(
					frc, s, visibleSectors, spatialsCount);
			}
		}
	}
	else
	{
		// eye inside a sector
		findVisibleSectorsInternal(
			frc, *(*it), visibleSectors, spatialsCount);
	}
}

//==============================================================================
void SectorGroup::findVisibleSectorsInternal(
	const FrustumComponent& frc,
	Sector& s,
	List<Sector*>& visibleSectors,
	U& spatialsCount)
{
	auto alloc = m_scene->getFrameAllocator();

	// Check if "s" is already there
	auto it = visibleSectors.getBegin();
	auto end = visibleSectors.getEnd();
	for(; it != end; ++it)
	{
		if(*it == &s)
		{
			// Sector already there, skip
			return;
		}
	}

	// Sector not in the list, push it
	visibleSectors.pushBack(alloc, &s);
	spatialsCount += s.m_spatials.getSize();

	// Check visible portals
	auto itp = s.m_portals.getBegin();
	auto itend = s.m_portals.getEnd();
	for(; itp != itend; ++itp)
	{
		Portal& p = *(*itp);
		if(frc.insideFrustum(p.getBoundingShape()))
		{
			it = p.m_sectors.getBegin();
			end = p.m_sectors.getEnd();
			for(; it != end; ++it)
			{
				if(*it != &s)
				{
					findVisibleSectorsInternal(
						frc, *(*it), visibleSectors, spatialsCount);
				}
			}
		}
	}
}

//==============================================================================
Bool SectorGroup::spatialInSector(
	const Sector& sector, const SpatialComponent& spatial)
{
	auto it = spatial.getSectorInfo().getBegin();
	auto end = spatial.getSectorInfo().getEnd();
	for(; it != end; ++it)
	{
		if(*it == &sector)
		{
			return true;
		}
	}

	return false;
}

//==============================================================================
void SectorGroup::prepareForVisibilityTests(const FrustumComponent& frc)
{
	auto alloc = m_scene->getFrameAllocator();

	// Bin spatials
	{
		auto it = m_spatialsDeferredBinning.getBegin();
		auto end = m_spatialsDeferredBinning.getEnd();
		for(; it != end; ++it)
		{
			binSpatial(*it);
		}
		m_spatialsDeferredBinning.destroy(alloc);
	}

	// Find visible sectors
	ListAuto<Sector*> visSectors(m_scene->getFrameAllocator());
	U spatialsCount = 0;
	findVisibleSectors(frc, visSectors, spatialsCount);

	if(ANKI_UNLIKELY(spatialsCount == 0))
	{
		m_visibleNodes = nullptr;
		m_visibleNodesCount = 0;
		return;
	}

	// Initiate storage of nodes
	m_visibleNodes = reinterpret_cast<SceneNode**>(
		alloc.allocate(spatialsCount * sizeof(void*)));

	SArray<SceneNode*> visibleNodes(m_visibleNodes, spatialsCount);

	// Iterate visible sectors and get the scene nodes. The array will contain
	// duplicates
	U nodesCount = 0;
	auto it = visSectors.getBegin();
	auto end = visSectors.getEnd();
	for(; it != end; ++it)
	{
		Sector& s = *(*it);
		for(auto itsp : s.m_spatials)
		{
			SpatialComponent& spc = *itsp;

			// Check if node already checked
			Bool checked = false;
			auto it1 = visSectors.getBegin();
			auto end1 = it;
			for(; it1 != end1; ++it1)
			{
				if(spatialInSector(*(*it1), spc))
				{
					checked = true;
					break;
				}
			}

			if(!checked)
			{
				visibleNodes[nodesCount++] = &spc.getSceneNode();
			}
		}
	}

	m_visibleNodesCount = nodesCount;
}

} // end namespace anki
