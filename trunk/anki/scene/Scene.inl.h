namespace anki {


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


} // end namespace
