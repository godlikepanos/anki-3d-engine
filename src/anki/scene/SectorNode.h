// Copyright (C) 2009-2018, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <anki/scene/SceneNode.h>
#include <anki/scene/components/SceneComponent.h>
#include <anki/Collision.h>
#include <anki/util/WeakArray.h>

namespace anki
{

// Forward
class SectorGroup;
class Renderer;
class SoftwareRasterizer;

/// @addtogroup scene
/// @{

/// The base for portals and sectors.
class PortalSectorBase : public SceneNode
{
	friend class PortalNode;
	friend class SectorNode;

public:
	PortalSectorBase(SceneGraph* scene, CString name)
		: SceneNode(scene, name)
	{
	}

	~PortalSectorBase();

	ANKI_USE_RESULT Error init(const CString& modelFname, Bool isSector);

	const CollisionShape& getBoundingShape() const
	{
		return *m_shape;
	}

	SectorGroup& getSectorGroup();

	const DynamicArray<Vec4>& getVertices() const
	{
		return m_shapeStorageLSpace;
	}

	const DynamicArray<U16>& getVertexIndices() const
	{
		return m_vertIndices;
	}

protected:
	DynamicArray<Vec4> m_shapeStorageLSpace;
	DynamicArray<Vec4> m_shapeStorageWSpace;
	CollisionShape* m_shape = nullptr;
	Aabb m_aabb;
	DynamicArray<U16> m_vertIndices; ///< Used in debug draw

	void updateTransform(const Transform& trf);
};

/// 2 way portal.
class PortalNode : public PortalSectorBase
{
	friend class SectorGroup;

public:
	using Base = PortalSectorBase;

	PortalNode(SceneGraph* scene, CString name)
		: PortalSectorBase(scene, name)
	{
	}

	~PortalNode();

	ANKI_USE_RESULT Error init(const CString& modelFname);

	ANKI_USE_RESULT Error frameUpdate(Second prevUpdateTime, Second crntTime) override;

	/// Add reference to sector.
	void tryAddSector(SectorNode* sector);

	/// Remove reference from sector.
	void tryRemoveSector(SectorNode* sector);

	void deferredUpdate();

private:
	List<SectorNode*> m_sectors;
};

/// A sector. It consists of an octree and some portals
class SectorNode : public PortalSectorBase
{
	friend class SectorGroup;

public:
	using Base = PortalSectorBase;

	/// Default constructor
	SectorNode(SceneGraph* scene, CString name)
		: Base(scene, name)
	{
	}

	~SectorNode();

	ANKI_USE_RESULT Error init(const CString& modelFname);

	void tryAddPortal(PortalNode* portal);
	void tryRemovePortal(PortalNode* portal);

	void tryAddSpatialComponent(SpatialComponent* sp);
	void tryRemoveSpatialComponent(SpatialComponent* sp);

	ANKI_USE_RESULT Error frameUpdate(Second prevUpdateTime, Second crntTime) override;

	void deferredUpdate();

private:
	List<PortalNode*> m_portals;
	List<SpatialComponent*> m_spatials;

	List<SpatialComponent*>::Iterator findSpatialComponent(SpatialComponent* sp);
};

/// The context for visibility tests from a single FrustumComponent (not for all of them).
class SectorGroupVisibilityTestsContext
{
	friend class SectorGroup;

public:
	U getVisibleSceneNodeCount() const
	{
		return m_visibleNodes.getSize();
	}

	template<typename Func>
	void iterateVisibleSceneNodes(PtrSize begin, PtrSize end, Func func)
	{
		ANKI_ASSERT(begin <= end);

		for(U i = begin; i < end; ++i)
		{
			func(*m_visibleNodes[i]);
		}
	}

private:
	WeakArray<SceneNode*> m_visibleNodes;
};

/// Sector group. This is supposed to represent the whole scene
class SectorGroup
{
public:
	/// Default constructor
	SectorGroup(SceneGraph* scene)
		: m_scene(scene)
	{
	}

	/// Destructor
	~SectorGroup();

	void spatialUpdated(SpatialComponent* sp);
	void spatialDeleted(SpatialComponent* sp);

	void portalUpdated(PortalNode* portal);
	void sectorUpdated(SectorNode* sector);

	void prepareForVisibilityTests();

	void findVisibleNodes(const FrustumComponent& frc,
		U threadId,
		const SoftwareRasterizer* r,
		SectorGroupVisibilityTestsContext& ctx) const;

private:
	SceneGraph* m_scene; ///< Keep it here to access various allocators

	List<SpatialComponent*> m_spatialsDeferredBinning;
	SpinLock m_mtx;

	List<PortalNode*> m_portalsUpdated;
	SpinLock m_portalsUpdatedLock;
	List<SectorNode*> m_sectorsUpdated;
	SpinLock m_sectorsUpdatedLock;

	void findVisibleSectors(const FrustumComponent& frc,
		const SoftwareRasterizer* r,
		List<const SectorNode*>& visibleSectors,
		U& spatialsCount) const;

	/// Recursive method
	void findVisibleSectorsInternal(const FrustumComponent& frc,
		const SectorNode& s,
		const SoftwareRasterizer* r,
		List<const SectorNode*>& visibleSectors,
		U& spatialsCount) const;

	void binSpatial(SpatialComponent* sp);
};
/// @}

} // end namespace anki
