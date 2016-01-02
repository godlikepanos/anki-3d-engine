// Copyright (C) 2009-2016, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <anki/scene/SceneNode.h>
#include <anki/scene/SceneComponent.h>
#include <anki/Collision.h>

namespace anki {

// Forward
class Sector;
class SectorGroup;
class FrustumComponent;
class SpatialComponent;
class Renderer;

/// @addtogroup scene
/// @{

/// Dummy component to identify a portal or sector.
class PortalSectorComponent: public SceneComponent
{
public:
	PortalSectorComponent(SceneNode* node)
		: SceneComponent(Type::SECTOR_PORTAL, node)
	{}

	static Bool classof(const SceneComponent& c)
	{
		return c.getType() == Type::SECTOR_PORTAL;
	}
};

/// The base for portals and sectors.
class PortalSectorBase: public SceneNode
{
	friend class Portal;
	friend class Sector;

public:
	PortalSectorBase(SceneGraph* scene)
		: SceneNode(scene)
	{}

	~PortalSectorBase();

	ANKI_USE_RESULT Error create(
		const CString& name, const CString& modelFname);

	const CollisionShape& getBoundingShape() const
	{
		return *m_shape;
	}

	SectorGroup& getSectorGroup();

	const DArray<Vec4>& getVertices() const
	{
		return m_shapeStorageLSpace;
	}

	const DArray<U16>& getVertexIndices() const
	{
		return m_vertIndices;
	}

protected:
	DArray<Vec4> m_shapeStorageLSpace;
	DArray<Vec4> m_shapeStorageWSpace;
	CollisionShape* m_shape = nullptr;
	Aabb m_aabb;
	DArray<U16> m_vertIndices; ///< Used in debug draw
	SpinLock m_mtx;

	void updateTransform(const Transform& trf);
};

/// 2 way portal.
class Portal: public PortalSectorBase
{
	friend class SectorGroup;

public:
	using Base = PortalSectorBase;

	Portal(SceneGraph* scene)
		: PortalSectorBase(scene)
	{}

	~Portal();

	ANKI_USE_RESULT Error create(
		const CString& name, const CString& modelFname);

	ANKI_USE_RESULT Error frameUpdate(
		F32 prevUpdateTime, F32 crntTime) override;

	/// Add reference to sector.
	void tryAddSector(Sector* sector);

	/// Remove reference from sector.
	void tryRemoveSector(Sector* sector);

private:
	List<Sector*> m_sectors;
	Bool m_open = true;
};

/// A sector. It consists of an octree and some portals
class Sector: public PortalSectorBase
{
	friend class SectorGroup;

public:
	using Base = PortalSectorBase;

	/// Default constructor
	Sector(SceneGraph* scene)
		: PortalSectorBase(scene)
	{}

	~Sector();

	ANKI_USE_RESULT Error create(
		const CString& name, const CString& modelFname);

	void tryAddPortal(Portal* portal);
	void tryRemovePortal(Portal* portal);

	void tryAddSpatialComponent(SpatialComponent* sp);
	void tryRemoveSpatialComponent(SpatialComponent* sp);

	ANKI_USE_RESULT Error frameUpdate(
		F32 prevUpdateTime, F32 crntTime) override;

private:
	List<Portal*> m_portals;
	List<SpatialComponent*> m_spatials;

	List<SpatialComponent*>::Iterator findSpatialComponent(
		SpatialComponent* sp);
};

/// Sector group. This is supposed to represent the whole scene
class SectorGroup
{
	friend class Sector;
	friend class Portal;

public:
	/// Default constructor
	SectorGroup(SceneGraph* scene)
		: m_scene(scene)
	{}

	/// Destructor
	~SectorGroup();

	void spatialUpdated(SpatialComponent* sp);
	void spatialDeleted(SpatialComponent* sp);

	void prepareForVisibilityTests(const FrustumComponent& frc,
		const Renderer& r);

	PtrSize getVisibleNodesCount() const
	{
		return m_visibleNodesCount;
	}

	template<typename Func>
	ANKI_USE_RESULT Error iterateVisibleSceneNodes(
		PtrSize begin, PtrSize end, Func func);

private:
	SceneGraph* m_scene; ///< Keep it here to access various allocators
	List<Sector*> m_sectors;
	List<Portal*> m_portals;

	SceneNode** m_visibleNodes = nullptr;
	U m_visibleNodesCount = 0;

	List<SpatialComponent*> m_spatialsDeferredBinning;
	SpinLock m_mtx;

	void findVisibleSectors(
		const FrustumComponent& frc,
		List<Sector*>& visibleSectors,
		U& spatialsCount,
		const Renderer& r);

	/// Recursive method
	void findVisibleSectorsInternal(
		const FrustumComponent& frc,
		Sector& s,
		List<Sector*>& visibleSectors,
		U& spatialsCount,
		const Renderer& r);

	void binSpatial(SpatialComponent* sp);
};

//==============================================================================
template<typename TFunc>
inline Error SectorGroup::iterateVisibleSceneNodes(
	PtrSize begin, PtrSize end, TFunc func)
{
	Error err = ErrorCode::NONE;

	SceneNode** it = m_visibleNodes + begin;
	SceneNode** itend = m_visibleNodes + end;
	for(; it != itend && !err; ++it)
	{
		SceneNode* sn = *it;
		ANKI_ASSERT(sn->getSectorVisited() == true);
		sn->setSectorVisited(false);
		err = func(*sn);
	}

	return err;
}
/// @}

} // end namespace anki

