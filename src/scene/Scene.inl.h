template<typename ContainerType, typename Type>
inline void Scene::putBackNode(ContainerType& container, Type* x)
{
	ASSERT(std::find(container.begin(), container.end(), x) == container.end());
	container.push_back(x);
}


template<typename ContainerType, typename Type>
inline void Scene::eraseNode(ContainerType& container, Type* x)
{
	typename ContainerType::iterator it =
		std::find(container.begin(), container.end(), x);
	ASSERT(it != container.end());
	container.erase(it);
}


inline PhysWorld& Scene::getPhysPhysWorld()
{
	return *physPhysWorld;
}


inline const PhysWorld& Scene::getPhysPhysWorld() const
{
	return *physPhysWorld;
}


inline const VisibilityTester& Scene::getVisibilityTester() const
{
	return *visibilityTester;
}


inline Scene::Types<SceneNode>::ConstRange Scene::getAllNodes() const
{
	return Types<SceneNode>::ConstRange(nodes.begin(), nodes.end());
}


inline Scene::Types<SceneNode>::MutableRange Scene::getAllNodes()
{
	return Types<SceneNode>::MutableRange(nodes.begin(), nodes.end());
}


inline Scene::Types<Light>::ConstRange Scene::getLights() const
{
	return Types<Light>::ConstRange(lights.begin(), lights.end());
}


inline Scene::Types<Light>::MutableRange Scene::getLights()
{
	return Types<Light>::MutableRange(lights.begin(), lights.end());
}


inline Scene::Types<Camera>::ConstRange Scene::getCameras() const
{
	return Types<Camera>::ConstRange(cams.begin(), cams.end());
}


inline Scene::Types<Camera>::MutableRange Scene::getCameras()
{
	return Types<Camera>::MutableRange(cams.begin(), cams.end());
}


inline Scene::Types<ParticleEmitterNode>::ConstRange
	Scene::getParticleEmitterNodes() const
{
	return Types<ParticleEmitterNode>::ConstRange(
		particleEmitterNodes.begin(),
		particleEmitterNodes.end());
}


inline Scene::Types<ParticleEmitterNode>::MutableRange
	Scene::getParticleEmitterNodes()
{
	return Types<ParticleEmitterNode>::MutableRange(
		particleEmitterNodes.begin(),
		particleEmitterNodes.end());
}


inline Scene::Types<ModelNode>::ConstRange Scene::getModelNodes() const
{
	return Types<ModelNode>::ConstRange(modelNodes.begin(), modelNodes.end());
}


inline Scene::Types<ModelNode>::MutableRange Scene::getModelNodes()
{
	return Types<ModelNode>::MutableRange(modelNodes.begin(), modelNodes.end());
}


inline Scene::Types<SkinNode>::ConstRange Scene::getSkinNodes() const
{
	return Types<SkinNode>::ConstRange(skinNodes.begin(), skinNodes.end());
}


inline Scene::Types<SkinNode>::MutableRange Scene::getSkinNodes()
{
	return Types<SkinNode>::MutableRange(skinNodes.begin(), skinNodes.end());
}
