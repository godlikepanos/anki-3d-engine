// Copyright (C) 2009-2015, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#ifndef ANKI_SCENE_SECTOR_H
#define ANKI_SCENE_SECTOR_H

#include "anki/scene/SceneNode.h"
#include "anki/scene/SceneComponent.h"
#include "anki/Collision.h"

namespace anki {

// Forward
class Sector;
class SectorGroup;
class FrustumComponent;
class SpatialComponent;

/// @addtogroup scene
/// @{

/// Dummy component to identify a portal or sector.
class PortalSectorComponent: public SceneComponent
{
public:
	PortalSectorComponent(SceneNode* node)
	:	SceneComponent(Type::SECTOR_PORTAL, node)
	{}

	static Bool classof(const SceneComponent& c)
	{
		return c.getType() == Type::SECTOR_PORTAL;
	}
};

/// The base for portals and sectors.
class PortalSectorBase: public SceneNode
{
public:
	PortalSectorBase(SceneGraph* scene)
	:	SceneNode(scene)
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
		return m_shapeStorage;
	}

	const DArray<U16>& getVertexIndices() const
	{
		return m_vertIndices;
	}

protected:
	DArray<Vec4> m_shapeStorage;
	CollisionShape* m_shape = nullptr;
	DArray<U16> m_vertIndices; ///< Used in debug draw
	SpinLock m_mtx;
};

/// 2 way portal.
class Portal: public PortalSectorBase
{
	friend class SectorGroup;

public:
	using Base = PortalSectorBase;

	Portal(SceneGraph* scene)
	:	PortalSectorBase(scene)
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
	:	PortalSectorBase(scene)
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
	:	m_scene(scene)
	{}

	/// Destructor
	~SectorGroup();

	void spatialUpdated(SpatialComponent* sp);
	void spatialDeleted(SpatialComponent* sp);

	void prepareForVisibilityTests(const FrustumComponent& frc);

	template<typename Func>
	ANKI_USE_RESULT Error iterateVisibleSceneNodes(
		PtrSize begin, PtrSize end, Func func);

private:
	SceneGraph* m_scene; ///< Keep it here to access various allocators
	List<Sector*> m_sectors;
	List<Portal*> m_portals;

	SceneNode** m_visibleNodes = nullptr;
	U m_visibleNodesCount = 0;

	void findVisibleSectors(
		const FrustumComponent& frc,
		List<Sector*>& visibleSectors,
		U& spatialsCount);

	/// Recursive method
	void findVisibleSectorsInternal(
		const FrustumComponent& frc,
		Sector& s,
		List<Sector*>& visibleSectors,
		U& spatialsCount);
};

//==============================================================================
template<typename TFunc>
inline Error SectorGroup::iterateVisibleSceneNodes(
	PtrSize begin, PtrSize end, TFunc func)
{
	Error err = ErrorCode::NONE;
	SceneNode* prevSn = nullptr;

	SceneNode** it = m_visibleNodes + begin;
	SceneNode** itend = m_visibleNodes + end;
	for(; it != itend && !err; ++it)
	{
		SceneNode* sn = *it;
		if(sn != prevSn)
		{
			err = func(*sn);
			prevSn = sn;
		}
	}

	return err;
}
/// @}

} // end namespace anki

#endif
