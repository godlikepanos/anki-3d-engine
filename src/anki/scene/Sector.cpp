// Copyright (C) 2009-2017, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <anki/scene/Sector.h>
#include <anki/scene/SpatialComponent.h>
#include <anki/scene/FrustumComponent.h>
#include <anki/scene/MoveComponent.h>
#include <anki/scene/SceneGraph.h>
#include <anki/scene/SoftwareRasterizer.h>
#include <anki/util/Logger.h>
#include <anki/resource/ResourceManager.h>
#include <anki/resource/MeshLoader.h>

namespace anki
{

template<typename TFunc>
static void iterateSceneSectors(SceneGraph& scene, TFunc func)
{
	scene.getSceneComponentLists().iterateComponents<SectorComponent>([&](SectorComponent& comp) {
		Sector& s = static_cast<Sector&>(comp.getSceneNode());
		if(func(s))
		{
			return;
		}
	});
}

template<typename TFunc>
static void iterateScenePortals(SceneGraph& scene, TFunc func)
{
	scene.getSceneComponentLists().iterateComponents<PortalComponent>([&](PortalComponent& comp) {
		Portal& s = static_cast<Portal&>(comp.getSceneNode());
		if(func(s))
		{
			return;
		}
	});
}

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

Error PortalSectorBase::init(const CString& meshFname, Bool isSector)
{
	// Create move component
	SceneComponent* comp = getSceneAllocator().newInstance<MoveComponent>(this);
	addComponent(comp, true);

	// Create portal or sector component
	if(isSector)
	{
		comp = getSceneAllocator().newInstance<SectorComponent>(this);
	}
	else
	{
		comp = getSceneAllocator().newInstance<PortalComponent>(this);
	}
	addComponent(comp, true);

	// Load mesh
	MeshLoader loader(&getSceneGraph().getResourceManager());
	ANKI_CHECK(loader.load(meshFname));

	// Convert Vec3 positions to Vec4
	const MeshLoader::Header& header = loader.getHeader();
	U vertsCount = header.m_totalVerticesCount;
	PtrSize vertSize = loader.getVertexSize();

	auto alloc = getSceneAllocator();
	m_shapeStorageLSpace.create(alloc, vertsCount);
	m_shapeStorageWSpace.create(alloc, vertsCount);

	for(U i = 0; i < vertsCount; ++i)
	{
		const Vec3& pos = *reinterpret_cast<const Vec3*>(loader.getVertexData() + vertSize * i);

		m_shapeStorageLSpace[i] = Vec4(pos, 0.0);
	}

	// Create shape
	ConvexHullShape* hull = alloc.newInstance<ConvexHullShape>();
	m_shape = hull;
	hull->initStorage(&m_shapeStorageWSpace[0], vertsCount);
	updateTransform(Transform::getIdentity());

	// Store indices
	ANKI_ASSERT(header.m_totalIndicesCount * sizeof(U16) == loader.getIndexDataSize());
	m_vertIndices.create(alloc, header.m_totalIndicesCount);
	const U16* indicesIn = reinterpret_cast<const U16*>(loader.getIndexData());
	for(U i = 0; i < header.m_totalIndicesCount; ++i)
	{
		m_vertIndices[i] = indicesIn[i];
	}

	return ErrorCode::NONE;
}

void PortalSectorBase::updateTransform(const Transform& trf)
{
	const U count = m_shapeStorageWSpace.getSize();
	for(U i = 0; i < count; ++i)
	{
		m_shapeStorageWSpace[i] = trf.transform(m_shapeStorageLSpace[i]);
	}

	m_shape->computeAabb(m_aabb);
}

Portal::~Portal()
{
	auto alloc = getSceneAllocator();

	// Remove from sectors
	for(Sector* s : m_sectors)
	{
		s->tryRemovePortal(this);
	}

	m_sectors.destroy(getSceneAllocator());
}

Error Portal::init(const CString& meshFname)
{
	ANKI_CHECK(Base::init(meshFname, false));
	return ErrorCode::NONE;
}

Error Portal::frameUpdate(F32 prevUpdateTime, F32 crntTime)
{
	MoveComponent& move = getComponent<MoveComponent>();
	if(move.getTimestamp() == getGlobalTimestamp())
	{
		// Move comp updated
		updateTransform(move.getWorldTransform());
		getSceneGraph().getSectorGroup().portalUpdated(this);
	}

	return ErrorCode::NONE;
}

void Portal::deferredUpdate()
{
	// Gather the sectors it collides
	iterateSceneSectors(getSceneGraph(), [&](Sector& sector) -> Bool {
		Bool collide = testCollisionShapes(m_aabb, sector.m_aabb);

		// Perform a more detailed test
		if(collide)
		{
			collide = testCollisionShapes(*m_shape, sector.getBoundingShape());
		}

		// Update graphs
		if(collide)
		{
			tryAddSector(&sector);
			sector.tryAddPortal(this);
		}
		else
		{
			tryRemoveSector(&sector);
			sector.tryRemovePortal(this);
		}

		return false;
	});
}

void Portal::tryAddSector(Sector* sector)
{
	ANKI_ASSERT(sector);

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

void Portal::tryRemoveSector(Sector* sector)
{
	ANKI_ASSERT(sector);

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
}

Error Sector::init(const CString& meshFname)
{
	ANKI_CHECK(PortalSectorBase::init(meshFname, true));
	return ErrorCode::NONE;
}

void Sector::tryAddPortal(Portal* portal)
{
	ANKI_ASSERT(portal);

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

void Sector::tryRemovePortal(Portal* portal)
{
	ANKI_ASSERT(portal);

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

void Sector::tryAddSpatialComponent(SpatialComponent* sp)
{
	ANKI_ASSERT(sp);

	// Find sector in spatial
	auto itsp = sp->getSectorInfo().getBegin();
	auto endsp = sp->getSectorInfo().getEnd();
	for(; itsp != endsp; ++itsp)
	{
		if(*itsp == this)
		{
			// Found, return
			ANKI_ASSERT(
				findSpatialComponent(sp) != m_spatials.getEnd() && "Spatial has reference to sector but sector not");
			return;
		}
	}

	ANKI_ASSERT(findSpatialComponent(sp) == m_spatials.getEnd());

	m_spatials.pushBack(getSceneAllocator(), sp);
	sp->getSectorInfo().pushBack(getSceneAllocator(), this);
}

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

		sp->getSectorInfo().erase(getSceneAllocator(), itsp);

		auto it = findSpatialComponent(sp);
		ANKI_ASSERT(it != m_spatials.getEnd());
		m_spatials.erase(getSceneAllocator(), it);
	}
	else
	{
#if ANKI_EXTRA_CHECKS
		ANKI_ASSERT(findSpatialComponent(sp) == m_spatials.getEnd());
#endif
	}
}

List<SpatialComponent*>::Iterator Sector::findSpatialComponent(SpatialComponent* sp)
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

Error Sector::frameUpdate(F32 prevUpdateTime, F32 crntTime)
{
	MoveComponent& move = getComponent<MoveComponent>();
	if(move.getTimestamp() == getGlobalTimestamp())
	{
		// Move comp updated.
		updateTransform(move.getWorldTransform());
		getSceneGraph().getSectorGroup().sectorUpdated(this);
	}

	return ErrorCode::NONE;
}

void Sector::deferredUpdate()
{
	// Spatials should get updated
	for(SpatialComponent* sp : m_spatials)
	{
		sp->markForUpdate();
	}

	iterateScenePortals(getSceneGraph(), [&](Portal& portal) -> Bool {
		Bool collide = testCollisionShapes(m_aabb, portal.m_aabb);

		if(collide)
		{
			collide = testCollisionShapes(*m_shape, portal.getBoundingShape());
		}

		if(collide)
		{
			portal.tryAddSector(this);
			tryAddPortal(&portal);
		}
		else
		{
			portal.tryRemoveSector(this);
			tryRemovePortal(&portal);
		}

		return false;
	});
}

SectorGroup::~SectorGroup()
{
}

void SectorGroup::spatialUpdated(SpatialComponent* sp)
{
	ANKI_ASSERT(sp);
	LockGuard<SpinLock> lock(m_mtx);
	m_spatialsDeferredBinning.pushBack(m_scene->getFrameAllocator(), sp);
}

void SectorGroup::portalUpdated(Portal* portal)
{
	ANKI_ASSERT(portal);
	LockGuard<SpinLock> lock(m_portalsUpdatedLock);
	m_portalsUpdated.pushBack(m_scene->getFrameAllocator(), portal);
}

void SectorGroup::sectorUpdated(Sector* sector)
{
	ANKI_ASSERT(sector);
	LockGuard<SpinLock> lock(m_sectorsUpdatedLock);
	m_sectorsUpdated.pushBack(m_scene->getFrameAllocator(), sector);
}

void SectorGroup::binSpatial(SpatialComponent* sp)
{
	ANKI_ASSERT(sp);

	// Iterate all sectors and bin the spatial
	iterateSceneSectors(*m_scene, [&](Sector& sector) -> Bool {
		Bool collide = false;
		if(!sp->getSingleSector())
		{
			// Spatial can belong to multiple sectors

			// Fast test
			collide = testCollisionShapes(sector.m_aabb, sp->getAabb());

			// Detailed test
			if(collide)
			{
				collide = testCollisionShapes(sector.getBoundingShape(), sp->getSpatialCollisionShape());
			}
		}
		else
		{
			// Spatial can belong to one sector

			// Make sure the origin of the spatial is inside the sector
			const Vec4& center = sp->getSpatialOrigin();

			if(center >= sector.m_aabb.getMin() && center <= sector.m_aabb.getMax())
			{
				collide = true;
			}

			// Detailed test
			const F32 smallf = EPSILON * 10.0;
			Aabb smallBox(center, center + Vec4(smallf, smallf, smallf, 0.0));
			if(collide)
			{
				collide = testCollisionShapes(sector.getBoundingShape(), smallBox);
			}
		}

		if(collide)
		{
			sector.tryAddSpatialComponent(sp);
		}
		else
		{
			sector.tryRemoveSpatialComponent(sp);
		}

		return false;
	});
}

void SectorGroup::spatialDeleted(SpatialComponent* sp)
{
	auto it = sp->getSectorInfo().getBegin();
	auto end = sp->getSectorInfo().getEnd();
	while(it != end)
	{
		(*it)->tryRemoveSpatialComponent(sp);
		++it;
	}
}

void SectorGroup::findVisibleSectors(const FrustumComponent& frc,
	const SoftwareRasterizer* r,
	List<const Sector*>& visibleSectors,
	U& spatialsCount) const
{
	// Find the sector the eye is in
	Sphere eye(frc.getFrustumOrigin(), frc.getFrustum().getNear());
	Bool eyeInsideASector = false;
	Sector* sectorEyeIsInside = nullptr;

	iterateSceneSectors(*m_scene, [&](Sector& sector) -> Bool {
		Bool collide = testCollisionShapes(eye, sector.m_aabb);

		if(collide)
		{
			collide = testCollisionShapes(eye, sector.getBoundingShape());
		}

		if(collide)
		{
			eyeInsideASector = true;
			sectorEyeIsInside = &sector;
			return true;
		}

		return false;
	});

	if(!eyeInsideASector)
	{
		// eye outside all sectors, find those the frustum collides

		iterateSceneSectors(*m_scene, [&](Sector& sector) -> Bool {
			if(frc.insideFrustum(sector.getBoundingShape()))
			{
				findVisibleSectorsInternal(frc, sector, r, visibleSectors, spatialsCount);
			}

			return false;
		});
	}
	else
	{
		// eye inside a sector
		findVisibleSectorsInternal(frc, *sectorEyeIsInside, r, visibleSectors, spatialsCount);
	}
}

void SectorGroup::findVisibleSectorsInternal(const FrustumComponent& frc,
	const Sector& s,
	const SoftwareRasterizer* r,
	List<const Sector*>& visibleSectors,
	U& spatialsCount) const
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
		const Portal& p = *(*itp);

		if(frc.insideFrustum(p.getBoundingShape())
			&& (r == nullptr || r->visibilityTest(p.getBoundingShape(), p.m_aabb)))
		{
			auto it = p.m_sectors.getBegin();
			auto end = p.m_sectors.getEnd();
			for(; it != end; ++it)
			{
				if(*it != &s)
				{
					findVisibleSectorsInternal(frc, *(*it), r, visibleSectors, spatialsCount);
				}
			}
		}
	}
}

void SectorGroup::prepareForVisibilityTests()
{
	// Update portals
	Error err = m_portalsUpdated.iterateForward([](Portal* portal) {
		portal->deferredUpdate();
		return ErrorCode::NONE;
	});
	(void)err;
	m_portalsUpdated.destroy(m_scene->getFrameAllocator());

	// Update sectors
	err = m_sectorsUpdated.iterateForward([](Sector* portal) {
		portal->deferredUpdate();
		return ErrorCode::NONE;
	});
	(void)err;
	m_sectorsUpdated.destroy(m_scene->getFrameAllocator());

	// Bin spatials
	err = m_spatialsDeferredBinning.iterateForward([this](SpatialComponent* spc) {
		binSpatial(spc);
		return ErrorCode::NONE;
	});
	(void)err;
	m_spatialsDeferredBinning.destroy(m_scene->getFrameAllocator());
}

void SectorGroup::findVisibleNodes(
	const FrustumComponent& frc, U testId, const SoftwareRasterizer* r, SectorGroupVisibilityTestsContext& ctx) const
{
	auto alloc = m_scene->getFrameAllocator();

	// Find visible sectors
	ListAuto<const Sector*> visSectors(alloc);
	U spatialsCount = 0;
	findVisibleSectors(frc, r, visSectors, spatialsCount);

	if(ANKI_UNLIKELY(spatialsCount == 0))
	{
		return;
	}

	// Initiate storage of nodes
	SceneNode** visibleNodesMem = reinterpret_cast<SceneNode**>(alloc.allocate(spatialsCount * sizeof(void*)));

	WeakArray<SceneNode*> visibleNodes(visibleNodesMem, spatialsCount);

	// Iterate visible sectors and get the scene nodes. The array will contain
	// duplicates
	U nodesCount = 0;
	auto it = visSectors.getBegin();
	auto end = visSectors.getEnd();
	for(; it != end; ++it)
	{
		const Sector& s = *(*it);
		for(auto itsp : s.m_spatials)
		{
			const SpatialComponent& spc = *itsp;
			SceneNode& node = const_cast<SceneNode&>(spc.getSceneNode());

			// Check if alrady visited
			if(!node.fetchSetSectorVisited(testId, true))
			{
				visibleNodes[nodesCount++] = &node;
			}
		}
	}

	// Update the context
	ctx.m_visibleNodes = WeakArray<SceneNode*>(visibleNodesMem, nodesCount);
}

} // end namespace anki
