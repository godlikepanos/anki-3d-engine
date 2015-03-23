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
class SceneGraph;
class Sector;
class SectorGroup;
class Renderer;

/// @addtogroup Scene
/// @{

/// 2 way Portal
class Portal
{
public:
	Portal();

	~Portal();

	ANKI_USE_RESULT Error create(const SArray<Vec4>& vertPositions);

private:
	Array<Sector*, 2> m_sectors = {{nullptr, nullptr}};
	CollisionShape* m_shape = nullptr;
	Bool8 m_open = true;
};

/// A sector. It consists of an octree and some portals
class Sector
{
	friend class SectorGroup;

public:
	/// Used to reserve some space on the portals vector to save memory
	static const U AVERAGE_PORTALS_PER_SECTOR = 4;

	/// Default constructor
	Sector(SectorGroup* group);

	ANKI_USE_RESULT Error create(const SArray<Vec4>& vertPositions);

	const CollisionShape& getShape() const
	{
		return *m_shape;
	}

	const SectorGroup& getSectorGroup() const
	{
		return *m_group;
	}
	SectorGroup& getSectorGroup()
	{
		return *m_group;
	}

	/// Sector does not take ownership of the portal
	void addNewPortal(Portal* portal);

	/// Remove a Portal from the portals container
	void removePortal(Portal* portal);

private:
	SectorGroup* m_group; ///< Know your father
	DArray<Portal*> m_portals;
	CollisionShape* m_shape;
};

/// Sector group. This is supposed to represent the whole scene
class SectorGroup
{
public:
	/// Default constructor
	SectorGroup(SceneGraph* scene);

	/// Destructor
	~SectorGroup();

	const SceneGraph& getSceneGraph() const
	{
		return *scene;
	}

	SceneGraph& getSceneGraph()
	{
		return *scene;
	}

	const List<Portal*>& getPortals() const
	{
		return portals;
	}

	const List<Sector*>& getSectors() const
	{
		return sectors;
	}

	/// The owner of the pointer is the sector group
	Sector* createNewSector(const SArray<Vec4>& vertexPositions);

	/// The owner of the pointer is the sector group
	Portal* createNewPortal(const SArray<Vec4>& vertexPositions);

private:
	SceneGraph* m_scene; ///< Keep it here to access various allocators
	List<Sector*> m_sectors;
	List<Portal*> m_portals;
};
/// @}

} // end namespace anki

#endif
