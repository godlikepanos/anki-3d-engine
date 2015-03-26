// Copyright (C) 2009-2015, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#ifndef ANKI_SCENE_SECTOR_H
#define ANKI_SCENE_SECTOR_H

#include "anki/scene/Common.h"
#include "anki/scene/Visibility.h"
#include "anki/Collision.h"

namespace anki {

// Forward
class SceneNode;
class Sector;
class SectorGroup;
class FrustumComponent;

/// @addtogroup Scene
/// @{

/// 2 way Portal
class Portal
{
	friend class SectorGroup;

public:
	Portal(SectorGroup* sectorGroup)
	:	m_group(sectorGroup)
	{}

	~Portal();

	ANKI_USE_RESULT Error create(const SArray<Vec4>& vertPositions);

	const CollisionShape& getBoundingShape() const
	{
		return *m_shape;
	}

	ANKI_USE_RESULT Error addSector(Sector* sector);

private:
	SectorGroup* m_group;
	List<Sector*> m_sectors;
	CollisionShape* m_shape = nullptr;
	DArray<Vec4> m_shapeStorage;
	Bool m_open = true;
};

/// A sector. It consists of an octree and some portals
class Sector
{
	friend class SectorGroup;

public:
	/// Default constructor
	Sector(SectorGroup* group)
	:	m_group(group)
	{}

	~Sector();

	ANKI_USE_RESULT Error create(const SArray<Vec4>& vertPositions);

	const CollisionShape& getBoundingShape() const
	{
		return *m_shape;
	}

	ANKI_USE_RESULT Error addPortal(Portal* portal);

	ANKI_USE_RESULT Error tryAddSpatialComponent(SpatialComponent* sp);

	void tryRemoveSpatialComponent(SpatialComponent* sp);

private:
	SectorGroup* m_group; ///< Know your father
	CollisionShape* m_shape;
	DArray<Vec4> m_shapeStorage;

	List<Portal*> m_portals;
	List<SpatialComponent*> m_spatials;

	SpinLock m_lock;

	Bool m_dirty = true;

	List<SpatialComponent*>::Iterator findSpatialComponent(
		SpatialComponent* sp);
};

/// Sector group. This is supposed to represent the whole scene
class SectorGroup
{
public:
	/// Default constructor
	SectorGroup(SceneGraph* scene)
	:	m_scene(scene)
	{}

	/// Destructor
	~SectorGroup();

	SceneAllocator<U8> getAllocator() const;

	/// The owner of the pointer is the sector group
	Sector* createNewSector(const SArray<Vec4>& vertexPositions);

	/// The owner of the pointer is the sector group
	Portal* createNewPortal(const SArray<Vec4>& vertexPositions);

	ANKI_USE_RESULT Error bake();

	ANKI_USE_RESULT Error spatialUpdated(SpatialComponent* sp);
	void spatialDeleted(SpatialComponent* sp);

	ANKI_USE_RESULT Error prepareForVisibilityTests(
		const FrustumComponent& frc);

	template<typename Func>
	ANKI_USE_RESULT Error iterateVisibleSceneNodes(
		PtrSize begin, PtrSize end, Func func);

	/// @privatesection
	/// @{
	ConvexHullShape* createConvexHull(
		const SArray<Vec4>& vertPositions,
		DArray<Vec4>& shapeStorage);
	/// @}

private:
	SceneGraph* m_scene; ///< Keep it here to access various allocators
	List<Sector*> m_sectors;
	List<Portal*> m_portals;

	SceneNode** m_visibleNodes = nullptr;
	U m_visibleNodesCount = 0;

	ANKI_USE_RESULT Error findVisibleSectors(
		const FrustumComponent& frc,
		List<Sector*>& visibleSectors,
		U& spatialsCount);

	/// Recursive method
	ANKI_USE_RESULT Error findVisibleSectorsInternal(
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
